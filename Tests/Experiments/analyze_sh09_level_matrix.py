"""Read-only analysis of Kevin's SH-09 Test #2: VCO mixer 10, 5, 0.

Fixed segment annotations come from the supplied order and recorded silent gaps.
No edits to the source WAV. VCO-off zero crossings describe an isolated filter;
on-take spectral ridges and narrow bands do not uniquely separate oscillators.
"""
from pathlib import Path
import sys,json,hashlib,argparse
ROOT=Path(__file__).resolve().parents[2]
sys.path.insert(0,str(ROOT/'.tmp/dsp-analysis-packages'))
import numpy as np
from scipy import signal,optimize
from scipy.io import wavfile
import matplotlib;matplotlib.use('Agg')
import matplotlib.pyplot as plt
parser=argparse.ArgumentParser()
parser.add_argument('--input',type=Path,default=Path('C:/Users/kevin/OneDrive/Desktop/SH-09 Test #2.wav'))
args=parser.parse_args()
OUT=ROOT/'artifacts/AnalogInvestigation/HardwareLevelMatrix';OUT.mkdir(parents=True,exist_ok=True)
sr,a=wavfile.read(args.input)
assert sr==96000 and a.dtype==np.float32 and a.ndim==2 and a.shape[1]==2
assert np.isfinite(a).all()
correlation=float(np.corrcoef(a.T)[0,1]);assert correlation>.99999
x=a.mean(axis=1).astype(np.float64)
segments=[dict(level=10,start=0.,end=18.),dict(level=5,start=26.,end=44.),dict(level=0,start=54.,end=70.)]

def levels(y,window):
    n=round(sr*window);b=y[:len(y)//n*n].reshape(-1,n)
    t=(np.arange(len(b))+.5)*window
    d=20*np.log10(np.maximum(np.sqrt(np.mean(b*b,axis=1)),1e-15))
    keep=(t>=1)&(t<len(y)/sr-1);q=np.percentile(d[keep],[10,50,90])
    return t,d,dict(p10_dbfs=float(q[0]),median_dbfs=float(q[1]),p90_dbfs=float(q[2]),p90_p10_db=float(q[2]-q[0]))

def fit_log_sine(t,f):
    lf=np.log(f);trials=[]
    for rate in np.linspace(.12,.3,500):
        design=np.c_[np.ones(len(t)),np.sin(2*np.pi*rate*t),np.cos(2*np.pi*rate*t)]
        coef=np.linalg.lstsq(design,lf,rcond=None)[0]
        trials.append((np.mean((design@coef-lf)**2),rate,coef))
    _,rate,coef=min(trials,key=lambda q:q[0])
    def residual(p):return p[0]+p[1]*np.sin(2*np.pi*p[3]*t)+p[2]*np.cos(2*np.pi*p[3]*t)-lf
    p=optimize.least_squares(residual,[*coef,rate],loss='soft_l1',f_scale=.01).x
    center=np.exp(p[0]);depth=np.hypot(p[1],p[2]);phase=np.arctan2(p[2],p[1])%(2*np.pi)
    return dict(center_hz=float(center),log_depth=float(depth),rate_hz=float(p[3]),phase=float(phase),
                low_hz=float(center*np.exp(-depth)),high_hz=float(center*np.exp(depth)),
                span_octaves=float(2*depth/np.log(2)),log_fit_error_rms=float(np.sqrt(np.mean(residual(p)**2))))

def free_trace(y):
    i=np.flatnonzero((y[:-1]<=0)&(y[1:]>0));cross=i-y[i]/(y[i+1]-y[i])
    t=(cross[1:]+cross[:-1])/(2*sr);f=sr/np.diff(cross)
    keep=(t>=1)&(t<len(y)/sr-1)
    t=t[keep];f=f[keep]
    assert np.all((f>50)&(f<10000)),(f.min(),f.max())
    tt=np.arange(1,len(y)/sr-1,.02);ff=np.interp(tt,t,f)
    return tt,ff,fit_log_sine(tt,ff)

def fit_periodic(t,f,order=5):
    # Preserve the measured shape instead of assuming a mathematically pure sine
    # LFO or perfectly exponential control path. Linear drift is a nuisance term.
    lf=np.log(f);origin=float(np.mean(t))
    def design(rate):
        return np.column_stack([np.ones(len(t)),t-origin,*[q for k in range(1,order+1) for q in (np.sin(k*2*np.pi*rate*t),np.cos(k*2*np.pi*rate*t))]])
    def loss(rate):
        a=design(rate);c=np.linalg.lstsq(a,lf,rcond=None)[0];return np.mean((a@c-lf)**2)
    rates=np.linspace(.17,.20,300);best=rates[np.argmin([loss(r) for r in rates])]
    fit=optimize.minimize_scalar(loss,bounds=(best-.001,best+.001),method='bounded',options={'xatol':1e-12})
    rate=float(fit.x);a=design(rate);coef=np.linalg.lstsq(a,lf,rcond=None)[0];error=a@coef-lf
    phase=np.linspace(0,1,10000,endpoint=False)
    pred=coef[0]+sum(coef[2+2*(k-1)]*np.sin(2*np.pi*k*phase)+coef[3+2*(k-1)]*np.cos(2*np.pi*k*phase) for k in range(1,order+1))
    return dict(rate_hz=rate,order=order,log_mean=float(coef[0]),log_drift_per_second=float(coef[1]),drift_origin_seconds=origin,
                sin_cos_coefficients=coef[2:].reshape(-1,2).tolist(),fitted_low_hz=float(np.exp(pred.min())),fitted_high_hz=float(np.exp(pred.max())),
                log_fit_error_rms=float(np.sqrt(np.mean(error**2))),log_error_percentiles=np.percentile(error,[1,50,99]).tolist(),
                caveat='Empirical periodic log-frequency description, not identification of an LFO circuit or a cutoff CV measurement.')

def spectrum(y):
    yy=signal.resample_poly(y,1,2)
    f,t,z=signal.stft(yy,sr/2,nperseg=8192,noverlap=7680,nfft=16384,boundary=None,padded=False)
    return f,t,z

def peak_trace(f,t,z,duration):
    mag=abs(z);allowed=(f>90)&(f<8000)
    m=mag.copy();m[~allowed]=0;k=np.argmax(m,axis=0);col=np.arange(len(t))
    aa,bb,cc=np.log(np.maximum(mag[k-1,col],1e-15)),np.log(np.maximum(mag[k,col],1e-15)),np.log(np.maximum(mag[k+1,col],1e-15))
    ff=(k+.5*(aa-cc)/(aa-2*bb+cc))*(f[1]-f[0])
    keep=(t>=1)&(t<duration-1)
    return t[keep],ff[keep],fit_log_sine(t[keep],ff[keep])

def refine(f,p,i):
    aa,bb,cc=np.log(np.maximum(p[i-1:i+2],1e-30))
    return float((i+.5*(aa-cc)/(aa-2*bb+cc))*(f[1]-f[0]))

results=[];spectra={};raw={};traces={};level_traces={}
for seg in segments:
    y=x[round(seg['start']*sr):round(seg['end']*sr)];level=seg['level'];raw[level]=y
    row=dict(**seg,levels={})
    for window in (.05,.25,.5):
        t,d,m=levels(y,window);row['levels'][str(window)]=m
        if window==.05:level_traces[level]=(t,d)
    f,t,z=spectrum(y);spectra[level]=(f,t,z)
    tt,ff,fit=peak_trace(f,t,z,len(y)/sr);row['dominant_component_log_sine_fit']=fit
    traces[level]=(tt,ff)
    if level==0:
        ft,fr,free_fit=free_trace(y);row['isolated_filter_log_sine_fit']=free_fit
        row['isolated_filter_periodic_fit']=fit_periodic(ft,fr)
        row['isolated_filter_observed_bounds_hz']=[float(fr.min()),float(fr.max())]
        np.savetxt(OUT/'isolated-filter-frequency.csv',np.c_[ft,fr],delimiter=',',header='local_seconds,positive_zero_crossing_frequency_hz',comments='')
    else:
        # The stationary 178 Hz family is visually identifiable in this recording.
        ff0,p=signal.welch(y[sr:-sr],sr,nperseg=524288)
        ix=np.flatnonzero((ff0>174)&(ff0<182));i=ix[np.argmax(p[ix])]
        row['whole_take_psd_peak_near_vco_hz']=refine(ff0,p,i)
    results.append(row)

f0=next(r['whole_take_psd_peak_near_vco_hz'] for r in results if r['level']==10)
for row in results:
    level=row['level'];y=raw[level];tt=np.arange(len(y))/sr
    # On-take low-frequency band is total signal, not uniquely isolated VCO.
    demod=2*y*np.exp(-2j*np.pi*f0*tt)
    sos=signal.butter(6,8,fs=sr,output='sos')
    analytic=signal.sosfiltfilt(sos,demod)
    rms=abs(analytic)/np.sqrt(2)
    phase_frequency=f0+np.gradient(np.unwrap(np.angle(analytic)))*sr/(2*np.pi)
    tt=tt[::1920];dd=20*np.log10(np.maximum(rms[::1920],1e-15))
    fit=row['dominant_component_log_sine_fit'] if level else row['isolated_filter_log_sine_fit']
    estimate=fit['center_hz']*np.exp(fit['log_depth']*np.sin(2*np.pi*fit['rate_hz']*tt+fit['phase']))
    select=(tt>1)&(tt<len(y)/sr-1)&(estimate>1800)
    q=np.percentile(dd[select],[10,50,90])
    if level:
        row['stationary_vco_frequency_hz']=float(np.median(phase_frequency[::1920][select]))
        row['stationary_vco_frequency_method']='Median demodulated phase derivative while estimated filter frequency >1800 Hz; excludes self-oscillation crossings that contaminate whole-take PSD.'
    row['carrier_band_when_filter_high']=dict(frequency_hz=f0,bandwidth_lowpass_hz=8,estimated_filter_above_hz=1800,
        p10_dbfs=float(q[0]),median_dbfs=float(q[1]),p90_dbfs=float(q[2]),samples=int(select.sum()),
        caveat='Total near-VCO-frequency output; approximate high-filter mask. Not an internal input voltage measurement.')
    np.savetxt(OUT/f'vco{level:02d}-carrier-band.csv',np.c_[tt,dd,estimate],delimiter=',',header='local_seconds,near_vco_rms_dbfs,fitted_sweep_hz',comments='')

report=dict(path=str(args.input.resolve()),sha256=hashlib.sha256(args.input.read_bytes()).hexdigest(),sample_rate=sr,dtype=str(a.dtype),channels=2,seconds=len(x)/sr,
    hardware_resonance_position=10,resonance_setting_provenance='Kevin confirmed RES was maximum on 13 October 2026; physical loop gain is unmeasured.',
    channel_correlation=correlation,peak=float(np.max(abs(a))),samples_abs_ge_one=int(np.sum(abs(a)>=1)),finite=True,
    channel_rule='average of near-identical stereo channels',segment_rule='User order 10/5/0, silent gaps around 18–26 and 44–54 seconds; one-second edge exclusions',
    assumptions='Only VCO mixer position changes, as requested; other physical controls, VOLUME and capture gain assumed fixed. No patch photo for this recording yet. User describes hardware as approximately 40 years old and poorly maintained; this is a specimen measurement, not a factory calibration target.',segments=results)
(OUT/'analysis.json').write_text(json.dumps(report,indent=2))

fig,axes=plt.subplots(3,1,figsize=(11,8),layout='constrained',sharex=True,sharey=True)
for ax,row in zip(axes,results):
    t,d=level_traces[row['level']];m=row['levels']['0.05'];ax.plot(t,d,color={10:'#bc553e',5:'#7160a5',0:'#167e90'}[row['level']],lw=1)
    ax.set(title=f"VCO {row['level']} — level range {m['p90_p10_db']:.2f} dB; median {m['median_dbfs']:.2f} dBFS",ylabel='50 ms RMS (dBFS)',xlim=(1,17),ylim=(-18,-11));ax.grid(alpha=.2)
axes[-1].set_xlabel('Seconds in each clip; one second excluded at each end')
fig.suptitle('SH-09: changing only the VCO mixer\nSame recorded gain retained; level range contains the middle 80% of short-window levels')
fig.savefig(OUT/'hardware-level-stability.png',dpi=140);plt.close(fig)

fig,axes=plt.subplots(3,1,figsize=(11,9),layout='constrained',sharex=True,sharey=True)
for ax,row in zip(axes,results):
    level=row['level'];f,t,z=spectra[level];mask=(f>=70)&(f<=7000)
    ax.pcolormesh(t,f[mask],20*np.log10(np.maximum(abs(z[mask]),1e-12)),vmin=-65,vmax=-10,cmap='magma',shading='auto')
    for harmonic in (1,2,3,4):ax.axhline(f0*harmonic,color='#88cfd6',ls=':',lw=.5)
    ax.set(title=f'VCO {level}',yscale='log',ylim=(70,7000),ylabel='Frequency (Hz)',xlim=(1,15))
axes[-1].set_xlabel('Seconds in each clip; LFO phases differ')
fig.suptitle('Hardware sweeps at mixer 10, 5 and 0\nDotted lines mark measured VCO harmonics, not classified lock intervals; common dBFS display scale')
fig.savefig(OUT/'hardware-three-sweeps.png',dpi=140);plt.close(fig)

fig,ax=plt.subplots(figsize=(10,4.6),layout='constrained')
ax.plot(ft,fr,color='#167e90',lw=1,label='VCO off: interpolated zero-crossing periods')
prediction=free_fit['center_hz']*np.exp(free_fit['log_depth']*np.sin(2*np.pi*free_fit['rate_hz']*ft+free_fit['phase']))
ax.plot(ft,prediction,color='#ce7645',lw=1,ls='--',label='Fit: sine modulation in log-frequency')
periodic=next(r['isolated_filter_periodic_fit'] for r in results if r['level']==0)
log_prediction=periodic['log_mean']+periodic['log_drift_per_second']*(ft-periodic['drift_origin_seconds'])
for k,(ss,cc) in enumerate(periodic['sin_cos_coefficients'],1):
    log_prediction+=ss*np.sin(2*np.pi*k*periodic['rate_hz']*ft)+cc*np.cos(2*np.pi*k*periodic['rate_hz']*ft)
ax.plot(ft,np.exp(log_prediction),color='#765cb2',lw=.8,ls=':',label='Empirical periodic fit (five harmonics)')
ax.set(yscale='log',xlabel='Seconds in VCO-off clip',ylabel='Frequency (Hz)',title='The SH-09 filter’s isolated sweep');ax.grid(alpha=.2);ax.legend(fontsize=9)
fig.savefig(OUT/'isolated-filter-fit.png',dpi=140);plt.close(fig)
print(json.dumps(report,indent=2))
