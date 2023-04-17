// Given
const { getVggSdk } = await import("https://s3.vgg.cool/test/js/vgg-sdk.esm.js");
const vggSdk = await getVggSdk();

// When
const documentString = vggSdk.getDocumentJson();
// Then
console.log("document json string from sdk is: ", documentString);