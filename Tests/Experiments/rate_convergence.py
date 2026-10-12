"""Offline experiment only: vary rate without changing published Engine sources.

Run with the bundled Python; uses MSVC installed on this workstation.
The generated engines have equal FIR transition bands and host latency.
Rate convergence is NOT an alias-only measurement or hardware validation.
"""
from pathlib import Path
import subprocess
import json
import sys
import wave
import numpy as np

ROOT = Path(__file__).resolve().parents[2]
OUT = ROOT / 'artifacts' / 'AnalogInvestigation' / 'RateConvergence'
OUT.mkdir(parents=True, exist_ok=True)
source = (ROOT / 'Source/DSP/Engine.cpp').read_text()
header = (ROOT / 'Source/DSP/Engine.h').read_text()
lfo = (ROOT / 'Source/DSP/LfoRate.h').read_text()

def replace_once(text, old, new):
    assert text.count(old) == 1, old
    return text.replace(old, new)

includes = []
calls = []
for factor in (4, 8, 16, 32):
    folder = OUT / f'r{factor}'
    folder.mkdir(exist_ok=True)
    ns = f'sh09_r{factor}'
    h = header.replace('namespace sh09 {', f'namespace {ns} {{')
    h = replace_once(h, 'oversamplingFactor=4', f'oversamplingFactor={factor}')
    h = replace_once(h, 'firLength=513, alignment=256',
                     f'firLength={128*factor+1}, alignment={64*factor}')
    h = replace_once(h, '(firLength+3)/4', '(firLength+oversamplingFactor-1)/oversamplingFactor')
    s = source.replace('namespace sh09 {', f'namespace {ns} {{')
    s = replace_once(s, 'fc=.115', 'fc=.46/oversamplingFactor')
    s = replace_once(s, 'j*4+os', 'j*oversamplingFactor+os')
    s = replace_once(s, 'inputHistory[index]*4', 'inputHistory[index]*oversamplingFactor')
    (folder / 'Engine.h').write_text(h)
    (folder / 'Engine.cpp').write_text(s)
    (folder / 'LfoRate.h').write_text(lfo.replace('namespace sh09 {', f'namespace {ns} {{'))
    includes.append(f'#include "r{factor}/Engine.cpp"')
    calls.append(f'render<{ns}::Engine,{ns}::Controls>({factor});')

harness = r'''
#include <fstream>
#include <iostream>
#include <chrono>
#include <cmath>
#include <string>
#include <vector>
#include <filesystem>
template<class E,class C> void render(int factor) {
 for(int rate:{44100,48000,96000}) for(int test=0;test<6;++test) {
  E engine;engine.prepare(rate);C c;
  c.vcaMode=0;c.vcfEnv=0;c.cutoff=1;c.volume=.7f;c.vcoLevel=1;
  c.wave=test==1?2:1;
  if(test==2){c.wave=3;c.pwmMode=1;c.pulseWidth=1;c.lfoRate=1;}
  if(test==3){c.subLevel=1;c.resonance=.65f;c.cutoff=.65f;}
  if(test==4){c.resonance=.8f;c.cutoff=.7f;c.vcfMod=.7f;c.lfoRate=.9f;}
  if(test==5){c.vcoMod=.75f;c.lfoRate=1;}
  engine.setControls(c);engine.noteOn(test==3?48:84,127);
  std::vector<float> output(rate*2);
  auto start=std::chrono::steady_clock::now();
  for(auto& value:output)value=engine.process(0);
  double wall=std::chrono::duration<double>(std::chrono::steady_clock::now()-start).count();
  std::ofstream f("r"+std::to_string(factor)+"_"+std::to_string(rate)+"_"+std::to_string(test)+".f32",std::ios::binary);
  f.write(reinterpret_cast<const char*>(output.data()),output.size()*sizeof(float));
  std::cout<<factor<<","<<rate<<","<<test<<","<<wall/2<<","<<engine.solverFailures()<<std::endl;
 }
}
int main(){std::cout<<"factor,rate,case,cpu_seconds_per_audio_second,solver_failures\n";
CALLS
}
'''.replace('CALLS', '\n'.join(calls))
(OUT / 'experiment.cpp').write_text('\n'.join(includes) + '\n' + harness)
vs = r'C:\Program Files\Microsoft Visual Studio\18\Community\Common7\Tools\VsDevCmd.bat'
(OUT / 'build.cmd').write_text(f'@echo off\ncall "{vs}" -arch=x64 -host_arch=x64 >nul\ncl /nologo /std:c++20 /O2 /EHsc /utf-8 experiment.cpp /Fe:experiment.exe > build.log 2>&1\nexit /b %errorlevel%\n')
if '--analyze-only' not in sys.argv:
    subprocess.run(['cmd', '/c', 'build.cmd'], cwd=OUT, check=True)
    with (OUT / 'performance.csv').open('w') as f:
        subprocess.run([str(OUT / 'experiment.exe')], cwd=OUT, stdout=f, check=True)

rows=[]
for rate in (44100,48000,96000):
    for test in range(6):
        reference=np.fromfile(OUT/f'r32_{rate}_{test}.f32',dtype=np.float32).astype(float)
        for factor in (4,8,16):
            x=np.fromfile(OUT/f'r{factor}_{rate}_{test}.f32',dtype=np.float32).astype(float)
            # First .3 seconds excluded; equal declared host latency, no fitted EQ/gain.
            x=x[int(.3*rate):];r=reference[int(.3*rate):]
            assert np.isfinite(x).all() and np.isfinite(r).all()
            def db(a,b):return float(10*np.log10(max(np.mean(a*a),1e-30)/max(np.mean(b*b),1e-30)))
            window=np.hanning(len(x))
            mask=np.fft.rfftfreq(len(x),1/rate)<12000
            low=np.abs(np.fft.rfft((x-r)*window))[mask]
            lowref=np.abs(np.fft.rfft(r*window))[mask]
            rows.append(dict(rate=rate,case=test,factor=factor,
                             difference_db=db(x-r,r),difference_below12k_db=db(low,lowref),
                             rms=float(np.sqrt(np.mean(x*x))),peak=float(np.max(np.abs(x)))))
(OUT/'convergence.json').write_text(json.dumps(rows,indent=2))
spectral=[]
for rate in (44100,48000,96000):
    for test in (0,1):
        fundamental=440*2**((84-69)/12)
        for factor in (4,8,16,32):
            x=np.fromfile(OUT/f'r{factor}_{rate}_{test}.f32',dtype=np.float32)[int(.3*rate):].astype(float)
            power=np.abs(np.fft.rfft(x*np.hanning(len(x))))**2
            frequencies=np.fft.rfftfreq(len(x),1/rate)
            wanted=frequencies<20
            for harmonic in range(1,int(rate/2/fundamental)+1):
                wanted|=np.abs(frequencies-harmonic*fundamental)<8*rate/len(x)
            residual=np.where(wanted,0,power)
            spectral.append(dict(rate=rate,case=test,factor=factor,
                nonharmonic_db=float(10*np.log10(residual.sum()/power[wanted].sum())),
                largest_residual_hz=float(frequencies[np.argmax(residual)])))
(OUT/'static_spectral_residual.json').write_text(json.dumps(spectral,indent=2))
# Same gain for both halves; no peak or loudness normalization concealing level changes.
for test,name in ((0,'saw'),(1,'square'),(3,'sub-resonance'),(4,'filter-modulation'),(5,'pitch-modulation')):
    parts=[np.fromfile(OUT/f'r{factor}_48000_{test}.f32',dtype=np.float32)[14400:] for factor in (4,32)]
    audio=np.concatenate([parts[0],np.zeros(12000),parts[1]])
    assert np.max(np.abs(audio))<1
    with wave.open(str(OUT/f'{name}-4x-then-32x.wav'),'wb') as wav:
        wav.setnchannels(1);wav.setsampwidth(2);wav.setframerate(48000)
        wav.writeframes((audio*32767).astype('<i2').tobytes())
print(f'Completed: {len(rows)} convergence comparisons, {len(spectral)} static spectral measurements.')
