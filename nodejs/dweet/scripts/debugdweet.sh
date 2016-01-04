
# verbose print
#node main.js -verbose $*

# node inspector
node-debug main.js $*
#
# F8  - Continue/run
# F10 - Step
#
#

# Default node debugger has known problems with programs that use stdin
#node debug main.js  $*

# bt - stacktrace
# next, n - step next
# step, s - step in
# out, o - step out
# c - continue
# quit

#node main.js testdevice $*


