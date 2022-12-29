const Card = (props) => {
  return (
    <div class="max-w-sm rounded overflow-hidden shadow-lg">
      <div class="font-bold text-xl mb-2">Card Title Goes Here</div>
      <img class="w-full filter hue-rotate-60" src="public/logo.png" alt="Sunset in the mountains"> </img>
      <div class="px-6 py-4">
        <p class="text-gray-700 text-base">
          Gauge Goes where image is above
        </p>
      </div>
  </div>
  );
}

export default Card;