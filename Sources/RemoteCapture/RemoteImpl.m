
#define REMOTE_HYBRID
//#define REMOTE_MINICAP
#define REMOTE_PORT 1313
//#define REMOTE_PNGFORMAT
//#undef REMOTEPLUGIN_SERVERIPS
//#define REMOTE_APPNAME GenericCapture

#define REMOTE_IMPL
#import "RemoteCapture.h"

#ifdef DEVELOPER_HOST
@implementation RemoteCapture(AutoConnect)
+ (void)load {
    [self startCapture:@DEVELOPER_HOST];
}
@end
#endif
