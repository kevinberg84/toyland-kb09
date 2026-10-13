"""Circuit-informed sensitivity prototypes, not a production filter replacement.

Test separate OTA headroom, a loaded antiparallel Shockley limiter, and loop
gain. Each model is recalibrated to the same free sweep and relative carrier.
The normalized diode/headroom values are explicit hypotheses, not measured
BA662/1S2473 parameters or a complete SH-09 nodal model. No AGC or added sine.
"""
from pathlib import Path
import sys,json,hashlib,subprocess,time
ROOT=Path(__file__).resolve().parents[2]
sys.path.insert(0,str(ROOT/'.tmp/dsp-analysis-packages'))
import numpy as np
from scipy import signal
from scipy.io import wavfile
import matplotlib;matplotlib.use('Agg')
import matplotlib.pyplot as plt
OUT=ROOT/'artifacts/AnalogInvestigation/LimiterPrototypes';OUT.mkdir(parents=True,exist_ok=True)
gradient_mode='--stage-gradient' in sys.argv
unity_mode='--unity-gain' in sys.argv
if gradient_mode:
    OUT=OUT/'StageGradient';OUT.mkdir(exist_ok=True)
if unity_mode:
    OUT=OUT/'UnityGain';OUT.mkdir(exist_ok=True)
hw=json.loads((ROOT/'artifacts/AnalogInvestigation/HardwareLevelMatrix/analysis.json').read_text())
off=next(r for r in hw['segments'] if r['level']==0);full=next(r for r in hw['segments'] if r['level']==10)
mid=next(r for r in hw['segments'] if r['level']==5)
fit=off['isolated_filter_periodic_fit'];f0=full['stationary_vco_frequency_hz'];target_carrier=full['carrier_band_when_filter_high']['median_dbfs']
middle_ratio=10**((mid['carrier_band_when_filter_high']['median_dbfs']-target_carrier)/20)
tune=65*np.log2(f0/(440*2**((53-69)/12)))
configs=[
    dict(name='baseline',stage_scale=1.1,k=4.65,limiter='rational'),
    dict(name='headroom',stage_scale=3.3,k=4.65,limiter='rational'),
    dict(name='diode_only',stage_scale=1.1,k=4.65,limiter='diode'),
    dict(name='diode_headroom',stage_scale=3.3,k=4.65,limiter='diode'),
    dict(name='diode_stronger_loop',stage_scale=3.3,k=5.5,limiter='diode'),
    dict(name='diode_moderate',stage_scale=2.2,k=5.0,limiter='diode'),
]
if gradient_mode:
    # The three later local-feedback pole stages have 100k/68k gain. Transform
    # their physical states to unity interstage gain: differential-input
    # headroom then differs by stage rather than being one shared number.
    # Last-stage scale remains hypothetical, not a measured BA662 voltage.
    pole_gain=100/68
    configs=[dict(name=f'gradient_{int(last*10)}_{limiter}',stage_scale=last,
                  stage_scales=[last*pole_gain**i for i in (3,2,1,0)],
                  k=4.65,limiter=limiter)
             for last in (1.1,2.2,3.3) for limiter in ('rational','diode')]
if unity_mode:
    # Change state coordinates to preserve small-signal input/output gain.
    # These factors come from free-filter level calibration, not envelope AGC.
    scalar_gain=.6126410666394724
    gradient_gain=.6200804173378996
    configs=[dict(name='unity_diode',stage_scale=3.3*scalar_gain,
                  feedback_scale=2.8*scalar_gain,k=4.65,limiter='diode'),
             dict(name='unity_gradient',stage_scale=2.2*gradient_gain,
                  stage_scales=[2.2*gradient_gain*(100/68)**i for i in (3,2,1,0)],
                  feedback_scale=2.8*gradient_gain,k=4.65,limiter='diode')]
source=(ROOT/'Tests/ReferenceDSP152/Engine.cpp').read_text();header=(ROOT/'Tests/ReferenceDSP152/Engine.h').read_text();lfo=(ROOT/'Tests/ReferenceDSP152/LfoRate.h').read_text()
def once(s,a,b):
    assert s.count(a)==1,(a,s.count(a))
    return s.replace(a,b)

# Five coupled unknowns, with the diode-node Newton increment eliminated through
# a Schur complement. This avoids nesting an approximate diode solve in each
# filter iteration, and retains instantaneous feedback in the prototype.
diode_filter=r'''
double Engine::filter(double input,double cutoff,double resonance) noexcept {
 const double g=std::tan(pi*clamp(cutoff,5,std::min(22000.0,internalFs*.2))/internalFs);
 const double k=LOOP_GAIN*resonance,scale=STAGE_SCALE,L=2.8;
 constexpr double alpha=2e-6,beta=12,normalization=1+alpha*beta;
 auto y=filterY;double d=diagnosticDiode;bool converged=false;
 for(int iteration=0;iteration<16;++iteration){
  const double argument=clamp(beta*d,-60,60);
  const double rd=d+alpha*std::sinh(argument)-normalization*k*y[3]/L;
  const double cd=1+alpha*beta*std::cosh(argument);
  const double feedback=L*d,feedbackDerivative=normalization*k/cd;
  std::array<double,4>a{},b{};double error=std::abs(L*rd);
  for(int j=0;j<4;++j){
   const double in=j==0?input-feedback:y[size_t(j-1)];
   const double v=(in-y[size_t(j)])/scale;
   const double residual=y[size_t(j)]-filterState[size_t(j)]-g*scale*sat(v);
   const double derivative=g*satDerivative(v),denominator=1+derivative;
   a[size_t(j)]=derivative/denominator;b[size_t(j)]=-residual/denominator;
   error=std::max(error,std::abs(residual));
  }
  if(error<1e-13){converged=true;break;}
  const double A=a[3]*a[2]*a[1],B=a[3]*(a[2]*b[1]+b[2])+b[3];
  const double q=a[0]*feedbackDerivative;
  double delta=(b[0]-q*B+a[0]*L*rd/cd)/(1+q*A);
  std::array<double,4>dy{};dy[0]=delta;
  for(int j=1;j<4;++j)dy[size_t(j)]=a[size_t(j)]*dy[size_t(j-1)]+b[size_t(j)];
  const double dd=(normalization*k*dy[3]/L-rd)/cd;
  const double damping=std::abs(dd)>.25?.25/std::abs(dd):1;
  for(int j=0;j<4;++j)y[size_t(j)]+=damping*dy[size_t(j)];
  d+=damping*dd;
 }
 if(!converged)++failures;
 for(size_t j=0;j<4;++j){
  if(!std::isfinite(y[j])||std::abs(y[j])>32||!std::isfinite(d)){
   filterState.fill(0);filterY.fill(0);diagnosticDiode=0;++failures;return 0;
  }
  filterState[j]=tiny(2*y[j]-filterState[j]);
 }
 diagnosticDiode=d;filterY=y;return y[3];
}
'''
for cfg in configs:
    name=cfg['name'];ns='proto_'+name;folder=OUT/name;folder.mkdir(exist_ok=True)
    h=once(header,'private:','double diagnosticCutoff=1000,diagnosticDrive=1;\nprivate:\n double diagnosticDiode=0;').replace('namespace sh09 {',f'namespace {ns} {{')
    s=once(source,'const double filtered=filter(','finalCutoff=diagnosticCutoff;\n  const double filtered=filter(')
    s=once(s,'const double mix=delayedFrame.source+external*delayedFrame.externalLevel*.8;','const double mix=diagnosticDrive*(delayedFrame.source+external*delayedFrame.externalLevel*.8);')
    s=once(s,'filterY.fill(0);\n randomState','filterY.fill(0);diagnosticDiode=0;\n randomState')
    if cfg['limiter']=='diode':
        lo=s.index('double Engine::filter(');hi=s.index('float Engine::process(',lo)
        body=diode_filter.replace('LOOP_GAIN',str(cfg['k'])).replace('STAGE_SCALE',str(cfg['stage_scale']))
        body=body.replace('L=2.8;',f"L={cfg.get('feedback_scale',2.8)};")
        s=s[:lo]+body+'\n'+s[hi:]
    else:
        s=once(s,'scale=1.1,feedbackScale=2.8',f"scale={cfg['stage_scale']},feedbackScale=2.8")
        s=once(s,'k=4.65*resonance',f"k={cfg['k']}*resonance")
    if 'stage_scales' in cfg:
        scales=','.join(format(v,'.16g') for v in cfg['stage_scales'])
        s=once(s,'const double v=(in-y[size_t(j)])/scale;',
               'constexpr std::array<double,4> stageScales{'+scales+'};\n   const double scale=stageScales[size_t(j)];\n   const double v=(in-y[size_t(j)])/scale;')
    (folder/'Engine.cpp').write_text(s.replace('namespace sh09 {',f'namespace {ns} {{'))
    (folder/'Engine.h').write_text(h);(folder/'LfoRate.h').write_text(lfo.replace('namespace sh09 {',f'namespace {ns} {{'))
code='\n'.join(f'#include "{c["name"]}/Engine.cpp"' for c in configs)+r'''
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
template<class E,class C>int run(int argc,char**argv){
 const std::string mode=argv[2];const double drive=std::stod(argv[4]);
 const int rate=96000,warm=int(std::stod(argv[6])*rate),capture=int(std::stod(argv[7])*rate);
 std::vector<float>trajectory;
 if(mode=="trajectory"){
  std::ifstream f(argv[3],std::ios::binary|std::ios::ate);const auto n=f.tellg();f.seekg(0);
  trajectory.resize(size_t(n)/sizeof(float));f.read((char*)trajectory.data(),n);
  if(trajectory.size()!=size_t(warm+capture))return 2;
 }
 E e;e.prepare(rate);C c;c.modern=1;c.age=0;c.vcoLevel=drive==0?0.f:1.f;c.subLevel=c.extLevel=0;
 c.vcoMod=c.vcfEnv=c.vcfMod=0;c.resonance=argc>8?std::stof(argv[8]):1;c.wave=argc>9?std::stof(argv[9]):1;
 c.vcaMode=1;c.attack=c.decay=0;c.sustain=.7f;c.volume=1;c.tune=TUNE;
 e.setControls(c);e.noteOn(argc>10?std::stoi(argv[10]):53,127);e.diagnosticDrive=drive;
 const double constant=mode=="constant"?std::stod(argv[3]):0;
 std::vector<float>out(capture);
 for(int i=0;i<warm+capture;++i){e.diagnosticCutoff=mode=="constant"?constant:trajectory[size_t(i)];const auto v=e.process(0);if(i>=warm)out[size_t(i-warm)]=v;}
 std::ofstream f(argv[5],std::ios::binary);f.write((char*)out.data(),out.size()*sizeof(float));
 std::cout<<argv[1]<<","<<mode<<","<<drive<<","<<e.solverFailures()<<std::endl;return e.solverFailures()?3:0;
}
int main(int argc,char**argv){if(argc<8||argc>11)return 1;std::string name=argv[1];
DISPATCH
return 1;}
'''.replace('TUNE',f'{tune:.15g}f').replace('DISPATCH','\n'.join(f'if(name=="{c["name"]}")return run<proto_{c["name"]}::Engine,proto_{c["name"]}::Controls>(argc,argv);' for c in configs))
(OUT/'prototypes.cpp').write_text(code)
vs=r'C:\Program Files\Microsoft Visual Studio\18\Community\Common7\Tools\VsDevCmd.bat'
(OUT/'build.cmd').write_text(f'@echo off\ncall "{vs}" -arch=x64 -host_arch=x64 >nul\ncl /nologo /std:c++20 /O2 /EHsc /utf-8 prototypes.cpp /Fe:prototypes.exe > build.log 2>&1\nexit /b %errorlevel%\n')
analyze_only='--analyze-only' in sys.argv
if not analyze_only:subprocess.run(['cmd','/c','build.cmd'],cwd=OUT,check=True)

def render(name,mode,value,drive,filename,warm=2,capture=1):
    path=OUT/name/filename
    if not analyze_only:
        r=subprocess.run([str(OUT/'prototypes.exe'),name,mode,str(value),str(drive),str(path),str(warm),str(capture)],capture_output=True,text=True)
        with (OUT/'solver-check.txt').open('a') as stream:stream.write(r.stdout+r.stderr)
        if r.returncode:raise RuntimeError(r.stdout+r.stderr)
    y=np.fromfile(path,dtype=np.float32).astype(float)
    assert len(y)==int(capture*96000) and np.isfinite(y).all(),filename
    return y

def frequency(y):
    i=np.flatnonzero((y[:-1]<=0)&(y[1:]>0));cross=i-y[i]/(y[i+1]-y[i]);return (cross[1:]+cross[:-1])/192000,96000/np.diff(cross)
def levels(y,window=.05):
    n=round(96000*window);b=y[:len(y)//n*n].reshape(-1,n);tt=(np.arange(len(b))+.5)*window
    dd=20*np.log10(np.maximum(np.sqrt(np.mean(b*b,axis=1)),1e-15));keep=(tt>1)&(tt<len(y)/96000-1);q=np.percentile(dd[keep],[10,50,90])
    return tt,dd,dict(spread_db=float(q[2]-q[0]),median_dbfs=float(q[1]))
def carrier_projection(y):
    w=np.hanning(len(y));amp=2*abs(np.sum(y*w*np.exp(-2j*np.pi*f0*np.arange(len(y))/96000)))/sum(w)/np.sqrt(2)
    return 20*np.log10(max(amp,1e-15))
def target(tt):
    z=np.full(len(tt),fit['log_mean'])
    for k,(ss,cc) in enumerate(fit['sin_cos_coefficients'],1):z+=ss*np.sin(2*np.pi*k*fit['rate_hz']*tt)+cc*np.cos(2*np.pi*k*fit['rate_hz']*tt)
    return np.exp(z)
def high_carrier(y):
    tt=np.arange(len(y))/96000;z=signal.sosfiltfilt(signal.butter(6,8,fs=96000,output='sos'),2*y*np.exp(-2j*np.pi*f0*tt))
    ct=tt[::1920];db=20*np.log10(np.maximum(abs(z[::1920])/np.sqrt(2),1e-15));mask=(ct>1)&(ct<17)&(target(ct)>1800)
    return float(np.median(db[mask]))

rows=[];all_raw={}
for cfg in configs:
    name=cfg['name'];begin=time.time();cal=[]
    for fc in (100,1000,4000):
        y=render(name,'constant',fc,0,f'frequency-{fc}.f32');ft,ff=frequency(y);cal.append([fc,float(np.median(ff[(ft>.1)&(ft<.9)]))])
    tt=np.arange(96000*24)/96000-6;command=np.exp(np.interp(np.log(target(tt)),np.log([c[1] for c in cal]),np.log([c[0] for c in cal])))
    trajectory=OUT/name/'trajectory.f32'
    if not analyze_only:command.astype(np.float32).tofile(trajectory)
    y0=render(name,'trajectory',trajectory,0,'free.f32',6,18)
    _,_,free=levels(y0);gain_db=off['levels']['0.05']['median_dbfs']-free['median_dbfs'];gain=10**(gain_db/20)
    high_fc=float(np.exp(np.interp(np.log(2250),np.log([c[1] for c in cal]),np.log([c[0] for c in cal]))))
    drive=1.;iterations=[]
    for iteration in range(3):
        yy=render(name,'constant',high_fc,drive,f'carrier-fit-{iteration}.f32')
        db=carrier_projection(yy*gain);iterations.append(dict(drive=drive,carrier_dbfs=db))
        if abs(db-target_carrier)<.03:break
        drive=float(np.clip(drive*10**((target_carrier-db)/20),.02,3))
    # If the last iteration updated drive, that untested update is verified by
    # the actual trajectory render and reported as a residual carrier error.
    y10=render(name,'trajectory',trajectory,drive,'full.f32',6,18)
    y5=render(name,'trajectory',trajectory,drive*middle_ratio,'middle.f32',6,18)
    all_raw[name]=(y0*gain,y10*gain,y5*gain)
    ft,ff=frequency(y0);mask=(ft>1)&(ft<17);err=1200*np.log2(ff[mask]/target(ft[mask]-64/96000))
    row=dict(**cfg,calibration=cal,fixed_gain_db=gain_db,input_drive=drive,carrier_fit_iterations=iterations,
             free_pitch_error_cents_rms=float(np.sqrt(np.mean(err*err))),levels={},high_filter_carrier_dbfs=high_carrier(y10*gain))
    row['carrier_error_db']=row['high_filter_carrier_dbfs']-target_carrier
    for label,yy in zip(('off','full','middle'),all_raw[name]):row['levels'][label]={str(w):levels(yy,w)[2] for w in (.05,.25,.5)}
    rows.append(row)
    print(name,json.dumps(dict(spread=row['levels']['full']['0.05']['spread_db'],carrier_error=row['carrier_error_db'],pitch_error=row['free_pitch_error_cents_rms'],drive=drive,wall=time.time()-begin)),flush=True)

report=dict(configs=configs,hardware_sha256=hw['sha256'],source_hashes={p:hashlib.sha256((ROOT/p.replace('Source/DSP/','Tests/ReferenceDSP152/')).read_bytes()).hexdigest() for p in ('Source/DSP/Engine.cpp','Source/DSP/Engine.h','Source/DSP/LfoRate.h')},
    diode_law='d + alpha*sinh(beta*d) = (1+alpha*beta)*k*y3/L; feedback=L*d; alpha=2e-6,beta=12,L=2.8; small-signal slope normalized to k. Coupled five-unknown Newton solve.',
    limitations='Sensitivity prototypes, not a complete circuit derivation. Scales and diode coefficients unmeasured. Each uses measured free sweep, fixed free-level output gain and a carrier-only input fit. Aged specimen is not factory calibration. Production DSP unchanged. See checks.json for limited held-out model and ordinary-patch checks; broader hardware, listening and real-time CPU validation remain outstanding.',results=rows)
(OUT/'results.json').write_text(json.dumps(report,indent=2))
fig,axes=plt.subplots(3,2,figsize=(12,10),layout='constrained',sharex=True,sharey=True)
for ax,row in zip(axes.flat,rows):
    name=row['name'];tt,db,m=levels(all_raw[name][1]);ax.plot(tt,db,lw=.85,color='#b65640')
    ax.set(title=f"{name.replace('_',' ')}: {m['spread_db']:.2f} dB",ylabel='50 ms RMS (dBFS)',ylim=(-28,-10),xlim=(1,17));ax.grid(alpha=.2)
for ax in axes[-1]:ax.set_xlabel('Seconds')
fig.suptitle('Offline limiter/headroom prototypes — each pitch and relative carrier calibrated\nOne fixed output gain per model; no AGC; assumed component scales, not hardware-certified values')
fig.savefig(OUT/'prototype-levels.png',dpi=140);plt.close(fig)
