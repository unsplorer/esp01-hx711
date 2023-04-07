import type { Component } from 'solid-js';
import Card from './Components/Card';
import Navbar from './Components/Navbar';
import { createSignal} from "solid-js";


const App: Component = () => {
  const [count, setCount] = createSignal(0);
  const [rssi, setRssi] = createSignal();
  const [darkmode, setDarkmode] = createSignal('Dark');

  const toggleDark = () => {
    let root = document.getElementById('html');
    if (root.classList.contains('dark')) {
      root.classList.remove('dark');
      setDarkmode("Light");
    } else {
      root.classList.add('dark');
      setDarkmode("Dark");
    }
  }

  // event listener
  if (!!window.EventSource) {
    var source = new EventSource('/events');
    source.addEventListener('open', function(e) {
      console.log("Events Connected");
    }, false);
    source.addEventListener('message', function(e) {
    }, false);
    source.addEventListener('report', function(e) {
      let report = JSON.parse(e.data);
      setCount(report["scale"].filament_remaining);
      setRssi(report["device"].rssi);
    }, false);
  }
  
  return (
    <main>
      <Navbar toggleDark={toggleDark}/>
      <div class='dark:bg-zinc-800 dark:text-zinc-300 mt-6 flex items-center text-center justify-center'>
        <Card title='Filament Remaining' filament={count()} rssi={rssi()} darkMode={darkmode} />
      </div>
    </main>
  )
}

export default App;