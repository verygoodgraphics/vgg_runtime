// Given
const { getVggSdk } = await import("./testDataDir/fake-sdk/vgg-sdk.esm.mjs");
const sdk = await getVggSdk();

const listener_code = "console.log('hello');"
const listener_code2 = "console.log('world');"

// When
sdk.addEventListener('/js/fake/run_added_listener', 'click', listener_code);
sdk.addEventListener('/js/fake/run_added_listener', 'click', listener_code);  // duplicated item will not be added
sdk.addEventListener('/js/fake/run_added_listener', 'click', listener_code2);
sdk.addEventListener('/js/fake/run_added_listener', 'click', listener_code2);  // duplicated item will not be added

// Then
const listeners = sdk.getEventListeners('/js/fake/run_added_listener', 'click');
console.log('js got listeners: ', listeners);
if (listeners.length != 2 || listeners[0] != listener_code || listeners[1] != listener_code2) {
  throw new Error("listeners.length != 2 || listeners[0] != listener_code || listeners[1] != listener_code2");
}