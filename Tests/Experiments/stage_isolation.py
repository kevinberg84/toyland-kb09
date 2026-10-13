"""Offline stage taps; production Engine sources are never edited.

Coherent steady frequencies separate nonharmonic residuals from wanted harmonics.
This is a numerical diagnostic, not a hardware fidelity score or audibility test.
"""
from pathlib import Path
import subprocess
import json
import sys
import numpy as np

ROOT = Path(__file__).resolve().parents[2]
OUT = ROOT / 'artifacts/AnalogInvestigation/StageIsolation'
OUT.mkdir(parents=True, exist_ok=True)
source = (ROOT/'Tests/ReferenceDSP152/Engine.cpp').read_text()
header = (ROOT/'Tests/ReferenceDSP152/Engine.h').read_text()
lfo = (ROOT/'Tests/ReferenceDSP152/LfoRate.h').read_text()
includes, calls = [], []
for factor in (4, 32):
    for stage, tap in [('source','delayedFrame.source'),
                       ('mixer','1.6*sat(mix/1.6)'),
                       ('filter','filtered'), ('output','out'),
                       ('linearMixer','out'),('adaaMixer','out')]:
        extra=stage in ('linearMixer','adaaMixer')
        if extra and factor!=4: continue
        if ('--extra' in sys.argv)!=extra: continue
        name=f'r{factor}_{stage}'
        folder=OUT/name
        folder.mkdir(exist_ok=True)
        ns=f'sh09_{name}'
        h=header.replace('namespace sh09 {',f'namespace {ns} {{')
        h=h.replace('oversamplingFactor=4',f'oversamplingFactor={factor}')
        h=h.replace('firLength=513, alignment=256',f'firLength={128*factor+1}, alignment={64*factor}')
        h=h.replace('(firLength+3)/4','(firLength+oversamplingFactor-1)/oversamplingFactor')
        s=source.replace('namespace sh09 {',f'namespace {ns} {{')
        s=s.replace('fc=.115','fc=.46/oversamplingFactor')
        s=s.replace('j*4+os','j*oversamplingFactor+os')
        s=s.replace('inputHistory[index]*4','inputHistory[index]*oversamplingFactor')
        if stage=='linearMixer':
            s=s.replace('filter(1.6*sat(mix/1.6)+rawNoise*1e-8,','filter(mix+rawNoise*1e-8,')
        if stage=='adaaMixer':
            # Only a memoryless mixer experiment. Do not insert ADAA inside
            # the stateful filter: its delay would change the feedback loop.
            h=h.replace('double noiseLP=0,','double previousMixer=0;\n double noiseLP=0,')
            s=s.replace('keys.fill(0);','previousMixer=0;keys.fill(0);')
            primitive='''double primitive(double x) noexcept {
 const double a=std::abs(x);
 if(a>3)return .5+(4.0/3)*std::log(12.0)+a-3;
 return x*x/18+(4.0/3)*std::log(x*x+3);
}
'''
            s=s.replace('double satDerivative(double x)',primitive+'double satDerivative(double x)')
            s=s.replace('const double filtered=filter(1.6*sat(mix/1.6)+rawNoise*1e-8,',
                '''const double currentMixer=mix/1.6;
  const double difference=currentMixer-previousMixer;
  const double antiMixer=std::abs(difference)<1e-5?sat((currentMixer+previousMixer)*.5):
    (primitive(currentMixer)-primitive(previousMixer))/difference;
  previousMixer=currentMixer;
  const double filtered=filter(1.6*antiMixer+rawNoise*1e-8,''')
        assert s.count('outputHistory[outputPos]=out;')==1
        s=s.replace('outputHistory[outputPos]=out;',f'outputHistory[outputPos]={tap};')
        (folder/'Engine.h').write_text(h)
        (folder/'Engine.cpp').write_text(s)
        (folder/'LfoRate.h').write_text(lfo.replace('namespace sh09 {',f'namespace {ns} {{'))
        includes.append(f'#include "{name}/Engine.cpp"')
        calls.append(f'render<{ns}::Engine,{ns}::Controls>("{name}");')
harness=r'''
#include <fstream>
#include <iostream>
#include <cmath>
#include <vector>
#include <string>
template<class E,class C> void render(const std::string& name){
 constexpr int rate=48000,N=32768,settle=8192;
 for(int note:{36,48,60,72,84,96,108})for(int wave:{1,2}){
  double nominal=440*std::exp2((note-69)/12.0);
  double hz=std::round(nominal*N/rate)*rate/N;
  E e;e.prepare(rate);C c;c.vcaMode=0;c.vcfEnv=0;c.cutoff=1;
  c.vcoLevel=1;c.volume=.7f;c.wave=float(wave);
  c.tune=float(1200*std::log2(hz/nominal));e.setControls(c);e.noteOn(note,127);
  for(int i=0;i<settle;++i)e.process(0);
  std::vector<float> data(N);for(auto& x:data)x=e.process(0);
  std::ofstream f(name+"_"+std::to_string(note)+"_"+std::to_string(wave)+".f32",std::ios::binary);
  f.write(reinterpret_cast<char*>(data.data()),N*sizeof(float));
  std::cout<<name<<","<<note<<","<<wave<<","<<hz<<","<<e.solverFailures()<<std::endl;
 }
}
int main(){std::cout<<"variant,note,wave,hz,solver_failures\n";
CALLS
}
'''.replace('CALLS','\n'.join(calls))
(OUT/'experiment.cpp').write_text('\n'.join(includes)+'\n'+harness)
vs=r'C:\Program Files\Microsoft Visual Studio\18\Community\Common7\Tools\VsDevCmd.bat'
(OUT/'build.cmd').write_text(f'@echo off\ncall "{vs}" -arch=x64 -host_arch=x64 >nul\ncl /nologo /std:c++20 /O2 /EHsc /utf-8 experiment.cpp /Fe:experiment.exe > build.log 2>&1\nexit /b %errorlevel%\n')
subprocess.run(['cmd','/c','build.cmd'],cwd=OUT,check=True)
with (OUT/('runs-extra.csv' if '--extra' in sys.argv else 'runs.csv')).open('w') as f:
    subprocess.run([str(OUT/'experiment.exe')],cwd=OUT,stdout=f,check=True)
rows=[]
N=32768
bins=np.arange(N//2+1)
for path in sorted(OUT.glob('r*_*.f32')):
    factor,stage,note,wave=path.stem.split('_')
    note=int(note)
    fundamental=round(440*2**((note-69)/12)*N/48000)
    x=np.fromfile(path,dtype=np.float32).astype(float)
    assert len(x)==N and np.isfinite(x).all()
    # Hann suppresses tiny frequency offsets from float tune quantization.
    power=np.abs(np.fft.rfft(x*np.hanning(N)))**2
    wanted=bins<4
    for harmonic in range(1,int((N//2)/fundamental)+1):
        wanted |= np.abs(bins-harmonic*fundamental)<=3
    residual=np.where(wanted,0,power)
    rows.append(dict(factor=int(factor[1:]),stage=stage,note=note,wave=int(wave),
        frequency_hz=fundamental*48000/N,
        rms=float(np.sqrt(np.mean(x*x))),peak=float(np.max(np.abs(x))),
        nonharmonic_db=float(10*np.log10(max(residual.sum(),1e-30)/power[wanted].sum())),
        strongest_residual_hz=float(bins[np.argmax(residual)]*48000/N)))
(OUT/'measurements.json').write_text(json.dumps(rows,indent=2))
print(f'Completed {len(rows)} stage measurements; see measurements.json and runs.csv.')
