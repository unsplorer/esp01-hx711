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
    <div class="dropdown bg-white">
      <button onclick={handler} type="button" data-mdb-ripple="true" data-mdb-ripple-color="light" class="dropbtn inline-block px-4 py-2 bg-blue-600 text-white font-medium text-xs leading-tight uppercase rounded-full shadow-md hover:bg-blue-700 hover:shadow-lg focus:bg-blue-700 focus:shadow-lg focus:outline-none focus:ring-0 active:bg-blue-800 active:shadow-lg transition duration-150 ease-in-out">
        <i class="fa-solid fa-1x fa-tools -mx-3 "></i>
      </button>
    <div id="myDropdown" class="mx-5 dropdown-content rounded bg-white">
      <form class="form text-center bg-white my-3" action="/spool" method="post">
        <input type="number" class="text-center border-b-2 border-gray-300" name="spool" id="spool" placeholder="Empty Spool Weight"></input>
        <button type="submit" class="w-full py-2.5 bg-green-500 text-white font-medium text-xs leading-tight uppercase rounded shadow-md hover:bg-green-600 hover:shadow-lg focus:bg-green-600 focus:shadow-lg focus:outline-none focus:ring-0 active:bg-green-700 active:shadow-lg transition duration-150 ease-in-out" value="submit" aria-placeholder="Empty">Set</button>
      </form>

      <form class="form inline-block bg-white mb-3" action="/calibrate" method="post">
        <input type="number" class="text-center border-b-2 border-gray-300" name="known_weight" id="known_weight" placeholder="Known Weight eg.. 100g"></input>
        <button type="submit" class="w-full py-2.5 bg-red-600 text-white font-medium text-xs leading-tight uppercase rounded shadow-md hover:bg-red-700 hover:shadow-lg focus:bg-red-700 focus:shadow-lg focus:outline-none focus:ring-0 active:bg-red-800 active:shadow-lg transition duration-150 ease-in-out" value="submit">Calibrate</button>
      </form>
      <div class="bg-white mb-3">
        <button class="w-full py-2.5 bg-yellow-500 text-white font-medium text-xs leading-tight uppercase rounded shadow-md hover:bg-blue-700 hover:shadow-lg focus:bg-blue-700 focus:shadow-lg focus:outline-none focus:ring-0 active:bg-blue-800 active:shadow-lg transition duration-150 ease-in-out" onclick={tareScale}>Tare</button>
      </div>
      
      <form class="form flex-col bg-white mb-3" action="/api" method="post">
        <span class="text-center underline">Restore Settings</span>
        <input type="number" class="text-center border-b-2 border-gray-300" name="offset" id="offset" placeholder="Tare offset"></input>
        <input type="number" step="0.01" class="text-center border-b-2 border-gray-300" name="calvalue" id="calvalue" placeholder="Calibration value"></input>
        <input type="number" class="text-center border-b-2 border-gray-300" name="spool_weight" id="spool_weight" placeholder="Empty Spool Weight"></input>
        <button type="submit" class="w-full py-2.5 bg-blue-600 text-white font-medium text-xs leading-tight uppercase rounded shadow-md hover:bg-red-700 hover:shadow-lg focus:bg-red-700 focus:shadow-lg focus:outline-none focus:ring-0 active:bg-red-800 active:shadow-lg transition duration-150 ease-in-out" value="submit">Restore</button>
      </form>
    </div>
  </div>
  );
}

export default Settings_Dropdown;