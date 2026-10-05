#include "systemperf.h"

#import <Foundation/Foundation.h>

static id<NSObject> s_StreamingActivity;

void SystemPerf::beginStreaming()
{ @autoreleasepool {
    if (s_StreamingActivity != nil) {
        return;
    }

    // Keep App Nap and timer coalescing from throttling us if the stream
    // window isn't frontmost, and request the highest timer and I/O precision
    // the system offers. We still allow idle system sleep since display sleep
    // is handled separately by the keep awake option.
    s_StreamingActivity = [[[NSProcessInfo processInfo]
                            beginActivityWithOptions:NSActivityUserInitiatedAllowingIdleSystemSleep | NSActivityLatencyCritical
                            reason:@"Game streaming"] retain];
}}

void SystemPerf::endStreaming()
{ @autoreleasepool {
    if (s_StreamingActivity == nil) {
        return;
    }

    [[NSProcessInfo processInfo] endActivity:s_StreamingActivity];
    [s_StreamingActivity release];
    s_StreamingActivity = nil;
}}
