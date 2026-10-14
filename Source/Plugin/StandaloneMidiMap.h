#pragma once
#include "../Parameters/Parameters.h"
#include <atomic>
#include <cmath>
#include <functional>

namespace sh09 {
// Fixed-size audio-thread state. Learning, persistence and APVTS notifications
// are delivered by the message-thread pump, never from a MIDI sample callback.
class StandaloneMidiMap final : private juce::AudioProcessorValueTreeState::Listener {
public:
    static constexpr size_t count = specs.size() - 1; // exclude inert legacy slot
    enum class Behavior {direct, toggleButton, momentaryHigh, momentaryLow};
    explicit StandaloneMidiMap(juce::AudioProcessorValueTreeState& tree) : state(tree) {
        for(size_t i=0;i<count;++i) {
            parameters[i]=state.getParameter(specs[i].id);
            state.addParameterListener(specs[i].id,this);
        }
    }
    ~StandaloneMidiMap() override {
        for(size_t i=0;i<count;++i)state.removeParameterListener(specs[i].id,this);
    }
    static int indexFor(const juce::String& id) {
        for(size_t i=0;i<count;++i)if(id==specs[i].id)return int(i);
        return -1;
    }
    int assignment(int index) const noexcept {return valid(index)?slots[size_t(index)].source.load():-1;}
    unsigned getRevision() const noexcept {return mapRevision.load();}
    bool isLearning() const noexcept {return learning.load()!=0;}
    std::uint64_t learnProgress() const noexcept {return learned.load();}
    bool isCheckingButton() const noexcept {
        const auto capture=learned.load();
        return capture!=0&&cycleCandidate(int((capture>>tokenShift)&255)-1)&&!(capture&middleFlag);
    }
    juce::String pendingDescription() const {
        const auto capture=learned.load();
        if(!capture||unsigned(capture>>tokenShift)!=learning.load())return {};
        const int source=int(capture&sourceMask);
        return "CC "+juce::String(source%128)+" / channel "+juce::String(source/128+1);
    }
    void learn(int index) {cancelLearn();if(valid(index))learning.store((++learnGeneration<<8)|unsigned(index+1));}
    void cancelLearn() {learning.store(0);learned.store(0);}
    void assign(int index,int channel,int cc,Behavior behavior=Behavior::direct,int initialButtonValue=-1) {
        if(!valid(index)||channel<1||channel>16||cc<0||cc>=120)return;
        const int source=(channel-1)*128+cc;
        // A physical control has one destination. Reassignment is explicit.
        for(size_t i=0;i<count;++i)if(int(i)==index||slots[i].source.load()==source)clearSlot(i);
        auto& slot=slots[size_t(index)];slot.behavior.store(int(cycleCandidate(index)?behavior:Behavior::direct));
        slot.initialButtonValue.store(initialButtonValue);slot.source.store(source);
        changed();
    }
    Behavior behavior(int index) const noexcept {return valid(index)?Behavior(slots[size_t(index)].behavior.load()):Behavior::direct;}
    void setBehavior(int index,Behavior next) {
        if(!valid(index)||assignment(index)<0)return;
        const int source=assignment(index);assign(index,source/128+1,source%128,next);
    }
    static bool supportsButtons(int index) noexcept {return cycleCandidate(index);}
    static juce::String behaviorName(Behavior mode) {
        switch(mode) {
            case Behavior::toggleButton:return "Button: cycle each press";
            case Behavior::momentaryHigh:return "Button: cycle on press, ignore release";
            case Behavior::momentaryLow:return "Button: cycle on press, ignore release (inverted)";
            default:return "Knob/fader: direct position";
        }
    }
    // DONE supplies the release boundary that plain MIDI cannot communicate.
    // During this brief check, an intermediate CC proves direct positioning;
    // a release message identifies a momentary button, otherwise it is toggle.
    void finishLearn() {
        const auto token=learning.exchange(0);const auto capture=learned.exchange(0);
        if(!token||unsigned(capture>>tokenShift)!=token)return;
        const int index=int(token&255)-1,source=int(capture&sourceMask);
        const int first=int((capture>>firstShift)&127),last=int((capture>>lastShift)&127);
        auto mode=Behavior::direct;
        if(cycleCandidate(index)&&!(capture&middleFlag))
            mode=(capture&releaseFlag)?(first==127?Behavior::momentaryHigh:Behavior::momentaryLow):Behavior::toggleButton;
        assign(index,source/128+1,source%128,mode,last);
    }
    void clear(int index) {cancelLearn();if(valid(index)){clearSlot(size_t(index));changed();}}
    void clearAll() {cancelLearn();for(size_t i=0;i<count;++i)clearSlot(i);changed();}
    juce::String describe(int index) const {
        const int source=assignment(index);
        return source<0?"Unassigned":"CC "+juce::String(source%128)+" / channel "+juce::String(source/128+1);
    }
    // The active value is discarded immediately by any real parameter edit.
    float value(size_t index,float fallback) const noexcept {
        if(index>=count)return fallback;
        const auto& s=slots[index];
        const auto serial=s.serial.load(std::memory_order_acquire);
        if((serial&1u)||s.source.load()<0||!s.active.load(std::memory_order_acquire))return fallback;
        const auto revision=s.feedbackRevision.load();
        const float result=s.actual.load();
        return serial==s.serial.load(std::memory_order_acquire)&&revision==s.revision.load()?result:fallback;
    }
    // Returns true when this CC belongs to the map (including pickup), or was
    // consumed for learning. Channel-mode CC120-127 are never intercepted.
    bool controller(int channel,int cc,int amount) noexcept {
        if(cc<0||cc>=120)return false;
        const int source=(channel-1)*128+cc;
        const auto token=learning.load(std::memory_order_acquire);
        if(token!=0) {
            // Keep learn armed until pump() commits it, consuming subsequent
            // events so CC1 cannot also change its built-in destination.
            const auto v=unsigned(juce::jlimit(0,127,amount));
            auto capture=learned.load(std::memory_order_acquire);
            if(capture==0) {
                const auto next=(std::uint64_t(token)<<tokenShift)|unsigned(source)
                    |(std::uint64_t(v)<<firstShift)|(std::uint64_t(v)<<lastShift)
                    |(v!=0&&v!=127?middleFlag:0);
                learned.compare_exchange_strong(capture,next);
            } else if(unsigned(capture>>tokenShift)==token&&int(capture&sourceMask)==source) {
                const auto first=unsigned((capture>>firstShift)&127);
                auto next=(capture&~(std::uint64_t(127)<<lastShift))|(std::uint64_t(v)<<lastShift);
                if(v!=0&&v!=127)next|=middleFlag;
                else if(v!=first)next|=releaseFlag;
                learned.compare_exchange_strong(capture,next);
            }
            return true;
        }
        for(size_t i=0;i<count;++i) {
            auto& s=slots[i];
            if(s.source.load(std::memory_order_acquire)!=source)continue;
            const auto revision=s.revision.load(std::memory_order_acquire);
            if(s.seenRevision!=revision){s.seenRevision=revision;s.pickedUp=false;s.previous=-1;}
            const auto binding=s.bindingRevision.load();
            if(s.seenBinding!=binding){s.seenBinding=binding;s.lastButtonValue=s.initialButtonValue.load();}
            const float position=float(juce::jlimit(0,127,amount))/127;
            const float current=parameters[i]->getValue();
            const auto mode=Behavior(s.behavior.load());
            if(mode!=Behavior::direct) {
                const int v=juce::jlimit(0,127,amount);
                if(v==s.lastButtonValue)return true;
                s.lastButtonValue=v;
                if((mode==Behavior::momentaryHigh&&v<64)||(mode==Behavior::momentaryLow&&v>=64))return true;
                const int positions=positionCount(int(i));
                // Use the pending audio value so several presses in one block
                // advance several positions before the GUI publishes anything.
                const float normalized=parameters[i]->convertTo0to1(value(i,parameters[i]->convertFrom0to1(current)));
                const int next=(juce::roundToInt(normalized*(positions-1))+1)%positions;
                const float actual=parameters[i]->convertFrom0to1(float(next)/float(positions-1));
                writeValue(s,source,revision,actual);return true;
            }
            const bool discrete=specs[i].choices[0]!=0||i==size_t(indexAge);
            if(discrete||std::abs(position-current)<=1.f/127
                ||(s.previous>=0&&(s.previous-current)*(position-current)<=0))s.pickedUp=true;
            s.previous=position;
            if(!s.pickedUp)return true;
            float actual=parameters[i]->convertFrom0to1(position);
            if(i==size_t(indexAge))actual=std::round(actual*4)/4;
            writeValue(s,source,revision,actual);
            return true;
        }
        return false;
    }
    void resetPickup() noexcept {
        for(auto& s:slots)s.revision.fetch_add(1,std::memory_order_release);
    }
    void pump() {
        const auto capture=learned.load();
        if(capture) {
            const auto token=unsigned(capture>>tokenShift);
            if(token!=learning.load()) {auto stale=capture;learned.compare_exchange_strong(stale,0);}
            else if(!cycleCandidate(int(token&255)-1)||(capture&middleFlag))finishLearn();
        }
        for(size_t i=0;i<count;++i)publish(i);
    }
    juce::String save() const {
        juce::XmlElement root("KB09MidiMap");root.setAttribute("version",1);
        for(size_t i=0;i<count;++i)if(const auto source=slots[i].source.load();source>=0) {
            auto* item=root.createNewChildElement("CC");item->setAttribute("parameter",specs[i].id);
            item->setAttribute("channel",source/128+1);item->setAttribute("number",source%128);
            item->setAttribute("behavior",slots[i].behavior.load());
        }
        return root.toString();
    }
    void restore(const juce::String& text) {
        clearAll();auto root=juce::parseXML(text);
        if(!root||!root->hasTagName("KB09MidiMap")||root->getIntAttribute("version")!=1)return;
        for(auto* item:root->getChildIterator())if(item->hasTagName("CC")) {
            const int index=indexFor(item->getStringAttribute("parameter"));
            const auto mode=Behavior(juce::jlimit(0,3,item->getIntAttribute("behavior",0)));
            assign(index,item->getIntAttribute("channel"),item->getIntAttribute("number",-1),
                cycleCandidate(index)?mode:Behavior::direct);
        }
    }
    std::function<void()> onChanged;
private:
    static constexpr int indexAge=28;
    static constexpr int firstShift=11,lastShift=18,tokenShift=28;
    static constexpr std::uint64_t sourceMask=2047,middleFlag=std::uint64_t(1)<<25,releaseFlag=std::uint64_t(1)<<26;
    static bool valid(int i) noexcept {return i>=0&&i<int(count);}
    static int positionCount(int i) noexcept {
        if(i==indexAge)return 5;
        if(!valid(i)||specs[size_t(i)].choices[0]==0)return 0;
        return int(specs[size_t(i)].max-specs[size_t(i)].min)+1;
    }
    static bool cycleCandidate(int i) noexcept {return positionCount(i)>2;}
    struct Slot {
        std::atomic<int> source{-1};
        std::atomic<unsigned> revision{1},serial{0},feedbackRevision{0};
        std::atomic<float> actual{0};std::atomic<bool> active{false};
        std::atomic<int> behavior{int(Behavior::direct)},initialButtonValue{-1};
        std::atomic<unsigned> bindingRevision{0};unsigned seenBinding=0;int lastButtonValue=-1;
        unsigned seenRevision=0,delivered=0;
        float previous=-1;bool pickedUp=false;
    };
    static void writeValue(Slot& s,int source,unsigned revision,float actual) noexcept {
        if(s.source.load()!=source||s.revision.load()!=revision)return;
        s.serial.fetch_add(1,std::memory_order_acq_rel);
        s.actual.store(actual);s.feedbackRevision.store(revision);
        s.active.store(true,std::memory_order_release);s.serial.fetch_add(1,std::memory_order_release);
    }
    void publish(size_t i) {
        auto& s=slots[i];const auto serial=s.serial.load(std::memory_order_acquire);
        if((serial&1u)||serial==s.delivered)return;
        const auto revision=s.feedbackRevision.load();const float actual=s.actual.load();
        if(serial!=s.serial.load(std::memory_order_acquire))return;
        s.delivered=serial;
        if(!s.active.load()||revision!=s.revision.load())return;
        auto* p=parameters[i];p->beginChangeGesture();
        if(revision==s.revision.load()) {
            const juce::ScopedValueSetter<const StandaloneMidiMap*> writer(feedbackWriter,this);
            const juce::ScopedValueSetter<int> index(feedbackIndex,int(i));
            const juce::ScopedValueSetter<float> expected(feedbackValue,actual);
            p->setValueNotifyingHost(p->convertTo0to1(actual));
        }
        p->endChangeGesture();
    }
    void clearSlot(size_t i) {publish(i);auto& s=slots[i];s.source.store(-1);s.revision.fetch_add(1);s.bindingRevision.fetch_add(1);s.active.store(false);}
    void changed() {mapRevision.fetch_add(1);if(onChanged)onChanged();}
    void parameterChanged(const juce::String& id,float v) override {
        const int i=indexFor(id);
        if(i<0)return;
        if(feedbackWriter==this&&feedbackIndex==i&&std::abs(v-feedbackValue)<1e-6f)return;
        slots[size_t(i)].revision.fetch_add(1,std::memory_order_release);
    }
    juce::AudioProcessorValueTreeState& state;
    std::array<juce::RangedAudioParameter*,count> parameters{};
    std::array<Slot,count> slots;
    unsigned learnGeneration=0;
    std::atomic<unsigned> learning{0},mapRevision{0};std::atomic<std::uint64_t> learned{0};
    inline static thread_local const StandaloneMidiMap* feedbackWriter=nullptr;
    inline static thread_local int feedbackIndex=-1;
    inline static thread_local float feedbackValue=0;
};
}
