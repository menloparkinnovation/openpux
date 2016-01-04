
//
// Amazon AWS SimpleDB storage support
//
// This interfaces the openpux schema to the specific requirements
// of AWS SimpleDB.
//
// https://aws.amazon.com/sdk-for-node-js/
//
// npm install aws-sdk
// npm install node-uuid
//
// Note: You must setup your AWS SimpleDB authentication to the SimpleDB
// cloud service and place the authentication file somewhere in your setups
// root, its parent, or a configured directory as per the AWS instructions.
//
// Provision openpux data domain with:
//
//   node dataproviders/awssimpledb/util/simpledb_util.js createdomain
//
//   util/bin/provision.sh
//
// Verify openpux data domain with:
//
//   dataproviders/awssimpledb/util/queryall.sh
//
//   node dataproviders/awssimpledb/util/simpledb_util.js query all
//
// General Utility:
//
//   node dataproviders/awssimpledb/util/simpledb_util.js help
//
// Delete openpux domain *** AND ALL DATA *** with:
//
//   node dataproviders/awssimpledb/util/simpledb_util.js dEleteDomaiN
//
// Regenerate openpux domain deleting *** ALL DATA ***
//
//   dataproviders/awssimpledb/util/regenerate.sh
//
//   util/bin/provision.sh
//
