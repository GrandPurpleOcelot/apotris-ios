// iOS implementations of the platform hooks that Android provides via JNI
// in source/liba_android.cpp.
#ifdef IOS

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>

#include <string>

// Saves, preferences, skins and extracted assets live in the app's
// Documents folder, which the Files app shows under "On My iPhone".
std::string getDocumentsPath() {
    static std::string path;
    if (path.empty()) {
        @autoreleasepool {
            NSArray<NSURL*>* urls = [[NSFileManager defaultManager]
                URLsForDirectory:NSDocumentDirectory
                       inDomains:NSUserDomainMask];
            path = std::string(urls.firstObject.path.UTF8String) + "/";
        }
    }
    return path;
}

static UIImpactFeedbackGenerator* feedbackGenerator = nil;

bool vibrationInit() {
    feedbackGenerator = [[UIImpactFeedbackGenerator alloc]
        initWithStyle:UIImpactFeedbackStyleLight];
    [feedbackGenerator prepare];
    return true;
}

// Android vibrates for durationMS; the Taptic Engine plays discrete taps,
// so map the duration onto tap intensity instead.
void vibrationStart(int durationMS) {
    if (!feedbackGenerator || durationMS <= 0)
        return;
    CGFloat intensity = MIN(1.0, MAX(0.2, durationMS / 32.0));
    [feedbackGenerator impactOccurredWithIntensity:intensity];
    [feedbackGenerator prepare];
}

void vibrationStop() {}

void vibrationDeinit() { feedbackGenerator = nil; }

#endif
