var vggEnv = '__vggDefaultEnv';
var vggContainer = /*#__PURE__*/new Map();
function vggGetEnvContainer() {
  if (!vggContainer.has(vggEnv)) {
    vggContainer.set(vggEnv, new Map());
  }
  return vggContainer.get(vggEnv);
}
function vggSetEnv(value) {
  vggEnv = value;
}
function vggGetEnv() {
  return vggEnv;
}
function vggSetObject(key, value) {
  vggGetEnvContainer().set(key, value);
}
function vggGetObject(key) {
  return vggGetEnvContainer().get(key);
}

export { vggGetEnv, vggGetObject, vggSetEnv, vggSetObject };
//# sourceMappingURL=vgg-di-container.esm.js.map