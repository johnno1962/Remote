#!/usr/bin/env python
#
# unpatch project's main.m to include Remote plugin's header source
#

import commands
import sys
import re

sys.argv.pop(0)

resources = sys.argv.pop(0)
workspace = sys.argv.pop(0)

projectRoot = re.match(r"^((.*?/)([^/]*)\.(xcodeproj|xcworkspace|idea/misc.xml))",workspace).group(2)

status, text = commands.getstatusoutput("find '"+projectRoot+"' | grep -E 'main\\.mm?$'")

if status:
    print text
    sys.exit(status)

for main in text.split("\n"):
    f = open(main, "r")
    source = f.read()
    source = re.sub(r"\s*// Remote Plugin patch start //.*// Remote Plugin patch end //\s*|\s*$", '''
''', source, 1, re.DOTALL)
    f = open(main, "w")
    f.write(source)
    commands.getstatusoutput("open '"+main+"'")
