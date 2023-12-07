let count = 0;

function handleEvent(event) {
  console.log('handle event:', event, ', type:', event.type, ', target:', event.target);

  // NOTE: _currentVgg is set by vgg_runtime
  const vggSdk = new _currentVgg.VggSdk();
  switch (event.type) {
    case 'mousedown': {
      vggSdk.updateAt('/frames/0/childObjects/1/style/fills/0/color/alpha', JSON.stringify(1.0));
      break;
    }

    case 'mouseup': {
      vggSdk.updateAt('/frames/0/childObjects/1/style/fills/0/color/alpha', JSON.stringify(0.5));
      break;
    }

    case 'click': {
      // const valuePath = '/frames/0/childObjects/3/content';
      // let count = valueNode.content;
      // count = parseInt(count) + 1;
      count++;

      vggSdk.updateAt('/frames/0/childObjects/3/content', JSON.stringify(count.toString()));
      break;
    }

    default:
      break;
  }

}

export default handleEvent;
