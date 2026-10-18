#pragma once
#include <juce_graphics/juce_graphics.h>
#include <algorithm>
#include <vector>

namespace kb09_gui {
// Demand-filled only: no startup/idle renders on the message thread. Images
// are immutable while retained. Evict least-recently-used entries by bytes.
class AgeRasterCache {
public:
    explicit AgeRasterCache(size_t limit=64u*1024u*1024u):budget(limit){}
    juce::Image find(const juce::String& key) {
        for(auto& entry:entries)if(entry.key==key){entry.used=++clock;return entry.image;}
        return {};
    }
    void put(const juce::String& key,const juce::Image& image) {
        const size_t cost=size_t(image.getWidth())*size_t(image.getHeight())*4;
        if(!image.isValid()||cost>budget)return;
        for(auto it=entries.begin();it!=entries.end();++it)if(it->key==key){bytes-=it->bytes;entries.erase(it);break;}
        while(bytes+cost>budget&&!entries.empty()) {
            const auto oldest=std::min_element(entries.begin(),entries.end(),[](const auto& a,const auto& b){return a.used<b.used;});
            bytes-=oldest->bytes;entries.erase(oldest);
        }
        entries.push_back({key,image,cost,++clock});bytes+=cost;
    }
    void setBudget(size_t limit) {
        budget=limit;
        while(bytes>budget&&!entries.empty()) {
            const auto oldest=std::min_element(entries.begin(),entries.end(),[](const auto& a,const auto& b){return a.used<b.used;});
            bytes-=oldest->bytes;entries.erase(oldest);
        }
    }
    size_t limitBytes() const{return budget;}
    void clear(){entries.clear();bytes=0;}
    size_t sizeBytes() const{return bytes;}
private:
    struct Entry {juce::String key;juce::Image image;size_t bytes;uint64_t used;};
    std::vector<Entry> entries;size_t bytes=0,budget;uint64_t clock=0;
};
}
