"""SH-09 recording analysis. Requires Python 3.10+ and numpy.

This measures supplied recordings; it does not certify emulation accuracy.
PCM and IEEE float WAV are accepted; no implicit loudness normalization.
"""
from __future__ import annotations
import argparse
import json
from pathlib import Path
import struct
import wave
import numpy as np


def read_wav(path):
    data = Path(path).read_bytes()
    if data[:4] != b'RIFF' or data[8:12] != b'WAVE':
        raise ValueError('Expected RIFF WAVE')
    fmt = payload = None
    offset = 12
    while offset + 8 <= len(data):
        kind, count = struct.unpack_from('<4sI', data, offset)
        offset += 8
        chunk = data[offset:offset + count]
        if len(chunk) != count:
            raise ValueError('Truncated WAV chunk')
        if kind == b'fmt ': fmt = chunk
        if kind == b'data': payload = chunk
        offset += count + (count & 1)
    if fmt is None or payload is None: raise ValueError('Missing WAV fmt/data')
    encoding, channels, rate, _, align, bits = struct.unpack_from('<HHIIHH', fmt)
    if encoding == 0xfffe and len(fmt) >= 40: encoding = struct.unpack_from('<H', fmt, 24)[0]
    if channels < 1 or rate < 1000 or align != channels * bits // 8:
        raise ValueError('Unsupported WAV layout')
    if encoding == 3 and bits in (32,64):
        x = np.frombuffer(payload, dtype='<f4' if bits == 32 else '<f8').astype(float)
    elif encoding == 1 and bits == 24:
        b = np.frombuffer(payload,dtype=np.uint8).reshape(-1,3).astype(np.int32)
        v = b[:,0] | (b[:,1]<<8) | (b[:,2]<<16)
        x = ((v ^ 0x800000) - 0x800000).astype(float) / 8388608
    elif encoding == 1 and bits in (8,16,32):
        x = np.frombuffer(payload,dtype={8:'u1',16:'<i2',32:'<i4'}[bits]).astype(float)
        if bits == 8: x -= 128
        x /= 2 ** (bits - 1)
    else: raise ValueError(f'Unsupported WAV encoding={encoding} bits={bits}')
    x = x.reshape(-1,channels)
    if not np.isfinite(x).all(): raise ValueError('Nonfinite recording samples')
    return rate, x


def db(x): return float(20*np.log10(max(float(x),1e-15)))


def spectrum(x, rate):
    x = np.asarray(x,dtype=float)
    if len(x)<16: raise ValueError('Need at least 16 samples')
    w = np.hanning(len(x))
    z = np.fft.rfft((x-x.mean())*w)
    return np.fft.rfftfreq(len(x),1/rate), 2*np.abs(z)/w.sum()


def estimate_frequency(x,rate,min_hz=10,max_hz=None):
    f,a = spectrum(x,rate)
    upper = min(rate*.48,max_hz or rate*.48)
    candidates = np.flatnonzero((f>=min_hz)&(f<=upper))
    if not len(candidates) or np.max(a[candidates])<1e-10: return None
    k=int(candidates[np.argmax(a[candidates])])
    # Parabolic interpolation in log magnitude: good for isolated stable tones.
    shift=0.0
    if 0<k<len(a)-1:
        l,c,r=np.log(np.maximum(a[k-1:k+2],1e-30))
        denominator=l-2*c+r
        if abs(denominator)>1e-20: shift=.5*(l-r)/denominator
    return float((k+np.clip(shift,-.5,.5))*rate/len(x))


def analyze(x,rate,fundamental=None):
    x=np.asarray(x,dtype=float)
    rms=float(np.sqrt(np.mean(x*x)))
    f0=fundamental or estimate_frequency(x,rate)
    result={'samples':len(x),'seconds':len(x)/rate,'sample_rate':rate,
            'peak_dbfs':db(np.max(np.abs(x))),'rms_dbfs':db(rms),'dc':float(x.mean()),
            'dominant_frequency_hz':estimate_frequency(x,rate),'fundamental_used_hz':f0,
            'note':'Dominant spectral peak is not always the fundamental; use --fundamental for harmonic signals.'}
    if f0:
        f,a=spectrum(x,rate); harmonics=[]
        for harmonic in range(1,min(32,int(rate*.49/f0))+1):
            centre=harmonic*f0; half=max(2*rate/len(x),centre*.001)
            bins=(f>=centre-half)&(f<=centre+half)
            harmonics.append(float(np.sqrt(np.sum(a[bins]**2))))
        if harmonics and harmonics[0]>1e-12:
            result['harmonics_relative_db']=[db(v/harmonics[0]) for v in harmonics]
            result['thd_ratio_through_harmonic_32']=float(np.linalg.norm(harmonics[1:])/harmonics[0])
    # A 5 ms RMS window for envelope comparison. Timing is descriptive, not ADSR fitting.
    window=max(1,int(rate*.005))
    p=np.convolve(x*x,np.ones(window)/window,mode='valid')
    envelope=np.sqrt(p)
    peak=float(np.max(envelope))
    if peak>1e-10:
        crossings={str(level):int(np.flatnonzero(envelope>=peak*level)[0])/rate for level in (.1,.5,.9)}
        result['rms_envelope_first_crossing_seconds']=crossings
        result['rms_envelope_10_90_seconds']=crossings['0.9']-crossings['0.1']
    return result


def transfer(reference,model,rate):
    # Welch averaged cross/auto spectra. Shared input/timebase required.
    n=min(16384,len(reference),len(model))
    n=2**int(np.floor(np.log2(n)))
    if n<64: raise ValueError('Transfer comparison needs >=64 samples')
    win=np.hanning(n); xy=np.zeros(n//2+1,complex); xx=np.zeros(n//2+1); yy=xx.copy(); count=0
    for start in range(0,min(len(reference),len(model))-n+1,n//2):
        x=np.fft.rfft(reference[start:start+n]*win); y=np.fft.rfft(model[start:start+n]*win)
        xy+=y*np.conj(x);xx+=np.abs(x)**2;yy+=np.abs(y)**2;count+=1
    h=xy/np.maximum(xx,1e-24)
    coherence=np.abs(xy)**2/np.maximum(xx*yy,1e-24)
    valid=xx>max(float(xx.max())*1e-8,1e-24)
    return np.column_stack((np.fft.rfftfreq(n,1/rate),20*np.log10(np.maximum(np.abs(h),1e-15)),np.angle(h),coherence,valid.astype(int)))


def write_pcm(path,x,rate):
    with wave.open(str(path),'wb') as f:
        f.setnchannels(1);f.setsampwidth(2);f.setframerate(rate)
        f.writeframes((np.clip(x,-1,1-1/32768)*32768).astype('<i2').tobytes())


def stimuli(directory,rate):
    directory.mkdir(parents=True,exist_ok=True)
    t=np.arange(rate*8)/rate
    write_pcm(directory/'external-220hz-bursts.wav',.25*np.sin(2*np.pi*220*t)*((t%2)<1),rate)
    duration=10; t=np.arange(rate*duration)/rate
    f1,f2=20,min(18000,rate*.4);k=np.log(f2/f1)/duration
    write_pcm(directory/'external-log-sweep.wav',.1*np.sin(2*np.pi*f1*(np.exp(k*t)-1)/k),rate)
    rng=np.random.default_rng(9009)
    write_pcm(directory/'external-noise.wav',rng.standard_normal(rate*8)*.08,rate)
    (directory/'stimuli.json').write_text(json.dumps({'rate':rate,'bursts':'220 Hz, peak -12.04 dBFS, one second on/off','sweep':'20 Hz to '+str(f2)+' Hz, peak -20 dBFS, 10 seconds','noise':'seed=9009, Gaussian std=.08; generated reference, not hardware'},indent=2))


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('recording',nargs='?',type=Path);p.add_argument('--compare',type=Path)
    p.add_argument('--fundamental',type=float);p.add_argument('--start',type=float,default=0)
    p.add_argument('--duration',type=float);p.add_argument('--channel',type=int,default=0)
    p.add_argument('--out',type=Path,default=Path('artifacts/Calibration'))
    p.add_argument('--stimuli',action='store_true');p.add_argument('--rate',type=int,default=48000)
    a=p.parse_args();a.out.mkdir(parents=True,exist_ok=True)
    if a.stimuli: stimuli(a.out,a.rate)
    if not a.recording:
        if a.stimuli:return
        p.error('recording or --stimuli required')
    sr,x=read_wav(a.recording)
    if not 0<=a.channel<x.shape[1]:p.error('channel outside recording')
    start=int(a.start*sr);end=len(x) if a.duration is None else start+int(a.duration*sr)
    x=x[start:end,a.channel]
    result={'recording':str(a.recording),'analysis':analyze(x,sr,a.fundamental)}
    f,s=spectrum(x,sr);np.savetxt(a.out/'spectrum.csv',np.column_stack((f,20*np.log10(np.maximum(s,1e-15)))),delimiter=',',header='frequency_hz,magnitude_dbfs',comments='')
    if a.compare:
        sr2,y=read_wav(a.compare)
        if sr2!=sr:p.error('Comparison sample rates must match; resample deliberately before analysis')
        y=y[start:end,a.channel];result['comparison']=analyze(y,sr,a.fundamental)
        result['comparison_rms_difference_db']=result['comparison']['rms_dbfs']-result['analysis']['rms_dbfs']
        np.savetxt(a.out/'transfer.csv',transfer(x,y,sr),delimiter=',',header='frequency_hz,gain_db,phase_radians,coherence,excited_bin',comments='')
        result['transfer_caution']='Cross-spectrum transfer requires time-aligned recordings of the same excitation. Free-running synths invalidate phase/coherence comparisons.'
    (a.out/'analysis.json').write_text(json.dumps(result,indent=2,allow_nan=False))
    print(json.dumps(result,indent=2,allow_nan=False))


if __name__=='__main__':main()
