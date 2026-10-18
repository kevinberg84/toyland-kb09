#pragma once
#include <juce_graphics/juce_graphics.h>
#include "AgeRasterCache.h"
#include "HardwareShadows.h"
#include <array>
#include <vector>
#include <cmath>
#include <limits>

namespace kb09_gui {
// Photo-informed moulding: a broad panel-side base, curved taper, narrow
// raised face and an aluminium inlay continuing down both end walls.
class FaderCapModel {
public:
    void draw(juce::Graphics& g,juce::Point<float> index,float width,float height,float age,uint32_t seed) {
        const Shape shape(width,height);
        const auto key=juce::String(seed)+":"+juce::String(age,5)+":"+juce::String(width,4)+":"+juce::String(height,4);
        auto image=cache.find(key);
        if(!image.isValid()) {image=render(shape,age,seed);cache.put(key,image);}
        // Average the supersampled material into destination-sized pixels
        // before translating it. Direct 6x reduction samples isolated texels:
        // the thin fold highlight can disappear at some slider positions.
        const float scale=juce::jlimit(1.f,6.f,g.getInternalContext().getPhysicalPixelScaleFactor());
        const auto filteredKey=key+":pixels:"+juce::String(scale,5);
        auto filtered=cache.find(filteredKey);
        if(!filtered.isValid()) {
            filtered=areaReduced(image,juce::roundToInt(shape.bounds.getWidth()*scale),juce::roundToInt(shape.bounds.getHeight()*scale));
            cache.put(filteredKey,filtered);
        }
        // This is already a display-resolution sprite. Translate its pixels
        // without re-filtering the highlight at every fractional position.
        g.setOpacity(1);g.setImageResamplingQuality(juce::Graphics::lowResamplingQuality);
        g.drawImageTransformed(filtered,juce::AffineTransform::scale(1.f/scale)
            .translated(shape.bounds.getX()+index.x,shape.bounds.getY()+index.y));
    }
private:
    static juce::Image areaReduced(const juce::Image& source,int width,int height) {
        juce::Image result(juce::Image::ARGB,width,height,true,juce::SoftwareImageType());
        const juce::Image::BitmapData input(source,juce::Image::BitmapData::readOnly);
        juce::Image::BitmapData output(result,juce::Image::BitmapData::writeOnly);
        const float sx=float(source.getWidth())/width,sy=float(source.getHeight())/height;
        for(int y=0;y<height;++y)for(int x=0;x<width;++x) {
            const float left=x*sx,right=(x+1)*sx,top=y*sy,bottom=(y+1)*sy;
            float alpha=0,red=0,green=0,blue=0;
            for(int yy=int(top);yy<juce::jmin(source.getHeight(),int(std::ceil(bottom)));++yy)
                for(int xx=int(left);xx<juce::jmin(source.getWidth(),int(std::ceil(right)));++xx) {
                    const float weight=(juce::jmin(right,float(xx+1))-juce::jmax(left,float(xx)))
                        *(juce::jmin(bottom,float(yy+1))-juce::jmax(top,float(yy)));
                    const auto c=input.getPixelColour(xx,yy);
                    const float a=c.getFloatAlpha()*weight;
                    alpha+=a;red+=c.getFloatRed()*a;green+=c.getFloatGreen()*a;blue+=c.getFloatBlue()*a;
                }
            if(alpha>0)output.setPixelColour(x,y,juce::Colour::fromFloatRGBA(red/alpha,green/alpha,blue/alpha,
                juce::jlimit(0.f,1.f,alpha/(sx*sy))));
        }
        return result;
    }
    struct V {
        float x=0,y=0,z=0;
        V operator+(V b)const{return {x+b.x,y+b.y,z+b.z};}
        V operator-(V b)const{return {x-b.x,y-b.y,z-b.z};}
        V operator*(float a)const{return {x*a,y*a,z*a};}
    };
    static float dot(V a,V b){return a.x*b.x+a.y*b.y+a.z*b.z;}
    static V cross(V a,V b){return {a.y*b.z-a.z*b.y,a.z*b.x-a.x*b.z,a.x*b.y-a.y*b.x};}
    static V unit(V a){return a*(1.f/juce::jmax(.000001f,std::sqrt(dot(a,a))));}
    static uint32_t hash(uint32_t a){a^=a>>16;a*=0x7feb352du;a^=a>>15;a*=0x846ca68bu;a^=a>>16;return a;}
    static float noise(uint32_t a){return float(hash(a)&65535u)/65535.f;}
    static float softNoise(float x,float y,uint32_t seed) {
        const int ix=int(std::floor(x)),iy=int(std::floor(y));
        const float fx=x-ix,fy=y-iy,u=fx*fx*(3-2*fx),v=fy*fy*(3-2*fy);
        const auto cell=[&](int dx,int dy){return noise(seed^uint32_t(ix+dx)*73856093u^uint32_t(iy+dy)*19349663u);};
        const float a=cell(0,0)*(1-u)+cell(1,0)*u,b=cell(0,1)*(1-u)+cell(1,1)*u;
        return a*(1-v)+b*v;
    }
    static constexpr float resolution=6.f,sine=.08f,cosine=.99679486f;
    static constexpr int corners=12,points=(corners+1)*4;
    static constexpr std::array<float,13> levels{{0,.08f,.15f,.27f,.45f,.68f,.85f,.94f,.97f,.985f,.996f,.9995f,1}};
    static constexpr std::array<float,13> depths{{.50f,.50f,.495f,.47f,.405f,.315f,.255f,.235f,.226f,.22305f,.215f,.208f,.204f}};
    struct Shape {
        float width,depth,rise,shift;
        juce::Rectangle<float> bounds;
        Shape(float w,float h):width(w),depth(h/cosine),rise(depth*.9f),shift(rise*sine),
            bounds(-w*.5f-5,-h*.5f+shift-5,w+10,h+10){}
        V point(int ring,int i)const {
            const float t=levels[size_t(ring)],roll=juce::jlimit(0.f,1.f,(t-.97f)/.03f);
            const float hx=width*(.5f-.012f*juce::jmin(1.f,t/.97f)-.013f*(1-std::sqrt(juce::jmax(0.f,1-roll*roll))));
            const float hy=depth*depths[size_t(ring)];
            const float rx=3.6f*(1-.45f*t),ry=juce::jmin(hy*.7f,3.6f*(1-.55f*t));
            const int q=i/(corners+1),j=i%(corners+1);
            const float a=(float(q)+float(j)/corners)*juce::MathConstants<float>::halfPi;
            return {(q==0||q==3?1.f:-1.f)*(hx-rx)+rx*std::cos(a),
                (q<2?1.f:-1.f)*(hy-ry)+ry*std::sin(a),rise*t};
        }
        juce::Point<float> project(V p)const{return {p.x,p.y*cosine-p.z*sine+shift};}
        float surfaceHeight(float y)const {
            const float r=std::abs(y)/depth;
            if(r<=depths.back())return rise;
            for(int i=int(levels.size())-2;i>=0;--i)if(r<=depths[size_t(i)]) {
                const float d=depths[size_t(i)]-depths[size_t(i+1)];
                const float t=d>0?(depths[size_t(i)]-r)/d:0;
                return rise*(levels[size_t(i)]+t*(levels[size_t(i+1)]-levels[size_t(i)]));
            }
            return 0;
        }
    };
    struct Vertex {V p,n;};
    static juce::Colour shade(V p,V normal,float age,uint32_t seed,float metalHalf,float capDepth,float capRise,float capWidth) {
        normal=unit(normal);
        const V light=unit({-.40f,-.55f,.73f}),half=unit(light+V{0,sine,cosine});
        const float diffuse=juce::jmax(0.f,dot(normal,light)),alignment=juce::jmax(0.f,dot(normal,half));
        const auto cell=seed^uint32_t(int(std::floor((p.x+40)*11)))*73856093u
            ^uint32_t(int(std::floor((p.y+40)*11)))*19349663u^uint32_t(int(p.z*11))*83492791u;
        const float grain=noise(cell),finish=1.f+.4f*(1-age);
        // The narrow rounded transition reflects an extended overhead source;
        // the broad shoulder remains black instead of acquiring a grey wash.
        const float edgeAlignment=juce::jmax(0.f,-.22f*normal.y+.9755f*normal.z);
        const float reflection=(.85f*std::pow(alignment,95.f)+.14f*std::pow(alignment,25.f)
            +.85f*std::pow(edgeAlignment,120.f))*finish;
        // Extended source along the actual upper fold, rather than just the
        // corner normals. Preserve proposal 208 Minty gloss at Fire Hazard.
        const float fold=std::exp(-std::pow((p.y+capDepth*.218f)/.13f,2.f))
            *juce::jlimit(0.f,1.f,(p.z/capRise-.97f)/.02f)*.65f*(1-age);
        const float black=juce::jlimit(0.f,1.f,(.028f+.068f*diffuse)*(.91f+.18f*grain)
            +(reflection+fold)*(.78f+.3f*grain)+(grain-.5f)*.009f);
        const float metalCoverage=juce::jlimit(0.f,1.f,(metalHalf+.035f-std::abs(p.y))/.07f);
        if(metalCoverage<=0)return juce::Colour::fromFloatRGBA(black,black*.99f,black*.98f,1);
        // Reflect the upper-left source across the band from left to right.
        // Its broad Minty footprint narrows with age; material dulling still
        // stops at Sold As-Is while pitting continues through Fire Hazard.
        const float bandAge=juce::jmin(age,.75f);
        const float widthFraction=.5f-age/3.f;
        const float across=juce::jlimit(0.f,1.f,(p.x+capWidth*.475f)/(capWidth*.95f));
        const float transition=juce::jlimit(0.f,1.f,(across-widthFraction*.4f)/(widthFraction*.6f));
        const float sheen=1-transition*transition*(3-2*transition);
        const float leftReflection=.35f*(1-bandAge*.25f)*sheen;
        const float lateWear=juce::jlimit(0.f,1.f,(age-.75f)*4.f);
        const float pitThreshold=.94f-.04f*lateWear;
        const float pit=grain>pitThreshold?(grain-pitThreshold)*(4.5f+.5f*lateWear)*age:0;
        // Fire Hazard adds local surface damage rather than lowering the
        // entire finish. Broad irregular tarnish leaves clean silver between.
        const float tarnish=lateWear*.36f*juce::jlimit(0.f,1.f,
            (softNoise(p.x*.35f+23.f,p.y*.90f+7.f,seed^0x232a71u)-.55f)/.22f);
        const float silver=juce::jlimit(0.f,1.f,.49f+.12f*diffuse+.10f*(1-bandAge)
            +leftReflection+(grain-.5f)*.045f-pit);
        const float red=silver*.96f*(1-tarnish)+.36f*tarnish;
        const float green=silver*(1-tarnish)+.32f*tarnish;
        const float blue=silver*.99f*(1-tarnish)+.24f*tarnish;
        return juce::Colour::fromFloatRGBA(black+(red-black)*metalCoverage,
            black*.99f+(green-black*.99f)*metalCoverage,
            black*.98f+(blue-black*.98f)*metalCoverage,1);
    }
    static juce::Image render(const Shape& shape,float age,uint32_t seed) {
        const int w=juce::roundToInt(shape.bounds.getWidth()*resolution),h=juce::roundToInt(shape.bounds.getHeight()*resolution);
        juce::Image result(juce::Image::ARGB,w,h,true,juce::SoftwareImageType());
        {
            juce::Graphics g(result);g.addTransform(juce::AffineTransform::scale(resolution)
                .translated(-shape.bounds.getX()*resolution,-shape.bounds.getY()*resolution));
            const juce::Rectangle<float> foot(-shape.width*.5f,-shape.depth*cosine*.5f+shape.shift,shape.width,shape.depth*cosine);
            roundedHardwareShadow(g,foot,3.6f,.6f);
        }
        std::array<std::array<Vertex,points>,levels.size()> mesh;
        for(int r=0;r<int(levels.size());++r)for(int i=0;i<points;++i) {
            const V tangent=shape.point(r,(i+1)%points)-shape.point(r,(i+points-1)%points);
            const V vertical=shape.point(juce::jmin(r+1,int(levels.size())-1),i)-shape.point(juce::jmax(0,r-1),i);
            mesh[size_t(r)][size_t(i)]={shape.point(r,i),unit(cross(tangent,vertical))};
        }
        std::vector<float> depth(size_t(w*h),-std::numeric_limits<float>::infinity());
        {
            juce::Image::BitmapData pixels(result,juce::Image::BitmapData::readWrite);
            const auto triangle=[&](Vertex a,Vertex b,Vertex c) {
                const auto screen=[&](V p){return (shape.project(p)-shape.bounds.getPosition())*resolution;};
                const auto pa=screen(a.p),pb=screen(b.p),pc=screen(c.p);
                const float area=(pb.x-pa.x)*(pc.y-pa.y)-(pb.y-pa.y)*(pc.x-pa.x);
                if(std::abs(area)<.00001f)return;
                const int x0=juce::jmax(0,int(std::floor(juce::jmin(pa.x,pb.x,pc.x)))),x1=juce::jmin(w-1,int(std::ceil(juce::jmax(pa.x,pb.x,pc.x))));
                const int y0=juce::jmax(0,int(std::floor(juce::jmin(pa.y,pb.y,pc.y)))),y1=juce::jmin(h-1,int(std::ceil(juce::jmax(pa.y,pb.y,pc.y))));
                for(int y=y0;y<=y1;++y)for(int x=x0;x<=x1;++x) {
                    const float px=x+.5f,py=y+.5f;
                    const float wa=((pb.x-px)*(pc.y-py)-(pb.y-py)*(pc.x-px))/area;
                    const float wb=((pc.x-px)*(pa.y-py)-(pc.y-py)*(pa.x-px))/area,wc=1-wa-wb;
                    if(wa<-.00001f||wb<-.00001f||wc<-.00001f)continue;
                    const V p=a.p*wa+b.p*wb+c.p*wc;
                    const float z=p.y*sine+p.z*cosine;
                    auto& old=depth[size_t(y*w+x)];if(z<old)continue;old=z;
                    pixels.setPixelColour(x,y,shade(p,a.n*wa+b.n*wb+c.n*wc,age,seed,1.12f,shape.depth,shape.rise,shape.width));
                }
            };
            for(int r=0;r<int(levels.size())-1;++r)for(int i=0;i<points;++i) {
                const int j=(i+1)%points;
                const auto a=mesh[size_t(r)][size_t(i)],b=mesh[size_t(r)][size_t(j)],c=mesh[size_t(r+1)][size_t(i)],d=mesh[size_t(r+1)][size_t(j)];
                triangle(a,b,d);triangle(a,d,c);
            }
            const Vertex centre{{0,0,shape.rise},{0,0,1}};
            for(int i=0;i<points;++i) {
                auto a=mesh.back()[size_t(i)],b=mesh.back()[size_t((i+1)%points)];a.n=b.n={0,0,1};triangle(centre,a,b);
            }
        }
        if(age>0) {
            juce::Graphics g(result);g.addTransform(juce::AffineTransform::scale(resolution)
                .translated(-shape.bounds.getX()*resolution,-shape.bounds.getY()*resolution));
            for(uint32_t i=0;i<uint32_t(32*age);++i) {
                const auto key=hash(seed+i*7919u);
                const float x=(noise(key+17)-.5f)*shape.width*.8f,y=(noise(key+31)-.5f)*shape.depth*.82f;
                if(std::abs(y)<2.1f)continue;
                const auto a=shape.project({x,y,shape.surfaceHeight(y)});
                const float dx=.25f+1.2f*noise(key+43),dy=.25f+.55f*noise(key+57);
                const auto b=shape.project({x+dx,y+dy,shape.surfaceHeight(y+dy)});
                juce::Path scuff;scuff.startNewSubPath(a);scuff.quadraticTo((a+b)*.5f+juce::Point<float>(.1f,-.1f),b);
                g.setColour(juce::Colour(0xffa0a6a0).withAlpha(.14f*age));
                g.strokePath(scuff,juce::PathStrokeType(.075f,juce::PathStrokeType::curved,juce::PathStrokeType::rounded));
            }
        }
        return result;
    }
    AgeRasterCache cache{24u*1024u*1024u};
};
}
