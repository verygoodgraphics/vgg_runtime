// Given
// const { getVggSdk } = await import("https://s3.vgg.cool/test/js/vgg-sdk.esm.js");
const { getVggSdk, DesignDocument } = await import("./testDataDir/fake-sdk/vgg-sdk.esm.mjs");
const doc = await DesignDocument.getDesignDocument();

// When
try {
  doc.artboard[0].layers[0].childObjects[0].style.fakeProperty = 'fakeValue';
}
catch (error) {
  console.log("error: ", error);
}
// Then
// console.log("document now is: ", doc);