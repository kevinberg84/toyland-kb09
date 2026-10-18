#pragma once
#include <mutex>
#include "PowerDetails.h"
#include "BinaryData.h"
#include "AgeAppearance.h"
#include <vector>

namespace kb09_gui {
// Separate clean mouldings and two distinct photo-informed wear sprites.
// No mirrored asset or shared damage stamp. Endpoints are prepared once and
// selected only in the static panel cache; moving controls do not rebuild them.
class CheekMaterial {
public:
    static constexpr int width=55,height=732,scale=3;
    explicit CheekMaterial(bool cornerPreview=false,bool edgeProposal=false) {
        if(cornerPreview&&!edgeProposal) {
            // Prepared from this generator at the native 3x material resolution.
            // Keep the approved photo pixels; no masks/noise/abrasions to build
            // on the message thread when the editor first opens.
            const auto atlas=juce::SoftwareImageType().convert(juce::ImageFileFormat::loadFrom(
                BinaryData::cheekspreparedv83_png,BinaryData::cheekspreparedv83_pngSize));
            if(atlas.isValid()&&atlas.getWidth()==width*scale*4&&atlas.getHeight()==height*scale*5) {
                for(int variant=0;variant<2;++variant)for(int side=0;side<2;++side)for(int stage=0;stage<5;++stage)
                    (variant?revisedImages:images)[size_t(side)][size_t(stage)]=atlas.getClippedImage(
                        {(variant*2+side)*width*scale,stage*height*scale,width*scale,height*scale});
                return;
            }
        }
        for(int side=0;side<2;++side)for(int endpoint=0;endpoint<1;++endpoint) {
            auto& image=images[size_t(side)][size_t(endpoint)];
            image=juce::Image(juce::Image::ARGB,width*scale,height*scale,true,juce::SoftwareImageType());
            juce::Image::BitmapData pixels(image,juce::Image::BitmapData::writeOnly);
            const auto seed=side?0x7e21a53bu:0x18b64cd9u;
            for(int y=0;y<image.getHeight();++y)for(int x=0;x<image.getWidth();++x) {
                const float px=(x+.5f)/scale,py=(y+.5f)/scale;
                const float qx=std::abs(px-width*.5f)-(width*.5f-12),qy=std::abs(py-height*.5f)-(height*.5f-12);
                const float dx=juce::jmax(qx,0.f),dy=juce::jmax(qy,0.f);
                const float distance=std::sqrt(dx*dx+dy*dy)+juce::jmin(juce::jmax(qx,qy),0.f)-12;
                const float alpha=juce::jlimit(0.f,1.f,.5f-distance*scale);
                if(alpha<=0)continue;
                const float grain=power_detail::smoothNoise(px*5.3f,py*5.3f,seed)-.5f;
                const float stipple=power_detail::smoothNoise(px*1.7f,py*1.7f,seed+17)-.5f;
                const float handling=power_detail::smoothNoise(px*.12f,py*.09f,seed+31)-.5f;
                const float broad=power_detail::smoothNoise(px*.026f,py*.018f,seed+73)-.5f;
                const float edge=px;
                const float inner=side?px:float(width)-px;
                const float edgeLight=std::exp(-std::pow((edge-6.5f)/2.6f,2.f));
                const float topLight=std::exp(-std::pow((py-6)/4.f,2.f));
                const float lowerFold=power_detail::smoothRange(height-12.f,height-1.f,py);
                float light=.122f+.030f*(1-px/width)+.025f*(1-py/height);
                light+=endpoint*(.058f+.045f*handling+.034f*broad);
                light+=grain*(.026f+endpoint*.085f)+stipple*(.014f+endpoint*.043f);
                light+=(side?.095f:.175f)*edgeLight+(.18f-endpoint*.13f)*topLight;
                light*=1-.36f*lowerFold;
                light*=.62f+.38f*power_detail::smoothRange(0,3,inner);
                pixels.setPixelColour(x,y,juce::Colour::fromFloatRGBA(
                    juce::jlimit(0.f,1.f,light*.99f),juce::jlimit(0.f,1.f,light*1.01f),juce::jlimit(0.f,1.f,light),alpha));
            }
        }
        const auto atlas=juce::SoftwareImageType().convert(juce::ImageFileFormat::loadFrom(
            BinaryData::cheekspersonalindependentv1_png,BinaryData::cheekspersonalindependentv1_pngSize));
        // Trace complete individual scratches and cuts, including the lower
        // halves. Each has a fixed birth stage; there is no final-stage catch-all.
        struct Mark {float x1,y1,x2,y2,radius;int birth;};
        const std::array<std::vector<Mark>,2> marks{{
            {{.14f,.048f,.30f,.085f,12.f,3}, {.90f,.21f,.98f,.212f,8.f,2},
             {.10f,.44f,.14f,.444f,7.f,1}, {.62f,.39f,.76f,.395f,4.5f,3},
             {.34f,.716f,.45f,.706f,5.f,2}, {.08f,.939f,.12f,.963f,7.f,3},
             {.10f,.966f,.28f,.987f,9.f,4},
             {.47f,.136f,.83f,.133f,2.5f,1}, {.14f,.260f,.38f,.250f,2.7f,2},
             {.15f,.315f,.82f,.365f,3.f,2}, {.15f,.375f,.73f,.433f,3.f,3},
             {.14f,.540f,.43f,.534f,2.5f,1}, {.33f,.526f,.46f,.519f,2.f,3},
             {.14f,.603f,.76f,.640f,3.f,3}, {.13f,.614f,.57f,.633f,2.5f,1},
             {.47f,.811f,.96f,.833f,2.7f,4}, {.13f,.865f,.32f,.859f,2.5f,2},
             {.31f,.946f,.54f,.951f,2.2f,3}, {.10f,.044f,.21f,.028f,7.f,4}},
            {{.88f,.11f,.98f,.114f,12.f,1}, {.96f,.27f,.98f,.274f,8.f,2},
             {.88f,.48f,.98f,.484f,8.f,3}, {.96f,.674f,.98f,.679f,8.f,2},
             {.51f,.052f,.61f,.082f,4.5f,2}, {.53f,.341f,.62f,.337f,2.5f,1},
             {.72f,.950f,.83f,.961f,6.f,3}, {.88f,.030f,.96f,.049f,9.f,3},
             {.89f,.018f,.97f,.025f,9.f,4}, {.75f,.978f,.94f,.966f,10.f,4},
             {.15f,.185f,.85f,.210f,2.8f,1}, {.20f,.331f,.94f,.383f,3.f,2},
             {.43f,.392f,.88f,.397f,2.5f,3}, {.56f,.434f,.87f,.430f,2.3f,1},
             {.10f,.522f,.50f,.555f,2.6f,3}, {.15f,.551f,.77f,.590f,2.8f,4},
             {.10f,.610f,.65f,.596f,2.7f,2}, {.18f,.670f,.70f,.731f,2.8f,3},
             {.50f,.751f,.91f,.737f,2.8f,1}, {.14f,.811f,.87f,.796f,2.8f,2},
             {.42f,.861f,.85f,.833f,2.5f,4}, {.35f,.903f,.80f,.877f,2.5f,3},
             {.30f,.941f,.51f,.952f,2.3f,2}}
        }};
        for(int side=0;side<2;++side) {
            juce::Image worn(juce::Image::ARGB,width*scale,height*scale,true,juce::SoftwareImageType());
            {juce::Graphics g(worn);g.drawImage(atlas.getClippedImage(side?juce::Rectangle<int>(704,15,132,1435):juce::Rectangle<int>(233,15,137,1435)),worn.getBounds().toFloat());}
            const juce::Image::BitmapData fresh(images[size_t(side)][0],juce::Image::BitmapData::readOnly);
            const juce::Image::BitmapData photograph(worn,juce::Image::BitmapData::readOnly);
            // Preserve the original photographic plastic, fine grain and
            // damage colour. Remove scheduled marks only from the undercoat;
            // reveal their original photo detail at each fixed birth stage.
            const int w=worn.getWidth(),h=worn.getHeight(),n=w*h;
            std::array<std::vector<float>,5> masks;
            std::array<std::vector<float>,5> retainedMasks;
            for(auto& mask:masks)mask.resize(size_t(n),0.f);
            for(auto& mask:retainedMasks)mask.resize(size_t(n),0.f);
            for(int y=0;y<h;++y)for(int x=0;x<w;++x) {
                const juce::Point<float> point((x+.5f)/scale,(y+.5f)/scale);
                for(const auto& mark:marks[size_t(side)]) {
                    const juce::Point<float> a(mark.x1*width,mark.y1*height),b(mark.x2*width,mark.y2*height);
                    const auto line=b-a;const float length=line.getDistanceSquaredFromOrigin();
                    const float along=length>0?juce::jlimit(0.f,1.f,((point.x-a.x)*line.x+(point.y-a.y)*line.y)/length):0.f;
                    const float distance=point.getDistanceFrom(a+line*along);
                    const float cover=1.f-power_detail::smoothRange(mark.radius-1.2f,mark.radius,distance);
                    for(int stage=mark.birth;stage<5;++stage) {
                        masks[size_t(stage)][size_t(y*w+x)]=juce::jmax(masks[size_t(stage)][size_t(y*w+x)],cover);
                        // Broad pale rubs are replaced in the proposal. Keep
                        // the separate deep cuts, edge chips and short scratches.
                        if(mark.radius>3.f||length<=25.f*25.f)
                            retainedMasks[size_t(stage)][size_t(y*w+x)]=juce::jmax(retainedMasks[size_t(stage)][size_t(y*w+x)],cover);
                    }
                }
            }
            // Copy intact photographic plastic from the same horizontal
            // position. Preserve real grain and the moulding's light profile;
            // do not smear a chip's shadow or edge through an averaged fill.
            std::vector<juce::Colour> undercoat(size_t(n),juce::Colours::transparentBlack);
            for(int y=0;y<h;++y)for(int x=0;x<w;++x) {
                const size_t at=size_t(y*w+x);
                if(masks[4][at]<=0){undercoat[at]=photograph.getPixelColour(x,y);continue;}
                int sy=-1;
                for(int offset:{137,-137,229,-229,317,-317,431,-431,613,-613}) {
                    const int candidate=y+offset;
                    if(candidate>60&&candidate<h-60&&masks[4][size_t(candidate*w+x)]<=0){sy=candidate;break;}
                }
                const auto original=fresh.getPixelColour(x,y);
                if(sy<0){undercoat[at]=original;continue;}
                const auto sample=photograph.getPixelColour(x,sy),reference=fresh.getPixelColour(x,sy);
                const float ratio=juce::jlimit(.15f,1.7f,original.getBrightness()/juce::jmax(.025f,reference.getBrightness()));
                const float tone=(sample.getFloatRed()+sample.getFloatGreen()+sample.getFloatBlue())/3.f;
                undercoat[at]=juce::Colour::fromFloatRGBA(juce::jlimit(0.f,1.f,tone*ratio*.99f),
                    juce::jlimit(0.f,1.f,tone*ratio*1.01f),juce::jlimit(0.f,1.f,tone*ratio),original.getFloatAlpha());
            }
            std::array<std::vector<float>,5> cornerDamage;
            if(cornerPreview)for(auto& mask:cornerDamage)mask.resize(size_t(n),0);
            std::vector<juce::Point<float>> cornerSources;
            for(const auto& mark:marks[size_t(side)]) {
                const juce::Point<float> a(mark.x1*width,mark.y1*height),b(mark.x2*width,mark.y2*height);
                auto source=(a+b)*.5f;
                if(cornerPreview&&((edgeProposal&&mark.radius>3.f)
                    ||(mark.radius>3.f&&(source.y<height*.09f||source.y>height*.93f)
                    &&(source.x<width*.35f||source.x>width*.70f)))) {
                    double weights=0,sx=0,sy=0;
                    const auto line=b-a;const float length=line.getDistanceSquaredFromOrigin();
                    for(int yy=juce::jmax(0,int((juce::jmin(a.y,b.y)-mark.radius)*scale));yy<juce::jmin(h,int((juce::jmax(a.y,b.y)+mark.radius)*scale));++yy)
                    for(int xx=juce::jmax(0,int((juce::jmin(a.x,b.x)-mark.radius)*scale));xx<juce::jmin(w,int((juce::jmax(a.x,b.x)+mark.radius)*scale));++xx) {
                        const juce::Point<float> point((xx+.5f)/scale,(yy+.5f)/scale);
                        const float along=length>0?juce::jlimit(0.f,1.f,((point.x-a.x)*line.x+(point.y-a.y)*line.y)/length):0;
                        if(point.getDistanceFrom(a+line*along)>mark.radius)continue;
                        const auto photo=photograph.getPixelColour(xx,yy),paint=undercoat[size_t(yy*w+xx)];
                        const float contrast=std::abs(photo.getBrightness()-paint.getBrightness());
                        const double weight=contrast*contrast*photo.getFloatAlpha();weights+=weight;sx+=point.x*weight;sy+=point.y*weight;
                    }
                    if(weights>0)source={float(sx/weights),float(sy/weights)};
                }
                cornerSources.push_back(source);
            }
            revisedImages[size_t(side)][0]=images[size_t(side)][0].createCopy();
            for(int variant=0;variant<2;++variant)for(int stage=1;stage<5;++stage) {
                auto& result=(variant?revisedImages:images)[size_t(side)][size_t(stage)];
                result=juce::Image(juce::Image::ARGB,width*scale,height*scale,true,juce::SoftwareImageType());
                juce::Image::BitmapData output(result,juce::Image::BitmapData::writeOnly);
                for(int y=0;y<result.getHeight();++y)for(int x=0;x<result.getWidth();++x) {
                    const auto clean=fresh.getPixelColour(x,y),photo=photograph.getPixelColour(x,y);
                    const size_t at=size_t(y*w+x);
                    const float all=masks[4][at],revealed=all>0?(variant?retainedMasks:masks)[size_t(stage)][at]/all:0.f;
                    const auto body=photo.interpolatedWith(undercoat[at],all);
                    const float coverage=.5f*(cornerPreview&&variant&&side==0&&y>h*.90f?clean.getFloatAlpha():photo.getFloatAlpha()),progress=float(stage)*.03125f;
                    const auto channel=[&](float c,float damaged,float paint) {
                        // Fire Hazard body tone reaches only the previous
                        // Seen Some Shit tone; damage remains independent.
                        // The separate damage term keeps every original mark,
                        // its full contrast and its original appearance stage.
                        return juce::jlimit(0.f,1.f,c+coverage*(progress*(paint-c)+revealed*(damaged-paint)));
                    };
                    // Full fixed-contrast local cuts are added one at a time.
                    output.setPixelColour(x,y,juce::Colour::fromFloatRGBA(
                        channel(clean.getFloatRed(),photo.getFloatRed(),body.getFloatRed()),
                        channel(clean.getFloatGreen(),photo.getFloatGreen(),body.getFloatGreen()),
                        channel(clean.getFloatBlue(),photo.getFloatBlue(),body.getFloatBlue()),clean.getFloatAlpha()));
                    if(variant) {
                        // Remove the original full-size cuts, then sample each
                        // photographic gouge at 2/3 size around its own centre.
                        // Scale its complete fracture and shadow, not just a mask.
                        const juce::Point<float> point((x+.5f)/scale,(y+.5f)/scale);
                        float dr=0,dg=0,db=0,best=0;
                        size_t markIndex=0;
                        for(const auto& mark:marks[size_t(side)]) {
                            const auto sourceCentre=cornerSources[markIndex++];
                            if(mark.birth>stage)continue;
                            const juce::Point<float> a(mark.x1*width,mark.y1*height),b(mark.x2*width,mark.y2*height);
                            const auto line=b-a;const float length=line.getDistanceSquaredFromOrigin();
                            if(mark.radius<=3.f&&length>625.f)continue;
                            const auto centre=(a+b)*.5f;
                            auto origin=centre,destination=centre;
                            // Move the complete reduced fracture, including its
                            // shadow, onto the rounded corner rather than near it.
                            if(cornerPreview&&mark.radius>3.f&&(centre.y<height*.09f||centre.y>height*.93f)) {
                                if(centre.x<width*.35f||centre.x>width*.70f)
                                    {origin=sourceCentre;destination={centre.x<width*.5f?6.f:width-6.f,centre.y<height*.5f?6.f:height-6.f};}
                            }
                            if(edgeProposal&&mark.radius>3.f) {
                                origin=sourceCentre;
                                // Only the large fractures move. The original
                                // small photographic scrapes stay in place.
                                const bool outerLeft=side==0;
                                if(centre.y<height*.09f||centre.y>height*.93f) {
                                    destination={outerLeft?6.f:width-6.f,centre.y<height*.5f?6.f:height-6.f};
                                    if(side==1&&centre.y>height*.93f)
                                        destination={width-3.5f,height-3.5f};
                                } else destination={outerLeft?1.25f:width-1.25f,sourceCentre.y};
                            }
                            // Retain the owner's chosen vertical scoring on
                            // the upper right face at its original position.
                            if(edgeProposal&&side==1&&markIndex==5) {
                                origin=centre;destination=centre;
                            }
                            const auto q=mark.radius>3.f?origin+(point-destination)*1.5f:point;
                            const float along=length>0?juce::jlimit(0.f,1.f,((q.x-a.x)*line.x+(q.y-a.y)*line.y)/length):0.f;
                            const float cover=1.f-power_detail::smoothRange(mark.radius-1.2f,mark.radius,q.getDistanceFrom(a+line*along));
                            const int sx=int(q.x*scale),sy=int(q.y*scale);
                            if(cover<=0||sx<0||sy<0||sx>=w||sy>=h)continue;
                            const size_t source=size_t(sy*w+sx);
                            const auto sample=photograph.getPixelColour(sx,sy),paint=undercoat[source];
                            const float score=cornerPreview?cover*std::abs(sample.getBrightness()-paint.getBrightness()):cover;
                            if(score<=best)continue;
                            const float amount=cover*masks[4][source];best=score;
                            if(cornerPreview)cornerDamage[size_t(stage)][at]=cover;
                            dr=(sample.getFloatRed()-paint.getFloatRed())*amount;
                            dg=(sample.getFloatGreen()-paint.getFloatGreen())*amount;
                            db=(sample.getFloatBlue()-paint.getFloatBlue())*amount;
                        }
                        const auto c=[&](float freshChannel,float paintChannel,float delta) {
                            return juce::jlimit(0.f,1.f,freshChannel+coverage*(progress*(paintChannel-freshChannel)+delta));
                        };
                        output.setPixelColour(x,y,juce::Colour::fromFloatRGBA(
                            c(clean.getFloatRed(),body.getFloatRed(),dr),c(clean.getFloatGreen(),body.getFloatGreen(),dg),
                            c(clean.getFloatBlue(),body.getFloatBlue(),db),clean.getFloatAlpha()));
                    }
                }
            }
            // Narrow reflected bands on the molded edges. Keep the body dark
            // and leave every revealed photographic scratch/cut unchanged.
            for(int variant=0;variant<2;++variant)for(int stage=0;stage<5;++stage) {
                juce::Image::BitmapData pixels((variant?revisedImages:images)[size_t(side)][size_t(stage)],juce::Image::BitmapData::readWrite);
                for(int y=0;y<h;++y)for(int x=0;x<w;++x) {
                    const auto colour=pixels.getPixelColour(x,y);
                    if(colour.isTransparent())continue;
                    const float px=(x+.5f)/scale,py=(y+.5f)/scale;
                    const float curved=py<12.f?12.f-std::sqrt(juce::jmax(0.f,144.f-(py-12.f)*(py-12.f))):0.f;
                    const float edge=px-curved;
                    const float vertical=std::exp(-std::pow((edge-5.65f)/.65f,2.f));
                    const float core=std::exp(-std::pow((edge-5.42f)/.22f,2.f));
                    const float end=power_detail::smoothRange(1.f,11.f,py)*(1.f-power_detail::smoothRange(height-24.f,height-3.f,py));
                    const float top=std::exp(-std::pow((py-4.1f)/.65f,2.f))*power_detail::smoothRange(10.f,17.f,px)
                        *(1.f-power_detail::smoothRange(width-16.f,width-5.f,px));
                    const float focus=.70f+.30f*power_detail::smoothNoise(py*.024f,0,seedForSide(side));
                    const float damage=(cornerPreview&&variant?cornerDamage:(variant?retainedMasks:masks))[size_t(stage)][size_t(y*w+x)];
                    const float reflection=((side?.085f:.145f)*vertical+.045f*core)*end*focus+.13f*top;
                    const float lift=reflection*(1.f-damage);
                    pixels.setPixelColour(x,y,juce::Colour::fromFloatRGBA(
                        juce::jmin(1.f,colour.getFloatRed()+lift*.99f),juce::jmin(1.f,colour.getFloatGreen()+lift),
                        juce::jmin(1.f,colour.getFloatBlue()+lift*1.025f),colour.getFloatAlpha()));
                }
            }
            // Same fine, curved abrasions as the black-key material: fixed
            // paths, low contrast, rounded ends and equal additions per age.
            for(int stage=1;stage<5;++stage) {
                juce::Graphics g(revisedImages[size_t(side)][size_t(stage)]);
                g.addTransform(juce::AffineTransform::scale(float(scale)));
                juce::Path outline;outline.addRoundedRectangle(0,0,float(width),float(height),12);g.reduceClipRegion(outline);
                const uint32_t seed=seedForSide(side)^0x9c1721u;
                const auto random=[](uint32_t key) {
                    key^=key>>16;key*=0x7feb352du;key^=key>>15;key*=0x846ca68bu;key^=key>>16;
                    return float(key&65535u)/65535.f;
                };
                for(uint32_t scratch=0;scratch<uint32_t(40*stage);++scratch) {
                    const float x=width*(.14f+.68f*random(seed+scratch*127));
                    const float y=height*(.04f+.90f*random(seed+scratch*419));
                    const float dx=(random(seed+scratch*199)-.5f)*16.f,dy=1.f+9.f*random(seed+scratch*733);
                    juce::Path trace;trace.startNewSubPath(x,y);trace.quadraticTo(x+dx*.2f+1,y+dy*.5f,x+dx,y+dy);
                    g.setColour(juce::Colour(0xffb7b9ae).withAlpha(.12f+.12f*random(seed+scratch*37)));
                    g.strokePath(trace,juce::PathStrokeType(.11f+.08f*random(seed+scratch*59),juce::PathStrokeType::curved,juce::PathStrokeType::rounded));
                }
                for(uint32_t i=0;i<uint32_t((cornerPreview?88:52)*stage);++i) {
                    const uint32_t key=seed^0x45a721u;
                    const float x=5.f+(width-10.f)*random(key+i*127),y=12.f+(height-24.f)*random(key+i*419);
                    const float dx=(random(key+i*199)-.5f)*3.4f,dy=.6f+3.2f*random(key+i*733);
                    juce::Path scratch;scratch.startNewSubPath(x,y);scratch.quadraticTo(x+dx*.4f,y+dy*.6f,x+dx,y+dy);
                    g.setColour(juce::Colour(0xff080b0b).withAlpha(.22f));
                    g.strokePath(scratch,juce::PathStrokeType(.20f,juce::PathStrokeType::curved,juce::PathStrokeType::rounded));
                    g.setColour(juce::Colour(0xffbdc1ba).withAlpha(.10f+.08f*random(key+i*59)));
                    g.strokePath(scratch,juce::PathStrokeType(.075f,juce::PathStrokeType::curved,juce::PathStrokeType::rounded),juce::AffineTransform::translation(-.08f,-.07f));
                }
                if(edgeProposal)for(uint32_t serial=0;serial<uint32_t(stage*16);++serial) {
                    // Shallow irregular scoring: a dark micro-groove and one
                    // softly lit edge, with tapered, interrupted ends. Fixed
                    // birth order prevents scratches jumping when age changes.
                    const uint32_t key=seed^0x8ba317u;
                    const float x=width*(.12f+.76f*random(key+serial*127));
                    const float y=height*(.025f+.95f*random(key+serial*419));
                    const float length=3.f+15.f*random(key+serial*199);
                    const float angle=-1.25f+2.5f*random(key+serial*733);
                    const float dx=std::sin(angle)*length,dy=std::cos(angle)*length;
                    const float bend=(random(key+serial*337)-.5f)*2.4f;
                    const float width=.19f+.22f*random(key+serial*59);
                    const float contrast=.20f+.21f*random(key+serial*37);
                    auto point=[&](float t){return juce::Point<float>(x+dx*t+bend*std::sin(t*juce::MathConstants<float>::pi),y+dy*t);};
                    for(uint32_t segment=0;segment<9;++segment) {
                        if(random(key+serial*997+segment*73)>.90f)continue;
                        const float t0=float(segment)/9,t1=float(segment+1)/9;
                        const float taper=std::pow(std::sin((t0+t1)*.5f*juce::MathConstants<float>::pi),.6f);
                        const auto a=point(t0),b=point(t1);
                        g.setColour(juce::Colour(0xff080a09).withAlpha(.26f*taper));g.drawLine({a,b},width+.15f);
                        g.setColour(juce::Colour(0xff9fa49c).withAlpha(contrast*taper));
                        g.drawLine({a.translated(-.10f,-.07f),b.translated(-.10f,-.07f)},width*taper);
                    }
                }
                if(cornerPreview)for(uint32_t rub=0;rub<uint32_t(stage*2);++rub) {
                    // A few broad, shallow handling rubs: many interrupted
                    // hairlines form a soft patch, without a gouge's dark wall
                    // or a single pale crayon-like stroke.
                    const uint32_t key=seed^0x176dc9u;

                    const float x=width*(.32f+.34f*random(key+rub*127));
                    const float y=height*(.09f+.82f*random(key+rub*419));
                    const float length=7.f+19.f*random(key+rub*199),breadth=1.4f+4.1f*random(key+rub*733);
                    const float angle=-.65f+1.3f*random(key+rub*331)+(side?.18f:0.f);
                    const auto point=[&](float u,float v){return juce::Point<float>(x+u*std::cos(angle)-v*std::sin(angle),y+u*std::sin(angle)+v*std::cos(angle));};
                    for(uint32_t track=0;track<24;++track) {
                        const float across=(float(track)+.5f)/12.f-1.f;
                        const float span=length*.5f*std::sqrt(juce::jmax(0.f,1.f-across*across));
                        const float offset=across*breadth*.5f;
                        const float trim=.12f+.32f*random(key+rub*997+track*79);
                        juce::Path scuff;scuff.startNewSubPath(point(-span,offset));
                        scuff.quadraticTo(point(-span*trim,offset+.18f),point(span*(1-trim),offset));
                        g.setColour(juce::Colour(0xffa7aaa3).withAlpha(.035f+.045f*random(key+rub*131+track*37)));
                        g.strokePath(scuff,juce::PathStrokeType(.10f+.05f*random(key+track*59),juce::PathStrokeType::curved,juce::PathStrokeType::rounded));
                    }
                }
            }
        }
    }
    bool exportPrepared(const juce::File& target,bool enhanced=false) const {
        juce::Image atlas(juce::Image::ARGB,width*scale*4,height*scale*5,true,juce::SoftwareImageType());
        juce::Graphics g(atlas);
        for(int variant=0;variant<2;++variant)for(int side=0;side<2;++side)for(int stage=0;stage<5;++stage) {
            const auto& source=enhanced&&variant&&stage>0?contrastImages[size_t(side)][size_t(stage)]
                :(variant?revisedImages:images)[size_t(side)][size_t(stage)];
            if(!source.isValid())return false;
            g.drawImageAt(source,(variant*2+side)*width*scale,stage*height*scale);
        }
        target.deleteFile();auto stream=target.createOutputStream();
        return stream&&juce::PNGImageFormat().writeImageToStream(atlas,*stream);
    }
    void paint(juce::Graphics& g,int side,float age,float y,float totalWidth,bool revised=false,bool contrastWear=false,float lowerFoldLift=0.f) const {
        juce::Graphics::ScopedSaveState save(g);
        const float x=side?totalWidth-width:0.f;
        const juce::Rectangle<float> bounds(x,y,float(width),float(height));
        g.setColour(juce::Colour(0xff0a0d0e));g.fillRect(bounds);
        juce::Path outline;outline.addRoundedRectangle(bounds,12);g.reduceClipRegion(outline);
        const int stage=appearanceStage(age);
        const auto& source=(revised?revisedImages:images)[size_t(side)];
        const juce::Image* surface=&source[size_t(stage)];
        if(contrastWear&&revised&&stage>0) {
            std::lock_guard<std::mutex> guard(contrastMutex);
            auto& enhanced=contrastImages[size_t(side)][size_t(stage)];
            if(!enhanced.isValid()) {
                enhanced=surface->createCopy();
                juce::Image::BitmapData fresh(source[0],juce::Image::BitmapData::readOnly),
                    worn(*surface,juce::Image::BitmapData::readOnly),output(enhanced,juce::Image::BitmapData::writeOnly);
                for(int yy=0;yy<enhanced.getHeight();++yy)for(int xx=0;xx<enhanced.getWidth();++xx) {
                    const auto a=fresh.getPixelColour(xx,yy),b=worn.getPixelColour(xx,yy);
                    const auto contrast=[](float clean,float aged){return juce::jlimit(0.f,1.f,clean+(aged-clean)*1.65f);};
                    output.setPixelColour(xx,yy,juce::Colour::fromFloatRGBA(contrast(a.getFloatRed(),b.getFloatRed()),
                        contrast(a.getFloatGreen(),b.getFloatGreen()),contrast(a.getFloatBlue(),b.getFloatBlue()),b.getFloatAlpha()));
                }
            }
            surface=&enhanced;
        }
        g.setOpacity(1);g.drawImage(*surface,bounds);
        if(lowerFoldLift>0.f) {
            // Raise only the lower moulding's shading transition. Preserve the
            // photographed grain, damage placement and rounded silhouette.
            const float start=height-12.f-lowerFoldLift;
            for(int row=int(start*scale);row<height*scale;++row) {
                const float py=(row+.5f)/scale;
                const float oldFold=power_detail::smoothRange(height-12.f,height-1.f,py);
                const float newFold=power_detail::smoothRange(start,height-1.f,py);
                const float opacity=1.f-(1.f-.36f*newFold)/(1.f-.36f*oldFold);
                g.setColour(juce::Colours::black.withAlpha(juce::jlimit(0.f,1.f,opacity)));
                g.fillRect(x,y+float(row)/scale,float(width),1.f/scale);
            }
        }
        if(side) {g.setColour(juce::Colours::black.withAlpha(.12f));g.fillPath(outline);}
        // The independently generated photo-informed endpoint already holds
        // each cheek's cuts, scuffs and larger corner damage. Do not stamp a
        // second native damage pattern over it or mirror either crop.

    }
private:
    static uint32_t seedForSide(int side) {return side?0x7e21a53bu:0x18b64cd9u;}
    std::array<std::array<juce::Image,5>,2> images;
    std::array<std::array<juce::Image,5>,2> revisedImages;
    mutable std::mutex contrastMutex;
    mutable std::array<std::array<juce::Image,5>,2> contrastImages;
};
inline const CheekMaterial& cheekMaterial(bool corners=false) {
    if(corners){static const CheekMaterial proposal(true);return proposal;}
    static const CheekMaterial material;return material;
}
inline const CheekMaterial& cheekEdgeProposal() {
    static const CheekMaterial material(true,true);return material;
}

// A separate recessed joint leaves the moulded cheek above the sheet-metal
// panel. Dust is confined to the top-panel section, not the lower bay/keybed.
inline void paintLowerBayJointDust(juce::Graphics& g,float age,float top,float synthWidth) {
    if(age<=0)return;
    static const auto powder=[] {
        std::array<juce::Image,2> images;
        for(int side=0;side<2;++side) {
            auto& image=images[size_t(side)];image=juce::Image(juce::Image::ARGB,20,370*4,true,juce::SoftwareImageType());
            juce::Image::BitmapData pixels(image,juce::Image::BitmapData::writeOnly);
            const uint32_t seed=side?29417u:13759u;
            for(int y=0;y<image.getHeight();++y)for(int x=0;x<20;++x) {
                const float px=(x+.5f)/4,py=(y+.5f)/4;
                const float trough=std::exp(-std::pow((px-2.f)/1.f,4.f));
                const float coarse=power_detail::smoothNoise(px*1.8f,py*.085f,seed);
                const float grain=power_detail::smoothNoise(px*13,py*13,seed+917u);
                const float fade=1-power_detail::smoothRange(side?362.f:340.f,side?370.f:348.f,py);
                const float alpha=trough*(.38f+.45f*coarse)*(.40f+.60f*grain)*fade;
                const float value=.34f+.13f*grain;
                pixels.setPixelColour(x,y,juce::Colour::fromFloatRGBA(value,value*.91f,value*.75f,alpha));
            }
        }
        return images;
    }();
    juce::Graphics::ScopedSaveState save(g);g.setOpacity(std::pow(age,1.25f));
    g.drawImage(powder[0],{54.25f,top+362.f,5.f,370.f});
    g.addTransform(juce::AffineTransform::scale(-1.f,1.f).translated(synthWidth,0));
    g.drawImage(powder[1],{54.25f,top+362.f,5.f,370.f});
}
inline void paintCheekJoint(juce::Graphics& g,float age,float top,float synthWidth) {
    static const auto joints=[] {
        std::array<std::array<juce::Image,5>,2> result;
        constexpr int scale=4;
        for(int side=0;side<2;++side)for(int stage=0;stage<5;++stage) {
            auto& image=result[size_t(side)][size_t(stage)];
            image=juce::Image(juce::Image::ARGB,20,732*scale,true,juce::SoftwareImageType());
            juce::Image::BitmapData pixels(image,juce::Image::BitmapData::writeOnly);
            const uint32_t seed=side?29417u:13759u;
            const float wear=stage*.25f;
            for(int y=0;y<image.getHeight();++y)for(int x=0;x<20;++x) {
                const float px=(x+.5f)/scale,py=(y+.5f)/scale;
                const float across=side?5.f-px:px;
                const float endFade=1.f-power_detail::smoothRange(722.f,730.f,py);
                // Thin raised cheek lip, deep slot, then a soft panel contact edge.
                const float lip=std::exp(-std::pow((across-.65f)/.23f,2.f));
                const float trough=std::exp(-std::pow((across-2.0f)/1.0f,4.f));
                const float panelShadow=std::exp(-std::pow((across-3.1f)/.9f,2.f));
                const float patches=power_detail::smoothNoise(across*1.8f,py*.085f,seed);
                const float grains=power_detail::smoothNoise(across*13.f,py*13.f,seed+917u);
                const float fine=power_detail::smoothNoise(across*5.1f,py*4.9f,seed+811u);
                const float topOnly=1.f-power_detail::smoothRange(362.f,365.f,py);
                const float deposit=wear*topOnly*trough*power_detail::smoothRange(.28f,.70f,patches)
                    *(.24f+.76f*power_detail::smoothRange(.37f,.72f,grains));
                float light=.021f+.15f*lip+.022f*panelShadow;
                light+=(fine-.5f)*.009f;
                auto colour=juce::Colour::fromFloatRGBA(light,light*1.01f,light,.92f*endFade);
                const auto dust=juce::Colour::fromFloatRGBA(.42f,.37f,.28f,.92f*endFade);
                colour=colour.interpolatedWith(dust,juce::jlimit(0.f,.8f,deposit*1.18f));
                const float edgeFade=1.f-power_detail::smoothRange(3.5f,5.f,across);
                pixels.setPixelColour(x,y,colour.withMultipliedAlpha(edgeFade));
            }
        }
        return result;
    }();
    const int stage=appearanceStage(age);
    // The recessed slot reaches the top edge on both sides. Do not expose
    // the photograph's pale panel bevel through a transparent upper fade.
    g.setColour(juce::Colour(0xff050606));
    g.fillRect(55.25f,top,2.6f,724.f);
    g.fillRect(synthWidth-57.85f,top,2.6f,724.f);
    g.drawImage(joints[0][size_t(stage)],{54.25f,top,5.f,732.f});
    g.drawImage(joints[1][size_t(stage)],{synthWidth-59.25f,top,5.f,732.f});
}

}
