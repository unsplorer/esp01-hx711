// Get current sensor readings when the page loads  
window.addEventListener('load', getReadings);

// Create Temperature Gauge
// var gaugeTemp = new LinearGauge({
//   renderTo: 'gauge-temperature',
//   width: 120,
//   height: 400,
//   units: "Temperature C",
//   minValue: 0,
//   startAngle: 90,
//   ticksAngle: 180,
//   maxValue: 40,
//   colorValueBoxRect: "#049faa",
//   colorValueBoxRectEnd: "#049faa",
//   colorValueBoxBackground: "#f1fbfc",
//   valueDec: 2,
//   valueInt: 2,
//   majorTicks: [
//       "0",
//       "5",
//       "10",
//       "15",
//       "20",
//       "25",
//       "30",
//       "35",
//       "40"
//   ],
//   minorTicks: 4,
//   strokeTicks: true,
//   highlights: [
//       {
//           "from": 30,
//           "to": 40,
//           "color": "rgba(200, 50, 50, .75)"
//       }
//   ],
//   colorPlate: "#fff",
//   colorBarProgress: "#CC2936",
//   colorBarProgressEnd: "#049faa",
//   borderShadowWidth: 0,
//   borders: false,
//   needleType: "arrow",
//   needleWidth: 2,
//   needleCircleSize: 7,
//   needleCircleOuter: true,
//   needleCircleInner: false,
//   animationDuration: 1500,
//   animationRule: "linear",
//   barWidth: 10,
// }).draw();
  
// Create Humidity Gauge
var gaugeHum = new RadialGauge({
  renderTo: 'gauge-humidity',
  width: 300,
  height: 300,
  units: "grams",
  minValue: 0,
  maxValue: 1000,
  colorValueBoxRect: "#ffffff",
  colorValueBoxRectEnd: "#ffffff",
  colorValueBoxBackground: "#ffffff",
  valueInt: 2,
  majorTicks: [
      "0",
      "200",
      "400",
      "600",
      "800",
      "1000"

  ],
  minorTicks: 4,
  strokeTicks: true,
  highlights: [
      {
          "from": 0,
          "to": 100,
          "color": "#FF0000"
      }
  ],
  colorPlate: "#fff",
  borderShadowWidth: 0,
  borders: false,
  needleType: "arrow",
  colorNeedle: "#000",
  colorNeedleEnd: "#0009",
  needleWidth: 4,
  needleCircleSize: 3,
  colorNeedleCircleOuter: "#000",
  needleCircleOuter: true,
  needleCircleInner: true,
  animationDuration: 1500,
  animationRule: "linear"
}).draw();

// Function to get current readings on the webpage when it loads for the first time
function getReadings(){
  var xhr = new XMLHttpRequest();
  xhr.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      const OFFSET = 286 // empty spool and roller weight
      var myObj = JSON.parse(this.responseText);
      console.log(myObj);
      // var temp = myObj.offset;
      var hum = myObj.reading;
      // gaugeTemp.value = temp;
      gaugeHum.value = hum - OFFSET;
    }
  }; 
  xhr.open("GET", "/readings", true);
  xhr.send();
}

if (!!window.EventSource) {
  var source = new EventSource('/events');
  
  source.addEventListener('open', function(e) {
    console.log("Events Connected");
  }, false);

  source.addEventListener('error', function(e) {
    if (e.target.readyState != EventSource.OPEN) {
      console.log("Events Disconnected");
    }
  }, false);
  
  source.addEventListener('message', function(e) {
    console.log("message", e.data);
  }, false);
  
  source.addEventListener('new_readings', function(e) {
    const OFFSET = 286 // empty spool and roller weight
    console.log("new_readings", e.data);
    var myObj = JSON.parse(e.data);
    console.log(myObj);
    // gaugeTemp.value = myObj.offset;
    gaugeHum.value = myObj.reading - OFFSET;
  }, false);
}