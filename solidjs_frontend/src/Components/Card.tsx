import Fila_Gauge from "./Fila_Gauge";

const Card = (props) => {
  return (
    <div class="rounded-2xl shadow-lg px-12 py-6">
        <div class="font-bold text-xl mb-2">{props.title}</div>
        <Fila_Gauge/>
  </div>
  );
}

export default Card;