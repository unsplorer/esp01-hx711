import { createEffect } from "solid-js";

const Fila_Gauge = (props) => {
    let colorPlate = "#222"
    let textColor = "#EEE";
    let colorMajorTicks = "#DDD";
    let colorMinorTicks = "#EEE";
    let colorUnits = "#DDD";
    let width = Math.max(document.documentElement.clientWidth, window.innerWidth || 0);
    width < 400 ? width = 300 : width = width *.3
    let height = width;
    createEffect(() => {
        let gauge = document.getElementById("gauge");
        console.log(gauge);
        if (props.darkMode() == "Dark") {
            console.log(`switching to ${props.darkMode()}`);
            colorPlate = "#222"
            textColor = "#FFF";
            colorMajorTicks = "#AAA";
            colorMinorTicks = "#AAA";
            colorUnits = "#CCC";

        } else {
            colorPlate = "#FFF";
            textColor = "#555";
            colorMajorTicks = "#333"
            colorMinorTicks = "#222";
            colorUnits = "#333";
        }
        gauge.setAttribute('data-color-plate', colorPlate);
        gauge.setAttribute('data-color-major-ticks', colorMajorTicks);
        gauge.setAttribute('data-color-minor-ticks', colorMinorTicks);
        gauge.setAttribute('data-color-numbers', textColor);
        gauge.setAttribute('data-color-units', colorUnits);

        
    });
    return (
    <canvas id="gauge" data-type="radial-gauge"
        data-width={width}
        data-height={height}
        data-units="grams"
        data-title=""
        data-value={props.value}
        data-min-value="0"
        data-max-value="1000"
        data-major-ticks="0,200,400,600,800,1000"
        data-minor-ticks="4"
        data-stroke-ticks="true"
        data-highlights='[
        {
            "from": 0,
            "to": 100,
            "color": "#FF0000"
            }
        ]'
        data-value-text-shadow="false"
        data-color-value-box-shadow="false"
        data-color-value-box-background="false"
        data-value-box-stroke="0"
        data-color-plate={colorPlate}
        data-color-major-ticks={colorMajorTicks}
        data-color-minor-ticks={colorMinorTicks}
        data-color-title="#444"
        data-color-units={colorUnits}
        data-color-numbers={textColor}
        data-color-needle-start="#000"
        data-color-needle-end="#00f9"
        data-color-needle-shadow-down="#222"
        data-borders="0"
        data-border-shadow-width="0"
        data-border-outer-width="0"
        data-border-radius="0"
        data-border-inner-width="0"
        data-border-middle-width="0"
        data-value-box="true"
        data-animation-rule="bounce"
        data-animation-duration="500"
        data-font-value="Led"
        data-animated-value="true"
    ></canvas>
    );
}


export default Fila_Gauge;