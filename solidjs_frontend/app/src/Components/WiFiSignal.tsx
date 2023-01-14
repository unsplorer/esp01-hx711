import { createEffect, createSignal } from "solid-js";

const WiFiSignal = (props) => {
  const map = (value, x1, y1, x2, y2) => (value - x1) * (y2 - x2) / (y1 - x1) + x2;

  const [color, setColor] = createSignal(0);

  createEffect(() => {
    props.rssi;
    setColor(map((props.rssi*-1),65,85,0,255));
  });

  return (
    <i class="fa-solid fa-wifi" id="wifi_signal"><span id="wifiText" class="font-mono" style={{color: `rgb(${color()}, ${128 - color()}, 128)`}}> {props.rssi} dBm</span></i>
  );
}

export default WiFiSignal;