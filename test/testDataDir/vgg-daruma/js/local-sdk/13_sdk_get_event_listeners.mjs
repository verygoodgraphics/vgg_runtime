// Given
const EventNameClick = 'click';
const EventListenerKey = 'listener';

const { getVggSdk } = await import("./testDataDir/fake-sdk/vgg-sdk.esm.mjs");
const sdk = await getVggSdk();

const listenerCode = "console.log('hello');"
sdk.addEventListener('/js/fake/run_added_listener', EventNameClick, listenerCode);


// When
const listeners = sdk.getEventListeners('/js/fake/run_added_listener')[EventNameClick];

// Then
if (listeners.length != 1 || listeners[0][EventListenerKey] != listenerCode) {
  throw new Error("listeners.length != 1 || listeners[0] != listenerCode");
}