import Fila_Gauge from "./Fila_Gauge";
import { createSignal, onCleanup } from "solid-js";

const Card = (props) => {
  const [count, setCount] = createSignal(0);
	const interval = setInterval(
		() => setCount(c => c + 10),
		1000
	);
	onCleanup(() => clearInterval(interval));

  return (
    <div class="rounded-3xl shadow-2xl px-12 py-6">
        <div class="font-bold text-xl mb-2" id={props.id}>{props.title}</div>
        <Fila_Gauge renderto={props.id} filament={props.filament}/>
  </div>
  );
}

export default Card;