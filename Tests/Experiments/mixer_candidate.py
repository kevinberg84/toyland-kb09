"""Exercise the isolated ADAA candidate, not a shipping engine change.
Requires stage_isolation.py and stage_isolation.py --extra first.
"""
from pathlib import Path
import subprocess
import json
import wave
import numpy as np
ROOT=Path(__file__).resolve().parents[2]
OUT=ROOT/'artifacts/AnalogInvestigation/StageIsolation'
code=r'''
#include "r4_output/Engine.cpp"
#include "r4_adaaMixer/Engine.cpp"
#include <chrono>
#include <fstream>
#include <iostream>
#include <vector>
#include <string>
template<class E,class C>void run(const std::string& variant){
 for(int rate:{44100,48000,96000})for(int test=0;test<8;++test){
 E e;e.prepare(rate);C c;c.vcaMode=0;c.cutoff=1;c.vcfEnv=0;c.vcoLevel=1;c.volume=.7f;
 if(test==1)c.wave=2;
 if(test==2){c.wave=3;c.pwmMode=1;c.pulseWidth=1;c.lfoRate=1;}
 if(test==3){c.subLevel=1;c.resonance=.7f;c.cutoff=.5f;}
 if(test==4){c.resonance=.8f;c.cutoff=.7f;c.vcfMod=.7f;c.lfoRate=1;}
 if(test==5){c.vcoMod=.75f;c.lfoRate=1;}
 if(test==6){c.age=1;c.subLevel=1;c.resonance=1;c.cutoff=.8f;}
 if(test==7){c.vcoLevel=0;c.resonance=1;c.cutoff=.65f;}
 e.setControls(c);e.noteOn(test==3?48:84,127);
 std::vector<float> data(rate*3);
 auto start=std::chrono::steady_clock::now();
 for(auto& x:data)x=e.process(0);
 double wall=std::chrono::duration<double>(std::chrono::steady_clock::now()-start).count();
 std::ofstream f(variant+"_"+std::to_string(rate)+"_"+std::to_string(test)+".f32",std::ios::binary);
 f.write(reinterpret_cast<char*>(data.data()),data.size()*sizeof(float));
 std::cout<<variant<<","<<rate<<","<<test<<","<<wall/3<<","<<e.solverFailures()<<std::endl;
 }
}
int main(){std::cout<<"variant,rate,case,cpu_seconds_per_audio_second,solver_failures\n";
run<sh09_r4_output::Engine,sh09_r4_output::Controls>("baseline");
run<sh09_r4_adaaMixer::Engine,sh09_r4_adaaMixer::Controls>("candidate");
}
'''
(OUT/'candidate.cpp').write_text(code)
vs=r'C:\Program Files\Microsoft Visual Studio\18\Community\Common7\Tools\VsDevCmd.bat'
(OUT/'candidate.cmd').write_text(f'@echo off\ncall "{vs}" -arch=x64 -host_arch=x64 >nul\ncl /nologo /std:c++20 /O2 /EHsc /utf-8 candidate.cpp /Fe:candidate.exe > candidate-build.log 2>&1\nexit /b %errorlevel%\n')
subprocess.run(['cmd','/c','candidate.cmd'],cwd=OUT,check=True)
with (OUT/'candidate-performance.csv').open('w') as f:
    subprocess.run([str(OUT/'candidate.exe')],cwd=OUT,stdout=f,check=True)
rows=[]
for variant in ('baseline','candidate'):
 for rate in (44100,48000,96000):
  for case in range(8):
   x=np.fromfile(OUT/f'{variant}_{rate}_{case}.f32',dtype=np.float32).astype(float)
   assert len(x)==rate*3 and np.isfinite(x).all()
   rows.append(dict(variant=variant,rate=rate,case=case,peak=float(np.max(np.abs(x))),
     rms=float(np.sqrt(np.mean(x[int(.5*rate):]**2))),
     largest_adjacent_step=float(np.max(np.abs(np.diff(x))))))
(OUT/'candidate-stability.json').write_text(json.dumps(rows,indent=2))
for case,name in ((0,'saw'),(1,'square'),(2,'pwm'),(3,'bass'),(4,'filter-modulation'),(5,'pitch-modulation')):
 parts=[np.fromfile(OUT/f'{v}_48000_{case}.f32',dtype=np.float32)[24000:] for v in ('baseline','candidate')]
 x=np.concatenate([parts[0],np.zeros(12000),parts[1]])
 assert np.max(np.abs(x))<1
 with wave.open(str(OUT/f'{name}-current-then-candidate.wav'),'wb') as w:
  w.setnchannels(1);w.setsampwidth(2);w.setframerate(48000)
  w.writeframes((x*32767).astype('<i2').tobytes())
# Check the derived primitive independently against numerical differentiation.
x=np.linspace(-5,5,10001)
def primitive(x):
 a=np.abs(x)
 return np.where(a>3,.5+4/3*np.log(12)+a-3,x*x/18+4/3*np.log(x*x+3))
def sat(x):return np.where(np.abs(x)>=3,np.sign(x),x*(27+x*x)/(27+9*x*x))
error=float(np.max(np.abs((primitive(x+1e-5)-primitive(x-1e-5))/2e-5-sat(x))))
assert error<1e-8
(OUT/'primitive-check.json').write_text(json.dumps(dict(max_derivative_error=error)))
print(f'48 finite renders; primitive max derivative error {error:.3g}. No hardware/listening validation claimed.')
