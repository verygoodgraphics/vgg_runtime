const { DesignDocument } = await import('https://s5.vgg.cool/vgg-sdk.esm.js');
const doc = await DesignDocument.getDesignDocument();

function handleEvent(event) {
  console.log('handleEvent:', event, ', type:', event.type, ', target:', event.target);
  switch (event.type) {
    case 'mousedown':
    case 'mouseup':
    case 'click': {
      console.log('altkey:', event.altkey);
      console.log('button:', event.button);
      console.log('clientX:', event.clientX);
      console.log('clientY:', event.clientY);
      console.log('ctrlkey:', event.ctrlkey);
      console.log('metakey:', event.metakey);
      console.log('shifkey:', event.shiftkey)
      console.log('x:', event.x);
      console.log('y:', event.y);
    }
      break;

    case 'mousemove': {
      console.log('altkey:', event.altkey);
      console.log('clientX:', event.clientX);
      console.log('clientY:', event.clientY);
      console.log('ctrlkey:', event.ctrlkey);
      console.log('metakey:', event.metakey);
      console.log('movementX:', event.movementX);
      console.log('movementY:', event.movementY);
      console.log('shifkey:', event.shiftkey)
      console.log('x:', event.x);
      console.log('y:', event.y);
    }
      break;


    case 'keyup':
    case 'keydown': {
      console.log('key:', event.key);
      console.log('repeat:', event.repeat);
      console.log('altkey:', event.altkey);
      console.log('ctrlkey:', event.ctrlkey);
      console.log('metakey:', event.metakey);
      console.log('shifkey:', event.shiftkey)
    }
      break;

    default:
      break;
  }
  event.preventDefault();
  doc.frames = [];
}

export default handleEvent;