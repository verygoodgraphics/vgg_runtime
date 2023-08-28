
const { DesignDocument, getVggSdk } = await import('https://s3.vgg.cool/test/js/editor/vgg-sdk.esm.js');
const doc = await DesignDocument.getDesignDocument();

function handleEvent(event) {
  console.log('handle event:', event, ', type:', event.type, ', target:', event.target);
  switch (event.type) {
    case 'click': {
      // const valuePath = '/frames/0/childObjects/3/content';
      const valueNode = doc.frames[0].childObjects[3];

      let count = valueNode.content;
      count = parseInt(count) + 1;

      valueNode.content = count.toString();
    }
      break;

    default:
      break;
  }

}

export default handleEvent;
