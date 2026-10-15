#include "DialogTheme.h"
#include <BinaryData.h>

namespace kb09_gui {
namespace {
const juce::Colour black(0xff090909);
juce::Image logoMask() {
    const auto source = juce::SoftwareImageType().convert(juce::ImageFileFormat::loadFrom(BinaryData::marks_png, BinaryData::marks_pngSize));
    const auto crop = source.getClippedImage({90, 324, 715, 133});
    juce::Image mask(juce::Image::ARGB, crop.getWidth(), crop.getHeight(), true,juce::SoftwareImageType());
    for (int y=0;y<crop.getHeight();++y) for(int x=0;x<crop.getWidth();++x) {
        const auto p=crop.getPixelAt(x,y);
        const float a=juce::jlimit(0.f,1.f,(float(p.getRed()+p.getGreen()+p.getBlue())/765.f-.065f)/.89f);
        mask.setPixelAt(x,y,juce::Colours::white.withAlpha(a));
    }
    return mask;
}
}
DialogFonts::DialogFonts()
    : regular(juce::Typeface::createSystemTypefaceFor(BinaryData::JostRegular_ttf, BinaryData::JostRegular_ttfSize)),
      semibold(juce::Typeface::createSystemTypefaceFor(BinaryData::JostSemibold_ttf, BinaryData::JostSemibold_ttfSize)) {}
DialogTheme::DialogTheme(float condition):age(juce::jlimit(0.f,1.f,condition)){setUsingNativeAlertWindows(false);refreshColours();}
juce::Colour DialogTheme::paper() const { return juce::Colours::white; }
juce::Font DialogTheme::lettering(float height,bool bold) {
    // Keep native font ownership scoped to active windows, avoiding
    // typeface ownership in DLL-static destructors.
    juce::SharedResourcePointer<DialogFonts> resources;
    return juce::Font(juce::FontOptions(bold?resources->semibold:resources->regular).withHeight(height)).withExtraKerningFactor(.015f);
}
float DialogTheme::conditionOf(juce::Component* owner) {
    if(owner) if(auto* look=dynamic_cast<DialogTheme*>(&owner->getLookAndFeel())) return look->age;
    return 0;
}
void DialogTheme::refreshColours() {
    setColour(juce::AlertWindow::backgroundColourId,black);setColour(juce::AlertWindow::textColourId,paper());
    setColour(juce::AlertWindow::outlineColourId,paper().withAlpha(.65f));
    setColour(juce::ResizableWindow::backgroundColourId,black);
    setColour(juce::Label::textColourId,paper());setColour(juce::ToggleButton::textColourId,paper());
    setColour(juce::ToggleButton::tickColourId,paper());setColour(juce::ToggleButton::tickDisabledColourId,paper().withAlpha(.3f));
    setColour(juce::TextButton::buttonColourId,black);setColour(juce::TextButton::buttonOnColourId,paper());
    setColour(juce::TextButton::textColourOffId,paper());setColour(juce::TextButton::textColourOnId,black);
    setColour(juce::ComboBox::backgroundColourId,black);setColour(juce::ComboBox::textColourId,paper());
    setColour(juce::ComboBox::outlineColourId,paper().withAlpha(.5f));setColour(juce::ComboBox::arrowColourId,paper());
    setColour(juce::PopupMenu::backgroundColourId,black);setColour(juce::PopupMenu::textColourId,paper());
    setColour(juce::PopupMenu::highlightedBackgroundColourId,paper());setColour(juce::PopupMenu::highlightedTextColourId,black);
    setColour(juce::ListBox::backgroundColourId,black);setColour(juce::ListBox::textColourId,paper());
    setColour(juce::ListBox::outlineColourId,paper().withAlpha(.45f));
    setColour(juce::TextEditor::backgroundColourId,black);setColour(juce::TextEditor::textColourId,paper());
    setColour(juce::TextEditor::outlineColourId,paper().withAlpha(.5f));
    setColour(juce::Slider::thumbColourId,paper());setColour(juce::Slider::trackColourId,paper());
    setColour(juce::ScrollBar::thumbColourId,paper().withAlpha(.5f));
}
void DialogTheme::wear(juce::Graphics& g,juce::Rectangle<int> bounds,int seed) const {
    // Application chrome and dialogs are outside the physical instrument.
    juce::ignoreUnused(g,bounds,seed);
}
void DialogTheme::brand(juce::Graphics& g,juce::Rectangle<float> area) const {
    static auto mask=logoMask();g.setColour(paper());g.drawImage(mask,area,juce::RectanglePlacement::centred,true);wear(g,area.toNearestInt(),83);
}
juce::Font DialogTheme::getAlertWindowTitleFont(){return lettering(titleFontHeight,true);}
juce::Font DialogTheme::getAlertWindowMessageFont(){return lettering(messageFontHeight);}
juce::Font DialogTheme::getAlertWindowFont(){return lettering(messageFontHeight);}
juce::Font DialogTheme::getTextButtonFont(juce::TextButton&,int){return lettering(buttonFontHeight);}
juce::Font DialogTheme::getComboBoxFont(juce::ComboBox&){return lettering(buttonFontHeight);}
juce::Font DialogTheme::getLabelFont(juce::Label&){return lettering(messageFontHeight);}
juce::Font DialogTheme::getPopupMenuFont(){return lettering(messageFontHeight);}
void DialogTheme::drawAlertBox(juce::Graphics& g,juce::AlertWindow& alert,const juce::Rectangle<int>&,juce::TextLayout& layout){
    g.fillAll(black);g.setColour(paper().withAlpha(.65f));g.drawRoundedRectangle(alert.getLocalBounds().toFloat().reduced(1),6,1);
    int inset=0;
    if(alert.getAlertType()!=juce::MessageBoxIconType::NoIcon){inset=80;g.setColour(paper().withAlpha(.7f));g.drawEllipse(24,32,32,32,1);
        g.setFont(lettering(24));g.drawText(alert.getAlertType()==juce::MessageBoxIconType::WarningIcon?"!":alert.getAlertType()==juce::MessageBoxIconType::QuestionIcon?"?":"i",24,31,32,32,juce::Justification::centred);}
    layout.draw(g,{float(inset+1),30.f,float(alert.getWidth()-inset-20),float(alert.getHeight()-getAlertWindowButtonHeight()-20)});
    wear(g,alert.getLocalBounds().reduced(3));
}
void DialogTheme::drawButtonBackground(juce::Graphics& g,juce::Button& b,const juce::Colour&,bool over,bool down){
    const bool selected=b.getToggleState()||b.hasKeyboardFocus(true)||down;
    const auto box=b.getLocalBounds().toFloat().reduced(1);
    g.setColour(selected?paper():over?juce::Colour(0xff202020):black);g.fillRoundedRectangle(box,4);
    g.setColour(paper().withAlpha(selected?1.f:.6f));g.drawRoundedRectangle(box,4,1);
}
void DialogTheme::drawButtonText(juce::Graphics& g,juce::TextButton& b,bool,bool down){
    g.setFont(getTextButtonFont(b,b.getHeight()));g.setColour(b.getToggleState()||b.hasKeyboardFocus(true)||down?black:paper());
    g.drawFittedText(b.getButtonText(),b.getLocalBounds().reduced(10,2),juce::Justification::centred,1);wear(g,b.getLocalBounds(),37);
}
void DialogTheme::drawToggleButton(juce::Graphics& g,juce::ToggleButton& b,bool,bool){
    g.setColour(paper());g.drawRoundedRectangle(3,float(b.getHeight())*.5f-7,14,14,2,1);
    if(b.getToggleState()){juce::Path check;const float y=float(b.getHeight())*.5f;check.startNewSubPath(5,y);check.lineTo(9,y+4);check.lineTo(15,y-5);g.strokePath(check,juce::PathStrokeType(1.5f));}
    g.setFont(lettering(buttonFontHeight));g.drawFittedText(b.getButtonText(),b.getLocalBounds().withTrimmedLeft(24),juce::Justification::centredLeft,2);wear(g,b.getLocalBounds());
}
void DialogTheme::drawLabel(juce::Graphics& g,juce::Label& label){juce::LookAndFeel_V4::drawLabel(g,label);wear(g,label.getLocalBounds());}
void DialogTheme::drawComboBox(juce::Graphics& g,int w,int h,bool down,int x,int y,int bw,int bh,juce::ComboBox& box){
    juce::LookAndFeel_V4::drawComboBox(g,w,h,down,x,y,bw,bh,box);wear(g,box.getLocalBounds());
}
void DialogTheme::drawPopupMenuBackground(juce::Graphics& g,int w,int h){g.fillAll(black);g.setColour(paper().withAlpha(.65f));g.drawRect(0,0,w,h);}
void DialogTheme::getIdealPopupMenuSectionHeaderSizeWithOptions(const juce::String& text,int,int& width,int& height,const juce::PopupMenu::Options&){
    width=text.startsWithIgnoreCase("Toyland")?380:juce::GlyphArrangement::getStringWidthInt(lettering(18,true),text)+28;
    height=text.startsWithIgnoreCase("Toyland")?44:44;
}
void DialogTheme::drawPopupMenuSectionHeader(juce::Graphics& g,const juce::Rectangle<int>& a,const juce::String& text){
    if(text.startsWithIgnoreCase("Toyland")){
        constexpr float width=130;
        brand(g,{float(a.getX()+20),float(a.getY()),width,float(a.getHeight())});
        g.setColour(paper());g.setFont(lettering(24,true).withExtraKerningFactor(0));
        g.drawText(text.substring(7).trim(),a.withTrimmedLeft(int(width)+32).withTrimmedRight(14),juce::Justification::centredLeft);
    }else{g.setColour(paper());g.setFont(lettering(18,true));g.drawText(text,a.reduced(14,0),juce::Justification::centredLeft);}
    wear(g,text.startsWithIgnoreCase("Toyland")?a.withTrimmedLeft(162):a);
}
void DialogTheme::drawPopupMenuItem(juce::Graphics& g,const juce::Rectangle<int>& a,bool sep,bool active,bool hot,bool tick,bool sub,const juce::String& text,const juce::String& shortcut,const juce::Drawable* icon,const juce::Colour*){
    juce::ignoreUnused(icon);if(hot){g.setColour(paper());g.fillRect(a.reduced(2));}
    g.setColour((hot?black:paper()).withAlpha(active?1.f:.4f));
    if(sep){g.fillRect(a.reduced(12).withHeight(1));return;}
    const bool large=getPopupMenuFont().getHeight()>24;
    g.setFont(getPopupMenuFont());g.drawFittedText(text,a.withTrimmedLeft(large?(tick?50:36):30).withTrimmedRight(20),juce::Justification::centredLeft,1);
    if(tick){juce::Path p;const float y=float(a.getCentreY());p.startNewSubPath(float(a.getX()+9),y);p.lineTo(float(a.getX()+(large?18:13)),y+(large?8:4));p.lineTo(float(a.getX()+(large?34:21)),y-(large?12:5));g.strokePath(p,juce::PathStrokeType(large?2.f:1.5f));}
    if(sub)g.drawText(">",a.withWidth(15).withX(a.getRight()-20),juce::Justification::centred);
    if(shortcut.isNotEmpty()){g.setFont(lettering(13));g.drawText(shortcut,a.reduced(12,0),juce::Justification::centredRight);}
    wear(g,a);
}
ThemedAlert::ThemedAlert(const juce::String& title,const juce::String& message,juce::MessageBoxIconType icon,juce::Component* owner,float condition)
    // AlertWindow derives its desktop scale from the associated component.
    // A control in the scaled instrument canvas must use the same top-level
    // window as Save / Don't Save, rather than inheriting the canvas zoom.
    : AlertWindow(title,message,icon,owner?owner->getTopLevelComponent():nullptr),theme(condition<0?DialogTheme::conditionOf(owner):condition){
    setLookAndFeel(&theme);
}
juce::AlertWindow* DialogTheme::createAlertWindow(const juce::String& title,const juce::String& msg,const juce::String& b1,const juce::String& b2,const juce::String& b3,juce::MessageBoxIconType icon,int n,juce::Component* owner){
    auto* a=new ThemedAlert(title,msg,icon,owner,conditionProvider?conditionProvider():age);
    if(n==1)a->addButton(b1,0,juce::KeyPress(juce::KeyPress::escapeKey),juce::KeyPress(juce::KeyPress::returnKey));
    if(n>=2){a->addButton(b1,1,juce::KeyPress(juce::KeyPress::returnKey));a->addButton(b2,n==2?0:2,n==2?juce::KeyPress(juce::KeyPress::escapeKey):juce::KeyPress());}
    if(n==3)a->addButton(b3,0,juce::KeyPress(juce::KeyPress::escapeKey));return a;
}
}
