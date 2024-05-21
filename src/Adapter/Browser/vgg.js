const vggOnRuntimeInitialized = Module["onRuntimeInitialized"];

Module["onRuntimeInitialized"] = function () {
  // If an initialize function is already configured, execute that first.
  vggOnRuntimeInitialized && vggOnRuntimeInitialized();

  Module.setUp = function (canvas, clientWidth, clientHeight, devicePixelRatio) {
    canvas.width = clientWidth * devicePixelRatio;
    canvas.height = clientHeight * devicePixelRatio;

    var contextAttributes = {
      "alpha": 1,
      "depth": 0,
      "stencil": 8,
      "antialias": 0,
      "premultipliedAlpha": 1,
      "preserveDrawingBuffer": 0,
      "preferLowPowerToHighPerformance": 0,
      "failIfMajorPerformanceCaveat": 0,
      "enableExtensionsByDefault": 1,
      "explicitSwapControl": 0,
      "renderViaOffscreenBackBuffer": 0,
    };

    var gl = canvas.getContext("webgl2", contextAttributes);
    if (!gl) {
      gl = canvas.getContext("webgl", contextAttributes);
    }
    var handle = GL.registerContext(gl, contextAttributes);
    GL.makeContextCurrent(handle);
    GL.currentContext.GLctx.getExtension('WEBGL_debug_renderer_info');

    const cppWebContainer = new Module.WebContainer(clientHeight, clientHeight, devicePixelRatio);
    Module.cppWebContainer = cppWebContainer;
  };

  let requestId;
  Module.tearDown = function () {
    if (Module.cppWebContainer) {
      Module.cppWebContainer.delete();
      delete (Module.cppWebContainer);
    }
    if (requestId)
      cancelAnimationFrame(requestId);
  };

  function runLoop() {
    requestId = requestAnimationFrame(function () {
      Module.cppWebContainer.run();
      runLoop();
    });
  }

  Module.load = function (int8ArrayValue) {
    const success = Module.cppWebContainer.load(int8ArrayValue);
    if (success)
      runLoop();
    return success;
  };

};
