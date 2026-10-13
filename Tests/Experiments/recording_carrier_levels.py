"""Narrow-band VCO-frequency measurements, not isolated oscillator volume.

The self-oscillating filter and intermodulation can enter the same band. No
automatic injection-locking classification is made. Original WAVs are read only.
"""
from pathlib import Path
import sys,json,argparse
ROOT=Path(__file__).resolve().parents[2]
sys.path.insert(0,str(ROOT/'.tmp/dsp-analysis-packages'))
import numpy as np
from scipy.io import wavfile
from scipy import signal
import matplotlib;matplotlib.use('Agg')
import matplotlib.pyplot as plt
parser=argparse.ArgumentParser();parser.add_argument('--input',type=Path,required=True)
args=parser.parse_args();out=ROOT/'artifacts/AnalogInvestigation/HardwareComparisons'
manifest=json.loads((out/'recording-manifest.json').read_text())
def read(path):
    rate,x=wavfile.read(path)
    if x.ndim==2:x=x.mean(axis=1) # manifest verifies near-identical stereo for all 22 files
    return rate,signal.resample_poly(x.astype(np.float64),1,2)
def db(x):return 20*np.log10(np.maximum(x,1e-12))
def refine(f,p,index):
    a,b,c=np.log(np.maximum(p[index-1:index+2],1e-30))
    return float((index+.5*(a-c)/(a-2*b+c))*(f[1]-f[0]))
reference={};summary=[];curves={}
# Dataset annotations from the persistent fundamental/harmonic families in
# the paired spectrograms. Pair 5 is ~256 Hz, absent from refs 9–11: do not force
# every sweep onto the three captured reference pitches or a loud subharmonic.
nominal_carrier={1:171.,2:228.,3:342.,4:342.,5:256.,6:171.,7:171.,8:271.}
for instrument in ('SH-09','KB-09'):
    reference[instrument]=[]
    for number in (9,10,11):
        rate,x=read(args.input/f'{number} {instrument} VCO.wav');fs=rate/2
        y=x[int(fs):int(fs*4)]
        f=np.fft.rfftfreq(len(y),1/fs);p=abs(np.fft.rfft(y*np.hanning(len(y))))**2
        allowed=np.flatnonzero((f>80)&(f<600));index=allowed[np.argmax(p[allowed])]
        f0=refine(f,p,index)
        reference[instrument].append(dict(number=number,hz=f0,rms_dbfs=float(db(np.sqrt(np.mean(y*y))))))
for number in range(1,9):
    for instrument in ('SH-09','KB-09'):
        rate,x=read(args.input/f'{number} {instrument} Sweep.wav');fs=rate/2
        f,p=signal.welch(x,fs,nperseg=131072)
        allowed=np.flatnonzero(abs(f-nominal_carrier[number])<4)
        index=allowed[np.argmax(p[allowed])];f0=refine(f,p,index)
        tt=np.arange(len(x))/fs
        demod=2*x*np.exp(-2j*np.pi*f0*tt)
        sos=signal.butter(6,10,fs=fs,output='sos')
        carrier_rms=np.abs(signal.sosfiltfilt(sos,demod))/np.sqrt(2)
        hop=int(fs*.02);ct=tt[::hop];carrier_db=db(carrier_rms[::hop])
        interior=(ct>=1)&(ct<len(x)/fs-1)
        q=np.percentile(carrier_db[interior],[10,50,90])
        summary.append(dict(number=number,instrument=instrument,inferred_stationary_carrier_hz=f0,
                            carrier_selection='Annotated stationary harmonic family; PSD maximum refined within +/-4 Hz of nominal',
                            narrowband_rms_p10_dbfs=float(q[0]),narrowband_rms_p50_dbfs=float(q[1]),
                            narrowband_rms_p90_dbfs=float(q[2]),narrowband_p90_p10_db=float(q[2]-q[0]),
                            method='complex demodulation, zero-phase 6th-order 10 Hz lowpass, 1 s edge exclusion',
                            caveat='Total signal near VCO frequency, not isolated VCO; filter oscillation/intermodulation may share band.'))
        curves[(number,instrument)]=(ct,carrier_db)
        np.savetxt(out/f'{number:02d}-{instrument}-carrier.csv',np.c_[ct,carrier_db],delimiter=',',
                   header='seconds,total_near_vco_band_rms_dbfs',comments='')
(out/'vco-reference-frequencies.json').write_text(json.dumps(reference,indent=2))
(out/'carrier-levels.json').write_text(json.dumps(summary,indent=2))

# A readable first comparison uses per-file centering for display only.
fig,axes=plt.subplots(2,1,figsize=(10,5.6),sharex=True,sharey=True,layout='constrained')
for ax,instrument,colour in zip(axes,('SH-09','KB-09'),('#197b8c','#ae4e43')):
    a=np.loadtxt(out/f'01-{instrument}-levels.csv',delimiter=',',skiprows=1)
    r=next(r for r in manifest if r['number']==1 and r['instrument']==instrument)
    ax.plot(a[:,0],a[:,1]-r['interior_rms_median_dbfs'],color=colour,lw=1.5)
    ax.axhline(0,color='#999999',lw=.7,ls=':')
    ax.set(title=f'{instrument} — middle 80% level range: {r["interior_p90_p10_db"]:.1f} dB',
           ylabel='Level change (dB)',ylim=(-16,8),xlim=(1,19));ax.grid(alpha=.18)
axes[-1].set(xlabel='Time in each original recording (seconds)')
fig.suptitle('Pair 1: how much does the output level move?\n50 ms RMS, each recording centered on its own median; LFO phases not aligned')
fig.savefig(out/'pair-01-level-comparison.png',dpi=140);plt.close(fig)
print(json.dumps(dict(references=reference,carrier=summary),indent=2))
