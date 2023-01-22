import {Component} from "solid-js";
import Navitem from "./Navitem";

const Navbar: Component = (props) => {  
  let settings = <Navitem name="" class="button h-12 w-12 bg-zinc-700 text-white rounded-lg text-base font-medium"/>
  let navTitle = <h1 class='text-2xl ml-6'>Fila-Scale</h1>
  let logo = <img src="logo.png" class='h-16 w-16' />
  
  
  return (
    <nav class="bg-zinc-800 dark:bg-zinc-900 w-full text-white flex items-center justify-between">
      <div class='flex items-center'>
        {logo}
        {navTitle}
      </div>
      <div class='mr-6' onclick={props.toggledark}>
        {settings}
      </div>
    </nav>

  )
}

export default Navbar;
