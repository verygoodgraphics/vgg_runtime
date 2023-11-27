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

#include "Domain/Daruma.hpp"

#ifdef EMSCRIPTEN
#include <emscripten/val.h>
#endif

#include <cstddef>
#include <string>
#include <memory>

class JsonDocument;

constexpr int main_or_editor_daruma_index = 0;
constexpr int edited_daruma_index = 1;

class VggSdk
{
public:
#ifdef EMSCRIPTEN
  using ListenersType = emscripten::val;
#else
  using ListenersType = VGG::Daruma::ListenersType;
#endif
  using IndexType = std::size_t;

  virtual ~VggSdk() = default;

  // env
  std::string getEnvKey();
  void        setContainerKey(const std::string& containerKey);
  void        setInstanceKey(const std::string& instanceKey);
  void        setListenerKey(const std::string& listenerKey);

  // design document
  const std::string designDocument();
  void              designDocumentAddAt(const std::string& jsonPointer, const std::string& value);
  void designDocumentReplaceAt(const std::string& jsonPointer, const std::string& value);
  void designDocumentDeleteAt(const std::string& jsonPointer);

  // event listener
  // event types: https://developer.mozilla.org/en-US/docs/Web/API/Element#events
  void addEventListener(
    const std::string& elementPath,
    const std::string& eventType,
    const std::string& listenerCode);
  void removeEventListener(
    const std::string& elementPath,
    const std::string& eventType,
    const std::string& listenerCode);
  ListenersType getEventListeners(const std::string& elementPath);

  // editor
  //   void undo();
  //   void redo();
  void save();

  //   // Production api

  //   // IDE & Production api

  //   // void openUrl(char *url);

  //   // void showView(char *id);
  //   // void hideView(char *id);

  //   // char *getInputText(char *id);

private:
  std::shared_ptr<JsonDocument> getDesignDocument(IndexType index = main_or_editor_daruma_index);
  std::shared_ptr<VGG::Daruma>  getModel(IndexType index = main_or_editor_daruma_index);
};
