import { createSignal, onCleanup } from "solid-js";


const Fila_Gauge = (props) => {
	const [count, setCount] = createSignal(0);
	const interval = setInterval(
		() => setCount(c => c + 10),
		1000
	);
	onCleanup(() => clearInterval(interval));

return (
    
 <canvas data-type="radial-gauge"
        data-width="300"
        data-height="300"
        data-units="grams"
        data-title=""
        data-value={props.value}
        data-min-value="0"
        data-max-value="1000"
        data-major-ticks="0,100,200,300,400,500,600,700,800,900,1000"
        data-minor-ticks="2"
        data-stroke-ticks="true"
        data-highlights='[
            {
                "from": 0,
                "to": 100,
                "color": "#FF0000"
            }
        ]'
        data-color-plate="#fff"
        data-color-major-ticks="#333"
        data-color-minor-ticks="#222"
        data-color-title="#444"
        data-color-units="#333"
        data-color-numbers="555"
        data-color-needle-start="#000"
        data-color-needle-end="#00f9"
        data-value-box="true"
        data-animation-rule="bounce"
        data-animation-duration="500"
        data-font-value="Led"
        data-animated-value="true"
></canvas>
 );}


export default Fila_Gauge;