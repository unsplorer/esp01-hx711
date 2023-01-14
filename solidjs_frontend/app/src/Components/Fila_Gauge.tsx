const Fila_Gauge = (props) => {
return (
 <canvas data-type="radial-gauge"
        data-width="300"
        data-height="300"
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
        data-color-plate="#fff"
        data-color-major-ticks="#333"
        data-color-minor-ticks="#222"
        data-color-title="#444"
        data-color-units="#333"
        data-color-numbers="555"
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
 );}


export default Fila_Gauge;