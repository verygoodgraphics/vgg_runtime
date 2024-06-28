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
#include <stdint.h>
#include <cstddef>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include "ISdk.hpp"
class JsonDocument;
namespace VGG
{
class Daruma;
class VggEnv;
class Controller;
class Presenter;
namespace app
{
struct PopOptions;
}
} // namespace VGG

namespace VGG
{
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

  // -
  std::string designDocument() override;
  std::string designDocumentValueAt(const std::string& jsonPointer) override;

  std::string getElement(const std::string& id) override;
  void        updateElement(const std::string& id, const std::string& contentJsonString) override;
  int         updateElementProperties(const std::string& updates, const AnimationOptions& animation)
    override;

  bool setElementFillEnabled(
    const std::string&      id,
    std::size_t             index,
    bool                    enabled,
    const AnimationOptions& animation) override;
  bool setElementFillColor(
    const std::string&      id,
    std::size_t             index,
    float                   a,
    float                   r,
    float                   g,
    float                   b,
    const AnimationOptions& animation) override;
  bool setElementFillOpacity(
    const std::string&      id,
    std::size_t             index,
    float                   opacity,
    const AnimationOptions& animation) override;
  bool setElementFillBlendMode(
    const std::string&      id,
    std::size_t             index,
    int                     mode,
    const AnimationOptions& animation) override;
  bool setElementFillRotation(
    const std::string&      id,
    std::size_t             index,
    float                   degree,
    const AnimationOptions& animation) override;

  bool setElementOpacity(const std::string& id, float opacity, const AnimationOptions& animation)
    override;
  bool setElementVisible(const std::string& id, bool visible, const AnimationOptions& animation)
    override;
  bool setElementMatrix(
    const std::string&      id,
    float                   a,
    float                   b,
    float                   c,
    float                   d,
    float                   tx,
    float                   ty,
    const AnimationOptions& animation) override;
  bool setElementSize(
    const std::string&      id,
    float                   width,
    float                   height,
    const AnimationOptions& animation) override;

  // -
  std::string getFramesInfo() const override;
  std::string currentFrameId() const override;

  // configure
  void              setFitToViewportEnabled(bool enabled) override;
  void              setContentMode(const std::string& newModel) override;
  const std::string launchFrameId() const override;
  bool              setLaunchFrameById(const std::string& id) override;
  const std::string currentTheme() const override;
  bool              setCurrentTheme(const std::string& theme) override;
  void              setBackgroundColor(uint32_t color) override;

  // frame
  bool pushFrame(const std::string& id, const FrameOptions& opts) override;
  bool popFrame(const app::PopOptions& opts) override;
  bool presentFrame(const std::string& id, const FrameOptions& opts) override;
  bool dismissFrame() override;
  bool nextFrame() override;
  bool previousFrame() override;

  bool setCurrentFrameById(const std::string& id, bool resetScrollPosition) override;
  bool setCurrentFrameByIdAnimated(
    const std::string&      id,
    bool                    resetScrollPosition,
    const AnimationOptions& option) override;
  bool presentFrameById(const std::string& id, bool resetScrollPosition) override;
  bool goBack(bool resetScrollPosition, bool resetState) override;

  // instance
  bool setState(
    const std::string&  instanceDescendantId,
    const std::string&  listenerId,
    const std::string&  masterId,
    const StateOptions& options) override;
  bool presentState(
    const std::string&  instanceDescendantId,
    const std::string&  listenerId,
    const std::string&  stateMasterId,
    const StateOptions& options) override;
  bool dismissState(const std::string& instanceDescendantId) override;

  // misc
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
  std::shared_ptr<Daruma> getModel(IndexType index = main_or_editor_daruma_index) const;

  std::shared_ptr<VggEnv>     env() const;
  std::shared_ptr<Controller> controller();
  std::shared_ptr<Presenter>  presenter();
};

} // namespace VGG
