
The var directory contains read/write state from a deployment.

This allows the openpux server and its applications to operate from a
readonly root.

var maybe a symlink to a writable volume on a deployment with a
readonly root, or readonly application root.

