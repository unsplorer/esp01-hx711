import Fila_Gauge from "./Fila_Gauge";
import WiFiSignal from "./WiFiSignal";
import Settings_Dropdown from "./Settings_Dropdown";

const Card = (props) => {
  const cardHeader = <h1 class="font-bold text-xl mb-2" id={props.id}>{props.title}</h1>;
  const cardFooter = 
  <span class="flex justify-between items-center -mt-8 -mx-6 py-1">
    <Settings_Dropdown />
    <WiFiSignal rssi={props.rssi} />
  </span>;

  return (
    <div class="dark:bg-zinc-800 dark:text-white rounded-3xl items-center shadow-2xl px-12 py-6 flex-col">
      {cardHeader}
      <Fila_Gauge value={props.filament} class="flex"/>
      {cardFooter}
    </div>
  );
}

export default Card;