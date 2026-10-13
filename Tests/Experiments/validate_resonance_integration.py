"""Build and run the real engine tests without rebuilding plugin artwork."""
from pathlib import Path
import subprocess,json,sys
ROOT=Path(__file__).resolve().parents[2]
OUT=ROOT/'artifacts/AnalogInvestigation/ResonanceIntegration';OUT.mkdir(parents=True,exist_ok=True)
vs=r'C:\Program Files\Microsoft Visual Studio\18\Community\Common7\Tools\VsDevCmd.bat'
tasks=[('dsp','Tests/DSPTests.cpp',[]),('legacy','Tests/RestoredDSPTests.cpp',['Tests/ReferenceDSP113/Engine.cpp']),
       ('resonance','Tests/ResonanceIntegrationTests.cpp',[])]
lines=['@echo off',f'call "{vs}" -arch=x64 -host_arch=x64 >nul','if errorlevel 1 exit /b 1']
for name,test,extras in tasks:
    extra_inputs=[]
    for i,extra in enumerate(extras):
        wrapper=OUT/f'{name}-reference-{i}.cpp'
        wrapper.write_text('#include "'+(ROOT/extra).as_posix()+'"\n')
        extra_inputs.append(wrapper)
    inputs=[ROOT/'Source/DSP/Engine.cpp',ROOT/test,*extra_inputs]
    lines+=[f'cl /nologo /std:c++20 /O2 /EHsc /utf-8 '+ ' '.join(f'"{p}"' for p in inputs)+f' /Fe:{name}.exe > {name}-build.log 2>&1','if errorlevel 1 exit /b 1']
(OUT/'build.cmd').write_text('\n'.join(lines)+'\n')
subprocess.run(['cmd','/c','build.cmd'],cwd=OUT,check=True)
reports=[]
for name,_,_ in tasks:
    print('Running',name,flush=True)
    p=subprocess.run([str(OUT/f'{name}.exe')],cwd=OUT,text=True,capture_output=True)
    (OUT/f'{name}.log').write_text(p.stdout+p.stderr)
    rows=p.stdout.splitlines();important=[r for r in rows if r.startswith(('FAIL','RESULT','MEASURE'))]
    reports.append(dict(name=name,exit_code=p.returncode,important=important))
    print('\n'.join(important),flush=True)
(OUT/'validation.json').write_text(json.dumps(reports,indent=2))
sys.exit(int(any(r['exit_code'] for r in reports)))
