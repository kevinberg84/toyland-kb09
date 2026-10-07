#include "Editor.h"
#include "PresetFiles.h"
#include "../Plugin/Processor.h"
#include "BinaryData.h"
#include <cmath>
#include <vector>
#include <array>

namespace {
constexpr int designW=1500, bodyH=750, headerH=34, designH=834;
const juce::Colour ink(0xffe9e7dd), muted(0xffb4b7b5), amber(0xffdbb572);
juce::Colour agedInk(float age) {
    return ink.interpolatedWith(juce::Colour(0xffd1bd86),juce::jlimit(0.0f,1.0f,age)*.86f);
}
juce::Font font(float size,bool bold=false) {
    return juce::Font(juce::FontOptions("Arial",size,bold?juce::Font::bold:juce::Font::plain));
}
void caption(juce::Graphics& g,const juce::String& text,juce::Rectangle<float> box,
             float size=10.0f,juce::Justification align=juce::Justification::centred,bool bold=true,
             juce::Colour colour=ink) {
    g.setColour(colour);g.setFont(font(size,bold));
    g.drawFittedText(text,box.toNearestInt(),align,text.containsChar('\n')?2:1,.8f);
}
struct Artwork {
    static juce::Image load(const void* data,int size) {
        // Keep persistent artwork in CPU memory. Native Windows images otherwise
        // retain Direct2D resources across editor destruction and DLL unloading.
        return juce::SoftwareImageType().convert(juce::ImageFileFormat::loadFrom(data,size));
    }
    juce::Image chassis=load(BinaryData::chassisblack_png,BinaryData::chassisblack_pngSize);
    juce::Image chassisAged=load(BinaryData::chassispatina_png,BinaryData::chassispatina_pngSize);
    juce::Image stains=load(BinaryData::chassisstains_png,BinaryData::chassisstains_pngSize);
    juce::Image knob=load(BinaryData::knob_png,BinaryData::knob_pngSize);
    juce::Image fader=load(BinaryData::fader_png,BinaryData::fader_pngSize);
    juce::Image keys=load(BinaryData::keys_png,BinaryData::keys_pngSize);
    juce::Image wear=load(BinaryData::wearatlas_png,BinaryData::wearatlas_pngSize);
    juce::Image whiteKey=keys.getClippedImage({296,60,218,1130});
    juce::Image blackKey=keys.getClippedImage({763,68,159,695});
    juce::Image wearCell(int index) const {
        const int w=wear.getWidth()/8,h=wear.getHeight()/6;
        // Returned photographic grid begins 36 px below the canvas top.
        // Two-pixel side gutters exclude isolated neighboring alpha flecks.
        return wear.getClippedImage({(index%8)*w+2,(index/8)*h+36,w-4,h});
    }
    juce::Image marks,marksAged;
    Artwork() {
        auto original=load(BinaryData::marks_png,BinaryData::marks_pngSize);
        auto worn=load(BinaryData::marksaged_png,BinaryData::marksaged_pngSize);
        marks=juce::Image(juce::Image::ARGB,original.getWidth(),original.getHeight(),true,juce::SoftwareImageType());
        marksAged=juce::Image(juce::Image::ARGB,original.getWidth(),original.getHeight(),true,juce::SoftwareImageType());
        juce::Image::BitmapData input(original,juce::Image::BitmapData::readOnly);
        juce::Image::BitmapData output(marks,juce::Image::BitmapData::writeOnly);
        juce::Image::BitmapData agedInput(worn,juce::Image::BitmapData::readOnly);
        juce::Image::BitmapData agedOutput(marksAged,juce::Image::BitmapData::writeOnly);
        // Preserve the supplied mark shapes; their white coverage becomes screen-print opacity.
        for(int y=0;y<original.getHeight();++y)for(int x=0;x<original.getWidth();++x) {
            const auto c=input.getPixelColour(x,y);
            const float light=(c.getFloatRed()+c.getFloatGreen()+c.getFloatBlue())/3.0f;
            const float coverage=juce::jlimit(0.0f,1.0f,(light-.065f)/.89f);
            output.setPixelColour(x,y,ink.withAlpha(coverage));
            const auto aged=agedInput.getPixelColour(x*worn.getWidth()/original.getWidth(),y*worn.getHeight()/original.getHeight());
            const float wornLight=(aged.getFloatRed()+aged.getFloatGreen()+aged.getFloatBlue())/3.0f;
            // The generated wear can only remove original ink, never change
            // the supplied letterforms or introduce coverage outside them.
            agedOutput.setPixelColour(x,y,agedInk(1).withAlpha(coverage*juce::jlimit(0.0f,1.0f,(wornLight-.07f)/.83f)));
        }
    }
};
const Artwork& artwork() {static const Artwork a;return a;}
const std::array<juce::Rectangle<int>,6> stainRegions{{
    {240,120,160,220},{710,125,200,235},{1330,240,90,185},
    {1420,0,300,320},{75,480,270,310},{990,305,260,155}}};
void caseStains(juce::Graphics& g,float age) {
    constexpr std::array<float,6> onsets{{.25f,.38f,.49f,.60f,.72f,.86f}};
    juce::Graphics::ScopedSaveState save(g);
    juce::Path metal;metal.addRectangle(55.0f,float(headerH),1389.0f,365.0f);
    metal.addRectangle(65.0f,float(headerH+366),182.0f,335.0f);g.reduceClipRegion(metal);
    g.setOpacity(.22f);
    for(size_t i=0;i<onsets.size();++i)if(age>=onsets[i]) {
        const auto box=stainRegions[i];
        const auto target=juce::Rectangle<float>((box.getX()-2)*1500.0f/1770,
            headerH+(box.getY()-7)*750.0f/873,box.getWidth()*1500.0f/1770,box.getHeight()*750.0f/873);
        g.drawImage(artwork().stains,target.getX(),target.getY(),target.getWidth(),target.getHeight(),
                    box.getX(),box.getY(),box.getWidth(),box.getHeight());
    }
}

void cap(juce::Graphics& g,juce::Rectangle<float> r,bool small=false,bool indexed=true,float age=0) {
    const auto ink=agedInk(age);
    if(!small) {
        juce::Graphics::ScopedSaveState save(g);
        g.setOpacity(1.0f);
        g.setImageResamplingQuality(juce::Graphics::highResamplingQuality);
        const auto photo=r.expanded(2,1);
        // The photograph retains a soft transparent contact shadow, but the
        // plastic body is solid. Its source alpha is 252-254, so an opaque
        // registered backing also prevents the slot showing through it.
        const auto point=[&](float x,float y){return juce::Point<float>(photo.getX()+x*photo.getWidth()/1536,
                                                                      photo.getY()+y*photo.getHeight()/1024);};
        juce::Path solid;solid.startNewSubPath(point(174,270));solid.lineTo(point(1360,270));
        solid.lineTo(point(1394,780));solid.lineTo(point(141,780));solid.closeSubPath();
        g.setColour(juce::Colour(0xff171719));g.fillPath(solid);
        g.setOpacity(1.0f);g.drawImage(artwork().fader,photo,juce::RectanglePlacement::stretchToFit);
        if(age>0) {
            // Gradual yellowing of the physical white index, not a dirt stamp.
            g.setColour(juce::Colour(0xffaa8b4f).withAlpha(age*.23f));
            g.fillRect(r.getX()+3,r.getY()+r.getHeight()*.39f,r.getWidth()-6,1.45f);
        }
        return;
    }
    for(int i=5;i>0;--i) {
        g.setColour(juce::Colours::black.withAlpha(.035f*float(6-i)));
        g.fillRoundedRectangle(r.expanded(float(i)*.65f).translated(1.3f,2.5f),1.7f);
    }
    juce::Path p;
    p.startNewSubPath(r.getX()+1.5f,r.getY());
    p.lineTo(r.getRight()-1.5f,r.getY());p.lineTo(r.getRight(),r.getBottom()-1);
    p.quadraticTo(r.getRight(),r.getBottom(),r.getRight()-1,r.getBottom());
    p.lineTo(r.getX()+1,r.getBottom());p.quadraticTo(r.getX(),r.getBottom(),r.getX(),r.getBottom()-1);
    p.closeSubPath();
    juce::ColourGradient plastic(juce::Colour(0xff555653),r.getX(),r.getY(),
                                juce::Colour(0xff0b0c0c),r.getX()+r.getWidth()*.15f,r.getBottom(),false);
    plastic.addColour(.13,juce::Colour(0xff252728));plastic.addColour(.58,juce::Colour(0xff303233));
    plastic.addColour(.67,juce::Colour(0xff191b1d));g.setGradientFill(plastic);g.fillPath(p);
    g.setColour(juce::Colour(0xff090b0c));g.strokePath(p,juce::PathStrokeType(.8f));
    g.setColour(juce::Colours::white.withAlpha(.20f));
    g.drawLine(r.getX()+2,r.getY()+1,r.getRight()-2,r.getY()+1,.8f);
    g.drawLine(r.getX()+1,r.getY()+2,r.getX()+1,r.getBottom()-3,.5f);
    for(int i=0;i<42;++i) {
        const float xx=r.getX()+2+float((i*13)%41)/41*(r.getWidth()-4);
        const float yy=r.getY()+3+float((i*19)%43)/43*(r.getHeight()-6);
        g.setColour(juce::Colours::white.withAlpha(i%3==0?.085f:.035f));g.fillRect(xx,yy,.42f,.33f);
    }
    g.setColour(ink.darker(.08f));
    if(small && indexed)g.fillRect(r.getX()+2,r.getCentreY()-.65f,r.getWidth()-4,1.3f);
    else if(small) {
        for(int i=0;i<4;++i) {
            const float yy=r.getY()+4+float(i)*2.6f;
            g.setColour(juce::Colour(0xff090b0c));g.drawLine(r.getX()+2,yy,r.getRight()-2,yy,.8f);
            g.setColour(juce::Colours::white.withAlpha(.16f));g.drawLine(r.getX()+2,yy+.7f,r.getRight()-2,yy+.7f,.45f);
        }
    }
    else {
        g.fillRect(r.getX()+3,r.getY()+r.getHeight()*.37f,r.getWidth()-6,1.55f);
        g.setColour(juce::Colours::white.withAlpha(.25f));
        g.drawLine(r.getX()+3,r.getY()+r.getHeight()*.37f,r.getRight()-3,r.getY()+r.getHeight()*.37f,.35f);
    }
}
class HardwareLook final : public juce::LookAndFeel_V4 {
public:
    float age=0;
    HardwareLook() {
        setColour(juce::ComboBox::backgroundColourId,juce::Colour(0xff202326));
        setColour(juce::ComboBox::textColourId,ink);
        setColour(juce::ComboBox::outlineColourId,juce::Colour(0xff454a4c));
        setColour(juce::ComboBox::arrowColourId,muted);
        setColour(juce::PopupMenu::backgroundColourId,juce::Colour(0xff202326));
        setColour(juce::PopupMenu::textColourId,ink);
        setColour(juce::PopupMenu::highlightedBackgroundColourId,juce::Colour(0xff514a3c));
        setColour(juce::Label::textColourId,ink);
        setColour(juce::TextButton::buttonColourId,juce::Colour(0xff242729));
        setColour(juce::TextButton::textColourOffId,ink);
        setColour(juce::TooltipWindow::backgroundColourId,juce::Colour(0xffeeeae0));
        setColour(juce::TooltipWindow::textColourId,juce::Colour(0xff22262a));
        setColour(juce::BubbleComponent::backgroundColourId,juce::Colour(0xffeeeae0));
        setColour(juce::BubbleComponent::outlineColourId,juce::Colour(0xff756850));
    }
    juce::Font getComboBoxFont(juce::ComboBox&) override {return font(11);}
    juce::Font getTextButtonFont(juce::TextButton&,int) override {return font(11,true);}
    juce::Font getLabelFont(juce::Label& l) override {return l.getFont();}
    void drawLinearSlider(juce::Graphics& g,int x,int y,int w,int h,float pos,float,float,
                          const juce::Slider::SliderStyle style,juce::Slider& slider) override {
        const auto ink=agedInk(age);
        const float cx=float(x)+float(w)*.5f,cy=float(y)+float(h)*.5f;
        if(style==juce::Slider::LinearVertical) {
            const bool compact=bool(slider.getProperties().getWithDefault("compact",false));
            for(int t=0;t<=20;++t) {
                const float yy=float(y)+float(h)*float(t)/20;
                const float len=t%5==0?10.0f:7.2f;
                g.setColour(ink.withAlpha(t%5==0?.86f:.62f));
                const float arm=compact?5.5f:len;
                g.drawLine(cx-arm-5,yy,cx-5,yy,.65f);g.drawLine(cx+5,yy,cx+arm+5,yy,.65f);
            }
            for(int i=0;i<3;++i) {
                const float yy=float(y)+float(h)*float(i)*.5f;
                const bool scaleLeft=bool(slider.getProperties().getWithDefault("scaleLeft",false));
                caption(g,juce::String(10-i*5),{cx+(scaleLeft?-26.0f:14.5f),yy-4,11,8},6.6f,
                        scaleLeft?juce::Justification::centredRight:juce::Justification::centredLeft,false,ink);
            }
            g.setColour(juce::Colour(0xff62696a));g.fillRoundedRectangle(cx-4,float(y)-2,8,float(h)+4,2);
            g.setColour(juce::Colour(0xff06090b));g.fillRoundedRectangle(cx-3.2f,float(y)-1,6.4f,float(h)+2,1.5f);
            g.setColour(juce::Colour(0xff000204));g.drawLine(cx-2.5f,float(y),cx-2.5f,float(y+h),1);
            g.setColour(juce::Colours::white.withAlpha(.13f));g.drawLine(cx+3.6f,float(y),cx+3.6f,float(y+h),.55f);
            // Keep caps fitted until the main-panel LFE9R-C16 actuator's
            // exposed assembled geometry is verified from original hardware.
            cap(g,{cx-(compact?12.5f:15),pos-10,compact?25.0f:30.0f,23},false,true,age);
        } else {
            if(slider.getComponentID()=="bender") {
                const float amount=float(slider.getValue());
                const float centre=float(slider.getWidth())*.5f;
                auto well=juce::Rectangle<float>(8,13,float(slider.getWidth())-16,22);
                g.setColour(juce::Colour(0xff596064));g.fillRect(well.expanded(1));
                g.setGradientFill(juce::ColourGradient(juce::Colour(0xff030303),well.getX(),well.getY(),
                                                     juce::Colour(0xff202020),well.getX(),well.getBottom(),false));
                g.fillRect(well);g.setColour(juce::Colours::black);g.drawRect(well,.9f);
                // PB-4 is a wheel/sector with an axle beneath the opening.
                // Project rotation in its vertical plane through the top view.
                const float angle=amount*.610865f;
                const auto project=[centre,angle](float radius) {
                    return juce::Point<float>(centre+radius*std::sin(angle),56-.5f*radius*std::cos(angle));
                };
                const auto base=project(58),tip=project(95);
                const auto direction=(tip-base)/(tip-base).getDistanceFromOrigin();
                const juce::Point<float> side(-direction.y,direction.x);
                {
                    juce::Graphics::ScopedSaveState save(g);g.reduceClipRegion(well.toNearestInt());
                    auto drum=juce::Rectangle<float>(centre-58,27,116,58);
                    g.setGradientFill(juce::ColourGradient(juce::Colour(0xff424242),centre-34,27,
                                                         juce::Colour(0xff070707),centre+35,49,false));
                    g.fillEllipse(drum);g.setColour(juce::Colours::white.withAlpha(.16f*(1-age*.6f)));
                    g.drawEllipse(drum.reduced(.8f),.7f);
                    g.setColour(juce::Colours::black.withAlpha(.65f));g.fillRect(well.getX(),well.getY(),well.getWidth(),3.0f);
                }
                juce::Path lever;lever.startNewSubPath(base-side*4);
                lever.lineTo(tip-side*6);lever.quadraticTo(tip+direction*2,tip+side*6);
                lever.lineTo(base+side*4);lever.closeSubPath();
                g.setColour(juce::Colours::black.withAlpha(.55f));g.fillPath(lever,juce::AffineTransform::translation(3,3));
                g.setGradientFill(juce::ColourGradient(juce::Colour(0xff515151).darker(age*.2f),tip.x-6,tip.y,
                                                     juce::Colour(0xff111111),tip.x+6,base.y,false));
                g.fillPath(lever);g.setColour(juce::Colour(0xff050505));g.strokePath(lever,juce::PathStrokeType(.9f));
                g.setColour(juce::Colours::white.withAlpha(.22f*(1-age*.6f)));g.drawLine({base-side*3,tip-side*4},.8f);
                for(int i=0;i<3;++i) {
                    const auto rib=tip-direction*float(4+i*3);
                    g.setColour(juce::Colour(0xff090909));g.drawLine({rib-side*4.5f,rib+side*4.5f},.9f);
                    g.setColour(juce::Colours::white.withAlpha(.12f));g.drawLine({rib-side*5-direction*.6f,rib+side*5-direction*.6f},.45f);
                }
            } else {
                g.setColour(juce::Colour(0xff05080a));g.fillRoundedRectangle(float(x),cy-2,float(w),4,2);
                g.setColour(juce::Colours::white.withAlpha(.18f));g.drawLine(float(x),cy+2,float(x+w),cy+2,.6f);
                g.setColour(ink.withAlpha(.35f));g.fillRect(float(x),cy-1,juce::jmax(0.0f,pos-float(x)),1.5f);
                cap(g,{pos-6,cy-5,12,11},true);
            }
        }
    }
    void drawRotarySlider(juce::Graphics& g,int x,int y,int w,int h,float value,
                          float start,float end,juce::Slider& slider) override {
        const auto area=juce::Rectangle<float>(float(x),float(y),float(w),float(h));
        const auto ink=agedInk(age);
        const auto c=area.getCentre();const float d=juce::jmin(float(w),float(h));
        const float a=start+(end-start)*value;
        if(slider.getComponentID()=="tune") {
            auto disc=juce::Rectangle<float>(c.x-d*.38f,c.y-d*.38f,d*.76f,d*.76f);
            g.setColour(juce::Colours::black.withAlpha(.5f));g.fillEllipse(disc.expanded(1).translated(1,2));
            g.setGradientFill(juce::ColourGradient(juce::Colour(0xff565b5c),disc.getX(),disc.getY(),
                                                juce::Colour(0xff141719),disc.getRight(),disc.getBottom(),false));
            g.fillEllipse(disc);
            g.setColour(juce::Colour(0xff0b0d0e));g.drawEllipse(disc,.8f);
            auto p=c+juce::Point<float>(std::sin(a),-std::cos(a))*d*.26f;
            g.setColour(ink);g.drawLine({c,p},1.4f);
        } else {
            g.setImageResamplingQuality(juce::Graphics::highResamplingQuality);
            g.drawImage(artwork().knob,area,juce::RectanglePlacement::stretchToFit);
            if(age>0) {
                juce::Graphics::ScopedSaveState save(g);
                g.setColour(juce::Colours::black.withAlpha(age*.12f));
                g.drawImage(artwork().knob,area,juce::RectanglePlacement::stretchToFit,true);
                const int mark=slider.getComponentID()=="range"?34:slider.getComponentID()=="wave"?35:36;
                const float dx=mark==34?-.20f:mark==35?.18f:-.17f;
                const float dy=mark==34?.04f:mark==35?-.19f:-.21f;
                const auto patch=juce::Rectangle<float>(c.x+d*dx-d*.14f,c.y+d*dy-d*.16f,d*.28f,d*.32f);
                // Three independent photographed corrosion islands. No aged
                // knob stamp is repeated on another control.
                juce::Path metal;metal.addEllipse(c.x-d*.30f,c.y-d*.37f,d*.60f,d*.58f);
                g.reduceClipRegion(metal);g.setOpacity(age*age*.74f);
                g.drawImage(artwork().wearCell(mark),patch,juce::RectanglePlacement::stretchToFit);
            }
            // Fixed studio lighting; rotate only the engraved pointer.
            const auto pointerCentre=c+juce::Point<float>(0,-d*.072f);
            auto p=pointerCentre+juce::Point<float>(std::sin(a),-std::cos(a))*d*.20f;
            auto q=pointerCentre+juce::Point<float>(std::sin(a),-std::cos(a))*d*.31f;
            g.setColour(juce::Colour(0xff141718));g.drawLine({p,q},d*.018f);
            g.setColour(juce::Colours::white.withAlpha(.13f));g.drawLine({p.translated(.5f,0),q.translated(.5f,0)},.5f);
        }
    }
};
class SlideSwitch final : public juce::Component, public juce::SettableTooltipClient {
public:
    SlideSwitch(juce::RangedAudioParameter& p,std::vector<int> positions,juce::StringArray names,bool left=false)
        : order(std::move(positions)),labels(std::move(names)),labelsLeft(left),
          attachment(p,[this](float v){value=juce::roundToInt(v);repaint();},nullptr) {
        initial=juce::roundToInt(p.convertFrom0to1(p.getDefaultValue()));
        setWantsKeyboardFocus(true);setMouseCursor(juce::MouseCursor::PointingHandCursor);
        attachment.sendInitialUpdate();
    }
    ~SlideSwitch() override {if(dragging)attachment.endGesture();}
    void paint(juce::Graphics& g) override {
        const auto* look=dynamic_cast<const HardwareLook*>(&getLookAndFeel());
        const float age=look!=nullptr?look->age:0;
        const auto ink=agedInk(getY()<headerH+bodyH?age:0);
        const float xx=labelsLeft?float(getWidth()-12):12.0f;
        const float middle=float(getHeight())*.5f,throwStep=order.size()==3?12.0f:10.0f;
        const float top=middle-throwStep*float(order.size()-1)*.5f,bottom=middle+throwStep*float(order.size()-1)*.5f;
        const float labelStep=order.size()==3?36.0f:20.0f;
        g.setColour(juce::Colour(0xff62696b));g.fillRoundedRectangle(xx-5,top-10,10,bottom-top+20,1.8f);
        g.setColour(juce::Colour(0xff030608));g.fillRoundedRectangle(xx-4,top-9,8,bottom-top+18,1.2f);
        for(size_t i=0;i<order.size();++i) {
            const float yy=top+throwStep*float(i),labelY=middle+(float(i)-float(order.size()-1)*.5f)*labelStep;
            g.setColour(ink);g.drawLine(xx+(labelsLeft?-9:6),yy,xx+(labelsLeft?-6:9),yy,.7f);
            auto textBox=labelsLeft?juce::Rectangle<float>(0,labelY-13,xx-11,26):
                                   juce::Rectangle<float>(xx+11,labelY-13,float(getWidth())-xx-11,26);
            if(getComponentID()=="lfoWave" && i<2) {
                // The original uses waveform glyphs, which remain readable in
                // the narrow space beside the short switch mechanism.
                juce::Path wave;const float left=textBox.getX()+1,right=textBox.getRight()-2;
                if(i==0) {
                    for(int n=0;n<=24;++n) {
                        const float xxWave=left+(right-left)*float(n)/24;
                        const float yyWave=labelY-4*std::sin(float(n)/24*juce::MathConstants<float>::twoPi);
                        if(n==0)wave.startNewSubPath(xxWave,yyWave);else wave.lineTo(xxWave,yyWave);
                    }
                } else {
                    wave.startNewSubPath(left,labelY+4);wave.lineTo(left+(right-left)*.2f,labelY+4);
                    wave.lineTo(left+(right-left)*.2f,labelY-4);wave.lineTo(left+(right-left)*.6f,labelY-4);
                    wave.lineTo(left+(right-left)*.6f,labelY+4);wave.lineTo(right,labelY+4);
                }
                g.setColour(ink);g.strokePath(wave,juce::PathStrokeType(.85f));
            } else if(getComponentID()=="lfoWave")
                caption(g,labels[int(i)],{0,labelY-12,float(getWidth()),24},7.6f,juce::Justification::centred,true,ink);
            else caption(g,labels[int(i)],textBox,7.6f,labelsLeft?juce::Justification::centredRight:juce::Justification::centredLeft,true,ink);
            if(value==order[i])cap(g,{xx-5.5f,yy-9,11,18},true,false,age);
        }
        if(hasKeyboardFocus(true)) {g.setColour(amber.withAlpha(.7f));g.drawRoundedRectangle(getLocalBounds().toFloat().reduced(.8f),2,.7f);}
    }
    void mouseDown(const juce::MouseEvent& e) override {
        dragging=true;const float xx=labelsLeft?float(getWidth()-12):12.0f;
        draggingSlot=std::abs(e.position.x-xx)<11;attachment.beginGesture();select(e.position.y,false);
    }
    void mouseDrag(const juce::MouseEvent& e) override {select(e.position.y,false);}
    void mouseUp(const juce::MouseEvent&) override {if(dragging){attachment.endGesture();dragging=false;}}
    void mouseDoubleClick(const juce::MouseEvent&) override {
        if(dragging){attachment.endGesture();dragging=false;}
        attachment.setValueAsCompleteGesture(float(initial));
    }
    bool keyPressed(const juce::KeyPress& key) override {
        auto it=std::find(order.begin(),order.end(),value);int i=int(std::distance(order.begin(),it));
        if(key==juce::KeyPress::upKey||key==juce::KeyPress::leftKey)--i;
        else if(key==juce::KeyPress::downKey||key==juce::KeyPress::rightKey)++i;
        else if(key==juce::KeyPress::homeKey)i=0;
        else if(key==juce::KeyPress::endKey)i=int(order.size())-1;
        else return false;
        attachment.setValueAsCompleteGesture(float(order[size_t(juce::jlimit(0,int(order.size())-1,i))]));return true;
    }
private:
    std::vector<int> order;juce::StringArray labels;bool labelsLeft=false,dragging=false,draggingSlot=false;int value=0,initial=0;
    juce::ParameterAttachment attachment;
    void select(float y,bool complete) {
        const float step=draggingSlot?(order.size()==3?12.0f:10.0f):(order.size()==3?36.0f:20.0f);
        const float top=float(getHeight())*.5f-step*float(order.size()-1)*.5f;
        int i=juce::jlimit(0,int(order.size())-1,juce::roundToInt((y-top)/step));
        if(complete)attachment.setValueAsCompleteGesture(float(order[size_t(i)]));
        else attachment.setValueAsPartOfGesture(float(order[size_t(i)]));
    }
};
bool isBlack(int n) {const int a=n%12;return a==1||a==3||a==6||a==8||a==10;}
class Keyboard final : public juce::Component {
public:
    explicit Keyboard(SH09Processor& p):processor(p) {
        setComponentID("keyboard");
        setMouseCursor(juce::MouseCursor::PointingHandCursor);setWantsKeyboardFocus(true);
        setTitle("32-key low-note-priority keyboard");
    }
    ~Keyboard() override {release();}
    void paint(juce::Graphics& g) override {
        g.fillAll(juce::Colour(0xff06090b));
        const float age=processor.state.getRawParameterValue("age")->load();
        for(int pass=0;pass<2;++pass)for(int n=53;n<=84;++n)if(int(isBlack(n))==pass) {
            auto r=keyRect(n);
            const bool down=n==mouseNote||processor.keyDisplay[size_t(n)].load(std::memory_order_relaxed)!=0;
            const float depressed=down?3.5f:0;
            r=r.withTrimmedTop(depressed);
            if(pass==0) {
                const auto ivory=juce::Colour(0xffefede6).interpolatedWith(juce::Colour(0xffd8c9a4),age*.57f);
                auto key=r.reduced(.65f,0);
                auto face=key.withTrimmedTop(key.getHeight()-20);
                auto top=key.withTrimmedBottom(20);
                juce::ColourGradient enamel(ivory.darker(.23f),0,top.getY(),ivory,0,top.getBottom(),false);
                enamel.addColour(.07,ivory.darker(.09f));enamel.addColour(.55,ivory.brighter(.04f));
                enamel.addColour(.975,ivory.brighter(.09f));g.setGradientFill(enamel);g.fillRect(top);
                g.setGradientFill(juce::ColourGradient(ivory.darker(.19f),0,face.getY(),ivory.darker(.04f),0,face.getBottom(),false));
                g.fillRoundedRectangle(face,1.4f);
                g.setColour(juce::Colours::white.withAlpha(.55f));g.drawLine(key.getX()+1,top.getY()+2,key.getX()+1,top.getBottom()-2,.8f);
                g.setColour(juce::Colour(0xff635e54).withAlpha(.28f));g.drawLine(key.getRight()-1,top.getY(),key.getRight()-1,top.getBottom(),1.2f);
                g.setColour(juce::Colours::white.withAlpha(.62f));g.drawLine(key.getX()+2,face.getY(),key.getRight()-2,face.getY(),.8f);
                g.setColour(juce::Colours::black.withAlpha(.26f));g.fillRect(key.getX()+1,key.getBottom()-2,key.getWidth()-2,2.0f);
                g.setOpacity(1.0f);g.setImageResamplingQuality(juce::Graphics::highResamplingQuality);
                g.drawImage(artwork().whiteKey,key,juce::RectanglePlacement::stretchToFit);
                const float variation=float(n-53)/31;
                g.setColour(juce::Colour(0xffb99d53).withAlpha(.008f+variation*.035f+age*(.16f+variation*.085f)));
                g.fillRoundedRectangle(key.reduced(.5f),1.4f);
            } else {
                for(int i=5;i>0;--i){g.setColour(juce::Colours::black.withAlpha(.07f));g.fillRoundedRectangle(r.expanded(float(i)*.45f).translated(2,3),1.8f);}
                auto face=r.withTrimmedTop(r.getHeight()-17),top=r.withTrimmedBottom(17);
                juce::ColourGradient plastic(juce::Colour(0xff111517),top.getX(),top.getY(),
                                            juce::Colour(0xff171b1d),top.getRight(),top.getY(),false);
                plastic.addColour(.08,juce::Colour(0xff464c4e));plastic.addColour(.17,juce::Colour(0xff292e30));
                plastic.addColour(.80,juce::Colour(0xff242a2c));plastic.addColour(.94,juce::Colour(0xff0b1013));
                g.setGradientFill(plastic);g.fillRoundedRectangle(top,1.5f);
                g.setGradientFill(juce::ColourGradient(juce::Colour(0xff0a0d0f),0,face.getY(),juce::Colour(0xff202629),0,face.getBottom(),false));g.fillRoundedRectangle(face,1.8f);
                g.setColour(juce::Colours::white.withAlpha(.24f));g.drawLine(top.getX()+3,top.getY()+2,top.getX()+3,top.getBottom()-2,.6f);
                g.drawLine(face.getX()+2,face.getY(),face.getRight()-2,face.getY(),.7f);
                g.setOpacity(1.0f);g.setImageResamplingQuality(juce::Graphics::highResamplingQuality);
                g.drawImage(artwork().blackKey,r,juce::RectanglePlacement::stretchToFit);
                g.setColour(juce::Colour(0xff6b6253).withAlpha(.004f+float(n-53)*.0012f));g.fillRoundedRectangle(r.reduced(.6f),1.2f);
            }
            // Each note owns one independent photographic decal. The case and
            // other keys never reuse it, including mirrored/rotated copies.
            const int serial=n-53;
            // Tiny individual polish reflections keep even pristine keys from
            // sharing an identical surface after 8-bit tint quantisation.
            const float polishX=r.getCentreX()+(pass==0?-8.0f:-6.0f)+float((serial*7)%(pass==0?17:13));
            const float polishY=r.getBottom()-(pass==0?65.0f:48.0f)+float((serial*13)%(pass==0?41:30));
            g.setColour(juce::Colours::white.withAlpha(pass==0?.24f:.055f));g.fillEllipse(polishX,polishY,.85f,1.5f);
            if(age>=.18f+float(serial)*.023f) {
                const float width=r.getWidth()*(.43f+float((serial*7)%11)*.025f);
                const float height=r.getHeight()*(pass==0?.07f:.095f)+float(serial%5);
                const auto spot=juce::Rectangle<float>(r.getX()+r.getWidth()*(.10f+float((serial*3)%9)*.015f),
                    r.getY()+r.getHeight()*(.45f+float((serial*13)%29)*.01f),width,height);
                juce::Graphics::ScopedSaveState save(g);g.reduceClipRegion(r.toNearestInt());
                const float angle=float((serial*11)%17-8)*.026f;
                g.addTransform(juce::AffineTransform::rotation(angle,spot.getCentreX(),spot.getCentreY()));
                g.setOpacity(.13f+float((serial*5)%13)*.009f);
                g.drawImage(artwork().wearCell(serial),spot,juce::RectanglePlacement::stretchToFit);
            }
            const float crackOnset=n==60?.82f:n==77?.94f:2.0f;
            if(age>=crackOnset) {
                const float progress=juce::jlimit(0.0f,1.0f,(age-crackOnset)/(1-crackOnset));
                const float width=r.getWidth()*(n==60?.56f:.43f),height=r.getHeight()*(n==60?.27f:.21f);
                const auto trace=juce::Rectangle<float>(n==60?r.getX()+2:r.getRight()-width-2,r.getBottom()-height-17,width,height);
                juce::Graphics::ScopedSaveState save(g);
                const float revealed=trace.getHeight()*(.78f+.22f*progress);
                g.reduceClipRegion(trace.withTrimmedTop(trace.getHeight()-revealed).toNearestInt());
                g.setOpacity(.86f);g.drawImage(artwork().wearCell(n==60?32:33),trace,juce::RectanglePlacement::stretchToFit);
            }
            if(down) {
                const juce::Colour glow(0xff65c8ff);
                g.setColour(glow.withAlpha(pass==0?.23f:.32f));g.fillRoundedRectangle(r.reduced(1),1.5f);
                g.setColour(glow.withAlpha(.9f));g.drawRoundedRectangle(r.reduced(1.5f),1.5f,1.4f);
                g.setColour(processor.displayedNote.load(std::memory_order_relaxed)==n?amber:glow);
                g.fillRoundedRectangle(r.getX()+4,r.getBottom()-9,r.getWidth()-8,5,1.2f);
            }
        }
        g.setGradientFill(juce::ColourGradient(juce::Colours::black.withAlpha(.68f),0,0,juce::Colours::transparentBlack,0,20,false));
        g.fillRect(0,0,getWidth(),20);
    }
    void mouseDown(const juce::MouseEvent& e) override {changeNote(noteAt(e.position));}
    void mouseDrag(const juce::MouseEvent& e) override {changeNote(noteAt(e.position));}
    void mouseUp(const juce::MouseEvent&) override {release();}
    void focusLost(FocusChangeType) override {release();}
    void visibilityChanged() override {if(!isShowing())release();}
private:
    SH09Processor& processor;int mouseNote=-1;
    juce::Rectangle<float> keyRect(int n) const {
        int whites=0;for(int i=53;i<n;++i)if(!isBlack(i))++whites;
        const float w=float(getWidth())/19;
        if(isBlack(n))return {float(whites)*w-w*.31f,0,w*.62f,float(getHeight())*.64f};
        return {float(whites)*w,0,w,float(getHeight())};
    }
    int noteAt(juce::Point<float> p) const {
        for(int pass=1;pass>=0;--pass)for(int n=53;n<=84;++n)if(int(isBlack(n))==pass&&keyRect(n).contains(p))return n;
        return -1;
    }
    void changeNote(int n) {
        if(n==mouseNote)return;const int previous=mouseNote;mouseNote=n;
        if(n>=0)processor.queueKeyboard(n,true);if(previous>=0)processor.queueKeyboard(previous,false);repaint();
    }
    void release() {if(mouseNote>=0){processor.queueKeyboard(mouseNote,false);mouseNote=-1;repaint();}}
};
class Meter final : public juce::Component {
public:
    void setLevels(float e,float f) {if(std::abs(e-env)>.005f||std::abs(f-ext)>.005f){env=e;ext=f;repaint();}}
    void paint(juce::Graphics& g) override {
        for(int k=0;k<2;++k) {
            const float xx=float(k*90);caption(g,k?"EXT":"ENV",{xx,0,25,20},9,juce::Justification::centredLeft);
            for(int i=0;i<8;++i) {
                const bool lit=(k?ext:env)>float(i)/8;
                g.setColour(lit?amber:juce::Colour(0xff373b39));g.fillRect(xx+28+float(i*6),6.0f,4.0f,8.0f);
            }
        }
    }
private:float env=0,ext=0;
};
class RateLamp final : public juce::Component {
public:
    void setOn(bool value) {if(on!=value){on=value;repaint();}}
    void paint(juce::Graphics& g) override {
        auto r=getLocalBounds().toFloat().reduced(1);
        g.setColour(juce::Colour(0xff05090b));g.fillEllipse(r);
        g.setColour(juce::Colour(on?0xffed5035:0xff471c17));g.fillEllipse(r.reduced(1.1f));
        g.setColour(juce::Colour(0xffffc493).withAlpha(on?.65f:.12f));g.fillEllipse(r.getX()+2,r.getY()+1.8f,1.6f,1.6f);
    }
private:bool on=false;
};
class PowerButton final : public juce::Button {
public:
    explicit PowerButton(SH09Processor& p):Button("Power"),processor(p) {
        setComponentID("power");setTooltip("Instrument audio on/off. This setting is not stored in patches.");
        onClick=[this]{processor.setPowerEnabled(!processor.isPowerEnabled());repaint();if(onPowerChanged)onPowerChanged();};
    }
    std::function<void()> onPowerChanged;
    void paintButton(juce::Graphics& g,bool over,bool down) override {
        auto r=getLocalBounds().toFloat().reduced(1.5f);const bool active=processor.isPowerEnabled();
        g.setColour(juce::Colours::black.withAlpha(.65f));g.fillRoundedRectangle(r.translated(1,2),1);
        if(active||down)r=r.translated(0,1);
        g.setGradientFill(juce::ColourGradient(juce::Colour(over?0xffb3b4ae:0xff92968f),0,r.getY(),
                                            juce::Colour(0xff454b4a),0,r.getBottom(),false));
        g.fillRoundedRectangle(r,1.2f);g.setColour(juce::Colour(0xff1a2123));g.drawRoundedRectangle(r,1.2f,.9f);
        g.setColour(juce::Colours::white.withAlpha(.36f));g.drawLine(r.getX()+2,r.getY()+1,r.getRight()-2,r.getY()+1,.6f);
    }
private:SH09Processor& processor;
};
class Header final : public juce::Component {
public:
    explicit Header(sh09::WindowSizing& s):sizing(s) {setComponentID("windowHeader");setTitle("Window header");setMouseCursor(juce::MouseCursor::DraggingHandCursor);}
    void paint(juce::Graphics& g) override {
        g.setGradientFill(juce::ColourGradient(juce::Colour(0xff25292b),0,0,juce::Colour(0xff111517),0,float(getHeight()),false));g.fillAll();
        caption(g,"TOYLAND / KB-09",{20,0,208,float(getHeight())},11,juce::Justification::centredLeft);
        caption(g,"MONOPHONIC",{241,0,140,float(getHeight())},9,juce::Justification::centredLeft,false);
        if(sizing.isSnapArmed()){g.setColour(amber);g.drawRect(getLocalBounds(),2);}
    }
    void mouseDown(const juce::MouseEvent& e) override {sizing.beginHeaderDrag(e);}
    void mouseDrag(const juce::MouseEvent& e) override {sizing.dragHeader(e);}
    void mouseUp(const juce::MouseEvent& e) override {sizing.endHeaderDrag(e);}
    void mouseDoubleClick(const juce::MouseEvent&) override {sizing.doubleClickHeader();}
private:sh09::WindowSizing& sizing;
};
class MaximiseButton final : public juce::Button {
public:
    explicit MaximiseButton(sh09::WindowSizing& s):Button("Maximise"),sizing(s) {
        setTooltip("Maximise / restore the instrument editor");onClick=[this]{sizing.toggleMaximise();};
    }
    void paintButton(juce::Graphics& g,bool over,bool down) override {
        if(over||down){g.setColour(juce::Colour(0xff5d6465).withAlpha(down?.65f:.4f));g.fillAll();}
#if JUCE_MAC
        g.setColour(juce::Colour(0xff29c941));g.fillEllipse(float(getWidth())*.5f-7,float(getHeight())*.5f-7,14,14);
#else
        const float x=float(getWidth())*.5f-5,y=float(getHeight())*.5f-5;
        g.setColour(ink);
        if(sizing.isLarge()) {g.drawRect(x+2,y-2,9.0f,9.0f,1.0f);g.setColour(juce::Colour(0xff1c2022));g.fillRect(x-1,y+1,9.0f,9.0f);g.setColour(ink);g.drawRect(x-1,y+1,9.0f,9.0f,1.0f);}
        else {g.drawRect(x,y,10.0f,10.0f,1.0f);g.fillRect(x,y,10.0f,1.8f);}
#endif
    }
private:sh09::WindowSizing& sizing;
};
void module(juce::Graphics& g,float x,float w,const char* title,float age=0) {
    const auto ink=agedInk(age);
    g.setColour(ink.withAlpha(.84f));g.drawRect(x,57.0f,w,182.0f,.85f);
    g.drawLine(x+1,83,x+w-1,83,.85f);caption(g,title,{x+3,59,w-6,22},13,juce::Justification::centred,true,ink);
}
void arcLabels(juce::Graphics& g,juce::Point<float> centre,
               const std::vector<std::pair<juce::String,float>>& labels,float radius,float age=0) {
    const auto ink=agedInk(age);
    for(const auto& [text,angle]:labels) {
        const auto p=centre+juce::Point<float>(std::sin(angle),-std::cos(angle))*radius;
        const auto q=centre+juce::Point<float>(std::sin(angle),-std::cos(angle))*(radius-6);
        g.setColour(ink);g.drawLine({p,q},.75f);
        auto label=centre+juce::Point<float>(std::sin(angle),-std::cos(angle))*(radius+10);
        // The neighbouring 2-foot and NOISE detents share the space between
        // selectors. Place NOISE above its knob while preserving the tick.
        if(text=="NOISE")label+=juce::Point<float>(3,-26.5f);
        caption(g,text,{label.x-21,label.y-7,42,14},8.4f,juce::Justification::centred,true,ink);
    }
}
}

class SH09Editor::Panel final : public juce::Component, private juce::Timer {
public:
    Panel(SH09Processor& p,sh09::WindowSizing& s)
        :processor(p),sizing(s),presetFiles(p,*this),keyboard(p),header(s),maximise(s),power(p),tooltip(this,650) {
        setLookAndFeel(&look);setSize(designW,designH);
        addAndMakeVisible(header);header.setBounds(0,0,designW,headerH);
        addAndMakeVisible(keyboard);keyboard.setBounds(286,headerH+370,1142,325);
        for(int i=0;i<processor.getNumPrograms();++i)presets.addItem(processor.getProgramName(i),i+1);
        addAndMakeVisible(presets);presets.setBounds(438,5,336,24);presets.setComponentID("factoryPreset");
        presets.setTooltip("Factory patch. Loading replaces current sound settings.");
        presets.setSelectedId(processor.getCurrentProgram()+1,juce::dontSendNotification);
        presets.onChange=[this]{if(presets.getSelectedId()>0){userPreset=false;processor.setCurrentProgram(presets.getSelectedId()-1);}};
        addAndMakeVisible(previous);previous.setButtonText("<");previous.setBounds(407,5,25,24);
        previous.onClick=[this]{userPreset=false;processor.setCurrentProgram((processor.getCurrentProgram()+processor.getNumPrograms()-1)%processor.getNumPrograms());};
        addAndMakeVisible(next);next.setButtonText(">");next.setBounds(780,5,25,24);
        next.onClick=[this]{userPreset=false;processor.setCurrentProgram((processor.getCurrentProgram()+1)%processor.getNumPrograms());};
        addAndMakeVisible(savePreset);savePreset.setButtonText("SAVE");savePreset.setBounds(817,5,56,24);savePreset.setComponentID("savePreset");
        savePreset.setTooltip("Save the complete sound as a reusable .kb09 preset.");savePreset.onClick=[this]{presetFiles.savePreset();};
        addAndMakeVisible(loadPreset);loadPreset.setButtonText("LOAD");loadPreset.setBounds(879,5,56,24);loadPreset.setComponentID("loadPreset");
        loadPreset.setTooltip("Load a saved .kb09 or .sh09state preset.");loadPreset.onClick=[this]{presetFiles.loadPreset();};
        presetFiles.onCompleted=[this](const juce::Result& result,const juce::File& file,bool wasLoad) {
            if(result.wasOk()) {
                if(!wasLoad)savePreset.setTooltip("Saved preset: "+file.getFileName());
                if(wasLoad){userPreset=true;lastProgramRevision=processor.getProgramRevision();lastProgram=processor.getCurrentProgram();presets.setSelectedId(0,juce::dontSendNotification);presets.setText(file.getFileNameWithoutExtension(),juce::dontSendNotification);}
            }
        };
        addAndMakeVisible(meter);meter.setBounds(1024,7,177,20);
        addAndMakeVisible(medium);medium.setButtonText("M");medium.setBounds(1332,5,35,24);medium.setComponentID("sizeM");
        medium.setTooltip("Medium: enlarged view, approximately three quarters of the screen area.");medium.onClick=[this]{sizing.chooseMedium();};
        addAndMakeVisible(large);large.setButtonText("L");large.setBounds(1373,5,35,24);large.setComponentID("sizeL");
        large.setTooltip("Large: fill this screen's available work area. For sharpness in Live, disable Auto-Scale Plug-in Window if available, then create a fresh instance.");large.onClick=[this]{sizing.chooseLarge();};
        addAndMakeVisible(maximise);maximise.setBounds(1453,0,47,34);maximise.setComponentID("maximise");
        sizing.onChange=[this]{header.repaint();maximise.repaint();};
        addSwitch("lfoWave",116,104,49,121,{0,1,2},{"SINE","SQUARE","RANDOM\n(S/H)"},"Modulator: sine, square or random sample-and-hold.");
        addFader("lfoRate","RATE",182,96,43,129);addFader("lfoDelay","DELAY",224,96,43,129);
        addAndMakeVisible(rateLamp);rateLamp.setBounds(193,headerH+104,8,8);rateLamp.setInterceptsMouseClicks(false,false);
        addFader("vcoMod","MOD",284,96,45,129);
        controls.back()->slider->getProperties().set("scaleLeft",true);
        addKnob("range",356,169,87);addKnob("wave",450,169,87);addKnob("tune",356,86,22);
        addFader("pulseWidth","PULSE\nWIDTH",536,96,45,129);
        addSwitch("pwmMode",566,104,51,121,{1,0,2},{"LFO","MANUAL","ENV"},"Pulse width: LFO, manual or envelope. LFO uses the undelayed sine.");
        addSwitch("subMode",633,104,60,121,{0,1,2},{"1 OCT\nDOWN","2 OCT\nDOWN","2 OCT\nPULSE"},"Sub oscillator is divided from the main VCO.");
        addFader("vcoLevel","VCO",721,96,41,129);addFader("subLevel","SUB\nOSC",770,96,41,129);addFader("extLevel","EXT",819,96,41,129);
        addFader("cutoff","CUTOFF\nFREQ",880,96,43,129);addFader("resonance","RESONANCE",923,96,45,129);
        addSwitch("vcfEnvMode",956,104,61,121,{2,0,1},{"ENV\nFOL'R","ENV +","ENV -"},"VCF envelope follower, positive envelope, or inverted envelope.");
        addFader("vcfEnv","ENV",1030,96,40,129);addFader("vcfMod","MOD",1075,96,40,129);
        addSwitch("vcaMode",1110,104,56,121,{0,1,2},{"HOLD","ENV","GATE"},"HOLD passes audio without MIDI; ENV uses ADSR; GATE follows held keys.");
        addSwitch("envMode",1175,104,55,121,{0,1,2},{"GATE +\nTRIG","GATE","LFO"},"Gate+Trigger re-attacks on low-note changes. Gate preserves legato. LFO repeats while held.",true);
        addFader("attack","A",1249,96,35,129,true);addFader("decay","D",1288,96,35,129,true);
        addFader("sustain","S",1327,96,35,129,true);addFader("release","R",1366,96,35,129,true);
        addKnob("volume",117,442,79);
        addAndMakeVisible(power);power.setBounds(194,headerH+416,31,24);power.onPowerChanged=[this]{repaint();};
        addFader("bendVco","VCO",98,496,37,112,true);addFader("bendVcf","VCF",142,496,37,112,true);
        addFader("portamento","PORTAMENTO",205,496,37,112,true);
        addAndMakeVisible(bender);bender.setBounds(85,headerH+638,143,48);bender.setComponentID("bender");
        bender.setRange(-1,1,.001);bender.setValue(0);bender.setSliderStyle(juce::Slider::LinearHorizontal);
        bender.setTextBoxStyle(juce::Slider::NoTextBox,false,0,0);bender.setScrollWheelEnabled(false);bender.setWantsKeyboardFocus(false);
        bender.setTooltip("Spring-return bender. VCO / VCF sliders set the depth.");
        bender.onValueChange=[this]{bendActive=std::abs(bender.getValue())>.0001;processor.queuePitchBend(float(bender.getValue()));};
        bender.onDragEnd=[this]{bender.setValue(0,juce::sendNotificationSync);};
        addSwitch("modern",22,designH-headerH-44,131,42,{0,1},{"AUTHENTIC","MODERN"},"Authentic ignores velocity, aftertouch, tempo sync and input trim.");
        addUtility("age","CONDITION / AGE",257,154);
        addUtility("velocity","VELOCITY",493,113);addUtility("aftertouch","AFTERTOUCH",692,113);
        addSwitch("lfoSync",811,designH-headerH-44,104,42,{0,1},{"FREE","TEMPO"},"Modern mode: free or tempo-synchronised LFO.");
        addUtility("extGain","INPUT TRIM",1070,127);
        addAndMakeVisible(help);help.setButtonText("EXT AUDIO");help.setBounds(1323,designH-35,120,24);
        help.onClick=[this]{externalHelp.setVisible(!externalHelp.isVisible());};
        addChildComponent(externalHelp);externalHelp.setBounds(390,headerH+418,900,225);
        externalHelp.setText("EXTERNAL AUDIO + ENVELOPE FOLLOWER\n\nRoute DAW audio to the instrument's External Input and raise EXT in the mixer.\nWithout MIDI, set VCA to HOLD and lower VCO / SUB.\nSelect ENV FOL'R in VCF, lower CUTOFF and raise ENV for touch-sensitive filtering.\nThe follower listens before the EXT fader. Click EXT AUDIO again to close.",juce::dontSendNotification);
        externalHelp.setFont(font(16));externalHelp.setJustificationType(juce::Justification::topLeft);
        externalHelp.setBorderSize(juce::BorderSize<int>(24));
        externalHelp.setColour(juce::Label::backgroundColourId,juce::Colour(0xf5101518));
        externalHelp.setColour(juce::Label::outlineColourId,amber);externalHelp.setInterceptsMouseClicks(true,true);
        updateModern();startTimerHz(24);
    }
    ~Panel() override {
        stopTimer();sizing.onChange=nullptr;if(bendActive)processor.queuePitchBend(0);setLookAndFeel(nullptr);
    }
    void paint(juce::Graphics& g) override {
        look.age=processor.state.getRawParameterValue("age")->load();
        const float age=look.age;
        const auto ink=agedInk(age);
        const auto& art=artwork();g.setImageResamplingQuality(juce::Graphics::highResamplingQuality);
        {
            juce::Graphics::ScopedSaveState save(g);
            juce::Path outline;outline.addRoundedRectangle(0,float(headerH),float(designW),float(bodyH),14);
            g.reduceClipRegion(outline);
            // Crop the empty photo border; the rounded chassis sits against the editor's dark surround.
            const auto source=juce::Rectangle<int>(juce::roundToInt(art.chassis.getWidth()*2.0/1774),
                juce::roundToInt(art.chassis.getHeight()*7.0/887),juce::roundToInt(art.chassis.getWidth()*1770.0/1774),
                juce::roundToInt(art.chassis.getHeight()*873.0/887));
            g.drawImage(art.chassis,0,headerH,designW,bodyH,source.getX(),source.getY(),source.getWidth(),source.getHeight());
            if(age>0) {
                // Registered photographic material endpoint. Only metal ages;
                // rust does not spill onto the molded side cheeks or empty keybed.
                juce::Path metal;
                metal.addRectangle(55.0f,float(headerH),1389.0f,365.0f);
                metal.addRectangle(65.0f,float(headerH+366),182.0f,335.0f);
                g.reduceClipRegion(metal);g.setOpacity(age*age*.68f);
                g.drawImage(art.chassisAged,0,headerH,designW,bodyH,source.getX(),source.getY(),source.getWidth(),source.getHeight());
            }
        }
        caseStains(g,age);
        // Keep the front rail's original hardware geometry. The generated
        // endpoint has extra fasteners here, so use sheen reduction only.
        if(age>0){g.setColour(juce::Colours::black.withAlpha(age*.12f));g.fillRect(55.0f,float(headerH+705),1389.0f,45.0f);}
        {
            juce::Graphics::ScopedSaveState save(g);g.addTransform(juce::AffineTransform::translation(0,float(headerH)));
            module(g,107,141,"MODULATOR",age);module(g,252,365,"VCO",age);module(g,621,72,"SUB OSC",age);
            module(g,697,146,"AUDIO MIXER",age);module(g,847,251,"VCF",age);module(g,1102,64,"VCA",age);module(g,1170,221,"ENV",age);
            caption(g,"RANGE",{319,96,74,12},9,juce::Justification::centred,true,ink);
            caption(g,"WAVEFORM",{408,91,84,17},9,juce::Justification::centred,true,ink);
            caption(g,"TUNE",{305,84,35,15},8.4f,juce::Justification::centred,true,ink);
            g.setColour(ink);g.drawLine(294,91,304,91,.8f);
            arcLabels(g,{356,163},{{"32'",-1.134464f},{"16'",-.567232f},{"8'",0.0f},{"4'",.567232f},{"2'",1.134464f}},43,age);
            arcLabels(g,{450,163},{{"NOISE",-1.047198f},{"SAW",-.349066f},{"SQR",.349066f},{"PULSE",1.047198f}},43,age);
            // Original user marks: split into manufacturer on right and instrument on left.
            const float printWear=age*age;
            for(int i=0;i<2;++i) {
                g.setColour(ink.withAlpha(i==0?1-printWear:printWear));
                const auto& printed=i==0?art.marks:art.marksAged;
                g.drawImage(printed,94,256,441,56,846,308,1253,158,true);
                g.drawImage(printed,1175,258,235,51,85,308,722,158,true);
            }
            g.setColour(ink.withAlpha(.94f));g.fillRect(60,312,1381,2);
            g.setColour(ink.withAlpha(.82f));g.drawRect(69,370,176,323,1);
            caption(g,"VOLUME",{80,393,75,19},11,juce::Justification::centred,true,ink);
            for(int i=0;i<=10;++i) {
                const float a=juce::MathConstants<float>::pi*(1.25f+1.5f*float(i)/10);
                const juce::Point<float> c(117,436),direction(std::sin(a),-std::cos(a));
                g.setColour(ink);g.drawLine({c+direction*42,c+direction*(i%5==0?47.0f:45.0f)},.7f);
                if(i==0||i==10) {const auto p=c+direction*53;caption(g,juce::String(i),{p.x-10,p.y-5,20,10},7.5f,juce::Justification::centred,true,ink);}
            }
            g.setColour(juce::Colour(0xff080b0e));g.fillEllipse(205,450,9,9);
            g.setColour(juce::Colour(processor.isPowerEnabled()?0xffe54c35:0xff491c16));g.fillEllipse(207,452,5,5);
            if(processor.isPowerEnabled()) {g.setColour(juce::Colour(0xffffaa6f).withAlpha(.65f));g.fillEllipse(207.5f,452.5f,1.8f,1.8f);}
            caption(g,"POWER",{185,463,49,12},8.4f,juce::Justification::centred,true,ink);
            g.setColour(ink.withAlpha(.76f));g.drawRect(77.0f,489.0f,91.0f,134.0f,.8f);g.drawRect(174.0f,489.0f,61.0f,134.0f,.8f);
            caption(g,"BENDER",{81,623,151,16},10,juce::Justification::centred,true,ink);
            g.setColour(ink.withAlpha(.8f));g.drawLine(93,686,220,686,.7f);
            caption(g,"-",{86,680,16,12},9,juce::Justification::centred,true,ink);
            caption(g,"0",{148,680,17,12},8,juce::Justification::centred,true,ink);
            caption(g,"+",{211,680,17,12},9,juce::Justification::centred,true,ink);
            // Screw heads and contact shadows are a foreground layer over ink.
            // Repaint tightly clipped registered photographic heads after rules.
            for(const auto p: {juce::Point<float>(100,457),{277,457},{98,799},{277,799}}) {
                juce::Graphics::ScopedSaveState save(g);
                const float xx=(p.x-2)*float(designW)/1770,yy=(p.y-7)*float(bodyH)/873;
                juce::Path head;head.addEllipse(xx-11.5f,yy-11.5f,23,23);g.reduceClipRegion(head);
                g.setOpacity(1);g.drawImage(art.chassis,0,0,designW,bodyH,2,7,1770,873);
                g.setOpacity(age*age*.94f);g.drawImage(art.chassisAged,0,0,designW,bodyH,2,7,1770,873);
            }
        }
        g.setGradientFill(juce::ColourGradient(juce::Colour(0xff222729),0,float(headerH+bodyH),
                                            juce::Colour(0xff121618),0,float(designH),false));
        g.fillRect(0,headerH+bodyH,designW,designH-headerH-bodyH);
        g.setColour(juce::Colour(0xff58605f));g.drawLine(0,float(headerH+bodyH),float(designW),float(headerH+bodyH),.65f);
        caption(g,"OPTIONAL",{1208,float(designH-35),93,24},8.5f,juce::Justification::centred,false);
    }
private:
    using SliderAttachment=juce::AudioProcessorValueTreeState::SliderAttachment;
    struct Control {
        std::unique_ptr<juce::Slider> slider;
        std::unique_ptr<SlideSwitch> selector;
        std::unique_ptr<juce::Label> label;
        std::unique_ptr<SliderAttachment> attachment;
    };
    SH09Processor& processor;sh09::WindowSizing& sizing;sh09::PresetFiles presetFiles;HardwareLook look;
    Keyboard keyboard;Header header;MaximiseButton maximise;PowerButton power;juce::TooltipWindow tooltip;
    juce::ComboBox presets;juce::TextButton previous,next,medium,large,help,savePreset,loadPreset;
    juce::Slider bender;juce::Label externalHelp;Meter meter;RateLamp rateLamp;
    std::vector<std::unique_ptr<Control>> controls;
    std::array<int,128> lastKeys{};float lastAge=-1;bool lastModern=false,bendActive=false,userPreset=false;int lastProgram=-1;unsigned lastProgramRevision=0;
    void timerCallback() override {
        const auto programRevision=processor.getProgramRevision();
        const int program=processor.getCurrentProgram();
        if(!userPreset||program!=lastProgram||programRevision!=lastProgramRevision){presets.setSelectedId(program+1,juce::dontSendNotification);userPreset=false;}
        lastProgram=program;lastProgramRevision=programRevision;
        meter.setLevels(processor.displayedEnvelope.load(),processor.displayedFollower.load());
        rateLamp.setOn(processor.isPowerEnabled()&&processor.displayedModulatorLamp.load(std::memory_order_relaxed));
        bool keysChanged=false;
        for(size_t i=0;i<lastKeys.size();++i){const int n=processor.keyDisplay[i].load(std::memory_order_relaxed);if(lastKeys[i]!=n){lastKeys[i]=n;keysChanged=true;}}
        if(keysChanged)keyboard.repaint();
        const float age=processor.state.getRawParameterValue("age")->load();
        if(age!=lastAge){
            lastAge=age;look.age=age;
            for(auto& c:controls)if(c->label&&bool(c->label->getProperties().getWithDefault("hardwarePrint",false)))
                c->label->setColour(juce::Label::textColourId,agedInk(age));
            repaint();keyboard.repaint();
        }
        const bool modern=processor.state.getRawParameterValue("modern")->load()>.5f;
        if(modern!=lastModern)updateModern();
    }
    void updateModern() {
        lastModern=processor.state.getRawParameterValue("modern")->load()>.5f;
        for(auto& c:controls) {
            juce::Component* component=c->slider?static_cast<juce::Component*>(c->slider.get()):static_cast<juce::Component*>(c->selector.get());
            const auto id=component->getComponentID();
            if(id=="velocity"||id=="aftertouch"||id=="lfoSync"||id=="extGain") {
                component->setEnabled(lastModern);component->setAlpha(lastModern?1.0f:.4f);
                if(c->label)c->label->setAlpha(lastModern?1.0f:.4f);
            }
        }
    }
    void addSwitch(const char* id,int x,int y,int w,int h,std::vector<int> order,
                   juce::StringArray labels,const juce::String& tip,bool left=false) {
        auto c=std::make_unique<Control>();auto* p=processor.state.getParameter(id);jassert(p);
        c->selector=std::make_unique<SlideSwitch>(*p,std::move(order),std::move(labels),left);
        c->selector->setComponentID(id);c->selector->setTitle(p->getName(100));c->selector->setTooltip(tip);
        c->selector->setBounds(x,headerH+y,w,h);addAndMakeVisible(*c->selector);controls.push_back(std::move(c));
    }
    void addSlider(const char* id,const char* name,int x,int y,int w,int h,juce::Slider::SliderStyle style) {
        auto c=std::make_unique<Control>();c->slider=std::make_unique<juce::Slider>();auto& slider=*c->slider;
        slider.setComponentID(id);slider.setTitle(name);slider.setBounds(x,headerH+y,w,h);slider.setSliderStyle(style);
        slider.setTextBoxStyle(juce::Slider::NoTextBox,false,0,0);slider.setPopupDisplayEnabled(true,true,this);
        slider.setScrollWheelEnabled(false);slider.setMouseDragSensitivity(190);
        const float twoPi=juce::MathConstants<float>::twoPi;
        if(juce::String(id)=="range")slider.setRotaryParameters(twoPi-1.134464f,twoPi+1.134464f,true);
        else if(juce::String(id)=="wave")slider.setRotaryParameters(twoPi-1.047198f,twoPi+1.047198f,true);
        else slider.setRotaryParameters(juce::MathConstants<float>::pi*1.25f,juce::MathConstants<float>::pi*2.75f,true);
        for(const auto& spec:sh09::specs)if(juce::String(spec.id)==id) {slider.setTooltip(spec.name);slider.setDoubleClickReturnValue(true,spec.initial);}
        addAndMakeVisible(slider);c->attachment=std::make_unique<SliderAttachment>(processor.state,id,slider);
        for(const auto& spec:sh09::specs)if(juce::String(spec.id)==id&&spec.choices[0]==0) {
            if(juce::String(id)=="tune")slider.textFromValueFunction=[](double v){return juce::String(v,1)+" ct";};
            else slider.textFromValueFunction=[](double v){return juce::String(v*10,2);};
        }
        controls.push_back(std::move(c));
    }
    void addLabel(Control& c,const juce::String& text,int x,int y,int w,int h,float size) {
        c.label=std::make_unique<juce::Label>();c.label->setText(text,juce::dontSendNotification);
        c.label->setBounds(x,headerH+y,w,h);c.label->setFont(font(size,true));c.label->setJustificationType(juce::Justification::centred);
        c.label->setBorderSize(juce::BorderSize<int>(0));c.label->setInterceptsMouseClicks(false,false);addAndMakeVisible(*c.label);
        const bool hardware=headerH+y<headerH+bodyH;
        c.label->getProperties().set("hardwarePrint",hardware);
        if(hardware)c.label->setColour(juce::Label::textColourId,agedInk(processor.state.getRawParameterValue("age")->load()));
    }
    void addFader(const char* id,const char* name,int centre,int y,int w,int h,bool compact=false) {
        addSlider(id,name,centre-w/2,y+20,w,h-11,juce::Slider::LinearVertical);
        auto& c=*controls.back();c.slider->getProperties().set("compact",compact);
        // Printed scale numerals extend past the narrow hit area, without
        // overlapping the neighbouring fader's interaction bounds.
        c.slider->setPaintingIsUnclipped(true);
        addLabel(c,name,centre-w/2-8,y-1,w+16,20,juce::String(id)=="portamento"?7.1f:8.4f);
    }
    void addKnob(const char* id,int cx,int cy,int d) {
        addSlider(id,id,cx-d/2,cy-d/2,d,d,juce::Slider::RotaryHorizontalVerticalDrag);
    }
    void addUtility(const char* id,const char* name,int centre,int width) {
        addSlider(id,name,centre-width/2,designH-headerH-27,width,18,juce::Slider::LinearHorizontal);
        addLabel(*controls.back(),name,centre-width/2-8,designH-headerH-46,width+16,18,9);
    }
};
SH09Editor::SH09Editor(SH09Processor& p)
    :AudioProcessorEditor(p),sizing(*this,designW,designH),panel(std::make_unique<Panel>(p,sizing)) {
    setResizable(true,false);addAndMakeVisible(*panel);setSize(designW,designH);sizing.chooseMedium();
#if JUCE_WINDOWS
    if(p.wrapperType==juce::AudioProcessor::wrapperType_Standalone) {
        juce::Component::SafePointer<SH09Editor> editor(this);
        juce::MessageManager::callAsync([editor] {
            if(editor==nullptr)return;
            if(auto* window=editor->findParentComponentOfClass<juce::DocumentWindow>()) {
                if(!window->isOnDesktop())return;
                window->setUsingNativeTitleBar(true);
                window->setTitleBarButtonsRequired(juce::DocumentWindow::allButtons,false);
                editor->sizing.peerAttached();
            }
        });
    }
#endif
}
void SH09Editor::visibilityChanged() {sizing.peerAttached();}
SH09Editor::~SH09Editor()=default;
void SH09Editor::parentHierarchyChanged() {
#if JUCE_WINDOWS
    // This is our JUCE peer (the VST3 wrapper child or standalone window),
    // never the DAW's native frame. Software rendering avoids graphics-driver
    // shutdown waits when a host repeatedly creates and unloads this editor.
    if(auto* peer=getPeer()) {
        const auto engines=peer->getAvailableRenderingEngines();
        const auto software=engines.indexOf("Software Renderer");
        if(software>=0)peer->setCurrentRenderingEngine(software);
    }
#endif
    sizing.peerAttached();
}
void SH09Editor::paint(juce::Graphics& g) {g.fillAll(juce::Colour(0xff101416));}
void SH09Editor::resized() {
    if(panel) {
        const auto bounds=sizing.contentBounds(getLocalBounds().toFloat());
        const float scale=bounds.getWidth()/float(designW);
        panel->setTransform(juce::AffineTransform::scale(scale).translated(bounds.getX(),bounds.getY()));
        panel->setTopLeftPosition(0,0);
    }
}
