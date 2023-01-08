import { createSignal, onCleanup } from "solid-js";
import { render } from "solid-js/web";

const Fila_Gauge = (props) => {
	const [count, setCount] = createSignal(0);
	const interval = setInterval(
		() => setCount(c => c + 10),
		1000
	);
	onCleanup(() => clearInterval(interval));
return (
<canvas data-type="radial-gauge"
        data-width="250"
        data-height="250"
        data-units="grams"
        data-title=""
        data-value={count()}
        data-min-value="0"
        data-max-value="1000"
        data-major-ticks="0,100,200,300,400,500,600,700,800,900,1000"
        data-minor-ticks="2"
        data-stroke-ticks="false"
        data-highlights='[
            { "from": 0, "to": 50, "color": "rgba(255,0,0,.25)" },
            { "from": 50, "to": 100, "color": "rgba(255,128,0,.25)" },
            { "from": 100, "to": 150, "color": "rgba(128,255,0,.25)" },
            { "from": 150, "to": 200, "color": "rgba(0,255,128,.25)" },
            { "from": 200, "to": 220, "color": "rgba(0,0,0,.25)" }
        ]'
        data-color-plate="#222"
        data-color-major-ticks="#f5f5f5"
        data-color-minor-ticks="#ddd"
        data-color-title="#fff"
        data-color-units="#ccc"
        data-color-numbers="#eee"
        data-color-needle-start="rgba(240, 128, 128, 1)"
        data-color-needle-end="rgba(255, 160, 122, .9)"
        data-value-box="true"
        data-animation-rule="bounce"
        data-animation-duration="500"
        data-font-value="Led"
        data-animated-value="true"
></canvas>
);
}

export default Fila_Gauge;