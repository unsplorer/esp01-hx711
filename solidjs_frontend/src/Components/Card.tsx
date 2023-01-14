import Fila_Gauge from "./Fila_Gauge";
import WiFiSignal from "./WiFiSignal";
import Settings_Dropdown from "./Settings_Dropdown";

const Card = (props) => {


  return (
    <div class="rounded-3xl shadow-2xl px-12 py-6">
        <div class="font-bold text-xl mb-2" id={props.id}>{props.title}</div>
        <Fila_Gauge value={props.filament}/>
        <div class="flex flex-row justify-between items-center -mt-8 -mx-6">
          <Settings_Dropdown/>
          <WiFiSignal rssi={props.rssi}/>
        </div>
    </div>
  );
}

export default Card;