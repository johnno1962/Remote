#!/usr/bin/env python
#
# patch project's main.m to include Remote plugin's header source
#

import commands
import sys
import re
import os

sys.argv.pop(0)

resources = sys.argv.pop(0)
workspace = sys.argv.pop(0)

while re.match( r"(127|169|172)\.", sys.argv[0] ):
    sys.argv.pop(0)

ipaddrs = sys.argv

remotePort = os.getenv('REMOTE_PORT', None)
projectRoot = re.match(r"^((.*?/)([^/]*)\.(xcodeproj|xcworkspace|idea/misc.xml))",workspace).group(2)

status, text = commands.getstatusoutput("find '"+projectRoot+"' | grep -E 'main\\.mm?$' 2>&1")

if status or text == "":
    print text or "\nPlease add a main.m to your project so the remote capture code can be patched in."
    sys.exit(1)

for main in text.split("\n"):
    f = open(main, "r")
    source = f.read()
    source = re.sub(r"\s*// Remote Plugin patch start //.*// Remote Plugin patch end //\s*|\s*$", '''

// Remote Plugin patch start //

#ifdef DEBUG%s
#include "%s/RemoteCapture.h"
#define REMOTEPLUGIN_SERVERIPS "%s"
@implementation RemoteCapture(Startup)
+ (void)load {
    [self performSelectorInBackground:@selector(startCapture:) withObject:@REMOTEPLUGIN_SERVERIPS];
}
@end
#endif

// Remote Plugin patch end //
''' % ("\n#define REMOTE_PORT "+remotePort if remotePort else "", resources, " ".join(ipaddrs)), source, 1, re.DOTALL)
    f = open(main, "w")
    f.write(source)
    commands.getstatusoutput("open '"+main+"'")

