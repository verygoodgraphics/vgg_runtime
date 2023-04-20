// Given
// const { getVggSdk } = await import("https://s3.vgg.cool/test/js/vgg-sdk.esm.js");
const { getVggSdk } = await import("./testDataDir/fake-sdk/vgg-sdk.esm.mjs");
const vggSdk = await getVggSdk();

// When
const documentString = vggSdk.getDocumentJson();
// Then
console.log("document json string from sdk is: ", documentString);

// When
const path = "/artboard";
vggSdk.deleteFromDocument(path);
// Then
console.log("document json string from sdk is: ", vggSdk.getDocumentJson());