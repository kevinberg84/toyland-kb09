#pragma once
#include <juce_graphics/juce_graphics.h>
#include <array>
#include <vector>
#include <cmath>

namespace kb09_gui {
// Registered photographic rust is revealed by a spatial growth front. Each
// connected site has its own onset; existing rust never fades back or relocates.
inline std::array<juce::Image,5> makePanelWearStages(const juce::Image& photo,const juce::Image& clean) {
    constexpr int cell=2;
    const int w=(photo.getWidth()+1)/cell,h=(photo.getHeight()+1)/cell,n=w*h;
    std::vector<unsigned char> warm(size_t(n),0),zone(size_t(n),0);
    std::vector<int> owner(size_t(n),-1),distance(size_t(n),0),queue;
    std::vector<float> arrival(size_t(n),2.f);
    const juce::Image::BitmapData source(photo,juce::Image::BitmapData::readOnly);
    const juce::Image::BitmapData fresh(clean,juce::Image::BitmapData::readOnly);
    const auto cleanAt=[&](int x,int y){return fresh.getPixelColour(x,y);};
    const auto neutralAt=[&](int x,int y) {
        const float t=juce::jlimit(0.f,1.f,(y-12.f)/20.f);
        return cleanAt(x,y).interpolatedWith(source.getPixelColour(x,y),t*t*(3-2*t));
    };
    for(int y=0;y<h;++y)for(int x=0;x<w;++x) {
        const auto c=source.getPixelColour(juce::jmin(photo.getWidth()-1,x*cell),juce::jmin(photo.getHeight()-1,y*cell));
        const float r=c.getFloatRed(),g=c.getFloatGreen(),b=c.getFloatBlue();
        warm[size_t(y*w+x)]=r>.13f&&r>g*1.11f&&r>b*1.20f;
    }
    // Include the chipped paint and dark lip around the oxide, so the entire
    // lesion grows together rather than leaving a faint full-sized outline.
    for(int y=0;y<h;++y)for(int x=0;x<w;++x)if(warm[size_t(y*w+x)])
        for(int dy=-2;dy<=2;++dy)for(int dx=-2;dx<=2;++dx)
            if(x+dx>=0&&x+dx<w&&y+dy>=0&&y+dy<h)zone[size_t((y+dy)*w+x+dx)]=1;
    // Reconstruct paint changes relative to the clean photograph's lighting.
    // Interpolating raw colours spreads edge highlights into erased rust sites.
    // Without this layer, withholding a site also withholds the surrounding
    // patina and leaves conspicuous square holes in the aged paint texture.
    std::vector<std::array<float,3>> paint(size_t(n),std::array<float,3>{});
    std::vector<unsigned char> filled(size_t(n),0);
    for(int at=0;at<n;++at)if(!zone[size_t(at)]) {
        const int x=juce::jmin(photo.getWidth()-1,(at%w)*cell),y=juce::jmin(photo.getHeight()-1,(at/w)*cell);
        const auto old=neutralAt(x,y),original=cleanAt(x,y);
        paint[size_t(at)]={old.getFloatRed()-original.getFloatRed(),old.getFloatGreen()-original.getFloatGreen(),old.getFloatBlue()-original.getFloatBlue()};filled[size_t(at)]=1;
    }
    const auto adjacent=[&](int at,const auto& visit) {
        const int x=at%w,y=at/w;
        if(x)visit(at-1);if(x+1<w)visit(at+1);if(y)visit(at-w);if(y+1<h)visit(at+w);
    };
    queue.clear();
    for(int at=0;at<n;++at)if(zone[size_t(at)]) {
        bool edge=false;adjacent(at,[&](int next){edge|=!zone[size_t(next)];});
        if(edge)queue.push_back(at);
    }
    for(size_t i=0;i<queue.size();++i) {
        const int at=queue[i];if(filled[size_t(at)])continue;
        float r=0,g=0,b=0;int count=0;
        adjacent(at,[&](int next){if(filled[size_t(next)]){const auto& c=paint[size_t(next)];r+=c[0];g+=c[1];b+=c[2];++count;}});
        if(!count)continue;
        paint[size_t(at)]={r/count,g/count,b/count};filled[size_t(at)]=1;
        adjacent(at,[&](int next){if(zone[size_t(next)]&&!filled[size_t(next)])queue.push_back(next);});
    }
    int component=0;
    const auto neighbours=[&](int at,const auto& visit) {
        const int x=at%w,y=at/w;
        if(x)visit(at-1);if(x+1<w)visit(at+1);if(y)visit(at-w);if(y+1<h)visit(at+w);
    };
    for(int start=0;start<n;++start)if(zone[size_t(start)]&&owner[size_t(start)]<0) {
        queue.clear();queue.push_back(start);owner[size_t(start)]=component;
        int seed=start;float strongest=-1;
        for(size_t i=0;i<queue.size();++i) {
            const int at=queue[i];
            const auto c=source.getPixelColour(juce::jmin(photo.getWidth()-1,(at%w)*cell),juce::jmin(photo.getHeight()-1,(at/w)*cell));
            const float oxide=c.getFloatRed()-c.getFloatBlue();
            if(oxide>strongest){strongest=oxide;seed=at;}
            neighbours(at,[&](int next){if(zone[size_t(next)]&&owner[size_t(next)]<0){owner[size_t(next)]=component;queue.push_back(next);}});
        }
        const auto members=queue;
        for(int at:members)distance[size_t(at)]=-1;
        queue.clear();queue.push_back(seed);distance[size_t(seed)]=0;int maximum=1;
        for(size_t i=0;i<queue.size();++i) {
            const int at=queue[i];
            neighbours(at,[&](int next){if(owner[size_t(next)]==component&&distance[size_t(next)]<0){
                distance[size_t(next)]=distance[size_t(at)]+1;maximum=juce::jmax(maximum,distance[size_t(next)]);queue.push_back(next);}});
        }
        unsigned int key=unsigned(seed)*7919u+731u;key^=key>>16;key*=0x7feb352du;key^=key>>15;
        const float onset=.025f+.56f*float(key&65535u)/65535.f;
        for(int at:members)arrival[size_t(at)]=onset+(.98f-onset)*std::pow(float(distance[size_t(at)])/maximum,.8f);
        ++component;
    }
    std::array<juce::Image,5> stages;
    for(int stage=0;stage<5;++stage) {
        auto& image=stages[size_t(stage)];image=juce::Image(juce::Image::ARGB,photo.getWidth(),photo.getHeight(),true,juce::SoftwareImageType());
        if(!stage)continue;
        juce::Image::BitmapData output(image,juce::Image::BitmapData::writeOnly);
        const float age=stage*.25f,patina=.9f*std::pow(age,.85f);
        for(int y=0;y<photo.getHeight();++y)for(int x=0;x<photo.getWidth();++x) {
            const int at=(y/cell)*w+x/cell;
            float coverage=patina;
            auto colour=neutralAt(x,y);
            if(zone[size_t(at)]) {
                // Only the narrow spatial growth front is antialiased. The
                // established rust has its final colour/opacity from birth.
                const float growth=stage==4?1.f:juce::jlimit(0.f,1.f,(age-arrival[size_t(at)])/.012f+.5f);
                coverage=patina+(.9f-patina)*growth;
                const auto original=cleanAt(x,y);const auto& residual=paint[size_t(at)];
                const auto uncorroded=juce::Colour::fromFloatRGBA(
                    juce::jlimit(0.f,1.f,original.getFloatRed()+residual[0]),
                    juce::jlimit(0.f,1.f,original.getFloatGreen()+residual[1]),
                    juce::jlimit(0.f,1.f,original.getFloatBlue()+residual[2]),original.getFloatAlpha());
                colour=uncorroded.interpolatedWith(source.getPixelColour(x,y),growth);
            }
            output.setPixelColour(x,y,colour.withMultipliedAlpha(coverage));
        }
    }
    return stages;
}
}
