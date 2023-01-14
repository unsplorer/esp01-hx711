const WiFiSignal = (props) => {


  return (
    <i class="fa-solid fa-wifi" id="wifi_signal"><span class="font-sans"> {props.rssi} dBm</span></i>
  );
}

export default WiFiSignal;