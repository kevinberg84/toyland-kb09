"""Measure Kevin's specific slow VCO/self-oscillation interaction.

Uses the already generated independent 4/8/16/32x convergence engines.
These are laboratory copies, never shipping code. A modulated signal has real
sidebands: broadband energy or a waveform difference is NOT an alias measure.
"""
from pathlib import Path
import subprocess
import json
import wave
import numpy as np

ROOT=Path(__file__).resolve().parents[2]
OUT=ROOT/'artifacts/AnalogInvestigation/SlowInteraction'
OUT.mkdir(parents=True,exist_ok=True)
for factor in (4,8,16,32):
    if not (OUT.parent/f'RateConvergence/r{factor}/Engine.cpp').exists():
        raise RuntimeError('Generate independent engines with rate_convergence.py first')
code='\n'.join(f'#include "../RateConvergence/r{x}/Engine.cpp"' for x in (4,8,16,32))+r'''
#include <fstream>
#include <iostream>
#include <chrono>
#include <cmath>
#include <vector>
template<class E,class C>void render(int factor){
 for(int rate:{48000,96000})for(int wave:{1,2})for(int direction:{0,1})for(float age:{0.f,1.f}){
  E e;e.prepare(rate);C c;c.vcaMode=0;c.vcfEnv=0;c.vcoLevel=.5f;c.volume=.7f;
  c.wave=float(wave);c.age=age;c.resonance=1;c.cutoff=.449f;c.lfoRate=0;
  c.vcfMod=direction==0?.016f:0;c.vcoMod=direction==1?.08f:0;
  e.setControls(c);e.noteOn(69,127);
  std::vector<float> data(rate*12);double peak=0,energy=0;bool finite=true;
  auto begin=std::chrono::steady_clock::now();
  for(auto& x:data){x=e.process(0);finite&=std::isfinite(x);peak=std::max(peak,double(std::abs(x)));energy+=double(x)*x;}
  const double wall=std::chrono::duration<double>(std::chrono::steady_clock::now()-begin).count();
  const auto stem="r"+std::to_string(factor)+"_"+std::to_string(rate)+"_w"+std::to_string(wave)+"_d"+std::to_string(direction)+"_a"+std::to_string(int(age));
  std::ofstream f(stem+".f32",std::ios::binary);f.write(reinterpret_cast<char*>(data.data()),data.size()*sizeof(float));
  std::cout<<factor<<","<<rate<<","<<wave<<","<<direction<<","<<int(age)<<","<<wall/12<<","<<e.solverFailures()<<","<<peak<<","<<std::sqrt(energy/data.size())<<","<<finite<<std::endl;
 }
}
int main(){std::cout<<"factor,rate,wave,direction,age,cpu_ratio,solver_failures,peak,rms,finite\n";
render<sh09_r4::Engine,sh09_r4::Controls>(4);
render<sh09_r8::Engine,sh09_r8::Controls>(8);
render<sh09_r16::Engine,sh09_r16::Controls>(16);
render<sh09_r32::Engine,sh09_r32::Controls>(32);
}
'''
(OUT/'slow.cpp').write_text(code)
vs=r'C:\Program Files\Microsoft Visual Studio\18\Community\Common7\Tools\VsDevCmd.bat'
(OUT/'build.cmd').write_text(f'@echo off\ncall "{vs}" -arch=x64 -host_arch=x64 >nul\ncl /nologo /std:c++20 /O2 /EHsc /utf-8 slow.cpp /Fe:slow.exe > build.log 2>&1\nexit /b %errorlevel%\n')
subprocess.run(['cmd','/c','build.cmd'],cwd=OUT,check=True)
with (OUT/'performance.csv').open('w') as f:
    subprocess.run([str(OUT/'slow.exe')],cwd=OUT,stdout=f,check=True)
measurements=[]
for rate in (48000,96000):
 for oscillator in (1,2):
  for direction in (0,1):
   for age in (0,1):
    data={factor:np.fromfile(OUT/f'r{factor}_{rate}_w{oscillator}_d{direction}_a{age}.f32',dtype=np.float32).astype(float) for factor in (4,8,16,32)}
    for factor,x in data.items():
     assert len(x)==rate*12 and np.isfinite(x).all()
     tail=x[rate:]
     # Short-time RMS quantifies the audible amplitude beating without calling
     # its physically possible intermodulation sidebands "digital distortion".
     bins=tail[:len(tail)//480*480].reshape(-1,480)
     envelope=np.sqrt(np.mean(bins*bins,axis=1))
     measurements.append(dict(factor=factor,rate=rate,wave=oscillator,direction=direction,age=age,
       rms=float(np.sqrt(np.mean(tail*tail))),peak=float(np.max(np.abs(tail))),
       envelope_p10=float(np.percentile(envelope,10)),envelope_p90=float(np.percentile(envelope,90)),
       reference_difference_rms=float(np.sqrt(np.mean((tail-data[32][rate:])**2)))))
    if rate==48000 and age==0:
     audio=np.concatenate([data[4][rate:],np.zeros(rate//2),data[32][rate:]])
     assert np.max(np.abs(audio))<1
     name=f'{"saw" if oscillator==1 else "square"}-{ "filter-moving" if direction==0 else "vco-moving"}-4x-then-32x.wav'
     with wave.open(str(OUT/name),'wb') as f:
      f.setnchannels(1);f.setsampwidth(2);f.setframerate(rate);f.writeframes((audio*32767).astype('<i2').tobytes())
(OUT/'measurements.json').write_text(json.dumps(measurements,indent=2))
print('64 twelve-second finite renders; 4x/32x same-gain audition files exported. These are numerical comparisons, not hardware validation.')
