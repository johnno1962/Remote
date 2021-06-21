
//#define REMOTE_HYBRID
//#define REMOTE_MINICAP
//#define REMOTE_PORT 1313
//#define REMOTE_PNGFORMAT
//#define REMOTE_APPNAME GenericCapture
//#define REMOTE_OVERSAMPLE *(float *)core.device.remote.scale
//#define REMOTE_BENCHMARK
//#define REMOTE_DEFER 0.5
//#define REMOTE_MAXDEFER 0.1

#define REMOTE_IMPL
#import "RemoteCapture.h"

#ifdef DEVELOPER_HOST
@implementation RemoteCapture(AutoConnect)
+ (void)load {
    [self setFormat:HYBRID_VERSION port:1313 retries:3 sleep:1.0];
    [self setDefer:0.5 maxDefer:0.1 benchmark:TRUE];
    [self startCapture:@DEVELOPER_HOST];
}
@end
#endif
