import type { Component } from 'solid-js';
import Card from './Components/Card';
import Fila_Gauge from './Components/Fila_Gauge';
import Navbar from './Components/Navbar';

const App: Component = () => {
  var filament_remaining = 200;
  let wifi;
  // event listener
  if (!!window.EventSource) {
    var source = new EventSource('/events');
    
    source.addEventListener('open', function(e) {
      console.log("Events Connected");
    }, false);

    // source.addEventListener('error', function(e) {
    //   if (e.target.readyState != EventSource.OPEN) {
    //     console.log("Events Disconnected");
    //   }
    // }, false);
    
    source.addEventListener('message', function(e) {
    }, false);
    
    source.addEventListener('report', function(e) {
      let report = JSON.parse(e.data);
      filament_remaining = report["scale"].filament_remaining;
    }, false);
  }
  return (
    <main>
      <Navbar/>
      <div class='mt-6 flex items-center text-center justify-center'>
        <Card title='Filament Remaining' filament={filament_remaining}/>
      </div>
    </main>
  )
}

export default App;
