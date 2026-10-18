#pragma once
#include <juce_audio_utils/juce_audio_utils.h>
#include "../DSP/Engine.h"
#include <atomic>
#include <vector>
#include <cstring>

// Local diagnostic only. Buffers are allocated before audio starts. The audio
// callback copies into bounded storage; it never allocates, locks or writes files.
class NoteClickCapture {
public:
    struct Event {int sample=0;unsigned char bytes[3]{};int length=0;};
    struct Block {int sample=0,length=0;double milliseconds=0;sh09::Controls controls;};
    NoteClickCapture(double rate,int seconds):sampleRate(rate),audio(size_t(rate*seconds)),events(65536),blocks(65536) {}
    void midi(const juce::uint8* data,int length,int offset) noexcept {
        if(finished.load(std::memory_order_relaxed))return;
        if(!recording&&length>=3&&(data[0]&0xf0)==0x90&&data[2]>0)recording=true;
        if(!recording)return;
        if(eventCount>=events.size()){overflow=true;return;}
        auto& e=events[eventCount++];e.sample=int(samples)+offset;e.length=juce::jmin(3,length);
        for(int i=0;i<e.length;++i)e.bytes[i]=data[i];
    }
    void block(const float* data,int count,double milliseconds,const sh09::Controls& controls) noexcept {
        if(!recording||finished.load(std::memory_order_relaxed))return;
        if(blockCount<blocks.size())blocks[blockCount++]={int(samples),count,milliseconds,controls};else overflow=true;
        const size_t copy=juce::jmin(size_t(juce::jmax(0,count)),audio.size()-samples);
        std::memcpy(audio.data()+samples,data,copy*sizeof(float));samples+=copy;
        if(samples>=audio.size())finished.store(true,std::memory_order_release);
    }
    bool ready()const noexcept{return finished.load(std::memory_order_acquire);}
    bool save(const juce::File& directory)const {
        if(!ready())return false;directory.createDirectory();
        const auto wave=directory.getChildFile("synth-output.wav");
        auto stream=wave.createOutputStream();if(!stream)return false;
        stream->setPosition(0);stream->truncate();
        stream->write("RIFF",4);stream->writeInt(36+int(samples*4));stream->write("WAVEfmt ",8);
        stream->writeInt(16);stream->writeShort(3);stream->writeShort(1);stream->writeInt(int(sampleRate));
        stream->writeInt(int(sampleRate)*4);stream->writeShort(4);stream->writeShort(32);
        stream->write("data",4);stream->writeInt(int(samples*4));stream->write(audio.data(),samples*sizeof(float));stream.reset();
        juce::DynamicObject::Ptr report=new juce::DynamicObject;
        report->setProperty("sampleRate",sampleRate);report->setProperty("samples",int(samples));report->setProperty("overflow",overflow);
        report->setProperty("controlOrder","tune,range,wave,pulseWidth,pwmMode,vcoLevel,subLevel,extLevel,subMode,lfoRate,lfoDelay,lfoWave,vcoMod,cutoff,resonance,vcfMod,vcfEnv,vcfEnvMode,vcaMode,attack,decay,sustain,release,envMode,portamento,bendVco,bendVcf,volume,age,modern,velocity,aftertouch,lfoSync,extGain");
        juce::Array<juce::var> midi;
        for(size_t i=0;i<eventCount;++i){const auto& e=events[i];juce::Array<juce::var> row;row.add(e.sample);for(int j=0;j<e.length;++j)row.add(int(e.bytes[j]));midi.add(juce::var(row));}
        report->setProperty("midi",juce::var(midi));juce::Array<juce::var> timing;
        static_assert(sizeof(sh09::Controls)==34*sizeof(float));
        for(size_t i=0;i<blockCount;++i){const auto& b=blocks[i];juce::Array<juce::var> row;row.add(b.sample);row.add(b.length);row.add(b.milliseconds);
            std::array<float,34> values{};std::memcpy(values.data(),&b.controls,sizeof(b.controls));for(float value:values)row.add(value);timing.add(juce::var(row));}
        report->setProperty("blocks",juce::var(timing));
        return directory.getChildFile("midi-and-timing.json").replaceWithText(juce::JSON::toString(juce::var(report.get()),true));
    }
private:
    double sampleRate;std::vector<float> audio;std::vector<Event> events;std::vector<Block> blocks;
    size_t samples=0,eventCount=0,blockCount=0;bool recording=false,overflow=false;std::atomic<bool> finished{false};
};
