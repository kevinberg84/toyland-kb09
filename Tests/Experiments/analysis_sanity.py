"""Known-signal checks before interpreting SH-09/KB-09 sweep recordings.

This is NOT an analog circuit model or a hardware comparison. It constructs
signals whose components are known, and illustrates why a dominant spectral
peak is not automatically the frequency of a self-oscillating filter.
"""
from pathlib import Path
import sys,json
ROOT=Path(__file__).resolve().parents[2]
sys.path.insert(0,str(ROOT/'.tmp/dsp-analysis-packages'))
import numpy as np
from scipy import signal
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt

OUT=ROOT/'artifacts/AnalogInvestigation/AnalysisSanity'
OUT.mkdir(parents=True,exist_ok=True)
rate=48000;seconds=8;t=np.arange(rate*seconds)/rate
frequency=200*np.exp(np.log(6)*t/seconds)
phase=2*np.pi*np.cumsum(frequency)/rate
chirp=.08*np.sin(phase)

# Morph the amplitudes of fixed tones smoothly. The chirp is added unchanged;
# there is no feedback, injection locking, or quantization in this construction.
partials=np.arange(1,13)*100.
weights=np.exp(-.5*(np.log2(partials[:,None]/frequency[None,:])/.12)**2)
weights/=np.sqrt(np.arange(1,13))[:,None]
weights/=weights.sum(axis=0)
fixed=.35*np.sum(weights*np.sin(2*np.pi*partials[:,None]*t[None,:]),axis=0)
mixed=chirp+fixed
step_frequencies=np.array([200.,300.,450.,650.,950.])
stepped_frequency=step_frequencies[np.minimum((t/(seconds/5)).astype(int),4)]
stepped=.2*np.sin(2*np.pi*np.cumsum(stepped_frequency)/rate)

def spectrum(x):
    f,tt,z=signal.stft(x,rate,window='hann',nperseg=4096,noverlap=3584,nfft=16384,
                       boundary=None,padded=False)
    return f,tt,z

def strongest(f,z):
    # Sub-bin peak interpolation avoids presenting FFT bin spacing as steps.
    magnitude=np.maximum(abs(z),1e-12)
    k=np.argmax(magnitude,axis=0);k=np.clip(k,1,len(f)-2)
    c=np.arange(z.shape[1]);a,b,d=np.log(magnitude[k-1,c]),np.log(magnitude[k,c]),np.log(magnitude[k+1,c])
    offset=.5*(a-d)/(a-2*b+d)
    return (k+offset)*(f[1]-f[0])

f,tt,z=spectrum(chirp);peak=strongest(f,z)
expected=200*np.exp(np.log(6)*tt/seconds)
smooth_error=np.abs(peak/expected-1)
assert np.percentile(smooth_error,99)<.01
f,tt,sz=spectrum(stepped);step_peak=strongest(f,sz)
reference=step_frequencies[np.minimum((tt/(seconds/5)).astype(int),4)]
away=np.min(abs(tt[:,None]-np.arange(1,5)[None,:]*(seconds/5)),axis=1)>.1
step_error=np.abs(step_peak[away]/reference[away]-1)
assert np.percentile(step_error,99)<.01
assert np.max(np.abs((mixed-fixed)-chirp))<1e-15

fig,axes=plt.subplots(2,1,figsize=(10,6.5),sharex=True,sharey=True,layout='constrained')
for ax,x,title in zip(axes,(chirp,mixed),('Smooth sine alone','Same smooth sine + fixed harmonics with changing strengths')):
    f,tt,z=spectrum(x);mask=(f>=170)&(f<=1250)
    ax.pcolormesh(tt,f[mask],20*np.log10(np.maximum(abs(z[mask]),1e-8)),
                  shading='auto',vmin=-65,vmax=-15,cmap='magma')
    ax.plot(tt,200*np.exp(np.log(6)*tt/seconds),color='#48e4e0',lw=1.6,label='Known smooth sine frequency')
    ax.plot(tt,strongest(f,z),color='white',lw=1,ls=':',label='Loudest spectral peak — not a pitch verdict')
    ax.set(title=title,ylabel='Frequency (Hz)',ylim=(170,1250),xlim=(.1,7.9))
axes[0].legend(loc='upper left',fontsize=8)
axes[-1].set(xlabel='Time (seconds)')
fig.suptitle('ANALYSIS SELF-TEST — SYNTHETIC SIGNALS, NOT SH-09 OR KB-09\nA smooth component can remain present while the loudest component jumps')
fig.savefig(OUT/'smooth-component-dominant-steps.png',dpi=150);plt.close(fig)
report=dict(kind='synthetic analysis sanity check, not hardware/model evidence',rate=rate,
            smooth_peak_relative_error_p99=float(np.percentile(smooth_error,99)),
            deliberate_steps_relative_error_p99=float(np.percentile(step_error,99)),
            smooth_component_reconstruction_error=float(np.max(np.abs((mixed-fixed)-chirp))),
            tests='pure smooth chirp peak, deliberately stepped tone peak away from transitions, unchanged smooth component in mixture',
            limitations='This validates specific analysis safeguards, not an automatic component separator or injection-locking detector.')
(OUT/'checks.json').write_text(json.dumps(report,indent=2))
print(json.dumps(report,indent=2))
