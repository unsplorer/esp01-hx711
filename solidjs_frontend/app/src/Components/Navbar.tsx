import {Component, createSignal} from "solid-js";
import Navitem from "./Navitem";

const Navbar: Component = () => {
  const [darkmode, setDarkmode] = createSignal('Light');

  const toggledark = () => {
    let root = document.getElementById('html');
    if(root.classList.contains('dark')){
      root.classList.remove('dark');
      setDarkmode("Dark");
    }else{
      root.classList.add('dark');
      setDarkmode("Light");
    }
  }

  let settings = <Navitem name={darkmode()} class="button h-12 w-24 bg-zinc-700 text-white rounded-lg text-base font-medium"/>
  let navTitle = <h1 class='text-2xl ml-6'>Fila-Scale</h1>
  let logo = <img src="logo.png" class='h-16 w-16' />
  
  
  return (
    <nav class="bg-zinc-800 dark:bg-indigo-800 text-white flex items-center justify-between">
      <div class='flex items-center'>
        {logo}
        {navTitle}
      </div>
      <div class='mr-6' onclick={toggledark}>
        {settings}
      </div>
    </nav>

  )
}

export default Navbar;
