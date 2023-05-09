/******************************************************************************
Copyright (c) Microsoft Corporation.

Permission to use, copy, modify, and/or distribute this software for any
purpose with or without fee is hereby granted.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
PERFORMANCE OF THIS SOFTWARE.
***************************************************************************** */
/* global Reflect, Promise */


function __awaiter(thisArg, _arguments, P, generator) {
    function adopt(value) { return value instanceof P ? value : new P(function (resolve) { resolve(value); }); }
    return new (P || (P = Promise))(function (resolve, reject) {
        function fulfilled(value) { try { step(generator.next(value)); } catch (e) { reject(e); } }
        function rejected(value) { try { step(generator["throw"](value)); } catch (e) { reject(e); } }
        function step(result) { result.done ? resolve(result.value) : adopt(result.value).then(fulfilled, rejected); }
        step((generator = generator.apply(thisArg, _arguments || [])).next());
    });
}

function _regeneratorRuntime() {
  _regeneratorRuntime = function () {
    return exports;
  };
  var exports = {},
    Op = Object.prototype,
    hasOwn = Op.hasOwnProperty,
    defineProperty = Object.defineProperty || function (obj, key, desc) {
      obj[key] = desc.value;
    },
    $Symbol = "function" == typeof Symbol ? Symbol : {},
    iteratorSymbol = $Symbol.iterator || "@@iterator",
    asyncIteratorSymbol = $Symbol.asyncIterator || "@@asyncIterator",
    toStringTagSymbol = $Symbol.toStringTag || "@@toStringTag";
  function define(obj, key, value) {
    return Object.defineProperty(obj, key, {
      value: value,
      enumerable: !0,
      configurable: !0,
      writable: !0
    }), obj[key];
  }
  try {
    define({}, "");
  } catch (err) {
    define = function (obj, key, value) {
      return obj[key] = value;
    };
  }
  function wrap(innerFn, outerFn, self, tryLocsList) {
    var protoGenerator = outerFn && outerFn.prototype instanceof Generator ? outerFn : Generator,
      generator = Object.create(protoGenerator.prototype),
      context = new Context(tryLocsList || []);
    return defineProperty(generator, "_invoke", {
      value: makeInvokeMethod(innerFn, self, context)
    }), generator;
  }
  function tryCatch(fn, obj, arg) {
    try {
      return {
        type: "normal",
        arg: fn.call(obj, arg)
      };
    } catch (err) {
      return {
        type: "throw",
        arg: err
      };
    }
  }
  exports.wrap = wrap;
  var ContinueSentinel = {};
  function Generator() {}
  function GeneratorFunction() {}
  function GeneratorFunctionPrototype() {}
  var IteratorPrototype = {};
  define(IteratorPrototype, iteratorSymbol, function () {
    return this;
  });
  var getProto = Object.getPrototypeOf,
    NativeIteratorPrototype = getProto && getProto(getProto(values([])));
  NativeIteratorPrototype && NativeIteratorPrototype !== Op && hasOwn.call(NativeIteratorPrototype, iteratorSymbol) && (IteratorPrototype = NativeIteratorPrototype);
  var Gp = GeneratorFunctionPrototype.prototype = Generator.prototype = Object.create(IteratorPrototype);
  function defineIteratorMethods(prototype) {
    ["next", "throw", "return"].forEach(function (method) {
      define(prototype, method, function (arg) {
        return this._invoke(method, arg);
      });
    });
  }
  function AsyncIterator(generator, PromiseImpl) {
    function invoke(method, arg, resolve, reject) {
      var record = tryCatch(generator[method], generator, arg);
      if ("throw" !== record.type) {
        var result = record.arg,
          value = result.value;
        return value && "object" == typeof value && hasOwn.call(value, "__await") ? PromiseImpl.resolve(value.__await).then(function (value) {
          invoke("next", value, resolve, reject);
        }, function (err) {
          invoke("throw", err, resolve, reject);
        }) : PromiseImpl.resolve(value).then(function (unwrapped) {
          result.value = unwrapped, resolve(result);
        }, function (error) {
          return invoke("throw", error, resolve, reject);
        });
      }
      reject(record.arg);
    }
    var previousPromise;
    defineProperty(this, "_invoke", {
      value: function (method, arg) {
        function callInvokeWithMethodAndArg() {
          return new PromiseImpl(function (resolve, reject) {
            invoke(method, arg, resolve, reject);
          });
        }
        return previousPromise = previousPromise ? previousPromise.then(callInvokeWithMethodAndArg, callInvokeWithMethodAndArg) : callInvokeWithMethodAndArg();
      }
    });
  }
  function makeInvokeMethod(innerFn, self, context) {
    var state = "suspendedStart";
    return function (method, arg) {
      if ("executing" === state) throw new Error("Generator is already running");
      if ("completed" === state) {
        if ("throw" === method) throw arg;
        return doneResult();
      }
      for (context.method = method, context.arg = arg;;) {
        var delegate = context.delegate;
        if (delegate) {
          var delegateResult = maybeInvokeDelegate(delegate, context);
          if (delegateResult) {
            if (delegateResult === ContinueSentinel) continue;
            return delegateResult;
          }
        }
        if ("next" === context.method) context.sent = context._sent = context.arg;else if ("throw" === context.method) {
          if ("suspendedStart" === state) throw state = "completed", context.arg;
          context.dispatchException(context.arg);
        } else "return" === context.method && context.abrupt("return", context.arg);
        state = "executing";
        var record = tryCatch(innerFn, self, context);
        if ("normal" === record.type) {
          if (state = context.done ? "completed" : "suspendedYield", record.arg === ContinueSentinel) continue;
          return {
            value: record.arg,
            done: context.done
          };
        }
        "throw" === record.type && (state = "completed", context.method = "throw", context.arg = record.arg);
      }
    };
  }
  function maybeInvokeDelegate(delegate, context) {
    var methodName = context.method,
      method = delegate.iterator[methodName];
    if (undefined === method) return context.delegate = null, "throw" === methodName && delegate.iterator.return && (context.method = "return", context.arg = undefined, maybeInvokeDelegate(delegate, context), "throw" === context.method) || "return" !== methodName && (context.method = "throw", context.arg = new TypeError("The iterator does not provide a '" + methodName + "' method")), ContinueSentinel;
    var record = tryCatch(method, delegate.iterator, context.arg);
    if ("throw" === record.type) return context.method = "throw", context.arg = record.arg, context.delegate = null, ContinueSentinel;
    var info = record.arg;
    return info ? info.done ? (context[delegate.resultName] = info.value, context.next = delegate.nextLoc, "return" !== context.method && (context.method = "next", context.arg = undefined), context.delegate = null, ContinueSentinel) : info : (context.method = "throw", context.arg = new TypeError("iterator result is not an object"), context.delegate = null, ContinueSentinel);
  }
  function pushTryEntry(locs) {
    var entry = {
      tryLoc: locs[0]
    };
    1 in locs && (entry.catchLoc = locs[1]), 2 in locs && (entry.finallyLoc = locs[2], entry.afterLoc = locs[3]), this.tryEntries.push(entry);
  }
  function resetTryEntry(entry) {
    var record = entry.completion || {};
    record.type = "normal", delete record.arg, entry.completion = record;
  }
  function Context(tryLocsList) {
    this.tryEntries = [{
      tryLoc: "root"
    }], tryLocsList.forEach(pushTryEntry, this), this.reset(!0);
  }
  function values(iterable) {
    if (iterable) {
      var iteratorMethod = iterable[iteratorSymbol];
      if (iteratorMethod) return iteratorMethod.call(iterable);
      if ("function" == typeof iterable.next) return iterable;
      if (!isNaN(iterable.length)) {
        var i = -1,
          next = function next() {
            for (; ++i < iterable.length;) if (hasOwn.call(iterable, i)) return next.value = iterable[i], next.done = !1, next;
            return next.value = undefined, next.done = !0, next;
          };
        return next.next = next;
      }
    }
    return {
      next: doneResult
    };
  }
  function doneResult() {
    return {
      value: undefined,
      done: !0
    };
  }
  return GeneratorFunction.prototype = GeneratorFunctionPrototype, defineProperty(Gp, "constructor", {
    value: GeneratorFunctionPrototype,
    configurable: !0
  }), defineProperty(GeneratorFunctionPrototype, "constructor", {
    value: GeneratorFunction,
    configurable: !0
  }), GeneratorFunction.displayName = define(GeneratorFunctionPrototype, toStringTagSymbol, "GeneratorFunction"), exports.isGeneratorFunction = function (genFun) {
    var ctor = "function" == typeof genFun && genFun.constructor;
    return !!ctor && (ctor === GeneratorFunction || "GeneratorFunction" === (ctor.displayName || ctor.name));
  }, exports.mark = function (genFun) {
    return Object.setPrototypeOf ? Object.setPrototypeOf(genFun, GeneratorFunctionPrototype) : (genFun.__proto__ = GeneratorFunctionPrototype, define(genFun, toStringTagSymbol, "GeneratorFunction")), genFun.prototype = Object.create(Gp), genFun;
  }, exports.awrap = function (arg) {
    return {
      __await: arg
    };
  }, defineIteratorMethods(AsyncIterator.prototype), define(AsyncIterator.prototype, asyncIteratorSymbol, function () {
    return this;
  }), exports.AsyncIterator = AsyncIterator, exports.async = function (innerFn, outerFn, self, tryLocsList, PromiseImpl) {
    void 0 === PromiseImpl && (PromiseImpl = Promise);
    var iter = new AsyncIterator(wrap(innerFn, outerFn, self, tryLocsList), PromiseImpl);
    return exports.isGeneratorFunction(outerFn) ? iter : iter.next().then(function (result) {
      return result.done ? result.value : iter.next();
    });
  }, defineIteratorMethods(Gp), define(Gp, toStringTagSymbol, "Generator"), define(Gp, iteratorSymbol, function () {
    return this;
  }), define(Gp, "toString", function () {
    return "[object Generator]";
  }), exports.keys = function (val) {
    var object = Object(val),
      keys = [];
    for (var key in object) keys.push(key);
    return keys.reverse(), function next() {
      for (; keys.length;) {
        var key = keys.pop();
        if (key in object) return next.value = key, next.done = !1, next;
      }
      return next.done = !0, next;
    };
  }, exports.values = values, Context.prototype = {
    constructor: Context,
    reset: function (skipTempReset) {
      if (this.prev = 0, this.next = 0, this.sent = this._sent = undefined, this.done = !1, this.delegate = null, this.method = "next", this.arg = undefined, this.tryEntries.forEach(resetTryEntry), !skipTempReset) for (var name in this) "t" === name.charAt(0) && hasOwn.call(this, name) && !isNaN(+name.slice(1)) && (this[name] = undefined);
    },
    stop: function () {
      this.done = !0;
      var rootRecord = this.tryEntries[0].completion;
      if ("throw" === rootRecord.type) throw rootRecord.arg;
      return this.rval;
    },
    dispatchException: function (exception) {
      if (this.done) throw exception;
      var context = this;
      function handle(loc, caught) {
        return record.type = "throw", record.arg = exception, context.next = loc, caught && (context.method = "next", context.arg = undefined), !!caught;
      }
      for (var i = this.tryEntries.length - 1; i >= 0; --i) {
        var entry = this.tryEntries[i],
          record = entry.completion;
        if ("root" === entry.tryLoc) return handle("end");
        if (entry.tryLoc <= this.prev) {
          var hasCatch = hasOwn.call(entry, "catchLoc"),
            hasFinally = hasOwn.call(entry, "finallyLoc");
          if (hasCatch && hasFinally) {
            if (this.prev < entry.catchLoc) return handle(entry.catchLoc, !0);
            if (this.prev < entry.finallyLoc) return handle(entry.finallyLoc);
          } else if (hasCatch) {
            if (this.prev < entry.catchLoc) return handle(entry.catchLoc, !0);
          } else {
            if (!hasFinally) throw new Error("try statement without catch or finally");
            if (this.prev < entry.finallyLoc) return handle(entry.finallyLoc);
          }
        }
      }
    },
    abrupt: function (type, arg) {
      for (var i = this.tryEntries.length - 1; i >= 0; --i) {
        var entry = this.tryEntries[i];
        if (entry.tryLoc <= this.prev && hasOwn.call(entry, "finallyLoc") && this.prev < entry.finallyLoc) {
          var finallyEntry = entry;
          break;
        }
      }
      finallyEntry && ("break" === type || "continue" === type) && finallyEntry.tryLoc <= arg && arg <= finallyEntry.finallyLoc && (finallyEntry = null);
      var record = finallyEntry ? finallyEntry.completion : {};
      return record.type = type, record.arg = arg, finallyEntry ? (this.method = "next", this.next = finallyEntry.finallyLoc, ContinueSentinel) : this.complete(record);
    },
    complete: function (record, afterLoc) {
      if ("throw" === record.type) throw record.arg;
      return "break" === record.type || "continue" === record.type ? this.next = record.arg : "return" === record.type ? (this.rval = this.arg = record.arg, this.method = "return", this.next = "end") : "normal" === record.type && afterLoc && (this.next = afterLoc), ContinueSentinel;
    },
    finish: function (finallyLoc) {
      for (var i = this.tryEntries.length - 1; i >= 0; --i) {
        var entry = this.tryEntries[i];
        if (entry.finallyLoc === finallyLoc) return this.complete(entry.completion, entry.afterLoc), resetTryEntry(entry), ContinueSentinel;
      }
    },
    catch: function (tryLoc) {
      for (var i = this.tryEntries.length - 1; i >= 0; --i) {
        var entry = this.tryEntries[i];
        if (entry.tryLoc === tryLoc) {
          var record = entry.completion;
          if ("throw" === record.type) {
            var thrown = record.arg;
            resetTryEntry(entry);
          }
          return thrown;
        }
      }
      throw new Error("illegal catch attempt");
    },
    delegateYield: function (iterable, resultName, nextLoc) {
      return this.delegate = {
        iterator: values(iterable),
        resultName: resultName,
        nextLoc: nextLoc
      }, "next" === this.method && (this.arg = undefined), ContinueSentinel;
    }
  }, exports;
}
function asyncGeneratorStep(gen, resolve, reject, _next, _throw, key, arg) {
  try {
    var info = gen[key](arg);
    var value = info.value;
  } catch (error) {
    reject(error);
    return;
  }
  if (info.done) {
    resolve(value);
  } else {
    Promise.resolve(value).then(_next, _throw);
  }
}
function _asyncToGenerator(fn) {
  return function () {
    var self = this,
      args = arguments;
    return new Promise(function (resolve, reject) {
      var gen = fn.apply(self, args);
      function _next(value) {
        asyncGeneratorStep(gen, resolve, reject, _next, _throw, "next", value);
      }
      function _throw(err) {
        asyncGeneratorStep(gen, resolve, reject, _next, _throw, "throw", err);
      }
      _next(undefined);
    });
  };
}
function _unsupportedIterableToArray(o, minLen) {
  if (!o) return;
  if (typeof o === "string") return _arrayLikeToArray(o, minLen);
  var n = Object.prototype.toString.call(o).slice(8, -1);
  if (n === "Object" && o.constructor) n = o.constructor.name;
  if (n === "Map" || n === "Set") return Array.from(o);
  if (n === "Arguments" || /^(?:Ui|I)nt(?:8|16|32)(?:Clamped)?Array$/.test(n)) return _arrayLikeToArray(o, minLen);
}
function _arrayLikeToArray(arr, len) {
  if (len == null || len > arr.length) len = arr.length;
  for (var i = 0, arr2 = new Array(len); i < len; i++) arr2[i] = arr[i];
  return arr2;
}
function _createForOfIteratorHelperLoose(o, allowArrayLike) {
  var it = typeof Symbol !== "undefined" && o[Symbol.iterator] || o["@@iterator"];
  if (it) return (it = it.call(o)).next.bind(it);
  if (Array.isArray(o) || (it = _unsupportedIterableToArray(o)) || allowArrayLike && o && typeof o.length === "number") {
    if (it) o = it;
    var i = 0;
    return function () {
      if (i >= o.length) return {
        done: true
      };
      return {
        done: false,
        value: o[i++]
      };
    };
  }
  throw new TypeError("Invalid attempt to iterate non-iterable instance.\nIn order to be iterable, non-array objects must have a [Symbol.iterator]() method.");
}

var dicUrl = 'https://s3.vgg.cool/test/js/vgg-di-container.esm.js';
var vggSdk;
var vggContainer$1;
var vggWasmKey = 'vggWasmKey';
function getVggSdk() {
  return _getVggSdk.apply(this, arguments);
}
function _getVggSdk() {
  _getVggSdk = _asyncToGenerator( /*#__PURE__*/_regeneratorRuntime().mark(function _callee() {
    var wasm;
    return _regeneratorRuntime().wrap(function _callee$(_context) {
      while (1) switch (_context.prev = _context.next) {
        case 0:
          if (vggSdk) {
            _context.next = 5;
            break;
          }
          _context.next = 3;
          return getVgg();
        case 3:
          wasm = _context.sent;
          vggSdk = new wasm.VggSdk();
        case 5:
          return _context.abrupt("return", vggSdk);
        case 6:
        case "end":
          return _context.stop();
      }
    }, _callee);
  }));
  return _getVggSdk.apply(this, arguments);
}
var sleep = function sleep(ms) {
  return new Promise(function (r) {
    return setTimeout(r, ms);
  });
};
function getVgg() {
  return _getVgg.apply(this, arguments);
}
function _getVgg() {
  _getVgg = _asyncToGenerator( /*#__PURE__*/_regeneratorRuntime().mark(function _callee4() {
    var container, vgg, i;
    return _regeneratorRuntime().wrap(function _callee4$(_context4) {
      while (1) switch (_context4.prev = _context4.next) {
        case 0:
          _context4.next = 2;
          return getContainer();
        case 2:
          container = _context4.sent;
          i = 0;
        case 4:
          if (!(i < 1000)) {
            _context4.next = 13;
            break;
          }
          // retry; wait for setVgg
          vgg = container.vggGetObject(vggWasmKey);
          if (!vgg) {
            _context4.next = 8;
            break;
          }
          return _context4.abrupt("break", 13);
        case 8:
          _context4.next = 10;
          return sleep(1);
        case 10:
          i++;
          _context4.next = 4;
          break;
        case 13:
          return _context4.abrupt("return", vgg);
        case 14:
        case "end":
          return _context4.stop();
      }
    }, _callee4);
  }));
  return _getVgg.apply(this, arguments);
}
function getContainer() {
  return _getContainer.apply(this, arguments);
}
function _getContainer() {
  _getContainer = _asyncToGenerator( /*#__PURE__*/_regeneratorRuntime().mark(function _callee5() {
    return _regeneratorRuntime().wrap(function _callee5$(_context5) {
      while (1) switch (_context5.prev = _context5.next) {
        case 0:
          if (vggContainer$1) {
            _context5.next = 4;
            break;
          }
          _context5.next = 3;
          return getRemoteContainer();
        case 3:
          vggContainer$1 = _context5.sent;
        case 4:
          return _context5.abrupt("return", Promise.resolve(vggContainer$1));
        case 5:
        case "end":
          return _context5.stop();
      }
    }, _callee5);
  }));
  return _getContainer.apply(this, arguments);
}
function getRemoteContainer() {
  return _getRemoteContainer.apply(this, arguments);
}
function _getRemoteContainer() {
  _getRemoteContainer = _asyncToGenerator( /*#__PURE__*/_regeneratorRuntime().mark(function _callee6() {
    return _regeneratorRuntime().wrap(function _callee6$(_context6) {
      while (1) switch (_context6.prev = _context6.next) {
        case 0:
          _context6.next = 2;
          return import( /* webpackIgnore: true */dicUrl);
        case 2:
          return _context6.abrupt("return", _context6.sent);
        case 3:
        case "end":
          return _context6.stop();
      }
    }, _callee6);
  }));
  return _getRemoteContainer.apply(this, arguments);
}

var _isProxyKey = /*#__PURE__*/Symbol();
var _proxyKey = /*#__PURE__*/Symbol();
var _parentKey = /*#__PURE__*/Symbol();
function getDesignDocument() {
  return _getDesignDocument.apply(this, arguments);
}
function _getDesignDocument() {
  _getDesignDocument = _asyncToGenerator( /*#__PURE__*/_regeneratorRuntime().mark(function _callee() {
    var sdk, docString, jsonDoc, designDoc;
    return _regeneratorRuntime().wrap(function _callee$(_context) {
      while (1) switch (_context.prev = _context.next) {
        case 0:
          _context.next = 2;
          return getVggSdk();
        case 2:
          sdk = _context.sent;
          docString = sdk.getDesignDocument();
          jsonDoc = JSON.parse(docString);
          designDoc = makeDeepProxy(jsonDoc, undefined);
          designDoc.sdk = sdk;
          return _context.abrupt("return", designDoc);
        case 8:
        case "end":
          return _context.stop();
      }
    }, _callee);
  }));
  return _getDesignDocument.apply(this, arguments);
}
function isProxy(doc) {
  var _doc$_isProxyKey;
  // @ts-ignore
  return (_doc$_isProxyKey = doc[_isProxyKey]) != null ? _doc$_isProxyKey : false;
}
// internal
var VggProxyHandler = /*#__PURE__*/function () {
  function VggProxyHandler() {}
  var _proto = VggProxyHandler.prototype;
  _proto.set = function set(target, prop, value) {
    // @ts-ignore
    var path = getPathToNode(target[_proxyKey]);
    var copiedValue = value;
    var isSdk = path === '/' && prop === 'sdk';
    if (!isSdk) {
      copiedValue = JSON.parse(JSON.stringify(value));
    }
    try {
      if (target[prop]) {
        // skip, array.length
        if (!Array.isArray(target) || prop !== 'length') {
          var _this$rootDesignDocPr, _this$rootDesignDocPr2;
          (_this$rootDesignDocPr = this.rootDesignDocProxy) == null ? void 0 : (_this$rootDesignDocPr2 = _this$rootDesignDocPr.sdk) == null ? void 0 : _this$rootDesignDocPr2.updateAt("" + path + prop, JSON.stringify(copiedValue));
        }
      } else {
        var _this$rootDesignDocPr3, _this$rootDesignDocPr4;
        (_this$rootDesignDocPr3 = this.rootDesignDocProxy) == null ? void 0 : (_this$rootDesignDocPr4 = _this$rootDesignDocPr3.sdk) == null ? void 0 : _this$rootDesignDocPr4.addAt("" + path + prop, JSON.stringify(copiedValue));
      }
    } catch (error) {
      throw error;
    }
    if (typeof copiedValue == 'object' && !isSdk) {
      var childProxy = makeDeepProxy(copiedValue, this.rootDesignDocProxy);
      // @ts-ignore
      defineParent(childProxy, target[_proxyKey]);
      target[prop] = childProxy;
    } else {
      target[prop] = copiedValue;
    }
    return true;
  };
  _proto.deleteProperty = function deleteProperty(target, prop) {
    if (prop in target) {
      // @ts-ignore
      var path = getPathToNode(target[_proxyKey]);
      try {
        var _this$rootDesignDocPr5, _this$rootDesignDocPr6;
        (_this$rootDesignDocPr5 = this.rootDesignDocProxy) == null ? void 0 : (_this$rootDesignDocPr6 = _this$rootDesignDocPr5.sdk) == null ? void 0 : _this$rootDesignDocPr6.deleteAt("" + path + prop);
      } catch (error) {
        throw error;
      }
      return delete target[prop];
    }
    return false;
  };
  _proto.defineProperty = function defineProperty(target, prop, descriptor) {
    if (typeof prop == 'string') {
      // @ts-ignore
      var path = getPathToNode(target[_proxyKey]);
      try {
        if (descriptor.value) {
          var value = descriptor.value;
          if (typeof value == 'object') {
            var proxyObj = makeDeepProxy(value, this.rootDesignDocProxy);
            descriptor.value = proxyObj;
            if (target[prop]) {
              var _this$rootDesignDocPr7, _this$rootDesignDocPr8;
              (_this$rootDesignDocPr7 = this.rootDesignDocProxy) == null ? void 0 : (_this$rootDesignDocPr8 = _this$rootDesignDocPr7.sdk) == null ? void 0 : _this$rootDesignDocPr8.updateAt("" + path + prop, value);
            } else {
              var _this$rootDesignDocPr9, _this$rootDesignDocPr10;
              (_this$rootDesignDocPr9 = this.rootDesignDocProxy) == null ? void 0 : (_this$rootDesignDocPr10 = _this$rootDesignDocPr9.sdk) == null ? void 0 : _this$rootDesignDocPr10.addAt("" + path + prop, value);
            }
          }
        }
      } catch (error) {
        throw error;
      }
    }
    return Reflect.defineProperty(target, prop, descriptor);
  };
  return VggProxyHandler;
}();
function getPathToNode(childProxy, _nodeProxy) {
  return calculatNodePath(childProxy);
}
/*
parent.childName = child
parent.children[0] = child
*/
function calculatNodePath(nodeProxy) {
  var path = "";
  if (!nodeProxy) {
    return path;
  }
  var childProxy = nodeProxy;
  // @ts-ignore
  while (childProxy[_parentKey]) {
    // @ts-ignore
    var parentProxy = childProxy[_parentKey];
    for (var _i = 0, _Object$entries = Object.entries(parentProxy); _i < _Object$entries.length; _i++) {
      var _Object$entries$_i = _Object$entries[_i],
        childName = _Object$entries$_i[0],
        childEntry = _Object$entries$_i[1];
      if (childEntry === childProxy) {
        path = childName + "/" + path;
        break;
      }
    }
    childProxy = parentProxy;
  }
  path = "/" + path;
  return path;
}
function makeDeepProxy(object, rootObject) {
  if (!object || typeof object != "object") {
    return object;
  }
  // @ts-ignore
  if (object[_isProxyKey] == true) {
    return object;
  }
  var handler = new VggProxyHandler();
  var selfProxy = new Proxy(object, handler);
  handler.rootDesignDocProxy = rootObject || selfProxy;
  Object.defineProperty(selfProxy, _isProxyKey, {
    value: true,
    writable: false,
    enumerable: false,
    configurable: false
  });
  var propNames = Reflect.ownKeys(object);
  for (var _iterator = _createForOfIteratorHelperLoose(propNames), _step; !(_step = _iterator()).done;) {
    var name = _step.value;
    // @ts-ignore
    var value = object[name];
    if (value && typeof value === "object") {
      var childProxy = makeDeepProxy(value, handler.rootDesignDocProxy);
      defineParent(childProxy, selfProxy);
      // @ts-ignore
      object[name] = childProxy;
    }
  }
  // must after Reflect.ownKeys
  Object.defineProperty(object, _proxyKey, {
    value: selfProxy,
    enumerable: false,
    configurable: false
  });
  return selfProxy;
}
function defineParent(childProxy, parentProxy) {
  if (parentProxy) {
    Object.defineProperty(childProxy, _parentKey, {
      value: parentProxy,
      enumerable: false,
      configurable: false
    });
  }
}

var design_document = {
  __proto__: null,
  getDesignDocument: getDesignDocument,
  isProxy: isProxy
};

function testAddColor() {
    return __awaiter(this, void 0, void 0, function* () {
        // Given
        const doc = yield design_document.getDesignDocument();
        let theStyle = doc.artboard[0].layers[0].childObjects[0].style;
        let newColor = { class: 'color', alpha: 0.5, red: 0.5, green: 0.5, blue: 0.5 };
        let contextSetting = {
            class: 'graphicsContextSettings',
            blendMode: 0,
            opacity: 0.5,
            isolateBlending: true,
            transparencyKnockoutGroup: 0
        };
        let aFill = {
            class: 'fill', isEnabled: true, color: newColor, fillType: 0,
            contextSettings: contextSetting
        };
        // When
        try {
            theStyle.fills.push(aFill);
        }
        catch (error) {
            console.log("error: ", error);
        }
        // Then
        // console.log("document now is: ", doc);
    });
}
testAddColor();
