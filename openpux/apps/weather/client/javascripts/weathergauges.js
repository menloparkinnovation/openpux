
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
// Main page script
//

// Set this to true for reports on screen width, height, and orientation changes.
g_enableInstrumentation = false;

//
// Device specific scale factors experimentally determined
//
// Mode: scale 1.0 == 180px per gauge.
//
// Landscape mode is two rows of three gauge columns, fit within the client area.
//
// Portrait mode is three rows of two gauge columns, fit within the client area.
//
// Popular Devices:
//
// ipad1 1024x768 pixels, 132 ppi, 9.56 inches diagonal
// 
// ipad2 1024x768 pixels, 132 ppi, 9.56 inches diagonal
// 
// ipad air (retina) 2048x1536 pixels, 264 ppi, 9.7 inch diagonal
//   - screen.width reports 768, screen.height reports 1024
//     - This is half of spec.
// 
// ipad mini (retina) 2048x1536 pixels, 326 ppi
// 
// iphone6 1134 x 750 pixels, 326 ppi, 16:9 aspect ratio, 4.7 inch diagonal
// 
// iphone5 1136 x 640 pixels, 326 ppi, 16:9 aspect ratio, 4.0 inch diagonal
//   - screen.width reports 320, screen.height reports 568
//     - This is half of spec.
// 
// iphone4 960 x 640 pixels, 326 ppi, 16:9 aspect ratio, 3.5 inch diagonal
// 
// iphone3GS 320 x 480 pixels, 163 ppi, 3.5 inch diagonal
// 
// Google Nexus 7 (Asus) 800x1280 pixels, 216 ppi, 7.0 inches diagonal
// 

function calculateScaleFactor() {
    var height = screen.height;
    var width = screen.width;
    var orientation = window.orientation;
    var landscape = false;
    var scaleFactor = 1.0;

    if ((typeof(orientation) == "undefined") || (orientation == null)) {
        if (g_enableInstrumentation) {
            alert("window.orientation undefined, default to landscape");
        }
        landscape = true;
    }
    else {

        switch(orientation) {
        case 0:
        case 180:
            // portrait
            landscape = false;
            break;
     
        case -90:
        case 90:
        default:
            // landscape
            landscape = true;
            break;
        }
    }

    if ((width >= 300) && (width < 370)) {

        if (g_enableInstrumentation) {
            alert("iphone detected width=" + width + " height=" + height);
        }

        // iphones 3, 4, 5
        if (landscape) {
            scaleFactor = 0.85;
        }
        else {
            scaleFactor = 2.5;
        }
    }
    else if ((width >= 370) && (width <= 750)) {

        if (g_enableInstrumentation) {
            alert("iphone6 detected width=" + width + " height=" + height);
        }

        // iphone 6
        if (landscape) {
            scaleFactor = 0.85;
        }
        else {
            scaleFactor = 2.5;
        }
    }
    else if ((width == 768) && (height == 1024)) {

        if (g_enableInstrumentation) {
            alert("ipad detected width=" + width + " height=" + height);
        }

        // ipad 1, 2, air retina
        if (landscape) {
            scaleFactor = 1.5;
        }
        else {
            scaleFactor = 2.0;
        }
    }
    else {

        if (g_enableInstrumentation) {
            alert("unknown device detected width=" + width + " height=" + height);
        }

        scaleFactor = 1.0;
    }

    g_config.scale_factor = scaleFactor;

    return;
}

var g_config = {

    // scale_factor == 1.0 == 180px
    //scale_factor: 1.0,
    
    // RaspberryPi2 with official 7" screen in landscape mode
    scale_factor: 0.75,

    // Google Nexus 7" screen portrait mode
    //scale_factor: 2.0,

    // Google Nexus 7" screen landscape mode
    //scale_factor: 1.0,

    // iPhone5 in portrait mode
    //scale_factor: 2.5,

    // iPhone 5 in landscape mode
    // 163 ppi
    //scale_factor: 0.85,

    // iPad air retina in portrait mode
    // iPad2 retina in portrait mode
    // iPad 1 in portrait mode
    //scale_factor: 2.0,

    // ipad air retina in landscape mode
    // ipad2 retina in landscape mode
    // iPad 1 in landscape mode
    //scale_factor: 1.5,

    //
    // This sets the default mode for when window.orientation is unavailable.
    //

    // Landscape
    default_orientation: 90,

    // Portrait
    //default_orientation: 0,

    // Update rate that it acquires from the server
    update_rate: 30000,

    // Runtime variables
    current_gauges: null,

    portrait_gauges: null,

    landscape_gauges: null,

    //
    // This is a function exported from a lambda scope, but
    // available globally
    //
    updateGauges: null
};

function showResolution() {

    if (!g_enableInstrumentation) return;

    //
    // Note: On iPhone the screen width and height report remains the
    // same regardless of orientation.
    //
    var orientation = "orientation: " + window.orientation;

    var str = "width=" + screen.width + " height=" + screen.height;

    var str2 = "availwidth=" + screen.availWidth + " availheight=" + screen.availHeight;

    alert(orientation + " resolution:" + str + " " + str2);
}


function touchStart(event) {
    var x = event.touches[0].pageX;
    var y = event.touches[0].pageY;

    var s = "touchStart x=" + x + " y=" + y;

    event.preventDefault();

    updateStatusBar(s);
}

function touchMove(event) {
    var x = event.touches[0].pageX;
    var y = event.touches[0].pageY;

    var s = "touchMove x=" + x + " y=" + y;

    updateStatusBar(s);

    event.preventDefault();

    tableMarginCSS(x, y);
}

function touchEnd(event) {
    var x = event.touches[0].pageX;
    var y = event.touches[0].pageY;

    var s = "touchEnd x=" + x + " y=" + y;

    updateStatusBar(s);
}

function touchCancel(event) {
    var x = event.touches[0].pageX;
    var y = event.touches[0].pageY;

    var s = "touchCancel x=" + x + " y=" + y;

    updateStatusBar(s);
}

//
// Register touch events
//
document.addEventListener("touchstart", touchStart, false);
document.addEventListener("touchmove", touchMove, false);
document.addEventListener("touchend", touchEnd, false);
document.addEventListener("touchcancel", touchCancel, false);

showResolution();

// Set the table margin in pixels (float within the page)
tableMarginCSS(0, 0);

google.load("visualization", "1", {packages:["gauge"]});

google.setOnLoadCallback(drawChart);

// Calculate scale factor based on device
calculateScaleFactor();

function refreshMode() {

      if (g_config.current_gauges == null) {
          return;
      }

      // Clear the gauges, and force a redraw
      g_config.current_gauges.sizeChanged();

      g_config.updateGauges(g_config.current_gauges);
  }

function generate_table_margin_css(width, height) {
      var str = ".class_table_margin { margin-left: " + width + "px; ";
      str += "margin-top: " + height + "px; }";
      return str;
}

// Update the style DOM for gauge size
function tableMarginCSS(width, height) {

      var tableStyle = generate_table_margin_css(width, height);

      var st = document.getElementById("table_margin_style");
      st.innerHTML = tableStyle;
}

function generate_gauge_css(width, height) {
      var str = ".class_gauge { width: " + width + "px; ";
      str += "height: " + height + "px; }";
      return str;
}

// Update the style DOM for gauge size
function updateScaleCSS(scale) {

      var width = 180 * scale;
      var height = 180 * scale;

      var gaugeStyle = generate_gauge_css(width, height);

      var st = document.getElementById("gauge_style");
      st.innerHTML = gaugeStyle;
}

function updateStatusBar(status) {
      document.getElementById("status_bar_input").value = status;
}

//
// This gets run when the browser page loads
//
function drawChart() {

      updateStatusBar("Initializing...");

      // Defined in main .html page
      var config = getOpenpuxConfig();
      var opclient = new OpenpuxData(config);

      // Update CSS for gauge scale factor
      updateScaleCSS(g_config.scale_factor);

      g_config.landscape_gauges = new MyGauges("landscape");
      g_config.portrait_gauges = new MyGauges("portrait");

      //
      // We export this function to allow others functions to call it
      // by initializing its variable outside the scope of this function/closure.
      //
      g_config.updateGauges = function(gauges) {

          var data = {};

          opclient.getLatestReading(function (error, opdata) {
              if (error != null) {
                  updateStatusBar("Error: " + error + " at " + 
                      new Date().toISOString());
                  return;
              }

              data.wind_speed = opdata.windspeed;
              data.wind_dir = opdata.winddirection;
              data.temp = opdata.temperature;

              data.humid = opdata.humidity;

              // Convert to inHg
              data.baro = (opdata.barometer / 1000) * 0.295300;
              data.baro = data.baro.toFixed(1);

              data.rain = opdata.rainfall;

              gauges.update(data);

              updateStatusBar("Online last update " + new Date().toISOString());
          });
      }

      // Read orientation, enable display
      updateOrientation();

      setInterval(function() {
          g_config.updateGauges(g_config.current_gauges);
      }, g_config.update_rate);
}

//
// Generic Gauge Function
//
// element_id - <div/> to attach to in order to position gauge on the page.
//

function GenericGauge(element_id, options, label, initialValue) {

    this.element_id = element_id;
    this.options = options;

    // Each data row create a gauge instance
    this.dataTable = google.visualization.arrayToDataTable([
          ['Label',  'Value'],
          [label,     initialValue]
        ]);

    // Create the gauge
    this.gauge = 
            new google.visualization.Gauge(document.getElementById(this.element_id));

    this.gauge.draw(this.dataTable, this.options);
}

GenericGauge.prototype.update = function(value) {

    //
    // 0 - position one in gauge cluster sharing attributes
    // 1 - data column 1
    // 2 - data column 2
    //
    this.dataTable.setValue(0, 1, value);

    this.gauge.draw(this.dataTable, this.options);
}

GenericGauge.prototype.sizeChanged = function() {

    // Clear it
    this.gauge.clearChart();

    // re-create the gauge
    this.gauge = 
            new google.visualization.Gauge(document.getElementById(this.element_id));

    // re-draw with current values
    this.gauge.draw(this.dataTable, this.options);
}

//
// Utility functions for when the scale changes
//

//
// This is invoked from the HTML range control
//
function scale_changed() {

   var s = document.getElementById("scale_input");
   //alert("slider value " + s.value);

   var range = s.value / 100; // .25 - 2

   g_config.scale_factor = range;

   if (g_enableInstrumentation) {
       alert("scale_factor " + g_config.scale_factor);
   }

   updateScaleCSS(g_config.scale_factor);

   refreshMode();
}

//
// Utility functions for changing orientation
//

function setLandscapeMode() {

    // If landscape_gauges == null we have not initialized yet
    if (g_config.landscape_gauges == null) {
        return;
    }

    g_config.current_gauges = g_config.landscape_gauges;
    document.getElementById("portrait_display").style.display = "none";
    document.getElementById("landscape_display").style.display = "block";

    g_config.updateGauges(g_config.current_gauges);
}

function setPortraitMode() {

      // If portrait_gauges == null we have not initialized yet
      if (g_config.portrait_gauges == null) {
          return;
      }

      g_config.current_gauges = g_config.portrait_gauges;
      document.getElementById("landscape_display").style.display = "none";
      document.getElementById("portrait_display").style.display = "block";

      g_config.updateGauges(g_config.current_gauges);
}

function flipMode() {
      if (g_config.current_gauges == g_config.landscape_gauges) {
          setPortraitMode();
      }
      else {
          setLandscapeMode();
      }
}

function updateOrientation() {

    // Note: window.orientation is undefined in Chrome on Mac
    var orientation = window.orientation;
    if ((typeof(orientation) == "undefined") || (orientation == null)) {
        orientation = g_config.default_orientation;
    }

    showResolution();

    switch(orientation)
    {
        // Portrait
        case 0:

            // fallthrough

        // Portrait (upside-down portrait)
        case 180:
            setPortraitMode();
            break;

        // Landscape (right, screen turned clockwise)
        case -90:

            // fallthrough

        // Landscape (left, screen turned counterclockwise)
        case 90:

            // fallthrough

        default:

            setLandscapeMode();
            break;
    }

    // Change scale
    calculateScaleFactor();
}

function status_button_clicked() {
    // Force an orientation change
    flipMode();
    var s = "scale_factor=" + g_config.scale_factor;
    updateStatusBar("scale_factor=" + g_config.scale_factor);
    alert(s);
}

//
// Create a custom gauge set
//
function MyGauges(basename) {

    basename = basename + "_";

    //
    // Wind Speed
    //

    var windspeed_options = {

      //
      // Note: By not specifying, it will auto size to the container element
      // size set by the style for gauge_style.
      //
      //width: width,
      //height: height,

      redFrom: 50, redTo: 105,
      yellowFrom:30, yellowTo: 50,
      minorTicks: 5,
      min: 0,
      max: 105
    };

  this.windspeed_gauge =
      new GenericGauge(basename + "windspeed_gauge", windspeed_options, "Wind(MPH)", 0);

    //
    // Wind Direction
    //

    var winddir_options = {
      //redFrom: 50, redTo: 105,
      //yellowFrom:30, yellowTo: 50,
      minorTicks: 5,
      min: 0,
      max: 359
    };

  this.winddir_gauge =
      new GenericGauge(basename + "winddir_gauge", winddir_options, "Wind(D)", 0);

    var temp = google.visualization.arrayToDataTable([
      ['Label',  'Value'],
      ['Temp',     0]
    ]);

    //
    // Temperature
    //

    var temp_options = {
      redFrom: 100, redTo: 120,
      yellowFrom:85, yellowTo: 100,
      minorTicks: 5,
      min: 0,
      max: 120
    };

    this.temp_gauge = new GenericGauge(basename + "temp_gauge", temp_options, "Temp(F)", 0);

    //
    // Humidity
    //

    var humid_options = {
      redFrom: 95, redTo: 100,
      yellowFrom:85, yellowTo: 95,
      minorTicks: 5,
      min: 0,
      max: 100
    };

    this.humid_gauge = new GenericGauge(basename + "humid_gauge", humid_options, "Humid", 0);

    //
    // Baro
    //

    var baro_options = {
      redFrom: 19, redTo: 23,
      yellowFrom:23, yellowTo: 28,
      minorTicks: 5,
      min: 15,
      max: 38
    };

    this.baro_gauge = new GenericGauge(basename + "baro_gauge", baro_options, "Baro(inHg)", 0);

    //
    // Rain
    //

    var rain_options = {
      redFrom: 9, redTo: 10,
      yellowFrom:7, yellowTo: 9,
      minorTicks: 5,
      min: 0,
      max: 10
    };

    this.rain_gauge = new GenericGauge(basename + "rain_gauge", rain_options, "Rain(In)", 0);
}

MyGauges.prototype.update = function(data) {
    this.windspeed_gauge.update(data.wind_speed);
    this.winddir_gauge.update(data.wind_dir);
    this.temp_gauge.update(data.temp);

    this.humid_gauge.update(data.humid);
    this.baro_gauge.update(data.baro);
    this.rain_gauge.update(data.rain);
}

MyGauges.prototype.sizeChanged = function() {

    this.windspeed_gauge.sizeChanged();
    this.winddir_gauge.sizeChanged();
    this.temp_gauge.sizeChanged();

    this.humid_gauge.sizeChanged();
    this.baro_gauge.sizeChanged();
    this.rain_gauge.sizeChanged();
}

