import {Component} from "solid-js";
import Navitem from "./Navitem";

const Navbar: Component = () => {
  // let settings = <Navitem name="Settings" class="button h-12 w-24 bg-zinc-700 text-white rounded-lg text-base font-medium" aria-current="page"/>
  let navTitle = <h1 class='text-xl ml-6'>Fila-Scale</h1>
  let logo = <img src="logo.png" class='h-16 w-16' />
  
  return (
    <nav class="bg-zinc-800 text-white flex items-center justify-between">
      <div class='flex items-center'>
        {logo}
        {navTitle}
      </div>
      {/* <div class='mr-6'>
        {settings}
      </div> */}
    </nav>

  )
}

export default Navbar;
