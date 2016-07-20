
Configuration files provide the master configuration for a deployment
of openpux.

applications.json provide the configured applications used by
all backing store types.

default_config.json - Configuration file used when "npm start" is invoked.
                      Defaults to memory_config.json, but may be any file for
                      a particular deployment.

awssimpledb_config.json - Amazon SimpleDB cloud storage.

resin_awssimpledb_config.json - Amazon SimpleDB cloud storage supported by resin.io.

jsondb_config.json - local database storage using jsondb (openpux internal)

memory_config.json - No persistent store, in memory only. Requires provisioning
                     at each startup.


