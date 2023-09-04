// Given
const { getVggSdk } = await import("./testDataDir/fake-sdk/vgg-sdk.esm.mjs");
const sdk = await getVggSdk();

const listenerCode = "console.log('hello');"
const listener_code2 = "console.log('world');"

// When
sdk.addEventListener('/js/fake/run_added_listener', 'click', listenerCode);
sdk.addEventListener('/js/fake/run_added_listener', 'click', listenerCode);  // duplicated item will not be added
sdk.addEventListener('/js/fake/run_added_listener', 'click', listener_code2);
sdk.addEventListener('/js/fake/run_added_listener', 'click', listener_code2);  // duplicated item will not be added

// Then
const EventNameClick = 'click';
const EventListenerKey = 'listener';

const listeners = sdk.getEventListeners('/js/fake/run_added_listener')[EventNameClick];
console.log('js got listeners: ', listeners);
if (listeners.length != 2 || listeners[0][EventListenerKey] != listenerCode || listeners[1][EventListenerKey] != listener_code2) {
  throw new Error("listeners.length != 2 || listeners[0] != listenerCode || listeners[1] != listener_code2");
}