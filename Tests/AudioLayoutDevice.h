#pragma once
#include <juce_audio_utils/juce_audio_utils.h>
// In-memory device: exercise input/output rows without touching PC audio devices.
namespace audio_layout_test {
class Device final : public juce::AudioIODevice {
public:
    Device():AudioIODevice("Layout test device","Layout test driver"){}
    juce::StringArray getOutputChannelNames() override{return {"Left","Right"};}
    juce::StringArray getInputChannelNames() override{return {"Input left","Input right"};}
    juce::Array<double> getAvailableSampleRates() override{return {44100,48000};}
    juce::Array<int> getAvailableBufferSizes() override{return {256,480,512};}
    int getDefaultBufferSize() override{return 480;}
    juce::String open(const juce::BigInteger& in,const juce::BigInteger& out,double rate,int buffer) override{inputs=in;outputs=out;sampleRate=rate;bufferSize=buffer;opened=true;return {};}
    void close() override{opened=false;}
    bool isOpen() override{return opened;}
    void start(juce::AudioIODeviceCallback* c) override{callback=c;if(c)c->audioDeviceAboutToStart(this);}
    void stop() override{if(callback)callback->audioDeviceStopped();callback=nullptr;}
    bool isPlaying() override{return callback!=nullptr;}
    juce::String getLastError() override{return {};}
    int getCurrentBufferSizeSamples() override{return bufferSize;}
    double getCurrentSampleRate() override{return sampleRate;}
    int getCurrentBitDepth() override{return 24;}
    juce::BigInteger getActiveOutputChannels() const override{return outputs;}
    juce::BigInteger getActiveInputChannels() const override{return inputs;}
    int getOutputLatencyInSamples() override{return 0;}
    int getInputLatencyInSamples() override{return 0;}
private:
    bool opened=false;double sampleRate=44100;int bufferSize=480;
    juce::BigInteger inputs,outputs;juce::AudioIODeviceCallback* callback=nullptr;
};
class Type final : public juce::AudioIODeviceType {
public:
    Type():AudioIODeviceType("Layout test driver"){}
    void scanForDevices() override{}
    juce::StringArray getDeviceNames(bool) const override{return {"Layout test device"};}
    int getDefaultDeviceIndex(bool) const override{return 0;}
    int getIndexOfDevice(juce::AudioIODevice*,bool) const override{return 0;}
    bool hasSeparateInputsAndOutputs() const override{return true;}
    juce::AudioIODevice* createDevice(const juce::String&,const juce::String&) override{return new Device;}
};
class Manager final : public juce::AudioDeviceManager {
public:
    void createAudioDeviceTypes(juce::OwnedArray<juce::AudioIODeviceType>& types) override{types.add(new Type);}
};
}
