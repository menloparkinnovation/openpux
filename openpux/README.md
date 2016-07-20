
openpux
=======

Internet Of Things (IOT) Application Server. Based on Node.js, Client side Javascript.

Installation
=======

npm install

  Installs the core server and its depenencies.

npm start

  Runs the default server against Amazon SimpleDB data store

npm run jsondb

  Runs the default server against a local database.

npm run memory

  Runs the default server against a local in memory database.

npm run test

  Runs regression test

bin/provision.sh

 Installs npm dependencies for optional application modules such
 as Twilio, Particle Cloud, etc.

