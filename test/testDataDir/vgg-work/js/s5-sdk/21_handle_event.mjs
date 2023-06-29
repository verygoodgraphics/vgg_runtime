const { DesignDocument } = await import('https://s5.vgg.cool/vgg-sdk.esm.js');
const doc = await DesignDocument.getDesignDocument();

function handleEvent(event) {
  console.log('handleEvent:', event, ', type:', event.type, ', target:', event.target);

  switch (event.type) {
    case 'click': {
      console.log('button:', event.button);
    }
      break;

    default:
      break;
  }
  //todo console.log('handleEvent, event.button is: ', event.button);
  event.preventDefault();
  doc.artboard = [];
}

export default handleEvent;