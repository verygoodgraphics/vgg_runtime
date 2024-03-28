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

#ifndef VGG_RUNTIME_DLL_DECLARE

#ifdef _MSC_VER
#ifdef VGG_RUNTIME_DLL_EXPORTING
#define VGG_RUNTIME_DLL_DECLARE __declspec(dllexport)
#else
#define VGG_RUNTIME_DLL_DECLARE __declspec(dllimport)
#endif
#else
#define VGG_RUNTIME_DLL_DECLARE
#endif

#endif