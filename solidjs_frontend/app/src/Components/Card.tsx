import Fila_Gauge from "./Fila_Gauge";
import WiFiSignal from "./WiFiSignal";
import Settings_Dropdown from "./Settings_Dropdown";

const Card = (props) => {


  return (
    <div class="rounded-3xl shadow-2xl px-12 py-6">
        <h1 class="font-bold text-xl mb-2" id={props.id}>{props.title}</h1>
        <Fila_Gauge value={props.filament}/>
        <span class="flex justify-between items-center -mt-8 -mx-6">
          <Settings_Dropdown/>
          <WiFiSignal rssi={props.rssi}/>
        </span>
    </div>
  );
}

export default Card;