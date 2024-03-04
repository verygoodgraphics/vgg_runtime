function handleEvent(event, wrapper) {
  console.log(new Date().getTime(), 'handle event:', event, ', type:', event.type, ', target:', event.target);

  const wasmInstanceKey = 'instance';

  const vggSdk = new wrapper[wasmInstanceKey].VggSdk();
  const buttonId = '#counterButton'
  const countId = '#count'
  switch (event.type) {
    case 'mousedown': {
      // NOTE: patch in array NOT WORK, other fileds in elemetn.style.fills[0] is removed
      // let patch = { style: { fills: [{ color: { alpha: 1.0 } }] } };
      // vggSdk.updateElement(buttonId, JSON.stringify(patch));
      let element = JSON.parse(vggSdk.getElement(buttonId));
      element.style.fills[0].color.alpha = 1.0;
      vggSdk.updateElement(buttonId, JSON.stringify(element));
      break;
    }

    case 'mouseup':
    case 'mouseleave': {
      let element = JSON.parse(vggSdk.getElement(buttonId));
      element.style.fills[0].color.alpha = 0.5;
      vggSdk.updateElement(buttonId, JSON.stringify(element));
      break;
    }

    case 'click': {
      let element = JSON.parse(vggSdk.getElement(countId));
      let count = parseInt(element.content) + 1;
      // object patch works
      let patch = { content: count.toString() };
      vggSdk.updateElement(countId, JSON.stringify(patch));
      break;
    }

    default:
      break;
  }

}

export default handleEvent;
