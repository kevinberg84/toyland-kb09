#include "../Source/Plugin/Processor.h"
#include "../Source/Parameters/Parameters.h"
#include "../Source/Presets/Factory.h"
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <set>

// Render the actual processor/program path, including its parameter defaults,
// MIDI routing and output gain. Peak checks precede writing; no normalization.
namespace {
void u16(std::ostream& out, unsigned v) { out.put(char(v)); out.put(char(v >> 8)); }
void u32(std::ostream& out, unsigned v) { u16(out, v); u16(out, v >> 16); }
bool wav(const std::filesystem::path& path, const std::vector<float>& audio, int rate) {
    std::ofstream out(path, std::ios::binary);
    out.write("RIFF",4); u32(out,36+unsigned(audio.size()*4)); out.write("WAVEfmt ",8);
    u32(out,16); u16(out,3); u16(out,1); u32(out,unsigned(rate)); u32(out,unsigned(rate*4));
    u16(out,4); u16(out,32); out.write("data",4); u32(out,unsigned(audio.size()*4));
    out.write(reinterpret_cast<const char*>(audio.data()),std::streamsize(audio.size()*4));
    return bool(out);
}
}
int main(int argc, char** argv) {
    juce::ScopedJuceInitialiser_GUI init;
    const std::filesystem::path output=argc>1?argv[1]:"factory-auditions";
    std::filesystem::create_directories(output);
    std::ofstream csv(output/"levels.csv");
    csv << "program,name,sample_rate,age,peak,rms,finite\n";
    int failed=0, rendered=0;
    const auto& bank=sh09::factoryPresets();
    if(bank.size()!=24) ++failed;
    for(const auto& patch:bank) {
        std::set<std::string> ids;
        for(const auto& [id,value]:patch.values) {
            const auto spec=std::find_if(sh09::specs.begin(),sh09::specs.end(),
                [&](const auto& p){return std::string_view(p.id)==id;});
            if(spec==sh09::specs.end() || !ids.insert(id).second || !std::isfinite(value)
               || value<spec->min || value>spec->max
               || (spec->choices[0] && value!=std::round(value))) ++failed;
        }
    }
    for(const int rate:{44100,48000,96000}) for(int program=0;program<int(bank.size());++program) {
        // Full listening bank at 48 kHz; new patches also cross-checked at 44.1/96 kHz.
        if(rate!=48000 && program<16) continue;
        SH09Processor processor; processor.setCurrentProgram(program);
        if(rate==48000) {
            const auto folder=output/"Factory Presets"; std::filesystem::create_directories(folder);
            const auto name=juce::File::createLegalFileName(processor.getProgramName(program).replace(" / "," - ")).toStdString();
            juce::MemoryBlock state; processor.getStateInformation(state);
            std::ofstream file(folder/(name+".sh09state"),std::ios::binary);
            file.write(static_cast<const char*>(state.getData()),std::streamsize(state.getSize()));
            if(!file) ++failed;
        }
        const int block=rate==44100?32:rate==48000?127:512;
        processor.prepareToPlay(rate,block);
        juce::AudioBuffer<float> audio(2,block); juce::MidiBuffer midi;
        std::vector<float> capture; capture.reserve(size_t(rate*8));
        double energy=0,peak=0; bool finite=true;
        const bool bass=(program==1 || program==6 || program==13 || (program>=16 && program<=19));
        const int root=bass?48:60;
        const int notes[]{root,root,root+7,root+3,root,root+12,root+7,root};
        for(int position=0;position<rate*8;position+=block) {
            const int count=std::min(block,rate*8-position); audio.setSize(2,count,false,false,true);
            audio.clear(); midi.clear();
            for(int sample=0;sample<count;++sample) {
                const int absolute=position+sample, beat=absolute/rate, offset=absolute%rate;
                if(offset==0) midi.addEvent(juce::MidiMessage::noteOn(1,notes[beat],juce::uint8(beat%2?92:120)),sample);
                if(offset==rate*3/4) midi.addEvent(juce::MidiMessage::noteOff(1,notes[beat]),sample);
                if(program==23 && offset==rate/2)
                    midi.addEvent(juce::MidiMessage::channelPressureChange(1,95),sample);
                if(program==11 || program==12) {
                    const float envelope=offset<rate/2?1.0f:.1f;
                    const float input=.25f*envelope*std::sin(float(2*juce::MathConstants<double>::pi*110*absolute/rate));
                    audio.setSample(0,sample,input); audio.setSample(1,sample,input);
                }
            }
            processor.processBlock(audio,midi);
            for(int sample=0;sample<count;++sample) {
                const float value=audio.getSample(0,sample); finite &= std::isfinite(value);
                peak=std::max(peak,double(std::abs(value))); energy+=double(value)*value;
                capture.push_back(value);
            }
        }
        const double rms=std::sqrt(energy/capture.size());
        const float age=processor.state.getRawParameterValue("age")->load()*10;
        csv << program+1 << ",\"" << bank[size_t(program)].name << "\"," << rate << ',' << age
            << ',' << std::setprecision(9) << peak << ',' << rms << ',' << finite << '\n';
        const bool passed=finite && peak<.98 && rms>1e-5;
        if(!passed) ++failed;
        if(rate==48000) {
            auto name=juce::File::createLegalFileName(bank[size_t(program)].name).replace(" / "," - ").toStdString();
            if(!wav(output/(name+".wav"),capture,rate)) ++failed;
        }
        std::cout << (passed?"PASS ":"FAIL ") << bank[size_t(program)].name << " rate=" << rate
                  << " AGE=" << age << " peak=" << peak << " rms=" << rms << '\n';
        ++rendered; processor.releaseResources();
    }
    std::cout << rendered << " factory renders; " << failed << " failures\n";
    return failed?1:0;
}
