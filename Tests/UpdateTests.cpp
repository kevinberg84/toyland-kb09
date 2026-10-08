#include "../Source/Plugin/Processor.h"
#include "../Source/GUI/UpdateChecker.h"
#include <iostream>
int main(){int fails=0;auto check=[&](bool x){if(!x)++fails;};
check(kb09::Updates::newer("0.13.14","0.13.13"));check(kb09::Updates::newer("0.14.0","0.13.99"));check(!kb09::Updates::newer("0.13.14","0.13.14"));check(!kb09::Updates::newer("0.9.0","0.13.14"));check(!kb09::Updates::newer("bad.1.0","0.13.14"));check(!kb09::Updates::validVersion("1.2"));check(!kb09::Updates::validVersion("1.2.3-beta"));check(!kb09::Updates::validVersion("1..3"));std::cout<<"Update version checks: "<<fails<<" failures\n";return fails?1:0;}
