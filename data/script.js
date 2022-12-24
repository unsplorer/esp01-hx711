// Get current sensor readings when the page loads  
window.addEventListener('load', getReadings);


// Function to get current readings on the webpage when it loads for the first time
function getReadings(){
  var xhr = new XMLHttpRequest();
  xhr.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      let myObj = JSON.parse(this.responseText);
      console.log(myObj);
      let weight = myObj.reading;   
      gaugeFilament.value = weight;
    }
  }; 
  xhr.open("GET", "/readings", true);
  xhr.send();
}


function startCalibration(){
  var xhr = new XMLHttpRequest();
  xhr.open("POST", "/calibrate?known_weight=62", true);
  xhr.send();
}


let tareScale = () =>{
  var xhr = new XMLHttpRequest();
  xhr.open("POST", "/tare", true);
  xhr.send();
}


// Create Humidity Gauge
var gaugeFilament = new RadialGauge({
  renderTo: 'gauge-filament',
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
  colorNeedleEnd: "#00f9",
  needleWidth: 6,
  needleCircleSize: 3,
  colorNeedleCircleOuter: "#000",
  needleCircleOuter: true,
  needleCircleInner: true,
  animationDuration: 1000,
  animationRule: "linear"
}).draw();



// event listener
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
    console.log("new_readings", e.data);
    var myObj = JSON.parse(e.data);
    console.log(myObj);
    gaugeFilament.value = myObj.reading;
  }, false);
}

