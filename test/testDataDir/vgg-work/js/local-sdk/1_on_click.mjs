// Given
// const { getVggSdk } = await import("https://s3.vgg.cool/test/js/vgg-sdk.esm.js");
const { getVggSdk } = await import("./testDataDir/fake-sdk/vgg-sdk.esm.mjs");
const vggSdk = await getVggSdk();

// When
const documentString = vggSdk.getDesignDocument();
// Then
console.log("document json string from sdk is: ", documentString);

// When
try {
  const path = "/artboard";
  vggSdk.deleteAt(path);
} catch (error) {
  console.error("delete throw exception, document not changed. Exception: ", error);
}
// Then
console.log("document json string from sdk is: ", vggSdk.getDesignDocument());