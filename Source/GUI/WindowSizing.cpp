#include "WindowSizing.h"
#include <algorithm>
#include <cmath>
#include <optional>

#if JUCE_WINDOWS
 #ifndef NOMINMAX
  #define NOMINMAX
 #endif
 #include <windows.h>
#endif

namespace sh09 {
namespace {
constexpr double smallFraction = .65, mediumFraction = .90;
juce::Rectangle<float> fitAspect(juce::Rectangle<float> area, double ratio) noexcept {
    if (area.isEmpty() || !std::isfinite(ratio) || ratio <= 0) return {};
    const auto width = juce::jmin(area.getWidth(), float(double(area.getHeight()) * ratio));
    const auto height = float(double(width) / ratio);
    return {area.getCentreX() - width * .5f, area.getCentreY() - height * .5f, width, height};
}

#if JUCE_WINDOWS
// Only scopes native geometry calls. Never changes PROCESS awareness or leaves the
// host message thread in another DPI context, and never spans a JUCE resize call.
struct NativeDpiScope {
    using Setter = HANDLE (WINAPI*)(HANDLE);
    Setter setter = nullptr;
    HANDLE previous = nullptr;
    NativeDpiScope() {
        if (auto module = GetModuleHandleW(L"user32.dll"))
            setter = reinterpret_cast<Setter>(GetProcAddress(module, "SetThreadDpiAwarenessContext"));
        if (setter != nullptr) previous = setter(reinterpret_cast<HANDLE>(static_cast<intptr_t>(-4))); // PER_MONITOR_AWARE_V2
    }
    ~NativeDpiScope() { if (setter != nullptr && previous != nullptr) setter(previous); }
    bool ready() const noexcept { return previous != nullptr; }
};
juce::Rectangle<int> rectangle(const RECT& r) { return {r.left, r.top, r.right-r.left, r.bottom-r.top}; }
struct NativeTarget {
    HWND window = nullptr;
    juce::Rectangle<int> outer, work, editor;
    double pixelsPerEditorUnit = 1;
    bool standalone = false;
};
struct NativeMetrics {
    HWND child = nullptr, frame = nullptr;
    juce::Rectangle<int> editor, work, outer, client;
    double scaleX = 1, scaleY = 1;
    bool frameMeasured = false, standalone = false;
    bool valid() const noexcept { return child != nullptr && !editor.isEmpty() && !work.isEmpty(); }
};
std::optional<juce::Point<int>> cursorPixels() {
    POINT p{};
    if(GetPhysicalCursorPos(&p)) return juce::Point<int>{p.x,p.y};
    return std::nullopt;
}
juce::Rectangle<int> workAt(juce::Point<int> p) {
    NativeDpiScope dpi;
    if(!dpi.ready()) return {};
    MONITORINFO info{}; info.cbSize = sizeof(info);
    return GetMonitorInfoW(MonitorFromPoint({p.x,p.y},MONITOR_DEFAULTTONEAREST),&info) ? rectangle(info.rcWork) : juce::Rectangle<int>{};
}
#endif
}

struct WindowSizing::Impl final : private juce::Timer, private juce::ComponentListener {
    struct InitialFitTimer final : private juce::Timer {
        explicit InitialFitTimer(Impl& value) : parent(value) {}
        void begin() { startTimer(120); }
        void stop() { stopTimer(); }
        bool active() const { return isTimerRunning(); }
        void timerCallback() override { parent.initialFitTick(); }
        Impl& parent;
    };
    struct CaptionTimer final : private juce::Timer {
        explicit CaptionTimer(Impl& value):parent(value) { startTimerHz(60); }
        void timerCallback() override { parent.captionTick(); }
        Impl& parent;
    };
    CaptionTimer captionTimer{*this};
    bool captionDown=false,captionTracking=false,captionMoved=false,captionWasLarge=false;
    juce::Point<int> captionStart;
    void captionTick() {
#if JUCE_WINDOWS
        if(dragging||applying||!editor.isShowing())return;
        const bool down=(GetAsyncKeyState(VK_LBUTTON)&0x8000)!=0;
        const auto cursor=cursorPixels();if(!cursor)return;
        const auto target=native();
        if(down&&!captionDown) {
            captionTracking=false;captionMoved=false;
            // Only the validated plug-in frame's title bar, never the DAW frame.
            if(target.window!=nullptr&&target.outer.contains(*cursor)&&cursor->y<target.editor.getY()) {
                NativeDpiScope dpi;
                const auto hit=SendMessageW(target.window,WM_NCHITTEST,0,MAKELPARAM(cursor->x,cursor->y));
                captionTracking=hit==HTCAPTION;
                captionStart=*cursor;captionWasLarge=large();
            }
        }
        if(down&&captionTracking&&target.window!=nullptr) {
            captionMoved=captionMoved||cursor->getDistanceFrom(captionStart)>3;
            if(captionWasLarge&&captionMoved&&cursor->y>captionStart.y+8) {
                restoreWindow();captionWasLarge=false;
            }
        }
        if(!down&&captionDown&&captionTracking) {
            const auto work=workAt(*cursor);
            if(captionMoved&&!work.isEmpty()&&cursor->y<=work.getY()+12&&work.contains(juce::Point<int>(cursor->x,work.getY())))
                request(Mode::large);
            captionTracking=false;
        }
        captionDown=down;
#endif
    }
    WindowSizing& owner;
    juce::AudioProcessorEditor& editor;
    const double aspect;
    const int designWidth, designHeight;
    Mode mode = Mode::custom, restoreMode = Mode::custom;
    juce::Point<int> restoreSize;
    bool hasRestore = false, snapArmed = false, dragging = false, dragMoved = false, dragBeganLarge = false;
    bool applying = false, movingDrag = false;
    int settleTicks = 0;
    InitialFitTimer initialFitTimer{*this};
    bool initialFitComplete = false;
    int initialFitTicks = 0, initialStableTicks = 0;
    juce::Point<double> initialScale;
    juce::Rectangle<int> initialWork;
    juce::Component::SafePointer<juce::ResizableWindow> ownedDragWindow;
    juce::ComponentDragger ownedDragger;
    juce::Point<int> ownedStartCursor, ownedLastCursor;
    double ownedAnchorX = .5;
    int ownedAnchorY = 0;
#if JUCE_WINDOWS
    HWND pendingWindow = nullptr, dragWindow = nullptr;
    juce::Rectangle<int> pendingWork, restoreOuter;
    juce::Point<int> startCursor, startPosition;
    double dragAnchorX = .5;
    int dragAnchorY = 0;
    bool pendingCentre = false;
#else
    juce::Point<int> startCursor, startPosition;
    juce::Component::SafePointer<juce::ResizableWindow> dragStandalone;
#endif

    Impl(WindowSizing& o, juce::AudioProcessorEditor& e, int w, int h)
        : owner(o), editor(e), aspect(double(juce::jmax(1,w))/juce::jmax(1,h)),
          designWidth(juce::jmax(1,w)), designHeight(juce::jmax(1,h)) { editor.addComponentListener(this); }
    ~Impl() override { initialFitTimer.stop(); stopTimer(); editor.removeComponentListener(this); }

    void assertMessageThread() const { jassert(juce::MessageManager::getInstance()->isThisTheMessageThread()); }
    void changed() { if(owner.onChange) owner.onChange(); }
    juce::ResizableWindow* standalone() const {
        if (editor.getAudioProcessor()->wrapperType != juce::AudioProcessor::wrapperType_Standalone) return nullptr;
        auto* window = editor.findParentComponentOfClass<juce::ResizableWindow>();
        return window != nullptr && window->isOnDesktop() ? window : nullptr;
    }
    const juce::Displays::Display* display() const {
        const auto& displays = juce::Desktop::getInstance().getDisplays();
        if (const auto* d=displays.getDisplayForRect(editor.getScreenBounds())) return d;
        return displays.getPrimaryDisplay();
    }
    juce::Rectangle<int> availableInEditorUnits() const {
        if(const auto* d=display()) return editor.getLocalArea(nullptr,d->userBounds.toNearestInt());
        return {0,0,designWidth,designHeight};
    }
    void freeAspect(int width, int height) {
        // Host-chosen dimensions remain legal; the panel itself keeps its aspect.
        editor.setResizeLimits(juce::jmin(480,width),juce::jmin(260,height),juce::jmax(16384,width),juce::jmax(16384,height));
        if(auto* c=editor.getConstrainer()) c->setFixedAspectRatio(0.0);
    }

#if JUCE_WINDOWS
    NativeMetrics measureNative() const {
        NativeMetrics result;
        auto* peer=editor.getPeer(); if(peer==nullptr) return result;
        const auto child=static_cast<HWND>(peer->getNativeHandle());
        if(child==nullptr || !IsWindow(child)) return result;
        NativeDpiScope dpi;
        if(!dpi.ready()) return result;
        // Measure the actual child peer in physical pixels. Display scaling is
        // insufficient here: hosts can virtualise or separately scale a VST3 view.
        // Convert BOTH client corners in the same DPI context; do not combine a
        // physical screen origin with an unconverted virtualised client extent.
        RECT childClient{};
        if(!GetClientRect(child,&childClient))return result;
        POINT childOrigin{childClient.left,childClient.top}, childEnd{childClient.right,childClient.bottom};
        if(!ClientToScreen(child,&childOrigin)||!ClientToScreen(child,&childEnd))return result;
        const auto& peerComponent=peer->getComponent();
        if(peerComponent.getWidth()<=0||peerComponent.getHeight()<=0)return result;
        const auto inPeer=peerComponent.getLocalArea(&editor,editor.getLocalBounds().toFloat());
        const float px=float(childEnd.x-childOrigin.x)/float(peerComponent.getWidth());
        const float py=float(childEnd.y-childOrigin.y)/float(peerComponent.getHeight());
        const auto physicalEditor=juce::Rectangle<float>(childOrigin.x+inPeer.getX()*px,childOrigin.y+inPeer.getY()*py,
                                                        inPeer.getWidth()*px,inPeer.getHeight()*py).toNearestIntEdges();
        if(physicalEditor.isEmpty() || editor.getWidth()<=0 || editor.getHeight()<=0) return result;
        const double scaleX=double(physicalEditor.getWidth())/editor.getWidth();
        const double scaleY=double(physicalEditor.getHeight())/editor.getHeight();
        if(!std::isfinite(scaleX)||!std::isfinite(scaleY)||scaleX<.1||scaleY<.1||scaleX>16||scaleY>16)return result;
        const RECT editorRect{physicalEditor.getX(),physicalEditor.getY(),physicalEditor.getRight(),physicalEditor.getBottom()};
        MONITORINFO info{};info.cbSize=sizeof(info);
        if(!GetMonitorInfoW(MonitorFromRect(&editorRect,MONITOR_DEFAULTTONEAREST),&info))return result;
        result.child=child;result.editor=physicalEditor;result.work=rectangle(info.rcWork);
        result.scaleX=scaleX;result.scaleY=scaleY;

        // Frame decoration is optional. A rejected frame must never discard a
        // valid child scale or turn a DAW's main-window content into "chrome".
        const auto root=GetAncestor(child,GA_ROOT); // NEVER GA_ROOTOWNER.
        if(root==nullptr || !IsWindow(root) || !IsWindowVisible(root) || IsIconic(root))return result;
        DWORD pid=0;GetWindowThreadProcessId(root,&pid);
        if(pid!=GetCurrentProcessId() || (GetWindowLongPtrW(root,GWL_STYLE)&WS_CHILD)!=0)return result;
        RECT outer{},client{};
        if(!GetWindowRect(root,&outer)||!GetClientRect(root,&client))return result;
        POINT origin{client.left,client.top},end{client.right,client.bottom};
        if(!ClientToScreen(root,&origin)||!ClientToScreen(root,&end))return result;
        const juce::Rectangle<int> clientScreen(origin.x,origin.y,end.x-origin.x,end.y-origin.y);
        const auto* own=standalone();
        if(own!=nullptr) {
            const auto* ownPeer=own->getPeer();
            if(ownPeer==nullptr || ownPeer->getNativeHandle()!=root) return result;
        } else {
            // Read-only chrome measurement does not authorise moving this frame.
            // Require a floating owned/tool frame tightly surrounding the editor.
            const auto exStyle=GetWindowLongPtrW(root,GWL_EXSTYLE);
            if(GetWindow(root,GW_OWNER)==nullptr && (exStyle&WS_EX_TOOLWINDOW)==0) return result;
            const double scale=scaleX;
            const int tolerance=juce::roundToInt(8*scale);
            if(!clientScreen.expanded(tolerance).contains(physicalEditor)) return result;
            const int extraW=clientScreen.getWidth()-physicalEditor.getWidth();
            const int extraH=clientScreen.getHeight()-physicalEditor.getHeight();
            if(extraW < -tolerance || extraH < -tolerance || extraW>96*scale || extraH>128*scale) return result;
            if(double(physicalEditor.getWidth())*physicalEditor.getHeight() < .65*double(clientScreen.getWidth())*clientScreen.getHeight()) return result;
        }
        result.frame=root;result.outer=rectangle(outer);result.client=clientScreen;
        result.frameMeasured=true;result.standalone=own!=nullptr;
        return result;
    }
    NativeTarget native() const {
        NativeTarget result;
        const auto metrics=measureNative();
        if(!metrics.valid()||!metrics.frameMeasured)return result;
        DWORD pid=0;const auto thread=GetWindowThreadProcessId(metrics.frame,&pid);
        if(pid!=GetCurrentProcessId()||thread!=GetCurrentThreadId())return result;
        if(!metrics.standalone) {
            // Retain the original movement whitelist in addition to the ownership
            // and tight surrounding geometry already required for frame metrics.
            wchar_t title[256]{};GetWindowTextW(metrics.frame,title,256);
            const juce::String text(title);
            if(!text.containsIgnoreCase("KB-09")&&!text.containsIgnoreCase("SH-09")
               &&!text.containsIgnoreCase("SH09")&&!text.containsIgnoreCase("Toyland"))return result;
        }
        result.window=metrics.frame;result.outer=metrics.outer;result.editor=metrics.editor;
        result.work=metrics.work;result.pixelsPerEditorUnit=metrics.scaleX;result.standalone=metrics.standalone;
        return result;
    }
    bool moveNative(HWND expected,juce::Point<int> position) {
        const auto target=native(); if(target.window==nullptr || target.window!=expected) return false;
        NativeDpiScope dpi;
        if(!dpi.ready()) return false;
        return SetWindowPos(target.window,nullptr,position.x,position.y,0,0,
             SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOOWNERZORDER)!=0;
    }
    void settlePosition() {
        const auto target=native();if(target.window==nullptr||target.window!=pendingWindow)return;
        auto position=pendingWork.getPosition();
        if(pendingCentre) position=pendingWork.getCentre()-juce::Point<int>(target.outer.getWidth()/2,target.outer.getHeight()/2);
        moveNative(target.window,position);
    }
#endif

    void finishInitialFit() {
        initialFitTimer.stop();initialFitComplete=true;
    }
    void peerAttached() {
        assertMessageThread();
        if(initialFitComplete||initialFitTimer.active()||editor.getPeer()==nullptr||!editor.isShowing())return;
        if(mode==Mode::custom){finishInitialFit();return;}
        initialFitTicks=initialStableTicks=0;initialScale={};initialWork={};
        initialFitTimer.begin();
    }
    void initialFitTick() {
        assertMessageThread();
        ++initialFitTicks;
        if(!editor.isShowing()||dragging||juce::ModifierKeys::getCurrentModifiersRealtime().isAnyMouseButtonDown()) {
            finishInitialFit();return;
        }
        juce::Point<double> scale;
        juce::Rectangle<int> work;
#if JUCE_WINDOWS
        const auto metrics=measureNative();
        if(metrics.valid()){scale={metrics.scaleX,metrics.scaleY};work=metrics.work;}
#else
        if(auto* peer=editor.getPeer()) {
            const double value=peer->getPlatformScaleFactor();scale={value,value};work=availableInEditorUnits();
        }
#endif
        if(!work.isEmpty()&&scale.x>0&&scale.y>0) {
            const bool stable=work==initialWork&&std::abs(scale.x-initialScale.x)<.01&&std::abs(scale.y-initialScale.y)<.01;
            initialStableTicks=stable?initialStableTicks+1:1;
            initialScale=scale;initialWork=work;
            // JUCE's Windows peer-scale fallback samples at 500 ms. Allow that
            // first sample, then require three matching measurements. One fit
            // only; never keep resizing in response to our own host negotiation.
            if(initialFitTicks>=6&&initialStableTicks>=3) {
                const auto desired=mode;finishInitialFit();
                if(desired!=Mode::custom)request(desired);
                return;
            }
        } else initialStableTicks=0;
        if(initialFitTicks>=12)finishInitialFit(); // bounded to 1.44 seconds
    }
    void componentMovedOrResized(juce::Component&,bool,bool wasResized) override {
        if(wasResized&&!applying&&juce::ModifierKeys::getCurrentModifiersRealtime().isAnyMouseButtonDown())
            finishInitialFit();
    }
    void request(Mode requested) {
        assertMessageThread(); if(applying)return;
        // The constructor's pre-peer fallback remains eligible for initial fit.
        // Any subsequent explicit size request is a user choice and takes priority.
        if(initialFitTimer.active()||(editor.getPeer()!=nullptr&&editor.isShowing()))finishInitialFit();
        const juce::ScopedValueSetter<bool> guard(applying,true);
        stopTimer();settleTicks=0;
        if(requested==Mode::large && !large()) {
            const auto precedingMode=currentMode();
            restoreSize={editor.getWidth(),editor.getHeight()};restoreMode=precedingMode;hasRestore=true;
#if JUCE_WINDOWS
            restoreOuter=native().outer;
#endif
        }
        if(auto* window=standalone()) if(window->isFullScreen()) window->setFullScreen(false);
        const bool large=requested==Mode::large;
        const double fraction=large?1.0:requested==Mode::compact?smallFraction:mediumFraction;
        auto area=availableInEditorUnits().toFloat();
#if JUCE_WINDOWS
        const auto target=native();
        const auto metrics=measureNative();
        if(metrics.valid()) {
            // Unknown/docked host decoration uses a small logical allowance;
            // the monitor and editor scale still use measured physical pixels.
            const double chromeW=metrics.frameMeasured?juce::jmax(0,metrics.outer.getWidth()-metrics.editor.getWidth()):16*metrics.scaleX;
            const double chromeH=metrics.frameMeasured?juce::jmax(0,metrics.outer.getHeight()-metrics.editor.getHeight()):40*metrics.scaleY;
            area={0,0,float((metrics.work.getWidth()*fraction-chromeW)/metrics.scaleX),
                      float((metrics.work.getHeight()*fraction-chromeH)/metrics.scaleY)};
        } else {
            // A host-owned caption still consumes space even when its frame is
            // outside the conservative movement whitelist. Keep the footer visible.
            area=area.withSizeKeepingCentre(juce::jmax(100.0f,area.getWidth()-16),juce::jmax(100.0f,area.getHeight()-40));
            if(!large)area=area.withSizeKeepingCentre(area.getWidth()*float(fraction),area.getHeight()*float(fraction));
        }
#else
        if(!large)area=area.withSizeKeepingCentre(area.getWidth()*float(fraction),area.getHeight()*float(fraction));
#endif
        if(!large) area=fitAspect(area,aspect);
        const int w=juce::jmax(100,int(std::floor(area.getWidth()))),h=juce::jmax(100,int(std::floor(area.getHeight())));
        freeAspect(w,h);mode=requested;snapArmed=false;
        if(auto* window=standalone();large&&window!=nullptr) {
            // Maximise the existing normal window. Resizing it to monitor size
            // first would overwrite Windows' own Restore placement/dimensions.
            // The standalone's content component resizes the editor with its frame.
            window->setFullScreen(true);changed();return;
        }
        editor.setSize(w,h);
        if(auto* window=standalone()) {
            if(large)window->setFullScreen(true);
            else if(const auto* d=display())window->setCentrePosition(d->userBounds.getCentre().roundToInt());
        }
#if JUCE_WINDOWS
        if(target.window!=nullptr && !(large&&target.standalone)) {
            pendingWindow=target.window;pendingWork=target.work;pendingCentre=!large;
            settlePosition();settleTicks=8;startTimer(35); // bounded host-resize settling, max 280 ms
        }
#endif
        changed();
    }
    void restoreWindow() {
        assertMessageThread();finishInitialFit();stopTimer();settleTicks=0;
        if(!hasRestore){request(Mode::medium);return;}
        const juce::ScopedValueSetter<bool> guard(applying,true);
        if(auto* window=standalone())if(window->isFullScreen())window->setFullScreen(false);
        const auto available=availableInEditorUnits();
        const int w=juce::jlimit(100,juce::jmax(100,available.getWidth()),restoreSize.x);
        const int h=juce::jlimit(100,juce::jmax(100,available.getHeight()),restoreSize.y);
        freeAspect(w,h);editor.setSize(w,h);mode=restoreMode;snapArmed=false;
#if JUCE_WINDOWS
        const auto target=native();
        if(target.window!=nullptr && !restoreOuter.isEmpty()) {
            const auto x=juce::jlimit(target.work.getX(),juce::jmax(target.work.getX(),target.work.getRight()-target.outer.getWidth()),restoreOuter.getX());
            const auto y=juce::jlimit(target.work.getY(),juce::jmax(target.work.getY(),target.work.getBottom()-target.outer.getHeight()),restoreOuter.getY());
            moveNative(target.window,{x,y});
        }
#endif
        changed();
    }
    bool movable() const {
        if(standalone()!=nullptr)return true;
#if JUCE_WINDOWS
        return native().window!=nullptr;
#else
        return standalone()!=nullptr;
#endif
    }
    bool large() const {if(auto* w=standalone())return w->isFullScreen();return mode==Mode::large;}
    Mode currentMode() const {
        if(auto* w=standalone()){
            if(w->isFullScreen())return Mode::large;
            if(mode==Mode::large)return hasRestore&&restoreSize==juce::Point<int>(editor.getWidth(),editor.getHeight())?restoreMode:Mode::custom;
        }
        return mode;
    }
    void begin(const juce::MouseEvent& e) {
        assertMessageThread();if(!e.mods.isLeftButtonDown()||!movable())return;
        finishInitialFit();
        stopTimer();dragMoved=false;snapArmed=false;dragBeganLarge=large();
        ownedDragWindow=nullptr;
        if(auto* window=standalone()) {
            // JUCE owns this exact window and handles desktop/DPI coordinates.
            // Native floating-host ownership checks are unnecessary for our own frame.
            // The initial down point must come from the event, even if a fast
            // input batch has already moved the live pointer before dispatch.
            ownedDragWindow=window;ownedStartCursor=ownedLastCursor=e.getMouseDownScreenPosition();
            const auto down=e.getEventRelativeTo(window).getMouseDownPosition();
            ownedAnchorX=juce::jlimit(0.0,1.0,double(down.x)/juce::jmax(1,window->getWidth()));
            ownedAnchorY=juce::jlimit(0,100,down.y);
            ownedDragger.startDraggingComponent(window,e);
            dragging=true;return;
        }
#if JUCE_WINDOWS
        const auto target=native();if(target.window==nullptr)return;
        const auto cursor=cursorPixels();if(!cursor)return;
        dragWindow=target.window;startPosition=target.outer.getPosition();startCursor=*cursor;
        dragAnchorX=juce::jlimit(0.0,1.0,double(startCursor.x-target.outer.getX())/juce::jmax(1,target.outer.getWidth()));
        dragAnchorY=juce::jlimit(0,juce::roundToInt(100*target.pixelsPerEditorUnit),startCursor.y-target.outer.getY());
#else
        dragStandalone=standalone();startPosition=dragStandalone->getPosition();startCursor=e.getScreenPosition();
#endif
        dragging=true;
    }
    void drag(const juce::MouseEvent& e) {
        assertMessageThread();if(!dragging||!e.mods.isLeftButtonDown()||movingDrag)return;
        const juce::ScopedValueSetter<bool> movingGuard(movingDrag,true);
        bool armed=false;
        if(ownedDragWindow!=nullptr) {
            auto* window=ownedDragWindow.getComponent();
            // Cached event-local coordinates become stale when preceding queued
            // events move a desktop window. Use the live input-source position.
            const auto cursor=e.source.getScreenPosition().roundToInt();ownedLastCursor=cursor;
            auto delta=cursor-ownedStartCursor;
            if(delta.getDistanceFromOrigin()<3&&!dragMoved)return;
            if(dragBeganLarge&&!dragMoved) {
                restoreWindow();
                const juce::Point<float> anchor(float(ownedAnchorX*window->getWidth()),float(ownedAnchorY));
                const juce::MouseEvent rebased(e.source,anchor,e.mods,e.pressure,e.orientation,e.rotation,e.tiltX,e.tiltY,
                    window,window,e.eventTime,anchor,e.eventTime,1,true);
                ownedDragger.startDraggingComponent(window,rebased);dragBeganLarge=false;
            }
            // JUCE's dragger converts pointer/peer/desktop coordinate spaces and
            // supports per-monitor positioning without mixing window-local units.
            ownedDragger.dragComponent(window,e,nullptr);dragMoved=true;
            if(const auto* d=juce::Desktop::getInstance().getDisplays().getDisplayForPoint(cursor.toFloat()))
                armed=cursor.y<=d->userBounds.getY()+12 && cursor.x>=d->userBounds.getX() && cursor.x<d->userBounds.getRight();
            if(snapArmed!=armed){snapArmed=armed;changed();}
            return;
        }
#if JUCE_WINDOWS
        const auto cursorResult=cursorPixels();if(!cursorResult){dragging=false;snapArmed=false;changed();return;}
        const auto cursor=*cursorResult;auto delta=cursor-startCursor;
        if(delta.getDistanceFromOrigin()<3&&!dragMoved)return;
        if(dragBeganLarge&&!dragMoved){
            // A click on a maximised header must not restore it. Restore only once
            // an actual drag begins, keeping the grabbed header under the pointer.
            restoreWindow();const auto restored=native();
            if(restored.window==nullptr){dragging=false;return;}
            dragWindow=restored.window;
            startPosition={cursor.x-juce::roundToInt(dragAnchorX*restored.outer.getWidth()),cursor.y-dragAnchorY};
            startCursor=cursor;delta={};dragBeganLarge=false;
        }
        if(!moveNative(dragWindow,startPosition+delta)){dragging=false;return;}
        dragMoved=true;const auto target=native();const auto work=workAt(cursor);
        if(!work.isEmpty()&&target.window!=nullptr)armed=cursor.y<=work.getY()+juce::roundToInt(12*target.pixelsPerEditorUnit)&&cursor.x>=work.getX()&&cursor.x<work.getRight();
#else
        if(dragStandalone==nullptr){dragging=false;return;}
        const auto cursor=e.getScreenPosition();auto delta=cursor-startCursor;
        if(delta.getDistanceFromOrigin()<3&&!dragMoved)return;
        if(dragBeganLarge&&!dragMoved){restoreWindow();startPosition=dragStandalone->getPosition();startCursor=cursor;delta={};dragBeganLarge=false;}
        dragStandalone->setTopLeftPosition(startPosition+delta);dragMoved=true;
        if(const auto* d=juce::Desktop::getInstance().getDisplays().getDisplayForPoint(cursor.toFloat()))armed=cursor.y<=d->userBounds.getY()+12;
#endif
        if(snapArmed!=armed){snapArmed=armed;changed();}
    }
    void end(const juce::MouseEvent& e) {
        assertMessageThread();
        if(ownedDragWindow!=nullptr && dragging && dragMoved) {
            ownedLastCursor=e.source.getScreenPosition().roundToInt();
            const auto* d=juce::Desktop::getInstance().getDisplays().getDisplayForPoint(ownedLastCursor.toFloat());
            snapArmed=d!=nullptr && ownedLastCursor.y<=d->userBounds.getY()+12
                && ownedLastCursor.x>=d->userBounds.getX() && ownedLastCursor.x<d->userBounds.getRight();
        }
        const bool snap=dragging&&dragMoved&&snapArmed;
        dragging=false;snapArmed=false;
        if(ownedDragWindow!=nullptr) {
            if(snap) {
                if(const auto* d=juce::Desktop::getInstance().getDisplays().getDisplayForPoint(ownedLastCursor.toFloat()))
                    ownedDragWindow->setCentrePosition(d->userBounds.getCentre().roundToInt());
                request(Mode::large);
            } else changed();
            ownedDragWindow=nullptr;return;
        }
        if(snap){
#if JUCE_WINDOWS
            // Select the monitor under the released pointer, rather than leaving
            // a wide dragged window maximised on its previous monitor.
            const auto cursor=cursorPixels();if(!cursor){changed();return;}
            const auto target=native();const auto work=workAt(*cursor);
            if(target.window!=nullptr&&!work.isEmpty()&&target.work!=work)
                moveNative(target.window,work.getCentre()-juce::Point<int>(target.outer.getWidth()/2,target.outer.getHeight()/2));
#endif
            request(Mode::large);
        }else changed();
    }
    void timerCallback() override {
#if JUCE_WINDOWS
        if(editor.isShowing()&&!dragging&&(GetAsyncKeyState(VK_LBUTTON)&0x8000)==0)settlePosition();
#endif
        if(--settleTicks<=0)stopTimer();
    }
};

WindowSizing::WindowSizing(juce::AudioProcessorEditor& e,int w,int h):impl(std::make_unique<Impl>(*this,e,w,h)){}
WindowSizing::~WindowSizing()=default;
void WindowSizing::chooseSmall(){impl->request(Mode::compact);}
void WindowSizing::chooseMedium(){impl->request(Mode::medium);}
void WindowSizing::chooseLarge(){impl->request(Mode::large);}
void WindowSizing::toggleMaximise(){if(isLarge())restore();else chooseLarge();}
void WindowSizing::restore(){impl->restoreWindow();}
void WindowSizing::peerAttached(){impl->peerAttached();}
void WindowSizing::beginHeaderDrag(const juce::MouseEvent& e){impl->begin(e);}
void WindowSizing::dragHeader(const juce::MouseEvent& e){impl->drag(e);}
void WindowSizing::endHeaderDrag(const juce::MouseEvent& e){impl->end(e);}
void WindowSizing::doubleClickHeader(){toggleMaximise();}
bool WindowSizing::isLarge()const{return impl->large();}
bool WindowSizing::canMoveWindow()const{return impl->movable();}
bool WindowSizing::isSnapArmed()const{return impl->snapArmed;}
WindowSizing::Mode WindowSizing::getMode()const{return impl->currentMode();}
juce::String WindowSizing::placementStatus()const{return canMoveWindow()?"Drag the header to the screen top to maximise.":"This host controls window placement; S, M and L request editor sizes.";}
juce::Rectangle<float> WindowSizing::contentBounds(juce::Rectangle<float> area)const noexcept{return fitAspect(area,impl->aspect);}
}

