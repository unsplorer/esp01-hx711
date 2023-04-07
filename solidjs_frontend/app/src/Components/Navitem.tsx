const Navitem = (props) => {

  return (
    <button class={props.class}>
      <i class="fa-solid fa-circle-half-stroke"></i>
      {props.name}
    </button>
  )
}

export default Navitem;