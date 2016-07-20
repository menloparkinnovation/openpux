
#
# This ensures all the application npm dependencies are installed as well.
#
# The root level package.json only includes the npm dependencies for the core
# server. Which application modules are activated is configuration dependent
# so they manage their own package.json for npm install.
#
npm install
cd apps && ./provision.sh
