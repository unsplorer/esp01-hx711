import { createEffect } from "solid-js";

const Fila_Gauge = (props) => {
    let colorPlate = "#222"
    let colorPlateEnd = "#222"
    let textColor = "#AAA";
    let colorMajorTicks = "#DDD";
    let colorMinorTicks = "#EEE";
    let colorUnits = "#DDD";
    let width = Math.max(document.documentElement.clientWidth, window.innerWidth || 0);
    // let width = Math.max(document.documentElement.clientHeight, window.innerHeight || 0);
    width = (width < 400) ? 300 : width *.75
    width = (width > 800) ? 500 : width
    let height = width;
    createEffect(() => {
        let gauge = document.getElementById("gauge");
        if (props.darkMode() == "Dark") {
            colorPlate = "#000"
            colorPlateEnd = "#333"
            textColor = "#AAA";
            colorMajorTicks = "#AAA";
            colorMinorTicks = "#AAA";
            colorUnits = "#CCC";

        } else {
            colorPlate = "#FFF";
            colorPlateEnd = "#FFF";
            textColor = "#555";
            colorMajorTicks = "#333"
            colorMinorTicks = "#222";
            colorUnits = "#333";
        }
        gauge.setAttribute('data-color-plate', colorPlate);
        gauge.setAttribute('data-color-plate-end', colorPlateEnd);
        gauge.setAttribute('data-color-major-ticks', colorMajorTicks);
        gauge.setAttribute('data-color-minor-ticks', colorMinorTicks);
        gauge.setAttribute('data-color-numbers', textColor);
        gauge.setAttribute('data-color-units', colorUnits);
        gauge.setAttribute('data-color-value-text', colorUnits);

        
    });
    return (
    <canvas class="inline-flex" id="gauge" data-type="radial-gauge"
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
        data-value-box-stroke="0"
        data-color-value-box-shadow="false"
        data-color-value-box-background="false"
        data-color-value-text={textColor}
        data-color-plate={colorPlate}
        data-color-plate-end={colorPlateEnd}
        data-color-major-ticks={colorMajorTicks}
        data-color-minor-ticks={colorMinorTicks}
        data-color-title="#444"
        data-color-units={colorUnits}
        data-color-numbers={textColor}
        data-needle-type="arrow"
        data-needle-start="15"
        data-needle-end="90"
        data-needle-width="5"
        data-color-needle="#924F"
        // data-color-needle-start="#00A9"
        data-color-needle-end="#00F9"
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