#include "public.sdk/source/vst/hosting/module.h"
#include "public.sdk/source/vst/hosting/plugprovider.h"
#include "public.sdk/source/vst/hosting/hostclasses.h"
#include "public.sdk/source/common/memorystream.h"
#include "pluginterfaces/vst/ivstcomponent.h"
#include "pluginterfaces/vst/ivsteditcontroller.h"
#include "pluginterfaces/vst/ivstaudioprocessor.h"
#include "public.sdk/source/vst/hosting/parameterchanges.h"
#include <iostream>
#include <string>
#include <vector>
#include <cstring>
using namespace Steinberg;
using namespace Steinberg::Vst;
std::string save(IComponent* component) {
 MemoryStream s;component->getState(&s); return {s.getData(),size_t(s.getSize())};
}
void restore(IComponent* component, IEditController* controller, const std::string& bytes) {
 MemoryStream s((void*)bytes.data(),bytes.size()); component->setState(&s);
 s.seek(0,IBStream::kIBSeekSet,nullptr);controller->setComponentState(&s);
}
struct Handler : IComponentHandler {
 std::vector<std::pair<ParamID,ParamValue>> edits;
 tresult PLUGIN_API queryInterface(const TUID id,void** out) override {
  if(std::memcmp(id,IComponentHandler::iid.toTUID(),16)==0||std::memcmp(id,FUnknown::iid.toTUID(),16)==0){*out=this;return kResultOk;}*out=nullptr;return kNoInterface;
 }
 uint32 PLUGIN_API addRef() override{return 1;}
 uint32 PLUGIN_API release() override{return 1;}
 tresult PLUGIN_API beginEdit(ParamID) override{return kResultOk;}
 tresult PLUGIN_API endEdit(ParamID) override{return kResultOk;}
 tresult PLUGIN_API performEdit(ParamID p,ParamValue v) override{edits.push_back({p,v});return kResultOk;}
 tresult PLUGIN_API restartComponent(int32) override{return kResultOk;}
};
int main(int argc,char**argv) {
 std::cout<<std::unitbuf;
 if(argc<2)return 2;
 HostApplication host;PluginContextFactory::instance().setPluginContext(&host);
 int failures=0,classesTested=0;
 std::string error;auto module=VST3::Hosting::Module::create(argv[1],error);
 if(!module){std::cerr<<error;return 2;}
 module->getFactory().setHostContext(&host);
 for(const auto& info:module->getFactory().classInfos()) {
  if(info.category()!="Audio Module Class")continue;
  ++classesTested;
  PlugProvider provider(module->getFactory(),info,true);if(!provider.initialize())return 3;
  auto c=provider.getComponentPtr();auto e=provider.getControllerPtr();
  ParamID program=0;int count=0;
  for(int i=0;i<e->getParameterCount();++i){ParameterInfo p{};e->getParameterInfo(i,p);if(p.flags&ParameterInfo::kIsProgramChange){program=p.id;count=p.stepCount+1;}}
  if(count!=24)return 4;
  std::cout<<"Program id="<<program<<" count="<<count<<"\n";
  {
   Handler handler;e->setComponentHandler(&handler);
   auto audio=U::cast<IAudioProcessor>(c);
   ProcessSetup setup{};setup.processMode=kRealtime;setup.symbolicSampleSize=kSample32;setup.maxSamplesPerBlock=64;setup.sampleRate=48000;audio->setupProcessing(setup);
   int failed=0;
   for(int preset=0;preset<24;++preset){
    e->setParamNormalized(program,double(preset)/23);
    auto custom=save(c);const auto cutoff=custom.find("id=\"cutoff\"");if(cutoff==std::string::npos)return 5;
    auto v=custom.find("value=\"",cutoff);if(v==std::string::npos)return 5;v+=7;auto end=custom.find('"',v);if(end==std::string::npos)return 5;custom.replace(v,end-v,"0.123456");
    auto xmlEnd=custom.find('\0',8);uint32 length=uint32(xmlEnd-8);std::memcpy(custom.data()+4,&length,4);
    e->setParamNormalized(program,double((preset+1)%24)/23);handler.edits.clear();
    e->setParamNormalized(program,double(preset)/23);auto pending=handler.edits;
    restore(c,e,custom);
    ParameterChanges changes;for(auto [id,value]:pending){int32 q=0,point=0;changes.addParameterData(id,q)->addPoint(0,value,point);}
    ProcessData data{};data.symbolicSampleSize=kSample32;data.processMode=kRealtime;data.inputParameterChanges=&changes;audio->process(data);
    const bool retained=save(c).find("0.123456")!=std::string::npos;
    std::cout<<"pending_factory_edits preset="<<preset<<" count="<<pending.size()<<" retained="<<retained<<"\n";
    if(!retained)++failed;
   }
   e->setComponentHandler(nullptr);
   std::cout<<"pending_factory_edits failures="<<failed<<"\n";failures+=failed;
  }

 }
 return failures||classesTested==0?1:0;
}
