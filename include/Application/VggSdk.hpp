/*
 * Copyright 2023-2024 VeryGoodGraphics LTD <bd@verygoodgraphics.com>
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

#ifdef EMSCRIPTEN
#include <emscripten/val.h>
#endif

#include "ISdk.hpp"

#include <cstddef>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

class JsonDocument;
namespace VGG
{
class Daruma;
class VggEnv;
class Controller;

constexpr int main_or_editor_daruma_index = 0;
constexpr int edited_daruma_index = 1;

class VggSdk : public ISdk
{
  std::weak_ptr<VggEnv> m_env;

public:
#ifdef EMSCRIPTEN
  using ListenersType = emscripten::val;
#else
  using ListenersType = std::unordered_map<std::string, std::vector<std::string>>;
#endif
  using IndexType = std::size_t;

  // env
  void        setEnv(const std::string& key);
  std::string getEnv();
  void        setContainerKey(const std::string& containerKey);
  void        setInstanceKey(const std::string& instanceKey);
  void        setListenerKey(const std::string& listenerKey);

  // design document
  std::string designDocument() override;
  std::string designDocumentValueAt(const std::string& jsonPointer) override;

  std::string getElement(const std::string& id) override;
  void        updateElement(const std::string& id, const std::string& contentJsonString) override;

  std::string getFramesInfo() const override;
  std::string currentFrameId() const override;

  // configure
  const std::string launchFrameId() const override;
  bool              setLaunchFrameById(const std::string& id) override;
  const std::string currentTheme() const override;
  bool              setCurrentTheme(const std::string& theme) override;

  // frame
  bool setCurrentFrameById(const std::string& id, bool resetScrollPosition) override;
  bool presentFrameById(const std::string& id, bool resetScrollPosition) override;
  bool dismissFrame() override;
  bool goBack(bool resetScrollPosition, bool resetState) override;
  bool nextFrame() override;
  bool previousFrame() override;

  // instance
  bool setState(
    const std::string& instanceDescendantId,
    const std::string& listenerId,
    const std::string& masterId,
    bool               resetScrollPosition) override;
  bool presentState(
    const std::string& instanceDescendantId,
    const std::string& listenerId,
    const std::string& stateMasterId,
    bool               resetScrollPosition) override;
  bool dismissState(const std::string& instanceDescendantId) override;

  // misc
  void        setFitToViewportEnabled(bool enabled) override;
  std::string requiredFonts() const override;
  bool        addFont(const uint8_t* data, size_t size, const char* defaultName) override;
#ifdef EMSCRIPTEN
  bool jsAddFont(const emscripten::val& jsFontUint8Array, const std::string& defaultName);
#endif

  std::vector<uint8_t> vggFileBuffer() override;
#ifdef EMSCRIPTEN
  emscripten::val vggFileUint8Array();
#endif
  std::vector<std::string> texts() override;

  std::vector<uint8_t> makeImageSnapshot(const ImageOptions& options) override;
#ifdef EMSCRIPTEN
  emscripten::val emMakeImageSnapshot(const ImageOptions& options);
#endif

  void openUrl(const std::string& url, const std::string& target) override;

  // event listener
  // event types: https://developer.mozilla.org/en-US/docs/Web/API/Element#events
  void addEventListener(
    const std::string& elementKey,
    const std::string& eventType,
    const std::string& listenerCode);
  void removeEventListener(
    const std::string& elementKey,
    const std::string& eventType,
    const std::string& listenerCode);
  ListenersType getEventListeners(const std::string& elementKey);

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
  std::shared_ptr<JsonDocument> getDesignDocument(
    IndexType index = main_or_editor_daruma_index) const;
  std::shared_ptr<VGG::Daruma> getModel(IndexType index = main_or_editor_daruma_index) const;

  std::shared_ptr<VGG::VggEnv>     env() const;
  std::shared_ptr<VGG::Controller> controller();
};

} // namespace VGG