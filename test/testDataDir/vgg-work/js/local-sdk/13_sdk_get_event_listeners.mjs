// Given
const { getVggSdk } = await import("./testDataDir/fake-sdk/vgg-sdk.esm.mjs");
const sdk = await getVggSdk();

const listener_code = "console.log('hello');"
sdk.addEventListener('/js/fake/run_added_listener', 'click', listener_code);

// When
const listeners = sdk.getEventListeners('/js/fake/run_added_listener', 'click');

// Then
if (listeners.length != 1 || listeners[0] != listener_code) {
  throw new Error("listeners.length != 1 || listeners[0] != listener_code");
}