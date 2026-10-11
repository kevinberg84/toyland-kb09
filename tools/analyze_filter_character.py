"""Analyze offline float WAV captures. Residual includes aliasing and other nonharmonic energy."""
from pathlib import Path
import csv, json, sys, struct
import numpy as np

def wavread(path):
    raw=path.read_bytes();assert raw[:4]==b'RIFF' and raw[8:12]==b'WAVE'
    offset=12;sr=None;audio=None
    while offset+8<=len(raw):
        kind=raw[offset:offset+4];size=struct.unpack_from('<I',raw,offset+4)[0];body=raw[offset+8:offset+8+size]
        if kind==b'fmt ':
            fmt,channels,sr=struct.unpack_from('<HHI',body);assert fmt==3 and channels==1
        if kind==b'data':audio=np.frombuffer(body,dtype='<f4')
        offset+=8+size+(size&1)
    return sr,audio

def maximize(func,lo,hi):
    ratio=(5**.5-1)/2
    a=hi-ratio*(hi-lo);b=lo+ratio*(hi-lo);fa=func(a);fb=func(b)
    for _ in range(36):
        if fa<fb:lo=a;a=b;fa=fb;b=lo+ratio*(hi-lo);fb=func(b)
        else:hi=b;b=a;fb=fa;a=hi-ratio*(hi-lo);fa=func(a)
    return (lo+hi)/2

folder=Path(sys.argv[1])
for model in ('baseline','current'):
    path=folder/(model+'.csv')
    if not path.exists():continue
    rows=[]
    for row in csv.DictReader(path.open()):
        sr,audio=wavread(folder/row['file']);x=audio.astype(float);n=len(x)
        f=float(row['frequency_hz']);rms=np.sqrt(np.mean(x*x));t=np.arange(n)/sr
        phase=2*np.pi*np.arange(n)/(n-1)
        w=.35875-.48829*np.cos(phase)+.14128*np.cos(2*phase)-.01168*np.cos(3*phase)
        row={k:float(v) if k!='file' else v for k,v in row.items()}
        row['rms_dbfs']=float(20*np.log10(max(rms,1e-15)))
        if f>10 and rms>1e-5:
            def amplitude(hz):return 2*abs(np.dot(x*w,np.exp(-2j*np.pi*hz*t)))/w.sum()
            f=maximize(amplitude,f-.8,f+.8);amps=np.array([amplitude(f*k) for k in range(1,min(20,int((sr*.49)//f))+1)])
            row.update(fundamental_hz=f,thd_percent=float(100*np.linalg.norm(amps[1:])/amps[0]),
                       h2_dbc=float(20*np.log10(max(amps[1]/amps[0],1e-15))) if len(amps)>1 else None,
                       h3_dbc=float(20*np.log10(max(amps[2]/amps[0],1e-15))) if len(amps)>2 else None)
            power=abs(np.fft.rfft(x*w))**2;bins=np.arange(len(power));mask=bins<6
            for k in range(1,int((sr/2)//f)+1):mask|=abs(bins-f*k*n/sr)<6
            row['nonharmonic_residual_db']=float(10*np.log10(max(power[~mask].sum()/power[mask].sum(),1e-30)))
        rows.append(row)
    (folder/(model+'-analysis.json')).write_text(json.dumps(rows,indent=2),encoding='utf-8')
    print(model,len(rows),'cases; max solver failures',max(r['solver_failures'] for r in rows))
    for r in rows:
        if r['sample_rate']==48000 and r['cutoff']==.6 and r['resonance']==1 and r['block_size']==256:
            print(' age',r['age'],'Hz',round(r.get('fundamental_hz',0),2),'RMS dBFS',round(r['rms_dbfs'],2),
                  'THD %',round(r.get('thd_percent',0),3),'H2/H3 dBc',r.get('h2_dbc'),r.get('h3_dbc'),
                  'residual',r.get('nonharmonic_residual_db'),'CPU',round(r['cpu_ratio'],3))
