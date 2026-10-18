#pragma once
#include <juce_graphics/juce_graphics.h>
#include <array>
#include <vector>
#include <cmath>

namespace kb09_gui {
// Transport the approved photographic rib motif around a cylinder. The fixed
// illumination field is separate from angular material coordinates, so neither
// the metal face nor the camera perspective rotates with the grip texture.
class KnobGripMotion {
public:
    explicit KnobGripMotion(const juce::Image& photograph,bool largerFace=false):photo(photograph),pixels(photo,juce::Image::BitmapData::readOnly) {
        for(int y=0;y<rows;++y) {
            const float t=(y+.5f)/rows;
            auto mean=average(0,t);
            for(int x=0;x<columns;++x) {
                const float theta=((x+.5f)/columns-.5f)*period;
                auto p=sample(theta,t);
                tile[size_t(y*columns+x)]={p.r/juce::jmax(.018f,mean.r),p.g/juce::jmax(.018f,mean.g),p.b/juce::jmax(.018f,mean.b)};
            }
        }
        for(int y=0;y<size;++y)for(int x=0;x<size;++x) {
            const float u=(x+.5f)/size,v=(y+.5f)/size,s=(u-.497f)/.326f;
            const float ry=(v-.410f)/.315f,ring=std::sqrt(s*s+ry*ry);
            const float silverFace=std::hypot((u-.497f)/.267f,(v-.407f)/.277f);
            if(ring>.785f&&ring<1.f&&(!largerFace||silverFace>1.012f)) {
                const float alpha=juce::jlimit(0.f,1.f,(ring-.785f)/.035f)*juce::jlimit(0.f,1.f,(1.f-ring)/.035f)
                    *(largerFace?juce::jlimit(0.f,1.f,(silverFace-1.012f)/.015f):1.f);
                rimPoints.push_back({x,y,alpha,at(u,v)});
            }
            if(std::abs(s)>=.998f)continue;
            const float theta=std::asin(s),top=.410f+.315f*std::cos(theta),t=(v-top)/.078f;
            if(t<0||t>1)continue;
            const float alpha=juce::jlimit(0.f,1.f,t*24.f)*juce::jlimit(0.f,1.f,(1-t)*24.f);
            points.push_back({x,y,theta,t,alpha,average(theta,t)});
        }
    }
    const juce::Image& frame(float angle,size_t slot,float wear=0.f) {
        auto& cache=caches[slot];
        wear=juce::jlimit(0.f,1.f,wear);
        const float phase=std::remainder(angle,period);
        if(cache.image.isValid()&&std::abs(cache.phase-phase)<.00001f&&cache.wear==wear)return cache.image;
        cache.phase=phase;cache.wear=wear;cache.image=juce::Image(juce::Image::ARGB,size,size,true,juce::SoftwareImageType());
        juce::Image::BitmapData output(cache.image,juce::Image::BitmapData::writeOnly);
        const float amount=.96f*std::pow(wear,1.05f);
        for(const auto& p:rimPoints)if(amount>0) {
            const auto c=matte(p.colour);
            output.setPixelColour(p.x,p.y,juce::Colour::fromFloatRGBA(c.r,c.g,c.b,p.alpha*amount));
        }
        for(const auto& p:points) {
            // Cylinder azimuth grows opposite to the clockwise top pointer.
            float u=(p.theta+phase)/period+.5f;u-=std::floor(u);
            const auto texture=textureAt(u,p.t);
            const RGB original{juce::jlimit(0.f,1.f,texture.r*p.light.r),juce::jlimit(0.f,1.f,texture.g*p.light.g),juce::jlimit(0.f,1.f,texture.b*p.light.b)};
            const auto c=mix(original,matte(original),amount);
            output.setPixelColour(p.x,p.y,juce::Colour::fromFloatRGBA(c.r,c.g,c.b,p.alpha));
        }
        return cache.image;
    }
private:
    struct RGB {float r,g,b;};
    static RGB matte(RGB c) {
        // Compress bright reflected peaks while retaining the dark grooves and
        // photographic rib profile. Only the black grip material is sampled.
        const float peak=juce::jmax(c.r,c.g,c.b);
        const float target=peak>.13f?.13f+.16f*(peak-.13f):peak;
        const float scale=peak>0?target/peak:1.f;
        return {c.r*scale,c.g*scale,c.b*scale};
    }
    static RGB mix(RGB a,RGB b,float t){return {a.r+(b.r-a.r)*t,a.g+(b.g-a.g)*t,a.b+(b.b-a.b)*t};}
    RGB at(float u,float v) const {
        const float x=juce::jlimit(0.f,float(photo.getWidth()-2),u*photo.getWidth()-.5f),y=juce::jlimit(0.f,float(photo.getHeight()-2),v*photo.getHeight()-.5f);
        const int ix=int(x),iy=int(y);
        const auto get=[&](int xx,int yy){const auto c=pixels.getPixelColour(xx,yy);return RGB{c.getFloatRed(),c.getFloatGreen(),c.getFloatBlue()};};
        return mix(mix(get(ix,iy),get(ix+1,iy),x-ix),mix(get(ix,iy+1),get(ix+1,iy+1),x-ix),y-iy);
    }
    RGB sample(float theta,float t) const {return at(.497f+.326f*std::sin(theta),.410f+.315f*std::cos(theta)+t*.078f);}
    RGB average(float theta,float t) const {
        RGB result{};
        for(int n=0;n<25;++n){const auto p=sample(juce::jlimit(-1.53f,1.53f,theta+period*(float(n)/24.f-.5f)),t);result.r+=p.r/25;result.g+=p.g/25;result.b+=p.b/25;}
        return result;
    }
    RGB textureAt(float u,float v) const {
        const float x=u*columns-.5f,y=juce::jlimit(0.f,float(rows-1),v*rows-.5f);
        const int ix=int(std::floor(x)),iy=int(y),x0=(ix+columns)%columns,x1=(x0+1)%columns,y1=juce::jmin(rows-1,iy+1);
        return mix(mix(tile[size_t(iy*columns+x0)],tile[size_t(iy*columns+x1)],x-ix),
                   mix(tile[size_t(y1*columns+x0)],tile[size_t(y1*columns+x1)],x-ix),y-iy);
    }
    static constexpr int size=384,columns=192,rows=64;
    static constexpr float period=juce::MathConstants<float>::twoPi/12.f;
    struct Point {int x,y;float theta,t,alpha;RGB light;};
    struct RimPoint {int x,y;float alpha;RGB colour;};
    struct Cache {float phase=0,wear=-1;juce::Image image;};
    juce::Image photo;
    juce::Image::BitmapData pixels;
    std::array<RGB,columns*rows> tile{};
    std::vector<Point> points;
    std::vector<RimPoint> rimPoints;
    std::array<Cache,3> caches;
};
}
