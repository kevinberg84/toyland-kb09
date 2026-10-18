#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <atomic>
#include <memory>

namespace kb09_gui {
// Reserve before asking JUCE to create an asynchronous dialog. The modal
// component list alone cannot see a dialog whose creation is still queued.
class StartupDialogReservation {
public:
    class Lease {
    public:
        ~Lease() { reserved.store(false); }
    private:
        Lease() = default;
        friend class StartupDialogReservation;
    };
    static std::shared_ptr<Lease> tryAcquire() {
        jassert(juce::MessageManager::getInstance()->isThisTheMessageThread());
        if(juce::Component::getNumCurrentlyModalComponents()>0)return {};
        bool free=false;
        if(!reserved.compare_exchange_strong(free,true))return {};
        return std::shared_ptr<Lease>(new Lease);
    }
private:
    inline static std::atomic<bool> reserved{false};
};
}
