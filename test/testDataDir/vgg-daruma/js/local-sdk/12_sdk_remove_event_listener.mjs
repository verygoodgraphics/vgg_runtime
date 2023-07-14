// Given
const { getVggSdk } = await import("./testDataDir/fake-sdk/vgg-sdk.esm.mjs");
const sdk = await getVggSdk();

const listener_code = "console.log('hello');"
sdk.addEventListener('/js/fake/run_added_listener', 'click', listener_code);

// When
sdk.removeEventListener('/js/fake/run_added_listener', 'click', listener_code);

// Then
const EventNameClick = 'click';

const listeners = sdk.getEventListeners('/js/fake/run_added_listener')[EventNameClick];
if (listeners.length != 0) {
  throw new Error("listeners.length != 0");
}