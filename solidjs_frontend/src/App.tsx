import type { Component } from 'solid-js';
import Card from './Components/Card';
import Navbar from './Components/Navbar';
import { createSignal, onCleanup } from "solid-js";

const App: Component = () => {
  const [count, setCount] = createSignal(0);
	const interval = setInterval(
		() => setCount(c => c + 10),
		1000
	);
	onCleanup(() => clearInterval(interval));
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
      setCount(report["scale"].filament_remaining);
    }, false);
  }
  
  return (
    <main>
      <Navbar/>
      <div class='mt-6 flex items-center text-center justify-center'>
        <Card title='Filament Remaining' filament={count()}/>
      </div>
    </main>
  )
}

export default App;
