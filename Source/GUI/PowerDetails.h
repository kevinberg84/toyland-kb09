#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <cmath>
#include <cstdint>
#include <array>
#include <vector>

namespace kb09_gui {
namespace power_detail {

inline float noise(std::uint32_t value) {
    value ^= value >> 16;
    value *= 0x7feb352du;
    value ^= value >> 15;
    value *= 0x846ca68bu;
    value ^= value >> 16;
    return float(value & 0xffffu) / 65535.0f;
}

inline void legendText(juce::Graphics& g, const juce::String& text,
                       juce::Rectangle<float> box, float size,
                       juce::Justification alignment) {
    g.setFont(juce::Font(juce::FontOptions("Arial", size, juce::Font::bold)));
    g.drawFittedText(text, box.toNearestInt(), alignment, 1, .8f);
}

inline float smoothNoise(float x, float y, std::uint32_t seed) {
    const int ix = int(std::floor(x)), iy = int(std::floor(y));
    float fx = x - float(ix), fy = y - float(iy);
    fx = fx * fx * (3.0f - 2.0f * fx);
    fy = fy * fy * (3.0f - 2.0f * fy);
    const auto sample = [seed](int xx, int yy) {
        return noise(seed + std::uint32_t(xx) * 7919u + std::uint32_t(yy) * 104729u);
    };
    const float a = sample(ix, iy) + fx * (sample(ix + 1, iy) - sample(ix, iy));
    const float b = sample(ix, iy + 1) + fx * (sample(ix + 1, iy + 1) - sample(ix, iy + 1));
    return a + fy * (b - a);
}

inline float smoothRange(float low, float high, float value) {
    const float t = juce::jlimit(0.0f, 1.0f, (value - low) / (high - low));
    return t * t * (3.0f - 2.0f * t);
}

inline float socketDistance(float x, float y, int variant) {
    constexpr std::array<float,4> angles{{-.23f,.36f,-.59f,.81f}};
    const float angle=angles[size_t(variant)],ca=std::cos(angle),sa=std::sin(angle);
    const float rx=x*ca+y*sa,ry=y*ca-x*sa;
    // Four narrow driver wings with rounded ends and a recessed central cup.
    // Actual cavity walls replace the previous soft clover-shaped dark stamp.
    const auto wing=[](float a,float b) {
        constexpr float radius=.045f;
        const float width=.115f+.038f*(1-smoothRange(.18f,.53f,std::abs(a)));
        const float qx=std::abs(a)-(.56f-radius),qy=std::abs(b)-(width-radius);
        return std::hypot(juce::jmax(0.f,qx),juce::jmax(0.f,qy))
            +juce::jmin(juce::jmax(qx,qy),0.f)-radius;
    };
    const float theta=std::atan2(ry,rx),phase=float(variant)*1.37f;
    const float erosion=.004f*std::sin(theta*13+phase)+.003f*std::sin(theta*23+1.1f+phase);
    return juce::jmin(wing(rx,ry),wing(ry,rx))-erosion;
}

struct ScrewSample {
    float radius = 0, dome = 0, patch = 0, grain = 0, pore = 0;
    float socket = 0, innerLight = 0, alpha = 0, specular = 0, rim = 0;
};

struct ScrewMaterial {
    static constexpr int pixels = 320;
    std::array<std::vector<ScrewSample>, 4> samples;
    std::array<juce::Image, 4> images;
    std::array<float, 4> lastAge {{-1.0f, -1.0f,-1.0f,-1.0f}};

    ScrewMaterial() {
        for (int index = 0; index < 4; ++index) {
            const bool lower = index%2 == 1;
            const std::uint32_t seed = (lower ? 0x7c2f301bu : 0x49326e51u)+uint32_t(index)*0x345ab17u;
            auto& field = samples[size_t(index)];
            field.resize(size_t(pixels * pixels));
            for (int iy = 0; iy < pixels; ++iy) for (int ix = 0; ix < pixels; ++ix) {
                auto& s = field[size_t(iy * pixels + ix)];
                const float x = (float(ix) + .5f) * 2.0f / float(pixels) - 1.0f;
                const float y = (float(iy) + .5f) * 2.0f / float(pixels) - 1.0f;
                s.radius = std::sqrt(x * x + y * y);
                s.alpha = 1.0f - smoothRange(.982f, 1.0f, s.radius);
                if (s.alpha <= 0) continue;

                const float z = std::sqrt(juce::jmax(.0025f, 1.0f - s.radius * s.radius));
                float nx = .46f * x / z, ny = .46f * y / z, nz = 1.0f;
                const float length = std::sqrt(nx * nx + ny * ny + 1.0f);
                nx /= length; ny /= length; nz /= length;
                const float diffuse = juce::jmax(0.0f, -.40f * nx - .52f * ny + .754f * nz);
                const float broadReflection = std::pow(juce::jmax(0.0f,
                    -.214f * nx - .279f * ny + .936f * nz), 20.0f);
                s.dome = .25f + .71f * diffuse + .055f * broadReflection;
                s.dome *= 1.0f - .21f * smoothRange(.88f, 1.0f, s.radius);
                s.specular=std::pow(juce::jmax(0.f,-.214f*nx-.279f*ny+.936f*nz),96.f);
                s.rim=std::exp(-std::pow((s.radius-.91f)/.018f,2.f))
                    *juce::jmax(0.f,-.45f*x-.60f*y+.22f);
                s.patch = .53f * smoothNoise(x * 4.3f, y * 4.3f, seed)
                        + .29f * smoothNoise(x * 9.7f, y * 9.7f, seed + 17u)
                        + .18f * smoothNoise(x * 21.0f, y * 21.0f, seed + 61u);
                s.grain = smoothNoise(x * 59.0f, y * 59.0f, seed + 103u) - .5f;
                s.pore = smoothNoise(x * 87.0f, y * 87.0f, seed + 179u);
                s.socket = socketDistance(x, y, index);
                const float dx = socketDistance(x + .003f, y, index)
                               - socketDistance(x - .003f, y, index);
                const float dy = socketDistance(x, y + .003f, index)
                               - socketDistance(x, y - .003f, index);
                const float edgeLength = std::sqrt(dx * dx + dy * dy) + .00001f;
                s.innerLight = juce::jlimit(0.0f, 1.0f,
                    .43f + .43f * (.40f * dx + .52f * dy) / edgeLength);
            }
            images[size_t(index)] = juce::Image(juce::Image::ARGB, pixels, pixels,
                                               true, juce::SoftwareImageType());
        }
    }

    const juce::Image& render(float age, bool lower,int pair=0,float radius=4.7f) {
        const size_t index = size_t(pair*2+(lower ? 1 : 0));
        if (lastAge[index] == age) return images[index];
        lastAge[index] = age;
        const float wear = std::pow(age, 1.12f);
        juce::Image::BitmapData output(images[index], juce::Image::BitmapData::writeOnly);
        const auto& field = samples[index];
        for (int y = 0; y < pixels; ++y) for (int x = 0; x < pixels; ++x) {
            const auto& s = field[size_t(y * pixels + x)];
            if (s.alpha <= 0) {
                output.setPixelColour(x, y, juce::Colours::transparentBlack);
                continue;
            }
            // Joined multiscale deposits, rather than painted circular dots.
            const float pale = smoothRange(lower ? .33f : .57f,
                                          lower ? .66f : .76f, s.patch);
            const float dark = 1.0f - smoothRange(.27f, .47f, s.patch);
            const float base = lower ? .76f - .14f * wear : .68f - .305f * wear;
            float light = base + wear * (lower ? .035f * pale - .31f * dark
                                                : .12f * pale - .12f * dark);
            light += s.grain * (.045f + .13f * wear);
            light -= smoothRange(.69f, .88f, s.pore) * (.025f + .085f * wear);
            // Broad chalky deposits diffuse the lower head's reflections;
            // retain grey metal shading instead of bleaching the entire dome.
            const float dome = lower ? s.dome + .18f * wear * (.84f - s.dome) : s.dome;
            light *= dome;
            light+=(.30f*s.specular+.10f*s.rim)*(1-age)*(1-age);
            float red = light, green = light + .013f, blue = light - .017f;
            const float rust=lower?0.f:wear*wear*smoothRange(.58f,.80f,s.patch)*.10f;
            red+=rust;green-=rust*.25f;blue-=rust*.55f;

            const float edge = s.socket + s.grain * (.007f + .015f * wear) - (lower?.018f:0.f)*wear;
            if (edge < .012f) {
                const float opening = 1.0f - smoothRange(-.014f, .010f, edge);
                // The opposing inner wall catches light while the cavity
                // floor remains dark, giving the slot actual visual depth.
                const float wall = .105f + .56f * s.innerLight
                    + .025f * s.grain - .05f * wear * dark;
                const float floor = .027f + .025f * s.innerLight + .012f * s.grain;
                const float depth = 1.0f - smoothRange(-.085f, -.020f, edge);
                const float recess = wall + depth * (floor - wall);
                red += opening * (recess - red);
                green += opening * (recess + .009f - green);
                blue += opening * (recess - .007f - blue);
            }
            output.setPixelColour(x, y, juce::Colour::fromFloatRGBA(
                juce::jlimit(0.0f, 1.0f, red), juce::jlimit(0.0f, 1.0f, green),
                juce::jlimit(0.0f, 1.0f, blue), s.alpha));
        }
        return images[index];
    }
};

inline ScrewMaterial& screwMaterial() {
    static ScrewMaterial material;
    return material;
}

} // namespace power_detail

namespace power_detail {

inline void paintLegend(juce::Graphics& g, juce::Colour ink) {
    juce::Graphics::ScopedSaveState save(g);
    g.setColour(ink);
    const auto textOutline=[](const char* text,juce::Rectangle<float> box,float size,
                              juce::Justification alignment) {
        juce::GlyphArrangement glyphs;
        glyphs.addFittedText(juce::Font(juce::FontOptions("Arial",size,juce::Font::bold)),text,
                            box.getX(),box.getY(),box.getWidth(),box.getHeight(),alignment,1,.8f);
        juce::Path result;glyphs.createPath(result);return result;
    };
    auto power=textOutline("POWER",{175,459,37,12},8.4f,juce::Justification::centred);
    auto on=textOutline("ON",{225,455,16,9},6.7f,juce::Justification::centredLeft);
    auto off=textOutline("OFF",{225,464,17,9},6.7f,juce::Justification::centredLeft);

    // The screen print is two filled side profiles: a low depressed button
    // above, and the same button standing on its taller body below.
    juce::Path depressed;
    depressed.startNewSubPath(214, 461.0f);
    depressed.lineTo(215.7f, 460.6f);
    depressed.lineTo(216.2f, 459.2f);
    depressed.lineTo(220.8f, 459.2f);
    depressed.lineTo(221.3f, 460.6f);
    depressed.lineTo(223, 461.0f);
    depressed.lineTo(223, 462.0f);
    depressed.lineTo(214, 462.0f);
    depressed.closeSubPath();


    juce::Path raised;
    raised.startNewSubPath(215.6f, 468.0f);
    raised.lineTo(221.4f, 468.0f);
    raised.lineTo(221.4f, 469.3f);
    raised.lineTo(220.7f, 469.3f);
    raised.lineTo(220.7f, 472.7f);
    raised.lineTo(223, 472.7f);
    raised.lineTo(223, 474.0f);
    raised.lineTo(214, 474.0f);
    raised.lineTo(214, 472.7f);
    raised.lineTo(216.3f, 472.7f);
    raised.lineTo(216.3f, 469.3f);
    raised.lineTo(215.6f, 469.3f);
    raised.closeSubPath();
    // Work in visible outline coordinates, not padded label boxes. The two
    // switch symbols and the ON/OFF labels form two shared-width columns.
    // The SH-09 prints the depressed and raised profiles as one tight stack.
    // Keep the stack's center fixed while closing the empty space between rows.
    depressed.applyTransform(juce::AffineTransform::translation(0,2.25f));
    raised.applyTransform(juce::AffineTransform::translation(0,-2.25f));
    const float left=160.5f+.425f,right=251.f-.5f;
    const float powerWidth=power.getBounds().getWidth();
    const float symbolWidth=juce::jmax(depressed.getBounds().getWidth(),raised.getBounds().getWidth());
    const float stateWidth=juce::jmax(on.getBounds().getWidth(),off.getBounds().getWidth());
    constexpr float symbolTextGap=3.f;
    const float stateGroupWidth=symbolWidth+symbolTextGap+stateWidth;
    const float gap=(right-left-powerWidth-stateGroupWidth)/3.f;
    const float powerLeft=left+gap,symbolLeft=powerLeft+powerWidth+gap;
    const float stateLeft=symbolLeft+symbolWidth+symbolTextGap;
    power.applyTransform(juce::AffineTransform::translation(powerLeft-power.getBounds().getX(),0));
    for(auto* symbol:{&depressed,&raised})
        symbol->applyTransform(juce::AffineTransform::translation(symbolLeft-symbol->getBounds().getX(),0));
    on.applyTransform(juce::AffineTransform::translation(stateLeft-on.getBounds().getX(),depressed.getBounds().getBottom()-on.getBounds().getBottom()));
    off.applyTransform(juce::AffineTransform::translation(stateLeft-off.getBounds().getX(),raised.getBounds().getBottom()-off.getBounds().getBottom()));
    const float stateMidpoint=(on.getBounds().getBottom()+off.getBounds().getY())*.5f;
    const float stateShift=power.getBounds().getCentreY()-stateMidpoint;
    for(auto* outline:{&depressed,&raised,&on,&off})
        outline->applyTransform(juce::AffineTransform::translation(0,stateShift));
    for(const auto* outline:{&power,&depressed,&raised,&on,&off})g.fillPath(*outline);

}

struct LegendMaterial {
    static constexpr int scale = 4, width = 92 * scale, height = 24 * scale;
    juce::Image original {juce::Image::ARGB, width, height, true, juce::SoftwareImageType()};
    juce::Image worn {juce::Image::ARGB, width, height, true, juce::SoftwareImageType()};
    float lastAge = -1.0f;
    float inkCentreY=0;

    LegendMaterial() {
        juce::Graphics mask(original);
        mask.addTransform(juce::AffineTransform::translation(-159.0f, -454.0f)
                          .scaled(float(scale)));
        paintLegend(mask, juce::Colours::white);
        int top=height,bottom=-1;
        for(int y=0;y<height;++y)for(int x=0;x<width;++x)
            if(original.getPixelAt(x,y).getAlpha()>0){top=juce::jmin(top,y);bottom=juce::jmax(bottom,y);}
        inkCentreY=454.f+float(top+bottom+1)/(2.f*scale);
    }

    const juce::Image& render(float age) {
        if (lastAge == age) return worn;
        lastAge = age;
        const float wear = std::pow(age, 1.4f);
        juce::Image::BitmapData input(original, juce::Image::BitmapData::readOnly);
        juce::Image::BitmapData output(worn, juce::Image::BitmapData::writeOnly);
        for (int y = 0; y < height; ++y) for (int x = 0; x < width; ++x) {
            const float coverage = input.getPixelColour(x, y).getFloatAlpha();
            if (coverage <= 0) {
                output.setPixelColour(x, y, juce::Colours::transparentBlack);
                continue;
            }
            const float field = smoothNoise(float(x) * .46f, float(y) * .46f, 0x36b29451u);
            const float pinhole = smoothRange(.70f, .91f, field);
            const auto adjacent = [&](int xx, int yy) {
                return xx < 0 || yy < 0 || xx >= width || yy >= height
                    ? 0.0f : input.getPixelColour(xx, yy).getFloatAlpha();
            };
            const float edge = 1.0f - juce::jmin(juce::jmin(adjacent(x - 1, y), adjacent(x + 1, y)),
                                              juce::jmin(adjacent(x, y - 1), adjacent(x, y + 1)));
            const float loss = wear * (.22f * pinhole + .025f * edge * field);
            output.setPixelColour(x, y, juce::Colours::white.withAlpha(coverage * (1.0f - loss)));
        }
        return worn;
    }
};

} // namespace power_detail

inline void drawPowerLegend(juce::Graphics& g, juce::Colour ink, float age = 0,float centreY=-1) {
    juce::Graphics::ScopedSaveState save(g);
    static power_detail::LegendMaterial material;
    if(centreY>=0)g.addTransform(juce::AffineTransform::translation(0,centreY-material.inkCentreY));
    g.setColour(ink);
    g.setImageResamplingQuality(juce::Graphics::highResamplingQuality);
    // Alpha-only print loss reveals the actual panel beneath it; the almost
    // intact lettering and solid silhouettes keep exactly the same outlines.
    g.drawImage(material.render(juce::jlimit(0.0f, 1.0f, age)),
                juce::Rectangle<float>(159, 454, 92, 24),
                juce::RectanglePlacement::stretchToFit, true);
}

inline void drawPowerScrew(juce::Graphics& g, juce::Point<float> centre,
                          float age, bool lower,int pair=0,float radius=4.7f) {
    juce::Graphics::ScopedSaveState save(g);
    age = juce::jlimit(0.0f, 1.0f, age);
    constexpr float viewCos = .981627f;
    const auto head = juce::Rectangle<float>(centre.x - radius,
        centre.y - radius * viewCos, 2 * radius, 2 * radius * viewCos);
    // Layered soft contact shadows anchor the curved head without a graphic
    // black outline. Dome and recess lighting come from the cached CPU field.
    g.setColour(juce::Colour(0xff020302).withAlpha(.23f));
    g.fillEllipse(head.expanded(.63f, .45f).translated(.16f, .38f));
    g.setColour(juce::Colour(0xff020302).withAlpha(.51f));
    g.fillEllipse(head.expanded(.20f, .10f).translated(.11f, .25f));
    g.setImageResamplingQuality(juce::Graphics::highResamplingQuality);
    g.setOpacity(1);
    g.drawImage(power_detail::screwMaterial().render(age, lower,pair), head,
                juce::RectanglePlacement::stretchToFit);
}

} // namespace kb09_gui
