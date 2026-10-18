#include <juce_gui_basics/juce_gui_basics.h>
#import <Cocoa/Cocoa.h>
// Capture our own hosted view directly; no screen-recording permission or
// dependence on another application's foreground/occlusion is required.
int kb09RateHostRedPixels(void* handle) {
    @autoreleasepool {
        NSView* view=(NSView*)handle;if(!view)return -1;
        [view displayIfNeeded];
        NSBitmapImageRep* bitmap=[view bitmapImageRepForCachingDisplayInRect:[view bounds]];
        if(!bitmap)return -1;
        [view cacheDisplayInRect:[view bounds] toBitmapImageRep:bitmap];
        int count=0;
        const int width=int([bitmap pixelsWide]),height=int([bitmap pixelsHigh]);
        for(int y=0;y<height;y+=2)for(int x=0;x<int(width*.25);x+=2) {
            NSColor* colour=[[bitmap colorAtX:x y:y] colorUsingColorSpace:[NSColorSpace deviceRGBColorSpace]];
            if(!colour)continue;CGFloat red=0,green=0,blue=0,alpha=0;
            [colour getRed:&red green:&green blue:&blue alpha:&alpha];
            if(red>.43&&red>green*1.7&&red>blue*1.5)++count;
        }
        return count;
    }
}
