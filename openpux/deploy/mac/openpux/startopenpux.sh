
DEPLOY=$HOME/openpux_test/openpux

cd $DEPLOY/deploy/mac/openpux

forever start -a --uid "openpux" -c sh openpux.sh
