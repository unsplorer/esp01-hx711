import Fila_Gauge from "./Fila_Gauge";
import WiFiSignal from "./WiFiSignal";
import Settings_Dropdown from "./Settings_Dropdown";


const Card = (props) => {
  const cardHeader = <h1 class="dark:text-inherit  font-bold text-xl mb-2">{props.title}</h1>;
  const cardFooter = 
  <span class="flex justify-between items-center md:-mt-8 md:-mx-6 pt-0 pb-4 md:pt-8 md:pb-0">
    <Settings_Dropdown />
    <WiFiSignal rssi={props.rssi} />
  </span>;

  return (
    <div class="dark:text-inherit rounded-3xl shadow-2xl py-0 px-4 md:px-12 md:py-3 w-full md:w-auto">
      {cardHeader}
      <Fila_Gauge value={props.filament} darkMode={props.darkMode}/>
      {cardFooter}
    </div>
  );
}

export default Card;