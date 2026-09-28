#include "Editor.h"
#include "../Plugin/Processor.h"
#include "../../Assets/Authentic/Branding.h"
#include "../../Assets/AlternativeBranding/Branding.h"
#include <cmath>

namespace {
constexpr int designW=1280, designH=660;
const juce::Colour ink(0xffe1e4df), muted(0xffabb3b4), accent(0xff88c4bf);
juce::Font font(float size, bool bold=false) {
    return juce::Font(juce::FontOptions("Arial",size,bold?juce::Font::bold:juce::Font::plain));
}
class HardwareLook final : public juce::LookAndFeel_V4 {
public:
    HardwareLook() {
        setColour(juce::ComboBox::backgroundColourId,juce::Colour(0xff232b30));
        setColour(juce::ComboBox::textColourId,ink);
        setColour(juce::ComboBox::outlineColourId,juce::Colour(0xff667078));
        setColour(juce::ComboBox::arrowColourId,muted);
        setColour(juce::PopupMenu::backgroundColourId,juce::Colour(0xff1d252c));
        setColour(juce::PopupMenu::textColourId,ink);
        setColour(juce::PopupMenu::highlightedBackgroundColourId,juce::Colour(0xff405b5d));
        setColour(juce::Slider::textBoxTextColourId,ink);
        setColour(juce::Slider::textBoxBackgroundColourId,juce::Colours::transparentBlack);
        setColour(juce::Slider::textBoxOutlineColourId,juce::Colours::transparentBlack);
        setColour(juce::Label::textColourId,ink);
        setColour(juce::TooltipWindow::backgroundColourId,juce::Colour(0xffedf0e9));
        setColour(juce::TooltipWindow::textColourId,juce::Colour(0xff1c242a));
    }
    juce::Font getComboBoxFont(juce::ComboBox&) override {return font(11.0f);}
    juce::Font getLabelFont(juce::Label& label) override {return label.getFont();}
    void drawLinearSlider(juce::Graphics& g,int x,int y,int w,int h,float pos,float,float,
                          const juce::Slider::SliderStyle style,juce::Slider& slider) override {
        const bool vertical=style==juce::Slider::LinearVertical;
        const float cx=float(x+w/2), cy=float(y+h/2);
        if(vertical) {
            for(int t=0;t<=20;++t) {
                const float yy=float(y)+float(h)*float(t)/20.0f;
                g.setColour(ink.withAlpha(t%5==0?.85f:.45f));
                const float len=t%5==0?13.0f:9.0f;
                g.drawLine(cx-len-6,yy,cx-7,yy,.85f);g.drawLine(cx+7,yy,cx+len+6,yy,.85f);
            }
            g.setColour(juce::Colour(0xff0d1115));g.fillRoundedRectangle(cx-4,float(y)-3,8,float(h)+6,3);
            g.setColour(juce::Colour(0xff4f5960));g.drawLine(cx+4,float(y),cx+4,float(y+h),1);
            juce::Rectangle<float> cap(cx-15,pos-9,30,18);
            g.setColour(juce::Colours::black.withAlpha(.45f));g.fillRoundedRectangle(cap.translated(1,3),3);
            g.setGradientFill(juce::ColourGradient(juce::Colour(0xff77818a),0,cap.getY(),juce::Colour(0xff20272e),0,cap.getBottom(),false));
            g.fillRoundedRectangle(cap,2);g.setColour(juce::Colour(0xff11171c));g.drawRoundedRectangle(cap,2,1);
            g.setColour(slider.isMouseOverOrDragging()?accent:ink);g.fillRect(cx-11,pos-3,22.0f,1.3f);
        } else {
            g.setColour(juce::Colour(0xff0e1317));g.fillRoundedRectangle(float(x),cy-5,float(w),10,4);
            g.setColour(juce::Colour(0xff606c73));g.drawLine(float(x),cy+5,float(x+w),cy+5,1);
            g.setGradientFill(juce::ColourGradient(juce::Colour(0xffaab4bb),pos-8,cy-11,juce::Colour(0xff293239),pos+8,cy+11,false));
            g.fillRoundedRectangle(pos-10,cy-11,20,22,3);
            g.setColour(ink);g.drawLine(pos,cy-7,pos,cy+7,1);
        }
    }
    void drawRotarySlider(juce::Graphics& g,int x,int y,int w,int h,float value,
                          float start,float end,juce::Slider& slider) override {
        const auto area=juce::Rectangle<float>(float(x),float(y),float(w),float(h)).reduced(5);
        const float r=juce::jmin(area.getWidth(),area.getHeight())*.45f;
        const auto c=area.getCentre();
        for(int t=0;t<=10;++t){const float a=start+(end-start)*float(t)/10;
            auto p=c+juce::Point<float>(std::sin(a),-std::cos(a))*(r+4);
            auto q=c+juce::Point<float>(std::sin(a),-std::cos(a))*(r+8);
            g.setColour(ink.withAlpha(.75f));g.drawLine({p,q},1);}
        auto disc=juce::Rectangle<float>(c.x-r,c.y-r,r*2,r*2);
        g.setColour(juce::Colours::black.withAlpha(.5f));g.fillEllipse(disc.translated(2,4));
        g.setGradientFill(juce::ColourGradient(juce::Colour(0xffe9eded),c.x-r,c.y-r,juce::Colour(0xff737e83),c.x+r,c.y+r,false));g.fillEllipse(disc);
        disc=disc.reduced(r*.2f);
        g.setGradientFill(juce::ColourGradient(juce::Colour(0xff3b444c),c.x,c.y-r,juce::Colour(0xff10171d),c.x,c.y+r,false));g.fillEllipse(disc);
        auto top=disc.reduced(r*.19f);
        g.setGradientFill(juce::ColourGradient(juce::Colour(0xffd8e0e3),top.getX(),top.getY(),juce::Colour(0xff7d8a90),top.getRight(),top.getBottom(),false));g.fillEllipse(top);
        const float a=start+value*(end-start);
        auto p=c+juce::Point<float>(std::sin(a),-std::cos(a))*(r*.67f);
        g.setColour(slider.isMouseOverOrDragging()?accent:juce::Colour(0xff212b31));g.drawLine({c,p},2);
    }
};

bool isBlack(int note){const int n=note%12;return n==1||n==3||n==6||n==8||n==10;}
class Keyboard final : public juce::Component {
public:
    explicit Keyboard(SH09Processor& p):processor(p){setMouseCursor(juce::MouseCursor::PointingHandCursor);setWantsKeyboardFocus(true);setTitle("32-key low-note-priority keyboard");}
    ~Keyboard() override {release();}
    void paint(juce::Graphics& g) override {
        g.fillAll(juce::Colour(0xff10171b));
        const float age=processor.state.getRawParameterValue("age")->load();
        for(int pass=0;pass<2;++pass)for(int n=53;n<=84;++n)if(int(isBlack(n))==pass){
            auto r=keyRect(n);const bool down=processor.keyDisplay[size_t(n)].load(std::memory_order_relaxed)!=0||n==mouseNote;
            if(pass==0){
                const auto ivory=juce::Colour(0xffe7e9e2).interpolatedWith(juce::Colour(0xffc5b996),age*.65f);
                g.setGradientFill(juce::ColourGradient(down?accent.darker(.3f):ivory.darker(.12f),r.getX(),r.getY(),down?accent:ivory.brighter(.15f),r.getX(),r.getBottom(),false));
                g.fillRoundedRectangle(r.reduced(.5f),2);g.setColour(juce::Colour(0xff777b75));g.drawRoundedRectangle(r.reduced(.5f),2,1);
                g.setColour(juce::Colours::black.withAlpha(.14f));g.fillRect(r.withTrimmedTop(r.getHeight()-8));
                if(n%12==0||n==53){g.setColour(juce::Colour(0xff5a6668));g.setFont(font(10));g.drawText(juce::MidiMessage::getMidiNoteName(n,true,true,4),r.withTrimmedTop(r.getHeight()-25).toNearestInt(),juce::Justification::centred);}
            }else{
                g.setColour(juce::Colours::black.withAlpha(.4f));g.fillRoundedRectangle(r.translated(2,4),2);
                g.setGradientFill(juce::ColourGradient(down?accent.darker(.5f):juce::Colour(0xff424d56),r.getX(),r.getY(),down?accent.darker(.8f):juce::Colour(0xff151c23),r.getRight(),r.getBottom(),false));g.fillRoundedRectangle(r,2);
                g.setColour(juce::Colour(0xff77838a).withAlpha(.5f));g.drawLine(r.getX()+2,r.getY()+3,r.getX()+2,r.getBottom()-4,1);
                g.setColour(juce::Colours::black.withAlpha(.45f));g.fillRect(r.withTrimmedTop(r.getHeight()-8));
            }
            if(down&&processor.displayedNote.load(std::memory_order_relaxed)==n){g.setColour(juce::Colour(0xffdab776));g.fillRect(r.getX()+3,r.getBottom()-4,r.getWidth()-6,3.0f);}
        }
        g.setGradientFill(juce::ColourGradient(juce::Colours::black.withAlpha(.45f),0,0,juce::Colours::transparentBlack,0,15,false));g.fillRect(0,0,getWidth(),15);
    }
    void mouseDown(const juce::MouseEvent& e) override {changeNote(noteAt(e.position));}
    void mouseDrag(const juce::MouseEvent& e) override {changeNote(noteAt(e.position));}
    void mouseUp(const juce::MouseEvent&) override {release();}
    void focusLost(FocusChangeType) override {release();}
    void visibilityChanged() override {if(!isShowing())release();}
private:
    SH09Processor& processor;int mouseNote=-1;
    juce::Rectangle<float> keyRect(int note) const {
        int whites=0;for(int n=53;n<note;++n)if(!isBlack(n))++whites;
        const float w=float(getWidth())/19;
        if(isBlack(note))return {float(whites)*w-w*.31f,0,w*.62f,float(getHeight())*.62f};
        return {float(whites)*w,0,w,float(getHeight())};
    }
    int noteAt(juce::Point<float> p) const {for(int pass=1;pass>=0;--pass)for(int n=53;n<=84;++n)if(int(isBlack(n))==pass&&keyRect(n).contains(p))return n;return -1;}
    void changeNote(int n){if(n==mouseNote)return;const int previous=mouseNote;mouseNote=n;if(n>=0)processor.queueKeyboard(n,true);if(previous>=0)processor.queueKeyboard(previous,false);repaint();}
    void release(){if(mouseNote>=0){processor.queueKeyboard(mouseNote,false);mouseNote=-1;repaint();}}
};
}

class SH09Editor::Panel final : public juce::Component,private juce::Timer {
public:
    explicit Panel(SH09Processor& p):processor(p),keyboard(p),tooltip(this,650){
        setLookAndFeel(&look);setSize(designW,designH);
        addAndMakeVisible(keyboard);keyboard.setBounds(191,376,1055,204);
        for(int i=0;i<processor.getNumPrograms();++i)presets.addItem(processor.getProgramName(i),i+1);
        addAndMakeVisible(presets);presets.setBounds(130,17,280,27);presets.setTooltip("Factory patch. Loading a patch replaces the current sound settings.");
        presets.setSelectedId(processor.getCurrentProgram()+1,juce::dontSendNotification);
        presets.onChange=[this]{if(presets.getSelectedId()>0)processor.setCurrentProgram(presets.getSelectedId()-1);};
        addChoice("branding",1001,17,244,27,"Panel identity");
        addChoice("lfoWave",42,110,108,25,"Modulator waveform: sine, square, or random sample-and-hold.");
        addFader("lfoRate","RATE",42,156,51,143);addFader("lfoDelay","DELAY",99,156,51,143);
        addFader("vcoMod","MOD",176,156,50,143);
        addKnob("range","RANGE",237,158,72,99);addKnob("wave","WAVEFORM",322,158,84,99);
        addKnob("tune","TUNE",283,99,50,57);
        addFader("pulseWidth","PULSE WIDTH",416,156,60,143);
        addChoice("pwmMode",362,110,111,25,"Pulse-width source. LFO uses the undelayed sine even when another modulator waveform is selected.");
        addChoice("subMode",493,181,71,27,"Sub oscillator mode; always divided from the main oscillator.");
        addFader("vcoLevel","VCO",580,139,50,160);addFader("subLevel","SUB",630,139,50,160);addFader("extLevel","EXT",680,139,50,160);
        addFader("cutoff","CUTOFF",746,139,55,160);addFader("resonance","RESONANCE",804,139,63,160);
        addChoice("vcfEnvMode",878,110,90,25,"Positive envelope, negative envelope, or external envelope follower.");
        addFader("vcfEnv","ENV",875,156,43,143);addFader("vcfMod","MOD",923,156,43,143);
        addChoice("vcaMode",986,181,74,27,"HOLD passes audio without MIDI. ENV uses ADSR. GATE passes audio while keys are held.");
        addChoice("envMode",1092,110,146,25,"Envelope trigger: Gate+Trigger re-attacks on effective low-note changes; Gate preserves legato; LFO repeats while a key is held.");
        addFader("attack","A",1075,156,40,143);addFader("decay","D",1118,156,40,143);addFader("sustain","S",1161,156,40,143);addFader("release","R",1204,156,40,143);
        addKnob("volume","VOLUME",39,384,62,68);
        addFader("bendVco","VCO",31,457,44,76);addFader("bendVcf","VCF",77,457,44,76);addFader("portamento","GLIDE",123,457,44,76);
        addAndMakeVisible(bender);bender.setBounds(38,544,125,25);bender.setRange(-1,1,.001);bender.setValue(0);bender.setSliderStyle(juce::Slider::LinearHorizontal);bender.setTextBoxStyle(juce::Slider::NoTextBox,false,0,0);bender.setScrollWheelEnabled(false);bender.setWantsKeyboardFocus(false);
        bender.setTooltip("Spring-return pitch/filter bender. Depth is set by the VCO and VCF sliders.");bender.onValueChange=[this]{processor.queuePitchBend(float(bender.getValue()));};bender.onDragEnd=[this]{bender.setValue(0,juce::sendNotificationSync);};
        addChoice("modern",42,613,120,25,"Authentic ignores the optional velocity, aftertouch, tempo sync and input-trim extensions.");
        addHorizontal("age","CONDITION / AGE",189,614,156);
        addHorizontal("velocity","VELOCITY",374,614,135);addHorizontal("aftertouch","AFTERTOUCH",536,614,135);
        addChoice("lfoSync",706,613,96,25,"Modern mode: switch between free LFO and host tempo synchronization.");
        addHorizontal("extGain","INPUT TRIM",837,614,144);
        addAndMakeVisible(help);help.setButtonText("EXT AUDIO");help.setBounds(1075,611,164,27);help.onClick=[this]{externalHelp.setVisible(!externalHelp.isVisible());};
        addChildComponent(externalHelp);externalHelp.setBounds(464,389,738,176);
        externalHelp.setText("EXTERNAL AUDIO + ENVELOPE FOLLOWER\n\nRoute DAW audio to the plug-in's External Input. Raise EXT in the mixer.\nWithout MIDI, set VCA to Hold and lower VCO / SUB.\nSelect Follower in VCF, lower Cutoff and raise ENV for touch-sensitive filtering.\nThe follower listens before the EXT fader. Click EXT AUDIO again to close.",juce::dontSendNotification);
        externalHelp.setFont(font(13));externalHelp.setJustificationType(juce::Justification::topLeft);externalHelp.setBorderSize(juce::BorderSize<int>(16));
        externalHelp.setColour(juce::Label::backgroundColourId,juce::Colour(0xff101b22));externalHelp.setColour(juce::Label::outlineColourId,accent);externalHelp.setInterceptsMouseClicks(true,true);
        startTimerHz(20);
    }
    ~Panel() override {stopTimer();if(std::abs(bender.getValue())>.0001)processor.queuePitchBend(0);setLookAndFeel(nullptr);}
    void paint(juce::Graphics& g) override {
        const float age=processor.state.getRawParameterValue("age")->load();
        g.fillAll(juce::Colour(0xff131b23));
        g.setGradientFill(juce::ColourGradient(juce::Colour(0xff3c454e),0,54,juce::Colour(0xff242d36),0,357,false));g.fillRoundedRectangle(19,62,1242,290,5);
        g.setColour(juce::Colour(0xff56616b));g.drawRoundedRectangle(19,62,1242,290,5,1);
        g.setColour(juce::Colour(0xff222b34));g.fillRoundedRectangle(20,364,153,222,4);
        g.setColour(juce::Colour(0xff859097));g.drawRect(28,373,137,206,1);
        for(int i=0;i<100;++i){const float x=30+float((i*131)%1208),y=68+float((i*67)%271);g.setColour(juce::Colour(0xffa79784).withAlpha(age*.12f));g.drawLine(x,y,x+float(i%19+2),y+float(i%3-1),.7f);}
        for(int x:{29,1250})for(int y:{71,341,586})screw(g,float(x),float(y),age);
        module(g,35,83,123,223,"MODULATOR");module(g,168,83,312,223,"VCO");module(g,490,83,77,223,"SUB OSC");module(g,576,83,158,223,"AUDIO MIXER");module(g,743,83,230,223,"VCF");module(g,982,83,83,223,"VCA");module(g,1074,83,173,223,"ENV");
        g.setColour(ink);g.setFont(font(10));
        g.drawText("1 / 2 OCT",493,220,71,17,juce::Justification::centred);
        g.drawText("KEYBOARD CV",752,287,212,14,juce::Justification::centred);
        const bool alternate=processor.state.getRawParameterValue("branding")->load()>.5f;
        g.setFont(font(27,true));g.drawText(alternate?sh09::alternative_branding::product:sh09::authentic_branding::product,38,314,420,33,juce::Justification::centredLeft);
        g.setFont(font(12,true));g.drawText(alternate?sh09::alternative_branding::model:sh09::authentic_branding::model,390,320,235,23,juce::Justification::centredLeft);
        g.setFont(font(27,true));g.drawText(alternate?sh09::alternative_branding::maker:sh09::authentic_branding::maker,1055,314,185,33,juce::Justification::centredRight);
        g.setColour(ink.withAlpha(.8f));g.fillRect(37,350,1208,3);
        g.setColour(muted);g.setFont(font(11,true));g.drawText("FACTORY PATCH",28,18,98,25,juce::Justification::centredLeft);
        g.setFont(font(11));g.drawText("MONOPHONIC  /  LOW-NOTE PRIORITY",438,18,292,25,juce::Justification::centredLeft);
        g.setColour(juce::Colour(0xff0a1016));g.fillRoundedRectangle(771,15,206,31,4);
        meter(g,782,26,processor.displayedEnvelope.load(),"ENV");meter(g,878,26,processor.displayedFollower.load(),"EXT");
        g.setColour(ink);g.setFont(font(9,true));g.drawText("BENDER",47,571,107,13,juce::Justification::centred);
        g.drawText("POWER",108,428,48,12,juce::Justification::centred);
        g.setColour(juce::Colour(0xff090e11));g.fillEllipse(124,400,13,13);
        g.setColour(juce::Colour(0xffdf5e50));g.fillEllipse(127,403,7,7);
        g.setColour(juce::Colour(0xff66767a));g.drawLine(26,595,1253,595,1);
    }
private:
    using SliderAttachment=juce::AudioProcessorValueTreeState::SliderAttachment;
    using ComboAttachment=juce::AudioProcessorValueTreeState::ComboBoxAttachment;
    struct Control {std::unique_ptr<juce::Slider> slider;std::unique_ptr<juce::ComboBox> combo;std::unique_ptr<juce::Label> label;std::unique_ptr<SliderAttachment> sa;std::unique_ptr<ComboAttachment> ca;};
    SH09Processor& processor;HardwareLook look;Keyboard keyboard;juce::TooltipWindow tooltip;
    juce::ComboBox presets;juce::Slider bender;juce::TextButton help;juce::Label externalHelp;std::vector<std::unique_ptr<Control>> controls;
    void timerCallback() override {
        presets.setSelectedId(processor.getCurrentProgram()+1,juce::dontSendNotification);
        const bool modern=processor.state.getRawParameterValue("modern")->load()>.5f;
        for(auto& c:controls){auto* comp=c->slider?static_cast<juce::Component*>(c->slider.get()):static_cast<juce::Component*>(c->combo.get());const auto id=comp->getComponentID();
            if(id=="velocity"||id=="aftertouch"||id=="lfoSync"||id=="extGain"){comp->setEnabled(modern);comp->setAlpha(modern?1.0f:.4f);}}
        keyboard.repaint();repaint();
    }
    void addChoice(const char* id,int x,int y,int w,int h,const juce::String& tip){
        auto c=std::make_unique<Control>();c->combo=std::make_unique<juce::ComboBox>();auto& b=*c->combo;b.setComponentID(id);b.setTooltip(tip);b.setBounds(x,y,w,h);
        for(const auto& s:sh09::specs)if(juce::String(s.id)==id){b.addItemList(juce::StringArray::fromTokens(s.choices,"|",""),1);b.setTitle(s.name);}
        if(juce::String(id)=="subMode"){b.clear(juce::dontSendNotification);b.addItemList({"-1 SQ","-2 SQ","-2 PLS"},1);}
        addAndMakeVisible(b);c->ca=std::make_unique<ComboAttachment>(processor.state,id,b);controls.push_back(std::move(c));
    }
    void addSlider(const char* id,const char* name,int x,int y,int w,int h,juce::Slider::SliderStyle style,bool textBox){
        auto c=std::make_unique<Control>();c->slider=std::make_unique<juce::Slider>();auto& s=*c->slider;s.setComponentID(id);s.setTitle(name);s.setSliderStyle(style);s.setBounds(x,y+17,w,h-17);s.setMouseDragSensitivity(190);
        s.setTextBoxStyle(textBox?juce::Slider::TextBoxBelow:juce::Slider::NoTextBox,false,w,17);s.setPopupDisplayEnabled(!textBox,!textBox,this);s.setScrollWheelEnabled(false);
        for(const auto& spec:sh09::specs)if(juce::String(spec.id)==id)s.setTooltip(spec.name);
        c->label=std::make_unique<juce::Label>();c->label->setText(name,juce::dontSendNotification);c->label->setBounds(x-6,y,w+12,17);c->label->setJustificationType(juce::Justification::centred);c->label->setFont(font(10,true));c->label->setInterceptsMouseClicks(false,false);
        addAndMakeVisible(s);addAndMakeVisible(*c->label);c->sa=std::make_unique<SliderAttachment>(processor.state,id,s);
        for(const auto& spec:sh09::specs)if(juce::String(spec.id)==id&&spec.choices[0]==0){
            if(juce::String(id)=="tune"){s.textFromValueFunction=[](double v){return juce::String(v,1)+" ct";};s.valueFromTextFunction=[](const juce::String& text){return text.getDoubleValue();};}
            else{s.textFromValueFunction=[](double v){return juce::String(v*10.0,2);};s.valueFromTextFunction=[](const juce::String& text){return text.getDoubleValue()/10.0;};}
        }
        s.updateText();controls.push_back(std::move(c));
    }
    void addFader(const char* id,const char* name,int x,int y,int w,int h){addSlider(id,name,x,y,w,h,juce::Slider::LinearVertical,false);}
    void addKnob(const char* id,const char* name,int x,int y,int w,int h){addSlider(id,name,x,y,w,h,juce::Slider::RotaryHorizontalVerticalDrag,true);}
    void addHorizontal(const char* id,const char* name,int x,int y,int w){addSlider(id,name,x,y-15,w,43,juce::Slider::LinearHorizontal,false);}
    static void module(juce::Graphics& g,int x,int y,int w,int h,const char* title){g.setColour(ink.withAlpha(.72f));g.drawRect(x,y,w,h,1);g.drawLine(float(x+4),float(y+22),float(x+w-4),float(y+22),1);g.setFont(font(12,true));g.drawText(title,x+3,y+2,w-6,19,juce::Justification::centred);}
    static void screw(juce::Graphics& g,float x,float y,float age){g.setColour(juce::Colour(0xff111a21));g.fillEllipse(x-4,y-4,8,8);g.setColour(juce::Colour(0xff758087).interpolatedWith(juce::Colour(0xff705a41),age));g.drawEllipse(x-3,y-3,6,6,1);g.drawLine(x-2,y+1,x+2,y-1,1);}
    static void meter(juce::Graphics& g,int x,int y,float value,const char* name){g.setFont(font(9,true));g.setColour(muted);g.drawText(name,x,y-5,27,15,juce::Justification::centredLeft);for(int i=0;i<7;++i){g.setColour(value>float(i)/7?accent:juce::Colour(0xff344147));g.fillRect(x+29+i*6,y-1,4,8);}}
};

SH09Editor::SH09Editor(SH09Processor& p):AudioProcessorEditor(p),panel(std::make_unique<Panel>(p)){
    setResizable(true,true);setResizeLimits(960,495,1920,990);getConstrainer()->setFixedAspectRatio(double(designW)/designH);
    addAndMakeVisible(*panel);setSize(designW,designH);
}
SH09Editor::~SH09Editor()=default;
void SH09Editor::resized(){if(panel){const float scale=float(getWidth())/designW;panel->setTransform(juce::AffineTransform::scale(scale));panel->setTopLeftPosition(0,0);}}
