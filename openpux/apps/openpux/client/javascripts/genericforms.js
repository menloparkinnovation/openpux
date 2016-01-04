
//
//   genericforms.js
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
// GenericForms Library
//

var Forms = {};

function getForms() {
    return Forms;
}

//
// Get a form.
//
// If a form of instanceName exists it is returned.
//
// If a form of instanceName does not exist it is created.
//
// If typeName is not supplied a form constructor function
// that matches instanceName is looked up. This works for most
// cases in which there is a single instance of a given form type.
//
// If typeName is supplied it is used to determine which form
// to create with a unique instance name. This allows multiple
// instances of a given form type.
//
// formArgs are passed to the constructor function for the
// form if present.
//
function getForm(instanceName, typeName, formArgs) {

    var form = Forms[instanceName];
    if (form == null) {
        form = createFormFactory(FormFactoryConfiguration, instanceName, typeName, formArgs);
    }

    return form;
}

function deleteForm(instanceName) {

    var formObj = Forms[instanceName];
    if (formObj != null) {
        Forms[instanceName] = null;
        formObj.form.parentNode.removeChild(formObj.form);
    }
}

//
// Enable a form by ensuring its created, and showing it.
//
function enableForm(name) {
    var form = getForm(name);
    form.show();
    return form;
}

function disableForm(name) {
    if (Forms[name] != null) {
        Forms[name].disable();
    }
}

function disableAllForms() {
    for (var prop in Forms) {
        disableForm(prop);
    }
}

function createFormFactory(formConfig, instanceName, typeName, formArgs) {

    var form = Forms[instanceName];
    if (form != null) {
        return form;
    }

    if ((typeof typeName == "undefined") || (typeName == null)) {
        // Use the instanceName as the typeName for a form singleton
        typeName = instanceName;
    }

    if (typeof formArgs == "undefined") {
        formArgs = null;
    }

    for (var index = 0; index < formConfig.length; index++) {

        // Form has a valid configuration
        if (formConfig[index] == typeName) {

            // Invoke form constructor function of the same name
            var script = ("form = new " + typeName + "(\"" + instanceName + "\", formArgs);");
            eval(script);

            Forms[instanceName] = form;
            return form;
        }
    }

    alert("invalid form typeName " + typeName);
    return null;
}

//
// Create a Form
//
function GenericForm(
    formname,
    formaction,
    formmethod,
    formonsubmit,
    formenctype
    )
{
    this.table = null;
    this.form = null;
    this.tr = null; // table row open
    this.td = null; // table data open

    //
    // Form tracks its input fields so that they may be set and
    // returned from configuration objects.
    //
    // Otherwise a naming convention would be required to sort DOM
    // items from document.forms[this.name][items] which could
    // be input/output fields, or structural elements such as formatting markup
    // tables, etc.
    //
    this.properties= {};

    var form = document.createElement("FORM");

    // We set the forms element id to its name. This must be unique.
    form.id = formname;

    form.name = formname;
    form.action = formaction;
    form.method = formmethod;
    form.enctype = formenctype;

    form.onsubmit = function() {
        eval(formonsubmit);
    };

    this.form = form;
    this.name = formname;
}

//
// Get a form attribute
//
// A form attribute is an input control with data.
//
GenericForm.prototype.getAttribute = function(attributeName) {

    if (document.forms[this.name][attributeName] != null) {
        return document.forms[this.name][attributeName].value;
    }
    else {
        return null;
    }
}

//
// Set a form attribute.
//
// Set the input controls value.
//
GenericForm.prototype.setAttribute = function(attributeName, attributeValue) {
    return document.forms[this.name][attributeName].value = attributeValue;
}

//
// Remove a form attribute
//
// Remove the input control.
//
GenericForm.prototype.removeAttribute = function(attributeName) {

    // removeChild takes a node as argument
    var node = document.forms[this.name][attributeName];
    if (node != null) {
        document.forms[this.name].removeChild(node);
        return true;
    }
    else {
        return false;
    }
}

//
// Determine where to append the new child.
//
GenericForm.prototype.appendChild = function(child) {
    if (this.td != null) {
        this.td.appendChild(child);
    }
    else {
        this.form.appendChild(child);
    }
}

//
// Close the form, no more elements will be added at the current time
// before we want the form to be shown.
//
GenericForm.prototype.stop = function() {
    // This will show the form
    document.body.appendChild(this.form);
}

GenericForm.prototype.appendSubmitButton = function(href, label) {
    var a = document.createElement("A");
    a.href = href;

    //
    // Some browsers such as "Midori" on RaspberryPi require a child text node
    // to display the anchor element
    //
    // We use this form for all browsers as it will display double on Chrome
    //
    //a.text = label;

    var t = document.createTextNode(label);
    a.appendChild(t);

    var br = document.createElement("BR");
    a.appendChild(br);

    // Append to currently opened form (this.form)
    this.appendChild(a);
}

GenericForm.prototype.appendBreak = function() {
    var br = document.createElement("BR");
    this.appendChild(br);
}

GenericForm.prototype.appendTextNode = function(text) {
    var t = document.createTextNode(text);
    this.appendChild(t);
}

//
// Create input item.
//
// args.inputName - DOM name of input control in document.forms[this.form][inputName]
// args.inputTitle - user readable tile for control
// args.inputType - type of input control
// args.inputValue - Default value to place into control. Can be null.
// args.onclick - function to execute when submit/click occurs.
// args.isAttribute - if true its tracked as an attribute returned from getProperties()
//
GenericForm.prototype.createInputByArgs = function(args)
{
    if (args.title != null) {
        this.appendTextNode(args.title);
    }

    var input = document.createElement("INPUT");
    input.type = args.type;
    input.name = args.name;

    if (args.value != null) {
        input.value = args.value;
    }

    if (args.onclick != null) {
        input.onclick = function() {
            eval(args.onclick);
        }
    }

    this.appendChild(input);

    this.appendBreak();

    //
    // Input fields are tracked to allow general case get/set all
    // properties from a configuration object.
    //
    if (args.isAttribute) {
        this.properties[args.name] = args.title;
    }

    return input;
}

GenericForm.prototype.deleteInput = function(inputName) {
    return this.removeAttribute(inputName);
}

GenericForm.prototype.getProperties = function() {
    var o = {};

    //
    // properties[] contains the ID's of form input controls which
    // are read to get their current values.
    //
    for (var prop in this.properties) {
        // Get the HTML DOM attribute of the input control
        o[prop] = this.getAttribute(prop);
    }

    return o;
}

//
// Table construction, similar to current form construction is
// as a stream. The class holds state about the current
// table being defined and commmits it to the current
// form on table end.
//
// Any elements added to the form while a table is open
// get nested within the table.
//
GenericForm.prototype.appendTableStart = function() {
    this.table = document.createElement("TABLE");
}

GenericForm.prototype.appendTableEnd = function() {
    this.form.appendChild(this.table);
    this.table = null;
}

GenericForm.prototype.appendTableRowStart = function() {
    this.tr = document.createElement("TR");
}

GenericForm.prototype.appendTableRowEnd = function() {
    this.table.appendChild(this.tr);
    this.tr = null;
}

GenericForm.prototype.appendTableDataStart = function() {
    this.td = document.createElement("TD");
}

GenericForm.prototype.appendTableDataEnd = function() {
    // Append table data to the currently active table row
    this.tr.appendChild(this.td);
    this.td = null;
}

GenericForm.prototype.show = function() {
    document.forms[this.name].hidden = false;
}

GenericForm.prototype.disable = function() {
    document.forms[this.name].hidden = true;
}
