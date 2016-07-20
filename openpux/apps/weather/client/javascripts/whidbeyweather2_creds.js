
//
//   Openpux Internet Of Things (IOT) Framework.
//
//   Copyright (C) 2016 Menlo Park Innovation LLC
//
//   menloparkinnovation.com
//   menloparkinnovation@gmail.com
//
//   Weather Station Google Gauges Library
//

//
// Return default credentials for read-only visitors to the weatherstation site.
//
// This should be setup with your applications credential that gives read-only
// permissions and resource limits to the world.
//
// It's returned in JSONP style from a <script> tag in the page.
//

function getOpenpuxConfig() {
    return {cloud_token: "12345678", cloud_account: "1", cloud_sensor_id: "2"};
}
