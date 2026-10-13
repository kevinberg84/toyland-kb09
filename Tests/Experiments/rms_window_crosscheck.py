"""Check recording-level conclusions with 50/250/500 ms RMS windows.

Reads the manifest's original files; excludes a fixed 1 s at either edge,
retaining internal dips. No normalization or source writes.
"""
from pathlib import Path
import sys,json
ROOT=Path(__file__).resolve().parents[2]
sys.path.insert(0,str(ROOT/'.tmp/dsp-analysis-packages'))
import numpy as np
from scipy.io import wavfile
out=ROOT/'artifacts/AnalogInvestigation/HardwareComparisons'
manifest=json.loads((out/'recording-manifest.json').read_text());results=[]
for record in manifest:
    rate,x=wavfile.read(record['file'])
    assert x.dtype==np.float32 and record['stereo_correlation']>.99999
    x=x.mean(axis=1).astype(float)
    result={'number':record['number'],'instrument':record['instrument']}
    for seconds in (.05,.25,.5):
        n=round(rate*seconds);a=x[:len(x)//n*n].reshape(-1,n)
        t=(np.arange(len(a))+.5)*seconds
        levels=20*np.log10(np.maximum(np.sqrt(np.mean(a*a,axis=1)),1e-12))
        interior=(t>=1)&(t<len(x)/rate-1)
        q=np.percentile(levels[interior],[10,90]);result[str(seconds)]=float(q[1]-q[0])
    results.append(result)
(out/'rms-window-crosscheck.json').write_text(json.dumps(results,indent=2))
print('RMS window crosscheck saved for all 22 recordings.')
