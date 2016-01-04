
//
// http://localhost:8080/weather/weatherpi.html
//

//
//   Openpux Internet Of Things (IOT) Framework.
//
//   Copyright (C) 2014,2015 Menlo Park Innovation LLC
//
//   menloparkinnovation.com
//   menloparkinnovation@gmail.com
//
//   Weather Station using AngularJS
//
// Moving gauges and AngularJS template on 10/19/2015 from:
// https://github.com/esquiloio/lib/blob/master/demos/weather/angular/weather.js
//
// With the following header:
//

//
// Weather Station Demo (AngularJS version)
//
// See README.md for more information.
//
// This work is released under the Creative Commons Zero (CC0) license.
// See http://creativecommons.org/publicdomain/zero/1.0/
//

var g_cloud_interval = 30000; // 30 seconds

var app = angular.module('app', [

    //
    // This is the module requires array where we List the services
    // we are dependent on
    //

  'openpuxServices'
]);

app.controller('MainController', [

    // List the dependency injection components. $scope is a default dependency.
    '$scope',

    'Openpux',

    // Define the controller function
    function($scope, Openpux) {

        // This is the model class that represents the current application state
        $scope.values = {
            temp: 0,
            humidity: 0,
            pressure: 0,
            windspeed: 0,
            windirection: 0
        };

        function getWeather() {

            Openpux.getLatestReading(function(error, result) {

                if (error != null) {
                    console.log('error: ', status);
                    setTimeout(getWeather, g_cloud_interval);
                    return;
                }

                //
                // Update the model with the current values
                //

                $scope.values.temp = result.temperature;

                $scope.values.humidity = result.humidity;

                $scope.values.pressure = result.barometer;

                $scope.values.windspeed = result.windspeed;

                $scope.values.winddirection = result.winddirection;

                //
                // Tell AngularJS that a model change occurred outside
                // of its system since openpux does not use the $http service
                // provided by AngularJS but communicates with XMLHttpRequest
                // directly.
                //

                $scope.$apply();

                // restart the timer
                setTimeout(getWeather, g_cloud_interval);
            });
        }

        // Kick it off
        getWeather();
    }]);

app.directive('gauge', function() {
    // This directive is ported from Marco Schweighauser's project here:
    // https://github.com/Schweigi/espruino-examples
    function polarToCartesian(centerX, centerY, radius, rad) {
        return {
            x: centerX + (radius * Math.cos(rad)),
            y: centerY + (radius * Math.sin(rad))
        };
    }

    function arc(x, y, radius, val, minVal, maxVal){
        var start = polarToCartesian(x, y, radius, -Math.PI);
        var end = polarToCartesian(x, y, radius, -Math.PI*(1 - 1/(maxVal-minVal) * (val-minVal)));

        var d = [
            "M", start.x, start.y,
            "A", radius, radius, 0, 0, 1, end.x, end.y
        ].join(" ");

        return d;
    }

    return {
        scope: {
            title:  '@',
            label:  '@',
            min:    '=',
            max:    '=',
            value:  '='
        },
        restrict: 'E',
        replace: true,
        template:
        '<div>'+
            '<h3>{{ title }}</h3>'+
            '<svg class="gauge" viewBox="0 0 200 145">'+
                '<path class="gauge-base" stroke-width="30" ng-attr-d="{{ baseArc }}" />'+
                '<path class="gauge-progress" stroke-width="30" ng-attr-d="{{ progressArc }}" />'+
                '<text class="gauge-value" x="100" y="105" text-anchor="middle">{{ value | number:1 }} {{ label }}</text>'+
                '<text class="gauge-min" x="40" y="125" text-anchor="middle">{{ min }}</text>'+
                '<text class="gauge-max" x="160" y="125" text-anchor="middle">{{ max }}</text>'+
            '</svg>'+
        '</div>',
        link: function(scope, element, attrs) {
            scope.baseArc = arc(100, 100, 60, 1, 0, 1);
            scope.progressArc = arc(100, 100, 60, scope.min, scope.min, scope.max);
            scope.$watch('value', function() {
                // Range-bound the value and update the gauge
                var value = scope.value;
                if (value < scope.min) value = scope.min;
                else if (value > scope.max) value = scope.max;

                scope.progressArc = arc(100, 100, 60, value, scope.min, scope.max);
            });
        }
    }
});
