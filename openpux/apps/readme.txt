
Applications:

Applications are modular and plug in depending on deployment requirements.

config\serverconfig.json, Applications[] configures which applications load and
their parameters.

Description of Applications:

openpux  - Main openpux IoT Sensor application based on REST/JSON.

smartpux - Simplified protocol for SmartPux (TM) low power sensors.

           Layers on top of the openpux application.

rester   - Pure REST data access operations against application controlled schema.

admin    - Administration application for the openpux IoT Application Platform.

default  - Application module that forwards default HTTP requests for client
           side content to the configured default application.

           Operates similar to the "/" -> default.html rules of an HTTP web server.

Demonstration Applications:

weather - AngularJS based weather application (simulated)

domweather - Basic HTML/Javascript/DOM weather application (simulated)

