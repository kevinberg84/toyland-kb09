#include <juce_audio_utils/juce_audio_utils.h>
#include <algorithm>
#include <chrono>
#include <cmath>
#include <iostream>
#include <limits>
#include <memory>
#include <vector>

// Loads the actual VST3 module; deliberately does not link instrument DSP.
namespace {
struct Checks {
    int failed=0,passed=0;
    void expect(bool ok,const juce::String& message) {
        if(ok){++passed;std::cout<<"PASS "<<message<<'\n';}
        else{++failed;std::cerr<<"FAIL "<<message<<'\n';}
    }
};
struct Host {
    juce::AudioPluginInstance& p; Checks& checks; bool prepared=false;
    juce::MemoryBlock initial;
    Host(juce::AudioPluginInstance& plugin,Checks& c):p(plugin),checks(c){p.getStateInformation(initial);}
    ~Host(){stop();}
    void stop(){if(prepared)p.releaseResources();prepared=false;}
    void prepare(double rate=48000,int block=256){stop();p.setRateAndBufferSizeDetails(rate,block);p.prepareToPlay(rate,block);prepared=true;}
    void restore(){stop();p.setStateInformation(initial.getData(),int(initial.getSize()));}
    juce::AudioProcessorParameter* find(const juce::String& id){
        const auto hash=juce::String(juce::VST3ClientExtensions::convertJuceParameterId(id));
        const auto hash2=juce::String(juce::VST3ClientExtensions::convertJuceParameterId(id,false));
        for(auto* param:p.getParameters())if(auto* hp=dynamic_cast<juce::HostedAudioProcessorParameter*>(param)){
            const auto actual=hp->getParameterID();if(actual==id||actual==hash||actual==hash2)return param;
        }
        return nullptr;
    }
    void set(const juce::String& id,float normalized){
        if(auto* param=find(id))param->setValueNotifyingHost(normalized);
        else checks.expect(false,"parameter exists: "+id);
    }
    juce::AudioBuffer<float> buffer(int n){return {juce::jmax(1,p.getTotalNumInputChannels(),p.getTotalNumOutputChannels()),n};}
    double process(juce::AudioBuffer<float>& audio,juce::MidiBuffer& midi,bool& finite){
        p.processBlock(audio,midi);double energy=0;
        for(int c=0;c<p.getTotalNumOutputChannels();++c)for(int s=0;s<audio.getNumSamples();++s){
            const auto x=audio.getSample(c,s);finite=finite&&std::isfinite(x)&&std::abs(x)<=8.0f;
            if(std::isfinite(x))energy+=double(x)*x;
        }
        return energy;
    }
    void flush(){auto audio=buffer(32);audio.clear();juce::MidiBuffer midi;bool finite=true;process(audio,midi,finite);}
    std::vector<float> values(){std::vector<float> result;for(auto* param:p.getParameters())result.push_back(param->getValue());return result;}
    bool match(const std::vector<float>& values){
        if(values.size()!=size_t(p.getParameters().size()))return false;
        for(int i=0;i<p.getParameters().size();++i){float x=p.getParameters()[i]->getValue();if(!std::isfinite(x)||std::abs(x-values[size_t(i)])>.0002f){std::cout<<"MISMATCH "<<p.getParameters()[i]->getName(128)<<" expected="<<values[size_t(i)]<<" actual="<<x<<'\n';return false;}}
        return true;
    }
    void synthPatch(){restore();if(!find("vcoLevel"))return;
        set("vcoLevel",.8f);set("subLevel",0);set("extLevel",0);set("vcaMode",1); // Gate choice 2/2
        set("age",0);set("resonance",0);set("cutoff",.8f);set("vcfEnv",0);set("attack",0);set("release",0);
    }
    void externalPatch(){restore();set("vcoLevel",0);set("subLevel",0);set("extLevel",.8f);
        set("vcaMode",0);set("age",0);set("resonance",0);set("cutoff",.85f);set("vcfEnv",0);set("modern",0);
    }
};
void matrix(Host& h){
    for(double rate:{44100.0,48000.0,88200.0,96000.0,192000.0})for(int block:{1,17,64,127,256,1024,8192}){
        h.synthPatch();h.prepare(rate,block);auto audio=h.buffer(block);juce::MidiBuffer midi;double energy=0;bool finite=true;
        for(int b=0;b<juce::jmax(4,int(rate*.08/block));++b){audio.clear();midi.clear();
            if(b==0)midi.addEvent(juce::MidiMessage::noteOn(1,69,juce::uint8(100)),block>17?17:0);
            energy+=h.process(audio,midi,finite);
        }
        const bool latencyOK=!h.find("extLevel")||h.p.getLatencySamples()==128;
        h.checks.expect(finite&&energy>1e-10&&latencyOK,"render "+juce::String(rate)+"/"+juce::String(block)+" energy="+juce::String(energy,7)+" latency="+juce::String(h.p.getLatencySamples()));
    }
}
void midiMappings(Host& h){
    if(!h.find("vcoMod"))return;
    h.restore();h.prepare(48000,128);
    auto audio=h.buffer(128);juce::MidiBuffer midi;
    int mapped=0;bool mappingOK=true,finite=true;
    for(auto* parameter:h.p.getParameters())if(parameter->isAutomatable()&&!parameter->isMetaParameter()){
        for(float requested:{0.0f,.25f,.5f,.75f,1.0f}){
            const auto steps=parameter->getNumSteps();
            if(steps>1&&steps<100000)requested=std::round(requested*(steps-1))/float(steps-1);
            parameter->setValueNotifyingHost(requested);
            audio.clear();midi.clear();
            midi.addEvent(juce::MidiMessage::controllerEvent(16,74,107),0);
            h.process(audio,midi,finite);
            mappingOK= mappingOK&&std::abs(parameter->getValue()-requested)<.0002f;
        }
        ++mapped;
    }
    h.checks.expect(mappingOK&&finite&&mapped>=34,
                    "all "+juce::String(mapped)+" automatable VST3 parameters follow host mappings at five legal positions with unrelated MIDI present");
    h.restore();h.prepare(48000,128);
    bool wheelOK=true;
    auto settle=[&]{
        // The DLL has its own JUCE Timer singleton. Dispatch real platform
        // messages, as a DAW does; pumping only the host's timers misses it.
        juce::MessageManager::getInstance()->runDispatchLoopUntil(35);
        audio.clear();midi.clear();h.process(audio,midi,finite);
    };
    // Deliberately begin above zero, as a physical wheel can skip its endpoint.
    for(int value:{4,18,42,64,83,108,127,111,87,62,38,13,0}){
        audio.clear();midi.clear();
        midi.addEvent(juce::MidiMessage::controllerEvent(1,1,value),17);
        midi.addEvent(juce::MidiMessage::noteOn(1,60,juce::uint8(100)),0);
        h.process(audio,midi,finite);settle();
        const auto actual=h.find("vcoMod")->getValue();
        wheelOK=wheelOK&&std::abs(actual-value/127.f)<.0002f;
    }
    h.checks.expect(wheelOK&&finite,"actual VST3 CC1 follows an up/down sweep, including first movement above zero");
    h.set("vcoMod",.6f);settle();
    h.set("vcoMod",.3f);audio.clear();midi.clear();
    midi.addEvent(juce::MidiMessage::controllerEvent(16,1,117),0);
    h.process(audio,midi,finite);settle();
    h.checks.expect(std::abs(h.find("vcoMod")->getValue()-117.f/127)<.0002f,
                    "actual VST3 CC1 resumes immediately after host automation on channel 16");
    h.restore();h.prepare();
    std::vector<std::pair<juce::AudioProcessorParameter*,float>> before;
    for(auto* parameter:h.p.getParameters())if(parameter->isAutomatable()&&!parameter->isMetaParameter())
        before.emplace_back(parameter,parameter->getValue());
    audio=h.buffer(256);audio.clear();midi.clear();
    for(int channel=1;channel<=16;++channel)for(int cc=0;cc<120;++cc)if(cc!=1)
        midi.addEvent(juce::MidiMessage::controllerEvent(channel,cc,(cc*7)%128),0);
    h.process(audio,midi,finite);
    bool unchanged=true;for(const auto& [parameter,value]:before)unchanged=unchanged&&std::abs(parameter->getValue()-value)<.0002f;
    // JUCE's hidden MIDI-transport parameters legitimately follow the CCs.
    h.checks.expect(unchanged&&finite,"all ordinary CCs on all MIDI channels leave host-assigned parameter mappings intact");
    h.restore();
}
void timingAndBlocks(Host& h){
    // VST3 setupProcessing.maxSamplesPerBlock is a host contract. The wrapper
    // allocates its channel mapping to that maximum, unlike a direct JUCE DSP call.
    h.synthPatch();h.prepare(48000,8192);auto audio=h.buffer(1024);audio.clear();juce::MidiBuffer midi;bool finite=true;
    midi.addEvent(juce::MidiMessage::noteOn(1,69,juce::uint8(100)),173);h.process(audio,midi,finite);
    double before=0,after=0;
    for(int s=0;s<1024;++s){double x=audio.getSample(0,s);(s<173?before:after)+=x*x;}
    h.checks.expect(finite&&before<1e-12&&after>1e-8,"MIDI offset 173: silence before note, audio after");
    for(int size:{0,1,257,17,8192,3,64,0}){
        auto variable=h.buffer(size);variable.clear();midi.clear();
        if(size==17)midi.addEvent(juce::MidiMessage::noteOff(1,69),16);
        h.process(variable,midi,finite);
    }
    h.checks.expect(finite,"changing/zero/large blocks within VST3 negotiated maximum remain finite");
    h.synthPatch();h.prepare();auto last=h.buffer(256);last.clear();midi.clear();
    midi.addEvent(juce::MidiMessage::noteOn(1,60,juce::uint8(100)),255);h.process(last,midi,finite);before=0;
    for(int s=0;s<255;++s)before+=std::abs(last.getSample(0,s));
    h.checks.expect(finite&&before<1e-9,"last-sample MIDI note does not start early");
}
void stateAndPrograms(Host& h){
    h.restore();h.prepare();int index=0;
    for(auto* param:h.p.getParameters())if(param->isAutomatable()&&!param->isMetaParameter()&&!param->isBoolean()){
        // Tenths also remain representable by the lab's legacy gain control
        // (0..0.5 in 0.01 units), which VST3 advertises as continuous.
        float value=float((++index*2)%8+2)/10.0f;
        const int steps=param->getNumSteps();
        // A host cache may temporarily retain a nonrepresentable value while the
        // processor quantises it. Test persistence using legal advertised steps.
        if(steps>1&&steps<100000)value=std::round(value*float(steps-1))/float(steps-1);
        param->setValueNotifyingHost(value);
    }
    h.flush();const auto saved=h.values();juce::MemoryBlock state;h.p.getStateInformation(state);
    for(auto* param:h.p.getParameters())if(param->isAutomatable()&&!param->isMetaParameter())param->setValueNotifyingHost(param->getDefaultValue());
    h.flush();h.p.setStateInformation(state.getData(),int(state.getSize()));h.flush();
    h.checks.expect(state.getSize()>0&&h.match(saved),"all exposed parameter values round-trip through actual VST3 state");
    const auto intact=h.values();const char invalid[]="invalid state\0\xff";
    h.p.setStateInformation(invalid,int(sizeof invalid));h.flush();
    h.checks.expect(h.match(intact),"malformed state leaves valid patch intact");
    const int programs=h.p.getNumPrograms();std::cout<<"INFO exposed programs="<<programs<<'\n';
    std::vector<std::vector<float>> presets;bool valid=true;
    for(int repeat=0;repeat<3;++repeat)for(int i=0;i<programs;++i){
        h.p.setCurrentProgram(i);h.flush();valid=valid&&h.p.getCurrentProgram()==i&&h.p.getProgramName(i).isNotEmpty();
        if(repeat==0)presets.push_back(h.values());else valid=valid&&h.match(presets[size_t(i)]);
        juce::MemoryBlock patch;h.p.getStateInformation(patch);h.p.setCurrentProgram((i+1)%programs);h.flush();
        h.p.setStateInformation(patch.getData(),int(patch.getSize()));h.flush();valid=valid&&h.p.getCurrentProgram()==i&&h.match(presets[size_t(i)]);
    }
    if(programs>0)h.checks.expect(valid,"three factory-program cycles, deterministic values and per-preset state restore");
    else if(h.find("extLevel"))h.checks.expect(false,"SH-09 factory programs exposed");
    else std::cout<<"SKIP program enumeration: VST3 wrapper omits the lab's single program\n";
    h.restore();
}
double externalRender(Host& h,const juce::AudioChannelSet& input,int channel,bool antiphase=false,bool extreme=false,float cutoff=.85f,bool follower=false){
    h.externalPatch();auto layout=h.p.getBusesLayout();layout.inputBuses.set(0,input);
    bool accepted=h.p.setBusesLayout(layout);h.checks.expect(accepted,"external arrangement accepted: "+input.getDescription());if(!accepted)return -1;
    h.set("cutoff",cutoff);if(follower){h.set("vcfEnvMode",1);h.set("vcfEnv",1);}
    h.prepare(48000,127);double energy=0;bool finite=true;auto audio=h.buffer(127);juce::MidiBuffer midi;
    for(int b=0;b<200;++b){audio.clear();midi.clear();
        for(int c=0;c<h.p.getTotalNumInputChannels();++c)for(int s=0;s<127;++s){
            float x=.2f*std::sin(float((b*127+s)*(juce::MathConstants<double>::twoPi*600/48000)));
            if(channel>=0&&c!=channel)x=0;if(antiphase&&c==1)x=-x;
            if(extreme)switch((b*127+s)%7){
                case 0:x=std::numeric_limits<float>::quiet_NaN();break;
                case 1:x=std::numeric_limits<float>::infinity();break;
                case 2:x=-std::numeric_limits<float>::infinity();break;
                case 3:x=1e30f;break;case 4:x=-1e30f;break;default:break;
            }
            audio.setSample(c,s,x);
        }
        double e=h.process(audio,midi,finite);if(b>20)energy+=e;
    }
    h.checks.expect(finite,"external output finite/bounded");h.stop();return energy;
}
void externalTests(Host& h){
    if(h.p.getBusCount(true)==0){h.checks.expect(false,"SH-09 external input bus exists");return;}
    auto stereo=juce::AudioChannelSet::stereo();
    double left=externalRender(h,stereo,0),right=externalRender(h,stereo,1),both=externalRender(h,stereo,-1);
    double cancelled=externalRender(h,stereo,-1,true),mono=externalRender(h,juce::AudioChannelSet::mono(),-1);
    double disabled=externalRender(h,juce::AudioChannelSet::disabled(),-1);
    h.checks.expect(left>1e-7&&right>1e-7&&std::abs(left-right)<std::max(left,right)*.02,"both stereo inputs consumed before in-place output writes");
    h.checks.expect(both>1e-7&&mono>1e-7&&std::abs(both-mono)<std::max(both,mono)*.02,"mono equals equal-channel stereo fold-down");
    h.checks.expect(cancelled<1e-10&&disabled<1e-10,"antiphase fold-down and disabled external bus silent");
    double low=externalRender(h,stereo,-1,false,false,.08f);
    h.checks.expect(low<both*.1,"external tone traverses cutoff-dependent filter");externalRender(h,stereo,-1,false,true);
    double withoutFollower=externalRender(h,stereo,-1,false,false,.3f);
    double withFollower=externalRender(h,stereo,-1,false,false,.3f,true);
    h.checks.expect(withFollower>withoutFollower*2&&withFollower>1e-8,"external envelope follower increases filtered output energy without MIDI");
    h.restore();auto layout=h.p.getBusesLayout();layout.inputBuses.set(0,stereo);h.checks.expect(h.p.setBusesLayout(layout),"restore stereo arrangement");
}
void stress(Host& h,double seconds){
    h.synthPatch();h.prepare(48000,127);bool finite=true;juce::Random random(9009);juce::MidiBuffer midi;auto audio=h.buffer(127);
    const bool synth=h.find("resonance")!=nullptr;
    for(int b=0;b<500;++b){
        if(synth){for(auto* id:{"cutoff","resonance","vcfMod","vcfEnv","vcoMod","pulseWidth","age","extLevel"})h.set(id,b%3==0?1.0f:b%3==1?0.0f:random.nextFloat());h.set("vcoLevel",1);h.set("subLevel",1);}
        audio.clear();midi.clear();for(int s=0;s<127;s+=3){int note=random.nextInt(128);
            midi.addEvent(juce::MidiMessage::noteOn(1,note,juce::uint8(1+random.nextInt(127))),s);
            midi.addEvent(juce::MidiMessage::noteOff(1,note),juce::jmin(126,s+1));midi.addEvent(juce::MidiMessage::pitchWheel(1,random.nextInt(16384)),s);
        }
        if(b%7==0)midi.addEvent(juce::MidiMessage::allSoundOff(1),126);h.process(audio,midi,finite);
    }
    h.checks.expect(finite,"500 blocks extreme/random automation, dense MIDI, pitch bend and panic");
    h.synthPatch();h.prepare(48000,128);audio=h.buffer(128);int blocks=int(seconds*48000/128);double energy=0,time=0,peak=0;
    auto start=std::chrono::steady_clock::now();
    for(int b=0;b<blocks;++b){audio.clear();midi.clear();if(b==0)midi.addEvent(juce::MidiMessage::noteOn(1,48,juce::uint8(100)),0);
        auto a=std::chrono::steady_clock::now();energy+=h.process(audio,midi,finite);double elapsed=std::chrono::duration<double>(std::chrono::steady_clock::now()-a).count();time+=elapsed;peak=std::max(peak,elapsed);
    }
    h.checks.expect(finite&&energy>1e-8,"sustained "+juce::String(seconds,1)+" second render audible/finite");
    std::cout<<"TIMING render_seconds="<<seconds<<" wall_seconds="<<std::chrono::duration<double>(std::chrono::steady_clock::now()-start).count()<<" process_seconds="<<time<<" realtime_percent="<<100*time/seconds<<" peak_block_ms="<<peak*1000<<" deadline_ms="<<128.0/48<<" (offline single instance including scheduling outliers; not a DAW CPU guarantee)\n";h.stop();
}
void editors(Host& h){
    h.restore();h.prepare();bool valid=h.p.hasEditor();
    for(int i=0;i<10&&valid;++i){std::unique_ptr<juce::AudioProcessorEditor> editor(h.p.createEditorIfNeeded());if(!editor){valid=false;break;}
        valid=editor->getWidth()>0&&editor->getHeight()>0;
        if(editor->isResizable()){int w=editor->getWidth(),height=editor->getHeight();editor->setBoundsConstrained({0,0,w+100,height+60});editor->setBoundsConstrained({0,0,w,height});}
        h.flush();h.p.editorBeingDeleted(editor.get());
    }
    h.checks.expect(valid,"10 native editor create/process/resize/destroy cycles (no mouse/visual assertion)");h.stop();
}
void exportPresets(Host& h,const juce::File& directory){
    if(!h.find("extLevel"))return;
    h.checks.expect(directory.createDirectory().wasOk(),"preset export directory available");
    h.restore();h.prepare();int count=0;bool good=true;
    for(int i=0;i<h.p.getNumPrograms();++i){
        h.p.setCurrentProgram(i);h.flush();const auto expected=h.values();
        juce::MemoryBlock state;h.p.getStateInformation(state);
        const auto name=juce::File::createLegalFileName(h.p.getProgramName(i).replace(" / "," - "))+".sh09state";
        const auto file=directory.getChildFile(name);
        const bool written=state.getSize()>0&&file.replaceWithData(state.getData(),state.getSize());
        good=good&&written;if(!written)continue;
        juce::MemoryBlock diskState;const bool loaded=file.loadFileAsData(diskState);
        h.p.setCurrentProgram((i+1)%h.p.getNumPrograms());h.flush();
        if(loaded)h.p.setStateInformation(diskState.getData(),int(diskState.getSize()));h.flush();
        good=good&&loaded&&h.p.getCurrentProgram()==i&&h.match(expected);++count;
    }
    h.checks.expect(good&&count==24,"export and disk round-trip all 24 factory .sh09state files");h.restore();
}
}
int main(int argc,char** argv){
    std::cout<<std::unitbuf;std::cerr<<std::unitbuf;
    juce::ScopedJuceInitialiser_GUI init;if(argc<2){std::cerr<<"Usage: SH09Host plugin.vst3 [render-seconds, default 12] [--export-presets absolute-directory]\n";return 2;}
    double seconds=argc>2?juce::jlimit(1.0,600.0,juce::String(argv[2]).getDoubleValue()):12;
    juce::VST3PluginFormat format;juce::OwnedArray<juce::PluginDescription> descriptions;
    format.findAllTypesForFile(descriptions,juce::String::fromUTF8(argv[1]));Checks checks;checks.expect(!descriptions.isEmpty(),"VST3 scan discovers component");
    for(auto* d:descriptions){juce::String error;auto plugin=format.createInstanceFromDescription(*d,48000,256,error);checks.expect(plugin!=nullptr,"instantiate actual VST3: "+error);if(!plugin)continue;
        std::cout<<"Loaded "<<d->name<<" using "<<juce::SystemStats::getJUCEVersion()<<'\n';Host h(*plugin,checks);
        matrix(h);midiMappings(h);timingAndBlocks(h);stateAndPrograms(h);if(h.find("extLevel"))externalTests(h);else std::cout<<"SKIP SH-09-specific external checks for lab component\n";
        stress(h,seconds);editors(h);
        if(argc>4&&juce::String(argv[3])=="--export-presets")exportPresets(h,juce::File(juce::String::fromUTF8(argv[4])));
        h.stop();bool repeatedOK=true;
        for(int i=0;i<8;++i){auto repeated=format.createInstanceFromDescription(*d,48000,64,error);if(!repeated){repeatedOK=false;break;}
            repeated->prepareToPlay(48000,64);juce::AudioBuffer<float> audio(juce::jmax(1,repeated->getTotalNumInputChannels(),repeated->getTotalNumOutputChannels()),64);
            audio.clear();juce::MidiBuffer midi;repeated->processBlock(audio,midi);repeated->releaseResources();
        }
        checks.expect(repeatedOK,"eight additional instance create/process/destroy cycles");
    }
    std::cout<<(checks.failed?"FAIL ":"PASS ")<<checks.failed<<" failures, "<<checks.passed<<" assertions passed\n";return checks.failed?1:0;
}
