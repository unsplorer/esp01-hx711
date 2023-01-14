import './Settings_Dropdown.css';

const Settings_Dropdown = () => {

  // Close the dropdown menu if the user clicks outside of it
  // window.onclick = function(event) {
  //   if (!event.target.matches('.dropbtn')) {
  //     console.log(event.target);
  //     var dropdowns = document.getElementsByClassName("dropdown-content");
  //     var i;
  //     for (i = 0; i < dropdowns.length; i++) {
  //       var openDropdown = dropdowns[i];
  //       if (openDropdown.classList.contains('show')) {
  //         openDropdown.classList.remove('show');
  //       }
  //     }
  //   }
  // }

    const handler = () => {
      document.getElementById("myDropdown").classList.toggle("show");
    }

    const tareScale = () =>{
      var xhr = new XMLHttpRequest();
      xhr.open("POST", "/tare", true);
      xhr.send();
    }
  return (
    <div class="dropdown">
      <button onclick={handler} type="button" data-mdb-ripple="true" data-mdb-ripple-color="light" class="dropbtn inline-block px-6 py-2.5 bg-blue-600 text-white font-medium text-xs leading-tight uppercase rounded-full shadow-md hover:bg-blue-700 hover:shadow-lg focus:bg-blue-700 focus:shadow-lg focus:outline-none focus:ring-0 active:bg-blue-800 active:shadow-lg transition duration-150 ease-in-out">
        <i onclick={handler} class="fa-solid fa-1x fa-tools -mx-3 "></i>
      </button>
    <div id="myDropdown" class="dropdown-content">
      <form class="form" action="/spool" method="post">
        <input type="number" class="text-center rounded" name="spool" id="spool" placeholder="Empty Spool Weight"></input>
        <button type="submit" class="inline-block px-6 py-2.5 bg-green-500 text-white font-medium text-xs leading-tight uppercase rounded-full shadow-md hover:bg-green-600 hover:shadow-lg focus:bg-green-600 focus:shadow-lg focus:outline-none focus:ring-0 active:bg-green-700 active:shadow-lg transition duration-150 ease-in-out" value="submit" aria-placeholder="Empty">Set Empty Weight</button>
      </form>

      <form class="form" action="/calibrate" method="post">
        <input type="number" class="text-center rounded" name="known_weight" id="known_weight" placeholder="Known Weight eg.. 100g"></input>
        <button type="submit" class="inline-block px-6 py-2.5 bg-red-600 text-white font-medium text-xs leading-tight uppercase rounded-full shadow-md hover:bg-red-700 hover:shadow-lg focus:bg-red-700 focus:shadow-lg focus:outline-none focus:ring-0 active:bg-red-800 active:shadow-lg transition duration-150 ease-in-out" value="submit">Calibrate</button>
        <button class="inline-block px-6 py-2.5 bg-yellow-600 text-white font-medium text-xs leading-tight uppercase rounded-full shadow-md hover:bg-blue-700 hover:shadow-lg focus:bg-blue-700 focus:shadow-lg focus:outline-none focus:ring-0 active:bg-blue-800 active:shadow-lg transition duration-150 ease-in-out" onclick={tareScale}>Tare</button>
      </form>
      
      <form class="form" action="/api" method="post">
        <span class="text-center">Restore Settings</span>
        <input type="number" class="text-center rounded" name="offset" id="offset" placeholder="Tare offset"></input>
        <input type="number" step="0.01" class="text-center rounded" name="calvalue" id="calvalue" placeholder="Calibration value"></input>
        <input type="number" class="text-center rounded" name="spool_weight" id="spool_weight" placeholder="Empty Spool Weight"></input>
        <button type="submit" class="inline-block px-6 py-2.5 bg-blue-600 text-white font-medium text-xs leading-tight uppercase rounded-full shadow-md hover:bg-red-700 hover:shadow-lg focus:bg-red-700 focus:shadow-lg focus:outline-none focus:ring-0 active:bg-red-800 active:shadow-lg transition duration-150 ease-in-out" value="submit">Submit</button>
      </form>
    </div>
  </div>
  );
}

export default Settings_Dropdown;