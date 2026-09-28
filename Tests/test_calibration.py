"""Independent synthetic checks for recording analysis, not hardware validation."""
import sys
from pathlib import Path
import tempfile
import numpy as np
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from calibrate import analyze,estimate_frequency,transfer,write_pcm,read_wav
sr=48000;t=np.arange(sr)/sr;x=.5*np.sin(2*np.pi*440*t)
assert abs(estimate_frequency(x,sr)-440)<.02
r=analyze(x,sr,440)
assert abs(r['rms_dbfs']-(-9.03089987))<.001
assert r['thd_ratio_through_harmonic_32']<1e-5
tr=transfer(x,.5*x,sr); valid=tr[:,-1]>0
assert np.max(np.abs(tr[valid,1]+6.0205999))<.001
with tempfile.TemporaryDirectory() as d:
    p=Path(d)/'sine.wav';write_pcm(p,x,sr);rate,y=read_wav(p)
    assert rate==sr and y.shape==(sr,1)
    assert np.max(np.abs(x-y[:,0]))<=1/32768
print('PASS: frequency, RMS, harmonics, transfer gain, PCM roundtrip')
