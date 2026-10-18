#pragma once
#include "InsetRedLamp.h"
#include "RateLampClock.h"
#include <atomic>
#include <functional>
#include <mutex>
#include <thread>
#if JUCE_WINDOWS
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#undef small
#undef DrawText

namespace kb09_gui {
// A per-pixel-alpha owned window owned and presented by its own thread.
// No JUCE message-thread paint, timer, native OpenGL surface or panel copy
// participates in its blink. Transparent pixels contain premultiplied zero.
class WindowsRateLamp {
public:
    struct Input {bool power;double hz,phase,stamp;};
    explicit WindowsRateLamp(std::function<Input()> read):readInput(std::move(read)),worker([this]{run();}) {}
    ~WindowsRateLamp() {quit.store(true);if(worker.joinable())worker.join();}
    void place(void* parent,juce::Rectangle<int> bounds,bool show) {
        std::lock_guard<std::mutex> lock(geometryMutex);
        geometry={static_cast<HWND>(parent),bounds,show};
    }
    unsigned frames()const{return presented.load();}
    unsigned transitions()const{return changed.load();}
    double largestGap()const{return maxGap.load();}
    unsigned errors()const{return failed.load();}
    unsigned lastError()const{return errorCode.load();}
    uintptr_t handle()const{return nativeHandle.load();}
    float level()const{return currentLevel.load();}
private:
    struct Geometry {HWND parent=nullptr;juce::Rectangle<int> bounds;bool show=false;};
    struct Bitmap {
        HDC dc=nullptr;HBITMAP bitmap=nullptr;HGDIOBJ previous=nullptr;
        Bitmap(int w,int h,float level) {
            dc=CreateCompatibleDC(nullptr);
            BITMAPINFO info{};info.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
            info.bmiHeader.biWidth=w;info.bmiHeader.biHeight=-h;
            info.bmiHeader.biPlanes=1;info.bmiHeader.biBitCount=32;info.bmiHeader.biCompression=BI_RGB;
            void* bits=nullptr;bitmap=CreateDIBSection(dc,&info,DIB_RGB_COLORS,&bits,nullptr,0);
            if(!dc||!bitmap||!bits)return;
            previous=SelectObject(dc,bitmap);
            juce::Image image(juce::Image::ARGB,w,h,true,juce::SoftwareImageType());
            {juce::Graphics g(image);g.addTransform(juce::AffineTransform::scale(w/11.f,h/10.f));
             drawInsetRedLamp(g,{5.5f,5.f},level,true);}
            const juce::Image::BitmapData pixels(image,juce::Image::BitmapData::readOnly);
            auto* bytes=static_cast<unsigned char*>(bits);
            for(int y=0;y<h;++y)for(int x=0;x<w;++x) {
                const auto c=pixels.getPixelColour(x,y);const unsigned a=c.getAlpha();
                auto* out=bytes+(y*w+x)*4;
                out[0]=static_cast<unsigned char>((c.getBlue()*a+127)/255);
                out[1]=static_cast<unsigned char>((c.getGreen()*a+127)/255);
                out[2]=static_cast<unsigned char>((c.getRed()*a+127)/255);out[3]=static_cast<unsigned char>(a);
            }
        }
        ~Bitmap(){if(previous)SelectObject(dc,previous);if(bitmap)DeleteObject(bitmap);if(dc)DeleteDC(dc);}
    };
    static LRESULT CALLBACK procedure(HWND window,UINT message,WPARAM w,LPARAM l) {
        if(message==WM_NCHITTEST)return HTTRANSPARENT;
        if(message==WM_ERASEBKGND)return 1;
        return DefWindowProcW(window,message,w,l);
    }
    void run() {
        static std::once_flag registered;
        std::call_once(registered,[]{WNDCLASSW c{};c.lpfnWndProc=procedure;c.hInstance=GetModuleHandleW(nullptr);
            c.lpszClassName=L"KB09IndependentRate245";RegisterClassW(&c);});
        HWND window=nullptr,parent=nullptr;juce::Rectangle<int> bounds;POINT lastScreen{LONG_MIN,LONG_MIN};bool shown=false;
        std::array<std::unique_ptr<Bitmap>,33> bitmaps;
        RateLampClock clock;double previousTime=0;int previousFrame=-1;
        while(!quit.load()) {
            MSG message;while(PeekMessageW(&message,nullptr,0,0,PM_REMOVE)){TranslateMessage(&message);DispatchMessageW(&message);}
            Geometry requested;{std::lock_guard<std::mutex> lock(geometryMutex);requested=geometry;}
            if(window&&(!IsWindow(window)||requested.parent!=parent)) {
                if(IsWindow(window))DestroyWindow(window);window=nullptr;nativeHandle.store(0);bitmaps={};
            }
            if(requested.show&&requested.parent&&IsWindow(requested.parent)&&IsWindowVisible(requested.parent)&&!IsIconic(requested.parent)&&!requested.bounds.isEmpty()) {
                if(!window) {
                    SetThreadDpiAwarenessContext(GetWindowDpiAwarenessContext(requested.parent));
                    window=CreateWindowExW(WS_EX_LAYERED|WS_EX_TRANSPARENT|WS_EX_NOACTIVATE|WS_EX_TOOLWINDOW,
                        L"KB09IndependentRate245",L"",WS_POPUP,0,0,1,1,requested.parent,nullptr,GetModuleHandleW(nullptr),nullptr);
                    parent=requested.parent;nativeHandle.store(reinterpret_cast<uintptr_t>(window));bounds={};lastScreen={LONG_MIN,LONG_MIN};shown=false;
                    if(!window){errorCode.store(GetLastError());++failed;}
                }
                if(window) {
                    const bool resized=bounds!=requested.bounds;
                    if(resized) {bounds=requested.bounds;bitmaps={};previousFrame=-1;}
                    // An owned popup avoids punching a child-window rectangle
                    // out of JUCE's parent backing surface. Keep it anchored in
                    // physical screen pixels, including while the owner moves.
                    POINT screen{bounds.getX(),bounds.getY()};ClientToScreen(parent,&screen);
                    if(resized||screen.x!=lastScreen.x||screen.y!=lastScreen.y) {
                        SetWindowPos(window,nullptr,screen.x,screen.y,bounds.getWidth(),bounds.getHeight(),SWP_NOACTIVATE|SWP_NOZORDER|SWP_ASYNCWINDOWPOS);
                        lastScreen=screen;
                    }
                    const double now=juce::Time::getMillisecondCounterHiRes();const auto input=readInput();
                    const auto value=clock.level(input.power,input.hz,now,input.phase+juce::jmax(0.,now-input.stamp)*.001*input.hz);
                    currentLevel.store(value);const int frame=juce::jlimit(0,32,juce::roundToInt(value*32));
                    if(!bitmaps[size_t(frame)])bitmaps[size_t(frame)]=std::make_unique<Bitmap>(bounds.getWidth(),bounds.getHeight(),frame/32.f);
                    const auto& bitmap=*bitmaps[size_t(frame)];SIZE size{bounds.getWidth(),bounds.getHeight()};POINT origin{};
                    BLENDFUNCTION blend{AC_SRC_OVER,0,255,AC_SRC_ALPHA};
                    if(bitmap.bitmap&&UpdateLayeredWindow(window,nullptr,nullptr,&size,bitmap.dc,&origin,0,&blend,ULW_ALPHA)) {
                        ++presented;if(previousFrame>=0&&frame!=previousFrame)++changed;previousFrame=frame;
                        if(previousTime>0)maxGap.store(juce::jmax(maxGap.load(),now-previousTime));previousTime=now;
                    }else {errorCode.store(GetLastError());++failed;}
                    if(!shown){ShowWindowAsync(window,SW_SHOWNOACTIVATE);shown=true;}
                }
            }else if(window){if(shown){ShowWindowAsync(window,SW_HIDE);shown=false;}previousTime=0;}
            std::this_thread::sleep_for(std::chrono::milliseconds(4));
        }
        if(window&&IsWindow(window))DestroyWindow(window);nativeHandle.store(0);
    }
    std::function<Input()> readInput;std::mutex geometryMutex;Geometry geometry;
    std::atomic<bool> quit{false};std::atomic<unsigned> presented{0},changed{0},failed{0},errorCode{0};
    std::atomic<double> maxGap{0};std::atomic<uintptr_t> nativeHandle{0};std::atomic<float> currentLevel{0};
    std::thread worker;
};
}
#endif
