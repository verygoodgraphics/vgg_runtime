// Given
const { getVggSdk } = await import("./testDataDir/fake-sdk/vgg-sdk.esm.mjs");
const sdk = await getVggSdk();

const listener_code = "console.log('hello');"

// When
try {
  sdk.addEventListener('/js/fake/run_added_listener', 'click', listener_code);
}
catch (error) {
  console.log("error: ", error);
}
// Then