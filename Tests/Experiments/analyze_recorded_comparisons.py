"""Read-only measurements of paired user recordings; no source normalization.

Does not infer settings from filenames or treat a dominant spectral peak as
the self-oscillator's frequency. Reports channel handling and fixed time trims.
"""
from pathlib import Path
import sys,argparse,hashlib,json,csv
ROOT=Path(__file__).resolve().parents[2]
sys.path.insert(0,str(ROOT/'.tmp/dsp-analysis-packages'))
import numpy as np
from scipy.io import wavfile
from scipy import signal
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt

parser=argparse.ArgumentParser()
parser.add_argument('--input',type=Path,required=True)
parser.add_argument('--out',type=Path,default=ROOT/'artifacts/AnalogInvestigation/HardwareComparisons')
args=parser.parse_args();out=args.out;out.mkdir(parents=True,exist_ok=True)
records=[];curves={};spectra={}

def db(x):return 20*np.log10(np.maximum(x,1e-12))

for number in range(1,12):
    for instrument in ('KB-09','SH-09'):
        kind='Sweep' if number<=8 else 'VCO'
        path=args.input/f'{number} {instrument} {kind}.wav'
        rate,data=wavfile.read(path)
        dtype=str(data.dtype)
        if np.issubdtype(data.dtype,np.integer):
            scale=2.**(data.dtype.itemsize*8-1)
            x=data.astype(np.float64)/scale
        else:x=data.astype(np.float64)
        if x.ndim==1:x=x[:,None]
        finite=bool(np.isfinite(x).all())
        if not finite:raise ValueError(f'Nonfinite samples in {path.name}')
        channel_rms=np.sqrt(np.mean(x*x,axis=0))
        channel_peak=np.max(np.abs(x),axis=0)
        correlation=None
        if x.shape[1]==2:
            correlation=float(np.corrcoef(x[::32,0],x[::32,1])[0,1])
        if x.shape[1]==1:
            mono=x[:,0];channel_rule='original mono'
        elif correlation is not None and correlation>.99999:
            mono=x.mean(axis=1);channel_rule='average of near-identical stereo channels'
        else:
            selected=int(np.argmax(channel_rms));mono=x[:,selected]
            channel_rule=f'channel {selected+1}, highest whole-file RMS; other channels retained in source'
        window=max(1,round(rate*.05));count=len(mono)//window
        frames=mono[:count*window].reshape(count,window)
        rms=np.sqrt(np.mean(frames*frames,axis=1))
        peaks=np.max(np.abs(frames),axis=1)
        tt=(np.arange(count)+.5)*window/rate
        # Exclude a fixed one second at each edge, not low-amplitude interior
        # frames. This deliberately retains internal dips and any note gaps.
        interior=(tt>=1)&(tt<len(mono)/rate-1)
        interior_rms=rms[interior]
        p10,p50,p90=np.percentile(interior_rms,[10,50,90])
        record=dict(number=number,instrument=instrument,file=str(path),
                    sha256=hashlib.sha256(path.read_bytes()).hexdigest(),sample_rate=rate,
                    dtype=dtype,channels=x.shape[1],seconds=len(x)/rate,
                    channel_rms_dbfs=db(channel_rms).tolist(),channel_peak_dbfs=db(channel_peak).tolist(),
                    stereo_correlation=correlation,channel_rule=channel_rule,
                    peak_dbfs=float(db(np.max(np.abs(mono)))),rms_dbfs=float(db(np.sqrt(np.mean(mono*mono)))),
                    dc=float(np.mean(mono)),samples_abs_ge_1=int(np.count_nonzero(np.abs(mono)>=1)),
                    sample_peak_repetition=int(np.count_nonzero(np.abs(mono)==np.max(np.abs(mono)))),
                    interior_rms_p10_dbfs=float(db(p10)),interior_rms_median_dbfs=float(db(p50)),
                    interior_rms_p90_dbfs=float(db(p90)),interior_p90_p10_db=float(db(p90/p10)),
                    envelope_method='non-overlapping 50 ms RMS; fixed 1 s edge exclusion; internal dips/gaps retained')
        curves[(number,instrument)]=(tt,db(rms),db(peaks))
        np.savetxt(out/f'{number:02d}-{instrument}-levels.csv',np.c_[tt,db(rms),db(peaks)],
                   delimiter=',',header='seconds,rms_dbfs_50ms,peak_dbfs_50ms',comments='')
        # Only analysis is downsampled; original files remain untouched.
        analysis=signal.resample_poly(mono.astype(np.float32),1,2) if rate==96000 else mono.astype(np.float32)
        analysis_rate=rate//2 if rate==96000 else rate
        f,st,z=signal.stft(analysis,analysis_rate,window='hann',nperseg=8192,
                         noverlap=7168,nfft=8192,boundary=None,padded=False)
        mask=(f>=20)&(f<=12000)
        spectra[(number,instrument)]=(f[mask],st,db(abs(z[mask])))
        # Time-averaged spectrum is descriptive; sweep maxima are not VCO F0.
        wf,psd=signal.welch(analysis,analysis_rate,nperseg=min(65536,len(analysis)))
        pk,_=signal.find_peaks(psd,distance=max(1,int(8/(wf[1]-wf[0]))))
        pk=pk[(wf[pk]>20)&(wf[pk]<15000)]
        pk=pk[np.argsort(psd[pk])[-12:][::-1]]
        record['strong_time_averaged_spectral_peaks_hz']=wf[pk].tolist()
        records.append(record)
        print(f'{number:02d} {instrument}: {len(mono)/rate:.3f}s {rate}Hz {dtype}, {x.shape[1]}ch; '
              f'RMS span P90-P10 {record["interior_p90_p10_db"]:.2f} dB',flush=True)

(out/'recording-manifest.json').write_text(json.dumps(records,indent=2))
with (out/'level-summary.csv').open('w',newline='') as stream:
    keys=['number','instrument','seconds','sample_rate','dtype','channels','rms_dbfs','peak_dbfs',
          'interior_rms_median_dbfs','interior_p90_p10_db','samples_abs_ge_1']
    writer=csv.DictWriter(stream,fieldnames=keys,extrasaction='ignore');writer.writeheader();writer.writerows(records)

for label,numbers in (('sweeps',range(1,9)),('vco-references',range(9,12))):
    fig,axes=plt.subplots(len(numbers),2,figsize=(13,len(numbers)*2.3),layout='constrained',squeeze=False)
    for row,number in enumerate(numbers):
        for col,instrument in enumerate(('SH-09','KB-09')):
            tt,rms,pk=curves[(number,instrument)];ax=axes[row,col]
            ax.plot(tt,pk,color='#aeb6bf',lw=.6,label='50 ms peak')
            ax.plot(tt,rms,color='#197b8c' if instrument=='SH-09' else '#ae4e43',lw=1,label='50 ms RMS')
            ax.set(title=f'Pair {number} — {instrument}',ylabel='Level (dBFS)',xlabel='Time (s)',ylim=(-80,0))
            ax.grid(alpha=.2)
    axes[0,0].legend(fontsize=8)
    fig.suptitle('Original recording levels — no gain matching or normalization\nOne second excluded at edges for summary metrics; internal gaps remain')
    fig.savefig(out/f'{label}-levels-overview.png',dpi=130);plt.close(fig)

for number in range(1,12):
    fig,axes=plt.subplots(2,1,figsize=(12,6),sharex=False,sharey=True,layout='constrained')
    for ax,instrument in zip(axes,('SH-09','KB-09')):
        f,tt,z=spectra[(number,instrument)]
        ax.pcolormesh(tt,f,z,vmin=-90,vmax=-15,shading='auto',cmap='magma')
        ax.set(title=instrument,yscale='log',ylim=(30,10000),ylabel='Frequency (Hz)',xlabel='Time (s)')
    fig.suptitle(f'Pair {number}: frequency content over time\nFixed color scale, -90 to -15 dBFS spectral magnitude; brightest component is not necessarily filter pitch')
    fig.savefig(out/f'pair-{number:02d}-spectra.png',dpi=120);plt.close(fig)
print('Read-only analysis complete. Settings and note timing still require interpretation.')
