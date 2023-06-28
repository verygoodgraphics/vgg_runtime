const { DesignDocument } = await import('https://s5.vgg.cool/vgg-sdk.esm.js');
const doc = await DesignDocument.getDesignDocument();

function handleEvent(event) {
  console.log('handleEvent: ', event);
  //todo console.log('handleEvent, event.button is: ', event.button);
  event.preventDefault();
  delete doc.artboard;
}

export default handleEvent;