import type { Component } from 'solid-js';
import Card from './Components/Card';
import Navbar from './Components/Navbar';

const App: Component = () => {
  return (
    <main>
      <Navbar/>
      <div class='mt-6 flex items-center text-center justify-center'>
        <Card title='Filament Remaining'/>
      </div>
    </main>
  )
}

export default App;
