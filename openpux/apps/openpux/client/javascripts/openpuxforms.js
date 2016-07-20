
//
//   openpuxforms.js
//
//   Openpux Internet Of Things (IOT) Framework.
//
//   Copyright (C) 2014,2015 Menlo Park Innovation LLC
//
//   menloparkinnovation.com
//   menloparkinnovation@gmail.com
//
//   Snapshot License
//
//   This license is for a specific snapshot of a base work of
//   Menlo Park Innovation LLC on a non-exclusive basis with no warranty
//   or obligation for future updates. This work, any portion, or derivative
//   of it may be made available under other license terms by
//   Menlo Park Innovation LLC without notice or obligation to this license.
//
//   There is no warranty, statement of fitness, statement of
//   fitness for any purpose, and no statements as to infringements
//   on any patents.
//
//   Menlo Park Innovation has no obligation to offer support, updates,
//   future revisions and improvements, source code, source code downloads,
//   media, etc.
//
//   This specific snapshot is made available under the following license:
//
//   Licensed under the Apache License, Version 2.0 (the "License");
//   you may not use this file except in compliance with the License.
//   You may obtain a copy of the License at
//
//       http://www.apache.org/licenses/LICENSE-2.0
//
//   Unless required by applicable law or agreed to in writing, software
//   distributed under the License is distributed on an "AS IS" BASIS,
//   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//   See the License for the specific language governing permissions and
//   limitations under the License.
//

//
// Openpux Forms Library
//

//
// This names all known forms to the application.
//
var FormFactoryConfiguration = [
    "NavigationForm",
    "SensorAccountForm",
    "QuerySensorReadingsForm",
    "QuerySensorReadingsResponseForm",
    "AddSensorReadingForm",
    "AddSensorReadingResponseForm",
    "QuerySensorSettingsForm",
    "QuerySensorSettingsResponseForm",
    "UpdateSensorSettingsForm",
    "AddSensorForm",
    "AddAccountForm"
]

var g_opclient = null;

// Invoked when the page loads
function showPage() {

    g_opclient = new OpenpuxClient();

    // Create a form for navigation buttons
    enableForm("NavigationForm");

    // Create a form for the sensor account information
    enableForm("SensorAccountForm");
}

//
// SensorSettingsAttributes, SensorReadingAttributes are set by specialization
// files and represents the names of the attributes sent and received to the
// server over the REST protocol.
//
// A user friendly display name can be used to bind the user name to the
// internal name used by the forms and REST protocol.
//
//  SensorReadingAttributes = {
//      SensorReading0: "SensorReading0",
//                  ...
//  }
//
//  SensorSettingsAttributes = {
//      TargetMask0: "TargetMask0",
//                  ...
//  }
//

//
// Openpux Navigation Form
//

NavigationForm.prototype = Object.create(GenericForm.prototype);
NavigationForm.prototype.constructor = NavigationForm;

function NavigationForm(name) {

    var script = null;

    GenericForm.call(
        this,
        name,
        name + "action",
        "get",
        null,
        "application/x-www-form-urlencoded"
    );

    this.appendTableStart();
    
    this.appendTableRowStart();

    this.appendTableDataStart();

      script = "getForm(\"" + name + "\").onNavigateQuerySensorReadings()";

      this.createInputByArgs({
          name:  "QuerySensorReadings",
          type:  "button",
          title: null,
          value: "Query Readings",
          onclick:    script,
          isAttribute: false
          });

    this.appendTableDataEnd();

    this.appendTableDataStart();

      script = "getForm(\"" + name + "\").onNavigateAddSensorReading()";

      this.createInputByArgs({
          name:  "AddSensorReading",
          type:  "button",
          title: null,
          value: "Add Reading",
          onclick:    script,
          isAttribute: false
          });

    this.appendTableDataEnd();

    this.appendTableDataStart();

      script = "getForm(\"" + name + "\").onNavigateQuerySensorSettings()";

      this.createInputByArgs({
          name:  "QuerySensorSettings",
          type:  "button",
          title: null,
          value: "Query Settings",
          onclick:    script,
          isAttribute: false
          });

    this.appendTableDataEnd();

    this.appendTableDataStart();

      script = "getForm(\"" + name + "\").onNavigateUpdateSensorSettings()";

      this.createInputByArgs({
          name:  "UpdateSensorSettings",
          type:  "button",
          title: null,
          value: "Update Settings",
          onclick:    script,
          isAttribute: false
          });

    this.appendTableDataEnd();

    this.appendTableDataStart();

      script = "getForm(\"" + name + "\").onNavigateAddSensor()";

      this.createInputByArgs({
          name:  "AddSensor",
          type:  "button",
          title: null,
          value: "Add Sensor",
          onclick:    script,
          isAttribute: false
          });

    this.appendTableDataEnd();

    this.appendTableDataStart();

      script = "getForm(\"" + name + "\").onNavigateAddAccount()";

      this.createInputByArgs({
          name:  "AddAccount",
          type:  "button",
          title: null,
          value: "Add Account",
          onclick:    script,
          isAttribute: false
          });

    this.appendTableDataEnd();

    this.appendTableDataStart();

      script = "location.reload();"

      this.createInputByArgs({
          name:  "Reload",
          type:  "button",
          title: null,
          value: "Reload",
          onclick:    script,
          isAttribute: false
          });

    this.appendTableDataEnd();

    this.stop();

    this.appendTableRowEnd();

    this.appendTableEnd();
}

NavigationForm.prototype.onNavigateQuerySensorReadings = function() {
    // Disable previous form(s)
    this.disableAppForms();

    enableForm("QuerySensorReadingsForm");
}

NavigationForm.prototype.onNavigateAddSensorReading = function() {
    // Disable previous form(s)
    this.disableAppForms();

    // Show Add Sensor Reading form
    enableForm("AddSensorReadingForm");
}

NavigationForm.prototype.onNavigateQuerySensorSettings = function() {
    // Disable previous form(s)
    this.disableAppForms();

    // Show sensor update form
    enableForm("QuerySensorSettingsForm");
}

NavigationForm.prototype.onNavigateUpdateSensorSettings = function() {
    // Disable previous form(s)
    this.disableAppForms();

    // Show sensor update form
    enableForm("UpdateSensorSettingsForm");
}

NavigationForm.prototype.onNavigateAddSensor = function() {
    // Disable previous form(s)
    this.disableAppForms();

    // Show add sensor form
    enableForm("AddSensorForm");
}

NavigationForm.prototype.onNavigateAddAccount = function() {
    // Disable previous form(s)
    this.disableAppForms();

    // Show add account form
    enableForm("AddAccountForm");
}

// Disable app forms, keeping navigation buttons and account information
NavigationForm.prototype.disableAppForms = function() {

    // Forms is defined by the forms package
    var forms = getForms();

    for (var prop in forms) {
        if (prop == "NavigationForm") continue;
        if (prop == "SensorAccountForm") continue;
        disableForm(prop);
    }
}

//
// End Openpux Navigation Form
//

//
// Openpux SensorAccount
//

SensorAccountForm.prototype = Object.create(GenericForm.prototype);
SensorAccountForm.prototype.constructor = SensorAccountForm;

function SensorAccountForm(name) {

    GenericForm.call(
        this,
        name,
        name + "action",
        "get",
        null,
        "application/x-www-form-urlencoded"
    );

    this.createInputByArgs({
        name:  "Ticket",
        type:  "text",
        title: "Ticket",
        value: "12345678",
        onclick: null,
        isAttribute: true
        });

    this.createInputByArgs({
        name:  "AccountID",
        type:  "text",
        title: "AccountID",
        value: "1",
        onclick: null,
        isAttribute: true
        });

    this.createInputByArgs({
        name:  "SensorID",
        type:  "text",
        title: "SensorID",
        value: "1",
        onclick: null,
        isAttribute: true
        });

    // Finished adding input controls
    this.stop();
}

//
// Validate credentials are input
//
function ValidateCredentialsInput(account) {

  // Clear any previous ticket since it could be updated
  account.ticket = null;

  var host_args = {
      scheme: location.protocol,
      hostname: location.host,
      port: null // embedded in hostname
  };

  //
  // An AccountID must be present.
  //
  if (account.AccountID == null || account.AccountID == "") {
      alert("Must supply AccountID");
      return false;
  }

  //
  // A ticket must be present
  //
  if ((account.Ticket != null) && (account.Ticket != "")) {

      account.ticket = g_opclient.createTicket(account.Ticket, "/", host_args);

      if (account.ticket == null) {
          return false;
      }

      return true;
  }
  else {
      alert("Must supply Ticket");
      return false;
  }

  return true;
}

//
// End Openpux SensorAccount
//

//
// Openpux QuerySensorSettings
//

//
// Create a form for querying sensor settings.
//
QuerySensorSettingsForm.prototype = Object.create(GenericForm.prototype);
QuerySensorSettingsForm.prototype.constructor = QuerySensorSettingsForm;

function QuerySensorSettingsForm(name) {

    GenericForm.call(
        this,
        name,
        name + "action",
        "post",
        null,
        "application/x-www-form-urlencoded"
    );

    var script = "javascript: getForm(\"" + name + "\").processOnSubmit(\"" + name + "\")";
    this.appendSubmitButton(script, "Query Sensor Settings");
    this.appendBreak();

    this.createInputByArgs({
        name:  "Status",
        type:  "text",
        title: "Status",
        value: "",
        onclick: null,
        isAttribute: false
        });

    this.createInputByArgs({
        name:  "Error",
        type:  "text",
        title: "Error",
        value: "",
        onclick: null,
        isAttribute: false
        });

    this.stop();
}

//
// Process the sensor query form
//
// This is invoked on submit of the sensor settings query form.
//
QuerySensorSettingsForm.prototype.processOnSubmit = function()
{
  var self = this;

  //
  // Reset form values for the new transaction
  //
  self.setAttribute("Status", "pending...");
  self.setAttribute("Error", "");

  deleteForm("QuerySensorSettingsResponseForm");

  //
  // Get input values from the DOM
  //

  // Get account information
  var account = getForm("SensorAccountForm").getProperties();

  //
  // A ticket and AccountId must be present
  //
  if (!ValidateCredentialsInput(account)) {
      return;
  }

  if (account.SensorID == null || account.SensorID == "") {
      alert("Must supply SensorID");
      return;
  }

  var args = {
      AccountID: account.AccountID,
      SensorID: account.SensorID
  };

  g_opclient.querySensorSettings(account.ticket, args, function(error, response) {

      if (error != null) {
          self.setAttribute("Status", error);
          return;
      }

      var status = response.status;
      self.setAttribute("Status", status);

      if (status != 200) {
          if (response.error != null) {
              self.setAttribute("Error", response.error);
          }
          return;
      }

      responseForm = getForm(
          "QuerySensorSettingsResponseForm", // instanceName
          "QuerySensorSettingsResponseForm", // formType
          response.sensorsettings            // args
          );

      self.displaySensorSettings(responseForm, response.sensorsettings, status);
  });

  return;
}

QuerySensorSettingsForm.prototype.displaySensorSettings =
    function(responseForm, sensorsettings, status) {

    if (typeof(sensorsettings) == "undefined") {
        // Update status
        this.setAttribute("Status", status + " No settings returned");
        return;
    }

    // Update status
    this.setAttribute("Status", status);

    for (var prop in sensorsettings) {

        // There may not be a form entry for the response item
        if (responseForm.getAttribute(prop) == null) continue;

        if (sensorsettings[prop] == null) continue;

        responseForm.setAttribute(prop, sensorsettings[prop]);
    }
}

//
// This form is used to display the response from querySensorSettings
//
QuerySensorSettingsResponseForm.prototype = Object.create(GenericForm.prototype);
QuerySensorSettingsResponseForm.prototype.constructor = QuerySensorSettingsResponseForm;

//
// Generate a response form for a set of sensor settings.
//
// Args contains the object that lists which readings have been provided.
//
function QuerySensorSettingsResponseForm(name, args) {

    // This is a display only form

    // Call GenericForm constructor with our configured properties
    GenericForm.call(
        this,
        name,
        name + "action",
        "post",
        null,
        "application/x-www-form-urlencoded"
    );

    this.createInputByArgs({
        name:  "TimeStamp",
        type:  "text",
        title: "TimeStamp",
        value: "",
        onclick: null,
        isAttribute: false
        });

    //
    // Attributes are from the configuration SensorSettingsAttributes
    //

    var attributes = SensorSettingsAttributes;
    var strictMode = SensorSettingsAttributesStrict;

    for (var prop in attributes) {
        if (attributes[prop] != null) {

            this.createInputByArgs({
                name:  prop,
                type:  "text",
                title: attributes[prop],
                value: "",
                onclick: null,
                isAttribute: true
                });
        }
    }

    //
    // If not strict attributes add additional entries for server response
    // items not in the configured set.
    //
    if (!strictMode) {
        var hasTitle = false;

        for (var prop2 in args) {

            if (attributes[prop2] == null) {
                // The configuration did not recognize the attribute

                //
                // If the attributes configuration has a property, but its
                // null it means the specialization form wants to suppress
                // the display of a known server property.
                //
                if (attributes.hasOwnProperty(prop2)) {
                    continue;
                }

                // Not already in configured attributes table
                if (!hasTitle) {
                    hasTitle = true;
                    this.appendBreak();
                    this.appendTextNode("Extra Items Returned From Server:");
                    this.appendBreak();
                    this.appendBreak();
                }

                this.createInputByArgs({
                    name:  prop2,
                    type:  "text",
                    title: prop2,
                    value: "",
                    onclick: null,
                    isAttribute: false
                    });
            }
        }
    }

    this.stop();
}

//
// End Openpux QuerySensorSettings
//

//
// Openpux UpdateSensorSettings
//

//
// Create a form for updating sensor settings.
//
// It gets the attributes to display from SensorSettingsAttributes.
//
UpdateSensorSettingsForm.prototype = Object.create(GenericForm.prototype);
UpdateSensorSettingsForm.prototype.constructor = UpdateSensorSettingsForm;

function UpdateSensorSettingsForm(name) {

    GenericForm.call(
        this,
        name,
        name + "action",
        "post",
        null,
        "application/x-www-form-urlencoded"
    );

    // SensorSettingsAttributes is set by the specialization files for a specific application
    var attributes = SensorSettingsAttributes;
    for (var prop in attributes) {
        if (attributes[prop] != null) {

            this.createInputByArgs({
                name:  prop,
                type:  "text",
                title: attributes[prop],
                value: "",
                onclick: null,
                isAttribute: true
                });
        }
    }

    var script = "javascript: getForm(\"" + name + "\").processOnSubmit(\"" + name + "\")";
    this.appendSubmitButton(script, "Update Sensor Settings");
    this.appendBreak();

    this.createInputByArgs({
        name:  "Status",
        type:  "text",
        title: "Status",
        value: "",
        onclick: null,
        isAttribute: false
        });

    this.createInputByArgs({
        name:  "Error",
        type:  "text",
        title: "Error",
        value: "",
        onclick: null,
        isAttribute: false
        });

    this.stop();
}

//
// Process the sensor update form
//
// This is invoked on submit of the sensor settings update form.
//
// It reads the entered values in the form text fields and sends
// the values to the server.
//
UpdateSensorSettingsForm.prototype.processOnSubmit = function()
{
  var self = this;

  //
  // Get input values from the DOM
  //

  // Get account information
  var account = getForm("SensorAccountForm").getProperties();

  if (!ValidateCredentialsInput(account)) {
      return;
  }

  if (account.SensorID==null || account.SensorID=="") {
    alert("Must supply SensorID");
    return;
  }

  //
  // Reset form values for the new transaction
  //
  this.setAttribute("Status", "pending...");
  this.setAttribute("Error", "");

  var sleeptime = this.getAttribute("SleepTime");

  // Must supply at least one update
  var update = false;

  if (sleeptime != null && sleeptime != "") update = true;

  var masks = new Object();

  var attributes = this.getProperties();
  for (var prop in attributes) {

      // If there is an entry for the item get the value from it
      var attr = attributes[prop];
      if (attr != null) {
          masks[prop] = attr;
          update = true;
      }
  }

  if (!update) {
      alert("At least one sensor update value must be entered");
      return;
  }

  var args = {
      AccountID: account.AccountID,
      SensorID: account.SensorID,
      SleepTime: sleeptime,
      items: masks
  };

  g_opclient.updateSensorSettings(account.ticket, args, function(error, response) {

      if (error != null) {
          self.setAttribute("Status", error);
          return;
      }

      var status = response.status;
      self.setAttribute("Status", status);

      if (status != 200) {
          if (response.error != null) {
              self.setAttribute("Error", response.error);
          }
          return;
      }
  });

  return;
}

//
// End Openpux UpdateSensorSettings
//

//
// Openpux AddSensorReading
//

//
// This is used to add a reading
//
AddSensorReadingForm.prototype = Object.create(GenericForm.prototype);
AddSensorReadingForm.prototype.constructor = AddSensorReadingForm;

//
// Form constructors get invoked once the first time the
// form is called for. It's then cached in Forms[] and
// re-enabled as required as the UI is navigated.
//
function AddSensorReadingForm(name) {

    //
    // These forms don't really post, but provide the
    // input controls read by the Javascript implementing
    // the client application.
    //
    GenericForm.call(
        this,
        name,
        name + "action",
        "post",
        null,
        "application/x-www-form-urlencoded"
    );

    // We manage local attributes
    this.localAttributes = {};

    var script = "javascript: getForm(\"" + name + "\").processOnSubmit(\"" + name + "\")";
    this.appendSubmitButton(script, "Send Reading");
    this.appendBreak();

    script = "javascript: getForm(\"" + name + "\").processOnSubmitShortForm(\"" + name + "\")";
    this.appendSubmitButton(script, "Send Reading (Short Form)");
    this.appendBreak();

    this.createInputByArgs({
        name:  "Status",
        type:  "text",
        title: "Status",
        value: "",
        onclick: null,
        isAttribute: false
        });

    this.createInputByArgs({
        name:  "Error",
        type:  "text",
        title: "Error",
        value: "",
        onclick: null,
        isAttribute: false
        });

    this.createInputByArgs({
        name:  "ResponseString",
        type:  "text",
        title: "ResponseString",
        value: "",
        onclick: null,
        isAttribute: false
        });

    // SensorReadingAttributes is set by the specialization files for a specific application
    for (var prop in SensorReadingAttributes) {
        if (SensorReadingAttributes[prop] != null) {

            this.createInputByArgs({
                name:  prop,
                type:  "text",
                title: SensorReadingAttributes[prop],
                value: "",
                onclick: null,
                isAttribute: true
                });
        }
    }

    this.stop();
}

AddSensorReadingForm.prototype.processOnSubmit = function()
{
    this.processOnSubmitCommon(false);
}

//
// ShortForm readings use a condensed protocol for low memory
// sensors using url encoding and one or two letter abbreviations
// for a generic set of settings and readings.
//
// The sensors exchange a simple protocol in which they update
// the server with their latest readings, and current internal settings,
// and the server responds with the latest target settings for the
// sensor.
//
// This simple exchange protocol saves energy for the sensors by
// only having to have one exchange per readings period, and allows
// applications to achieve sensor settings convergence even when
// messages are lost.
//
AddSensorReadingForm.prototype.processOnSubmitShortForm = function()
{
    this.processOnSubmitCommon(true);
}

//
// Process the sensor update form
//
// This is called when an addReading form is submitted and is invoked
// from the submit button "Send Reading".
//
AddSensorReadingForm.prototype.processOnSubmitCommon = function(useShortForm)
{
  var self = this;

  //
  // Get input values from the DOM
  //

  // Get account information
  var account = getForm("SensorAccountForm").getProperties();

  if (!ValidateCredentialsInput(account)) {
      return;
  }

  if (account.SensorID==null || account.SensorID=="") {
    alert("Must supply SensorID");
    return;
  }

  //
  // Reset form values for the new transaction
  //
  this.setAttribute("Status", "pending...");
  this.setAttribute("Error", "");
  this.setAttribute("ResponseString", "");

  //
  // Clear any per response items from any previous exchange
  //
  for (var prop in this.localAttributes) {
      this.deleteInput(prop);
  }

  var o = this.getProperties();

  var args = {
      AccountID: account.AccountID,
      SensorID: account.SensorID,
      items: o
  };

  if (useShortForm) {

      // This converts parameters to short form querystring format
      // The response is a querystring.
      g_opclient.addSensorReadingShortForm(account.ticket, args, function(error, response) {
          self.processAddSensorReadingShortFormResponse(error, response);
      });
  }
  else {
      // Response is an object
      g_opclient.addSensorReading(account.ticket, args, function(error, response) {
          self.processAddSensorReadingResponse(error, response);
      });
  }

  return;
}

//
// responseDocument is a querystring that is processed by this
// function with the help of the openpuxclient.js library.
//
AddSensorReadingForm.prototype.processAddSensorReadingShortFormResponse = 
    function(error, response)
{
      if (error != null) {
          this.setAttribute("Status", error);

          if (response != null) {
              this.setAttribute("Error", response);
          }
          return;
      }
      else {
          this.setAttribute("Status", "200");
      }

      //
      // addSensorReadingShortForm responds with the latest
      // sensor settings from the server. This is to allow a low
      // power sensor to keep its state up to date with a single server
      // exchange when it reports its readings.
      //
      // The responseDocument contains these settings as a series
      // of short form masks, sleep time, and command.
      //
      // Since this client is not a sensor this information is just
      // displayed.
      //
      this.setAttribute("ResponseString", response.responseText);

      // C=10&S=30&M0=0&M1=ff&M2=0&M3=0
      var shortFormItems = g_opclient.processQueryString(response.responseText);

      // items are now available in long (normal) form.
      var items = g_opclient.convertFromShortForm(shortFormItems);

      //
      // Display the server response items
      //
      // Note the internal input field name is given a prefix
      // "FromServer" to avoid collisions with the same named fields
      // that are sent.
      //

      //
      // Delete old form if present so that stale settings are deleted.
      // This is because the forms constructor will only run once for
      // a given set of items and be cached.
      //
      // Rather than track and delete the items through the DOM,
      // re-creating the form is more straightforward. Since there
      // are no user settings in a display only form input field
      // preservation is not a concern.
      //
      deleteForm("AddSensorReadingResponseForm");

      responseForm = getForm(
          "AddSensorReadingResponseForm", // instanceName
          "AddSensorReadingResponseForm", // formType
          items                           // formArgs
          );

      responseForm.show();
}

//
// Standard JSON exchange without short form processing.
//
// Response is an object as its already been parsed from
// the returned JSON by openpuxclient.js library.
//
AddSensorReadingForm.prototype.processAddSensorReadingResponse = 
    function(error, response)
{
      if (error != null) {
          this.setAttribute("Status", error);
          return;
      }

      var status = response.status;
      this.setAttribute("Status", status);

      if (status != 200) {
          if (response.error != null) {
              this.setAttribute("Error", response.error);
          }
          return;
      }

      //
      // Currently the long form JSON based add reading
      // does not return current sensor settings.
      //
      // They are available through the regular querySensorSettings()
      // function.
      //
}

//
// This form is used to display the response from addSensorReading
//
AddSensorReadingResponseForm.prototype = Object.create(GenericForm.prototype);
AddSensorReadingResponseForm.prototype.constructor = AddSensorReadingResponseForm;

//
// Generate a response form for the returned sensor settings.
//
// Args contains the object that lists which settings have been provided.
//
function AddSensorReadingResponseForm(name, args) {

    // This is a display only form

    // Call GenericForm constructor with our configured properties
    GenericForm.call(
        this,
        name,
        name + "action",
        "post",
        null,
        "application/x-www-form-urlencoded"
    );

    this.appendBreak();
    this.appendTextNode("Current Settings From Server To Sensor:");
    this.appendBreak();
    this.appendBreak();

    for (var prop in args) {
        this.createInputByArgs({
            name:  prop,
            type:  "text",
            title: prop,
            value: args[prop],
            onclick: null,
            isAttribute: false
            });
    }

    this.stop();
}

//
// End Openpux AddSensorReading
//

//
// Openpux QuerySensorReadings
//

QuerySensorReadingsForm.prototype = Object.create(GenericForm.prototype);
QuerySensorReadingsForm.prototype.constructor = QuerySensorReadingsForm;

function QuerySensorReadingsForm(name) {

    // Call GenericForm constructor with our configured properties
    GenericForm.call(
        this,
        name,
        name + "action",
        "post",
        null,
        "application/x-www-form-urlencoded"
        );

    this.createInputByArgs({
        name:  "ReadingCount",
        type:  "text",
        title: "Reading Count",
        value: "1",
        onclick: null,
        isAttribute: false
        });

    this.createInputByArgs({
        name:  "StartDate",
        type:  "text",
        title: "StartDate",
        value: "2015:01:01:00:00:00",
        onclick: null,
        isAttribute: false
        });

    this.createInputByArgs({
        name:  "EndDate",
        type:  "text",
        title: "EndDate",
        value: "2022:01:01:00:00:00",
        onclick: null,
        isAttribute: false
        });

    //
    // compose action script that names the form.
    // Note that the script only has access to variables in the global scope so
    // getForm("name") is used and not "this" or "self". Even getForm(name) does not
    // work since name is a local.
    //
    var self = this;

    var script = "javascript: getForm(\"" + name + "\").processOnSubmit(\"" + name + "\")";

    // Add submit button to current form
    this.appendBreak();
    this.appendSubmitButton(script, "Submit Query");
    this.appendBreak();

    this.createInputByArgs({
        name:  "Status",
        type:  "text",
        title: "Status",
        value: "",
        onclick: null,
        isAttribute: false
        });

    this.createInputByArgs({
        name:  "Error",
        type:  "text",
        title: "Error",
        value: "",
        onclick: null,
        isAttribute: false
        });

    //
    // Track response forms so they can be deleted when a new
    // set of readings are returned.
    //
    this.responseForms = {};

    this.stop();
}

//
// This is invoked when submit occurs on a sensor readings query form
// and sends the attribute values to the server.
//
QuerySensorReadingsForm.prototype.processOnSubmit = function(name) {

    var self = this;

    //
    // Reset form values for the new transaction
    //
    this.setAttribute("Status", "pending...");
    this.setAttribute("Error", "");

    // Clear any forms current being displayed
    for (var prop in this.responseForms) {
        var form = this.responseForms[prop];
        deleteForm(form);
        delete this.responseForms[prop];
    }

    // Get account information
    var account = getForm("SensorAccountForm").getProperties();

    if (!ValidateCredentialsInput(account)) {
        return;
    }

    // Get our attributes as form inputs
    var readingcount = this.getAttribute("ReadingCount");
    var startdate = this.getAttribute("StartDate");
    var enddate = this.getAttribute("EndDate");

    if (readingcount == null) readingcount = 1;

    var args = {
        AccountID: account.AccountID,
        SensorID: account.SensorID,
        readingcount: readingcount,
        startdate: startdate,
        enddate: enddate
    };

    g_opclient.querySensorReadings(account.ticket, args, function(error, response) {
        self.processQuerySensorReadingsResponse(error, response);
    });
}

//
//
// 04/02/2016
//
// args could be the direct values, or contained in an items object.
//
// Input:
//
// Return Value:
//
// { itemName: "name_string", item: object }
//
function TranslateResponseItem(name, args) {

    if (args == null) {
        return null;
    }

    var o = {};

    if (typeof(args.itemName) != "undefined") {
        o.itemName = args.itemName;
    }
    else {
        o.itemName = name;
    }

    if (typeof(args.items) != "undefined") {
        o.item = args.item;
    }
    else {
        o.item = args;
    }

    return o;
}

//
// This is invoked when a sensor readings query response is received
// from the server.
//
// response:
//
// {
//     status: 200,
//     error: "error_value",
//     items: [
//       {
//         AccountID: "1",
//         SensorID: "2",
//         TimeStamp: "timestamp",
//         Reading1: "reading1",
//         Reading2: "reading2"
//       }
//     ]
// }
//
QuerySensorReadingsForm.prototype.processQuerySensorReadingsResponse =
    function (error, response) {

    var self = this;

    var responseForm = null;

    if (error != null) {
        // Create a response form to display the error
        this.setAttribute("Status", error);
        return;
    }

    var status = response.status;
    this.setAttribute("Status", status);

      if (status != 200) {
        if (response.error != null) {
            this.setAttribute("Error", response.error);
        }
        return;
    }

    if (typeof response.items == "undefined") {
        this.setAttribute("Status", "200 OK, no readings returned");
        return;
    }

    // If no entries, give the indicated response
    if (response.items.length == 0) {
        this.setAttribute("Status", "200 OK, no readings returned");
        return;
    }

    //
    // There can be multiple sets of sensor readings in the returned document
    // represented as an array.
    //
    // We generate a unique response form to display each set of readings.
    //
    // Each entry in the array is an object of:
    //
    // { itemName: "name_string", item: object }
    //
    for(var index = 0; index < response.items.length; index++) {

        //
        // This generates a unique response form.
        //
        // A form instance name is given to getForm() along with a type
        // to allow creation of multiple forms of the same type.
        //
        // The sensor readings are provided to the form constructor
        // so it can generate the entries for display as required.
        //
        var instanceName = "QuerySensorReadingsResponseForm" + index;

        var item = TranslateResponseItem("Readings", response.items[index]);

        responseForm = getForm(
            instanceName,
            "QuerySensorReadingsResponseForm", // formType
            item                               // formArgs
            );

        this.responseForms[instanceName] = responseForm;

        responseForm.show();

        self.displaySensorReading(responseForm, item);
    }
}

//
// Process a single set of sensor readings from a response document.
//
// The readings are displayed in responseForm.
//
// sensorreading consists of:
//
// { itemName: "name_string", item: object }
//
QuerySensorReadingsForm.prototype.displaySensorReading = function(responseForm, sensorreading) {

    var o = sensorreading.item;

    for (var prop in o) {

        if (o[prop] == null) continue;

        // There may not be a form entry for the response item
        if (responseForm.getAttribute(prop) == null) continue;

        responseForm.setAttribute(prop, o[prop]);
    }
}

//
// This form is used to display the response from querySensorReadings
//
QuerySensorReadingsResponseForm.prototype = Object.create(GenericForm.prototype);
QuerySensorReadingsResponseForm.prototype.constructor = QuerySensorReadingsResponseForm;

//
// Generate a response form for a set of sensor readings.
//
// Args contains the object that lists which readings have been provided.
//
// // { itemName: "name_string", item: object }
//
function QuerySensorReadingsResponseForm(name, args) {

    // This is a display only form

    // Call GenericForm constructor with our configured properties
    GenericForm.call(
        this,
        name,
        name + "action",
        "post",
        null,
        "application/x-www-form-urlencoded"
    );

    // Get the itemName and item from the response object
    var itemName = args.itemName;
    var o = args.item;

    // Display the name
    this.createInputByArgs({
        name:  prop,
        type:  "text",
        title: "ItemName",
        value: itemName,
        onclick: null,
        isAttribute: false
        });

    //
    // Attributes are from the configuration SensorReadingAttributes
    //

    var attributes = SensorReadingAttributes;
    var strictMode = SensorReadingAttributesStrict;

    for (var prop in attributes) {

        if (attributes[prop] != null) {

            this.createInputByArgs({
                name:  prop,
                type:  "text",
                title: attributes[prop],
                value: "",
                onclick: null,
                isAttribute: true
                });
        }
    }

    //
    // If not strict attributes add additional entries for server response
    // items not in the configured set.
    //
    if (!strictMode) {
        var hasTitle = false;

        for (var prop2 in o) {

            if (attributes[prop2] == null) {
                // The configuration did not recognize the attribute

                //
                // If the attributes configuration has a property, but its
                // null it means the specialization form wants to suppress
                // the display of a known server property.
                //
                if (attributes.hasOwnProperty(prop2)) {
                    continue;
                }

                // Not already in configured attributes table
                if (!hasTitle) {
                    hasTitle = true;
                    this.appendBreak();
                    this.appendTextNode("Extra Items Returned From Server:");
                    this.appendBreak();
                    this.appendBreak();
                }

                this.createInputByArgs({
                    name:  prop2,
                    type:  "text",
                    title: prop2,
                    value: "",
                    onclick: null,
                    isAttribute: false
                    });
            }
        }
    }

    this.stop();
}

//
// End Openpux QuerySensorReadings
//

//
// Openpux AddAccount
//

//
// Create a form for adding a new account.
//
AddAccountForm.prototype = Object.create(GenericForm.prototype);
AddAccountForm.prototype.constructor = AddAccountForm;

function AddAccountForm(name) {

    GenericForm.call(
        this,
        name,
        name + "action",
        "post",
        null,
        "application/json"
    );

    this.createInputByArgs({
        name:  "NewAccountID",
        type:  "text",
        title: "NewAccountID",
        value: "",
        onclick: null,
        isAttribute: true
        });

    this.createInputByArgs({
        name:  "NewAccountTicketID",
        type:  "text",
        title: "NewAccountTicketID",
        value: "",
        onclick: null,
        isAttribute: true
        });

    var script = "javascript: getForm(\"" + name + "\").processOnSubmit(\"" + name + "\")";
    this.appendSubmitButton(script, "Add Account");
    this.appendBreak();

    this.createInputByArgs({
        name:  "Status",
        type:  "text",
        title: "Status",
        value: "",
        onclick: null,
        isAttribute: false
        });

    this.createInputByArgs({
        name:  "Error",
        type:  "text",
        title: "Error",
        value: "",
        onclick: null,
        isAttribute: false
        });

    this.stop();
}

//
// Process the submit.
//
// This is invoked on submit of classes form.
//
// It reads the entered values in the form text fields and sends
// the values to the server.
//
AddAccountForm.prototype.processOnSubmit = function()
{
  var self = this;

  //
  // Get input values from the DOM
  //

  // Get account information
  var account = getForm("SensorAccountForm").getProperties();

  //
  // A ticket with administrator rights must be present with
  // to create new accounts.
  //
  if (!ValidateCredentialsInput(account)) {
      return;
  }

  var itemsArray = new Object();

  //
  // This is the ID and passcode for the new account
  //
  itemsArray.NewAccountID = this.getAttribute("NewAccountID");
  itemsArray.NewAccountTicketID = this.getAttribute("NewAccountTicketID");

  if (itemsArray.NewAccountID==null || itemsArray.NewAccountID=="") {
    alert("Must supply NewAccountID");
    return;
  }

  if (itemsArray.NewAccountTicketID==null || itemsArray.NewAccountTicketID =="") {
    alert("Must supply NewAccountTicketID");
    return;
  }

  // Update status
  this.setAttribute("Status", "pending...");
  this.setAttribute("Error", "");

  var args = {
      AccountID: account.AccountID,
      items: itemsArray
  };

  g_opclient.addAccount(account.ticket, args, function(error, response) {

      if (error != null) {
          self.setAttribute("Status", error);
          return;
      }

      var status = response.status;
      self.setAttribute("Status", status);

      if (status != 200) {
          if (response.error != null) {
              self.setAttribute("Error", response.error);
          }
          return;
      }
  });

  return;
}

//
// End Openpux AddAccount
//

//
// Openpux AddSensor
//

//
// Create a form for adding a new sensor.
//
AddSensorForm.prototype = Object.create(GenericForm.prototype);
AddSensorForm.prototype.constructor = AddSensorForm;

function AddSensorForm(name) {

    GenericForm.call(
        this,
        name,
        name + "action",
        "post",
        null,
        "application/json"
    );

    this.createInputByArgs({
        name:  "NewSensorID",
        type:  "text",
        title: "NewSensorID",
        value: "",
        onclick: null,
        isAttribute: true
        });

    this.createInputByArgs({
        name:  "NewSensorAccountID",
        type:  "text",
        title: "NewSensorAccountID",
        value: "",
        onclick: null,
        isAttribute: true
        });

    var script = "javascript: getForm(\"" + name + "\").processOnSubmit(\"" + name + "\")";
    this.appendSubmitButton(script, "Add Sensor");
    this.appendBreak();

    this.createInputByArgs({
        name:  "Status",
        type:  "text",
        title: "Status",
        value: "",
        onclick: null,
        isAttribute: false
        });

    this.createInputByArgs({
        name:  "Error",
        type:  "text",
        title: "Error",
        value: "",
        onclick: null,
        isAttribute: false
        });

    this.stop();
}

//
// Process the submit.
//
// This is invoked on submit of classes form.
//
// It reads the entered values in the form text fields and sends
// the values to the server.
//
AddSensorForm.prototype.processOnSubmit = function()
{
  var self = this;

  //
  // Get input values from the DOM
  //

  // Get account information
  var account = getForm("SensorAccountForm").getProperties();

  //
  // To add a sensor the administrators account ID, or the account
  // of a registered user can be supplied.
  //
  // Note that some registered users may not have add sensor permission,
  // or limits on the number of sensors that could be added.
  //
  if (!ValidateCredentialsInput(account)) {
      return;
  }

  var itemsArray = new Object();

  //
  // This contains the new SensorID and the AccountID to create the sensor under.
  //
  itemsArray.NewSensorID = this.getAttribute("NewSensorID");
  itemsArray.NewSensorAccountID = this.getAttribute("NewSensorAccountID");

  if (itemsArray.NewSensorID==null || itemsArray.NewSensorID=="") {
    alert("Must supply NewSensorID");
    return;
  }

  // If a NewSensorAccountID is not supplied, use the authentication account
  if (itemsArray.NewSensorAccountID==null || itemsArray.NewSensorAccountID=="") {

      itemsArray.NewSensorAccountID = account.AccountID;

      // Update form item
      this.setAttribute("NewSensorAccountID", itemsArray.NewSensorAccountID);
  }

  // Update status
  this.setAttribute("Status", "pending...");
  this.setAttribute("Error", "");

  var args = {
      AccountID: account.AccountID,
      items: itemsArray
  };

  g_opclient.addSensor(account.ticket, args, function(error, response) {

      if (error != null) {
          self.setAttribute("Status", error);
          return;
      }

      var status = response.status;
      self.setAttribute("Status", status);

      if (status != 200) {
          if (response.error != null) {
              self.setAttribute("Error", response.error);
          }
          return;
      }
  });

  return;
}

//
// End Openpux AddSensor
//
