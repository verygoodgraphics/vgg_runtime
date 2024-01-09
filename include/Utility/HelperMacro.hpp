/*
 * Copyright 2023 VeryGoodGraphics LTD <bd@verygoodgraphics.com>
 *
 * Licensed under the VGG License, Version 1.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://www.verygoodgraphics.com/licenses/LICENSE-1.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#pragma once
// NOLINTBEGIN
template<typename T>
static inline T* GetImplPtrHelper(T* ptr)
{
  return ptr;
}
template<typename Wrapper>
static inline typename Wrapper::pointer GetImplPtrHelper(const Wrapper& p)
{
  return p.data();
}
// NOLINTEND

#define VGG_DECL_IMPL(Class)                                                                       \
  inline Class##__pImpl* d_func()                                                                  \
  {                                                                                                \
    return reinterpret_cast<Class##__pImpl*>(GetImplPtrHelper(d_ptr.get()));                       \
  }                                                                                                \
  inline const Class##__pImpl* d_func() const                                                      \
  {                                                                                                \
    return reinterpret_cast<const Class##__pImpl*>(GetImplPtrHelper(d_ptr.get()));                 \
  }                                                                                                \
  friend class Class##__pImpl;                                                                     \
  std::unique_ptr<Class##__pImpl> const d_ptr;

#define VGG_DECL_IMPL_REF(Class)                                                                   \
  inline Class##__pImpl* d_func()                                                                  \
  {                                                                                                \
    return reinterpret_cast<Class##__pImpl*>(GetImplPtrHelper(d_ptr.get()));                       \
  }                                                                                                \
  inline const Class##__pImpl* d_func() const                                                      \
  {                                                                                                \
    return reinterpret_cast<const Class##__pImpl*>(GetImplPtrHelper(d_ptr.get()));                 \
  }                                                                                                \
  friend class Class##__pImpl;                                                                     \
  VGG::layer::Ref<Class##__pImpl> const d_ptr;

#define VGG_DECL_API(Class)                                                                        \
  inline Class* q_func()                                                                           \
  {                                                                                                \
    return static_cast<Class*>(q_ptr);                                                             \
  }                                                                                                \
  inline const Class* q_func() const                                                               \
  {                                                                                                \
    return static_cast<const Class*>(q_ptr);                                                       \
  }                                                                                                \
  friend class Class;                                                                              \
  Class* const q_ptr = nullptr;

#define VGG_IMPL(Class) Class##__pImpl* const _ = d_func();
#define VGG_API (Class) Class* const _ = q_func();

#define VGG_CALL __cdecl

#define VGG_EXPORTS_C_BEGIN                                                                        \
  extern "C"                                                                                       \
  {
#define VGG_EXPORTS_C_END }
#define VGG_EXPORTS_C(EXPR_OR_FUNC) extern "C" EXPR_OR_FUNC

#define VGG_INLINE_DECL inline
