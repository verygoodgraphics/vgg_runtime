/*
 * Copyright (C) 2021 Chaoya Li <harry75369@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef __JAVA_SCRIPT_INTERPRETER_HPP__
#define __JAVA_SCRIPT_INTERPRETER_HPP__

#include <quickjs/quickjs.h>
#include <quickjs/quickjs-libc.h>
#include <nlohmann/json.hpp>
#include <optional>

#include "Entity/Entity.hpp"
#include "Entity/InputManager.hpp"
#include "Utils/Types.hpp"
#include "Utils/Utils.hpp"

namespace VGG
{

class JavaScriptInterpreter : Uncopyable
{
  using json = nlohmann::json;
  using MouseMove = MouseEntity::MouseMove;
  using MouseClick = MouseEntity::MouseClick;
  using MouseRelease = MouseEntity::MouseRelease;
  using KeyboardPress = KeyboardEntity::KeyboardPress;
  using KeyboardRelease = KeyboardEntity::KeyboardRelease;
  using KeyboardText = KeyboardEntity::KeyboardText;
  using EachFrame = InputManager::EachFrame;

  struct Events
  {
  };

  template<typename EventType>
  struct EventCallProgram
  {
    static const std::string get()
    {
      if constexpr (std::is_same_v<EventType, MouseMove>)
      {
        return "(events?.onmousemove || (() => {}))(evt)";
      }
      else if constexpr (std::is_same_v<EventType, MouseClick>)
      {
        return "(events?.onmousedown || (() => {}))(evt)";
      }
      else if constexpr (std::is_same_v<EventType, MouseRelease>)
      {
        return "(events?.onmouseup || (() => {}))(evt)";
      }
      else if constexpr (std::is_same_v<EventType, KeyboardPress>)
      {
        return "(events?.onkeydown || (() => {}))(evt)";
      }
      else if constexpr (std::is_same_v<EventType, KeyboardText>)
      {
        return "(events?.oninput || (() => {}))(evt)";
      }
      else if constexpr (std::is_same_v<EventType, EachFrame>)
      {
        return "(events?.onframe || (() => {}))(evt)";
      }

      WARN("Unknown event type to call.");
      return "(() => {})()";
    }
  };

private: // private members
  JSRuntime* m_rt;
  JSContext* m_ctx;
  EntityRaw m_ent;

private: // private static methods
  static JSValue js_log(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv)
  {
    ASSERT(ctx);
    ASSERT(argc >= 0);
    ASSERT(argv);

    for (int i = 0; i < argc; i++)
    {
      auto str = JS_ToCString(ctx, argv[i]);
      printf("%s%c", str, (i + 1 < argc) ? ' ' : '\n');
      JS_FreeCString(ctx, str);
    }
    return JS_UNDEFINED;
  }

  static JSValue wrap_json(JSContext* ctx, const json& val)
  {
    ASSERT(ctx);

    auto valStr = val.dump();
    return JS_ParseJSON(ctx, valStr.data(), valStr.size(), nullptr);
  }

  static json unwrap_json(JSContext* ctx, JSValueConst val)
  {
    auto ptr = JS_ToCString(ctx, val);
    auto j = json::parse(ptr);
    JS_FreeCString(ctx, ptr);
    return j;
  }

  template<typename EventType>
  static JSValue wrap_event(JSContext* ctx, const EventType& val)
  {
    ASSERT(ctx);

    if constexpr (std::is_same_v<EventType, MouseMove>)
    {
      auto evt = JS_NewObject(ctx);
      JS_SetPropertyStr(ctx, evt, "modKey", JS_NewUint32(ctx, val.modKey));
      if (auto src = val.source)
      {
        JS_SetPropertyStr(ctx, evt, "x", JS_NewFloat64(ctx, src->x));
        JS_SetPropertyStr(ctx, evt, "y", JS_NewFloat64(ctx, src->y));
      }
      JS_SetPropertyStr(ctx, evt, "dx", JS_NewFloat64(ctx, val.dx));
      JS_SetPropertyStr(ctx, evt, "dy", JS_NewFloat64(ctx, val.dy));
      return evt;
    }
    else if constexpr (std::is_same_v<EventType, MouseClick>)
    {
      auto evt = JS_NewObject(ctx);
      JS_SetPropertyStr(ctx, evt, "modKey", JS_NewUint32(ctx, val.modKey));
      JS_SetPropertyStr(ctx, evt, "nClicks", JS_NewUint32(ctx, val.nClicks));
      JS_SetPropertyStr(ctx, evt, "button", JS_NewUint32(ctx, val.button));
      if (auto src = val.source)
      {
        JS_SetPropertyStr(ctx, evt, "x", JS_NewFloat64(ctx, src->x));
        JS_SetPropertyStr(ctx, evt, "y", JS_NewFloat64(ctx, src->y));
      }
      return evt;
    }
    else if constexpr (std::is_same_v<EventType, MouseRelease>)
    {
      auto evt = JS_NewObject(ctx);
      JS_SetPropertyStr(ctx, evt, "button", JS_NewUint32(ctx, val.button));
      if (auto src = val.source)
      {
        JS_SetPropertyStr(ctx, evt, "x", JS_NewFloat64(ctx, src->x));
        JS_SetPropertyStr(ctx, evt, "y", JS_NewFloat64(ctx, src->y));
      }
      return evt;
    }
    else if constexpr (std::is_same_v<EventType, KeyboardPress>)
    {
      auto evt = JS_NewObject(ctx);
      JS_SetPropertyStr(ctx, evt, "modKey", JS_NewUint32(ctx, val.modKey));
      JS_SetPropertyStr(ctx, evt, "key", JS_NewInt32(ctx, val.key));
      return evt;
    }
    else if constexpr (std::is_same_v<EventType, KeyboardRelease>)
    {
      auto evt = JS_NewObject(ctx);
      JS_SetPropertyStr(ctx, evt, "modKey", JS_NewUint32(ctx, val.modKey));
      JS_SetPropertyStr(ctx, evt, "key", JS_NewInt32(ctx, val.key));
      return evt;
    }
    else if constexpr (std::is_same_v<EventType, KeyboardText>)
    {
      auto evt = JS_NewObject(ctx);
      JS_SetPropertyStr(ctx, evt, "text", JS_NewStringLen(ctx, val.text.c_str(), val.text.size()));
      return evt;
    }
    return JS_UNDEFINED;
  }

private: // private static methods
  inline bool isValTrue(const JSValue& val)
  {
    ASSERT(m_ctx);

    if (JS_IsException(val))
    {
      auto e = JS_GetException(m_ctx);

      if (auto str = JS_ToCString(m_ctx, e))
      {
        FAIL("%s", str);
        JS_FreeCString(m_ctx, str);
      }

      if (JS_IsError(m_ctx, e))
      {
        if (auto s = JS_GetPropertyStr(m_ctx, e, "stack"); !JS_IsUndefined(s))
        {
          if (auto str = JS_ToCString(m_ctx, s))
          {
            FAIL("%s", str);
            JS_FreeCString(m_ctx, str);
          }
          JS_FreeValue(m_ctx, s);
        }
      }

      JS_FreeValue(m_ctx, e);

      return false;
    }
    else if (JS_IsBool(val))
    {
      auto isFalse = (JS_VALUE_GET_BOOL(val) == 0);
      return !isFalse;
    }
    else if (JS_IsNumber(val))
    {
      auto isZero = (JS_VALUE_GET_INT(val) == 0) || (JS_VALUE_GET_FLOAT64(val) == 0.0);
      auto isNaN = JS_VALUE_IS_NAN(val);
      return !isZero && !isNaN;
    }
    else if (JS_IsString(val))
    {
      size_t len;
      auto str = JS_ToCStringLen(m_ctx, &len, val);
      auto isEmpty = (len == 0);
      JS_FreeCString(m_ctx, str);
      return !isEmpty;
    }
    else if (JS_IsUndefined(val) || JS_IsNull(val) || JS_IsUninitialized(val))
    {
      return false;
    }
    return true;
  }

  void setupPresets()
  {
    ASSERT(m_ctx);
    JSValue globalObj = JS_GetGlobalObject(m_ctx);

    // add logging utils
    JSValue console = JS_NewObject(m_ctx);
    JS_SetPropertyStr(m_ctx, console, "log", JS_NewCFunction(m_ctx, js_log, "log", 1));
    JS_SetPropertyStr(m_ctx, globalObj, "console", console);

    // add empty events object
    JS_SetPropertyStr(m_ctx, globalObj, "events", JS_NewObject(m_ctx));

    // add entity object
    if (m_ent)
    {
      json entJson = *m_ent;
      JS_SetPropertyStr(m_ctx, globalObj, "entity", wrap_json(m_ctx, entJson));
    }

    JS_FreeValue(m_ctx, globalObj);
  }

public: // public methods
  JavaScriptInterpreter(EntityRaw ent)
    : m_rt(JS_NewRuntime())
    , m_ctx(JS_NewContext(m_rt))
    , m_ent(ent)
  {
    setupPresets();
  }

  ~JavaScriptInterpreter()
  {
    JS_FreeContext(m_ctx);
    JS_FreeRuntime(m_rt);
  }

  bool eval(const std::string& program)
  {
    auto val = JS_Eval(m_ctx, program.c_str(), program.size(), "inline", JS_EVAL_TYPE_GLOBAL);
    auto res = isValTrue(val);
    JS_FreeValue(m_ctx, val);
    return res;
  }

  template<typename EventType>
  bool callOnEvent(const EventType& evt)
  {
    ASSERT(m_ctx);
    JSValue globalObj = JS_GetGlobalObject(m_ctx);
    JS_SetPropertyStr(m_ctx, globalObj, "evt", wrap_event(m_ctx, evt));
    JS_FreeValue(m_ctx, globalObj);

    auto program = EventCallProgram<EventType>::get();
    auto val = JS_Eval(m_ctx, program.c_str(), program.size(), "inline", JS_EVAL_TYPE_GLOBAL);
    auto res = isValTrue(val);
    JS_FreeValue(m_ctx, val);
    return res;
  }

  std::optional<json> getEntity()
  {
    JSValue globalObj = JS_GetGlobalObject(m_ctx);
    auto entObj = JS_GetPropertyStr(m_ctx, globalObj, "entity");
    if (!isValTrue(entObj))
    {
      return std::nullopt;
    }

    auto entStr = JS_JSONStringify(m_ctx, entObj, JS_UNDEFINED, JS_NewInt32(m_ctx, 0));
    // TODO optimize unwrap_json performance
    auto res =
      isValTrue(entStr) ? std::make_optional<json>(unwrap_json(m_ctx, entStr)) : std::nullopt;

    JS_FreeValue(m_ctx, entStr);
    JS_FreeValue(m_ctx, entObj);
    JS_FreeValue(m_ctx, globalObj);

    return res;
  }
};

}; // namespace VGG

#endif // __JAVASCRIPT_INTERPRETER_HPP__
