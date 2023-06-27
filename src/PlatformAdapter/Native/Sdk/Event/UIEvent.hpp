#pragma once

#include "Presenter/UIEvent.hpp"

#include "js_native_api.h"

#include <memory>

namespace VGG
{
namespace NodeAdapter
{

class UIEvent
{
  static napi_ref sm_constructor;
  static VGG::EventPtr tmp_event_ptr; // todo, use map

  napi_env m_env;
  napi_ref m_wrapper;
  VGG::EventPtr m_event_ptr;

public:
  static void Init(napi_env env, napi_value exports);
  static void Destructor(napi_env env, void* nativeObject, void* finalize_hint);

  static void store(VGG::EventPtr event)
  {
    tmp_event_ptr = event;
  }

  ~UIEvent();

private:
  static napi_value New(napi_env env, napi_callback_info info);

  static napi_value preventDefault(napi_env env, napi_callback_info info);
  static napi_value stopImmediatePropagation(napi_env env, napi_callback_info info);
  static napi_value stopPropagation(napi_env env, napi_callback_info info);

  static napi_value bindCppEvent(napi_env env, napi_callback_info info);
};

} // namespace NodeAdapter
} // namespace VGG