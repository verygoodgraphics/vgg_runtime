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

#include <cstddef>
#include <functional>
#include <memory>
#include <string>
#include <vector>
#include "Application/AppRenderable.hpp"
#include "Application/UIEvent.hpp"
#include "Application/UIOptions.hpp"
#include "Domain/Daruma.hpp"
#include "Domain/Layout/LayoutContext.hpp"
#include "Domain/Layout/Rect.hpp"

class JsonDocument;
namespace VGG
{
class Editor;
class IVggEnv;
class Presenter;
class Reporter;
class RunLoop;
class VggExec;
namespace Layout
{
class ExpandSymbol;
class Layout;
} // namespace Layout
struct ViewModel;
} // namespace VGG
struct VTouchEvent;

namespace VGG
{

class Controller : public std::enable_shared_from_this<Controller>
{
public:
  enum class ERunMode
  {
    NORMAL_MODE,
    EDIT_MODE
  };

  using EventListener = std::function<void(UIEventPtr event)>;

private:
  std::weak_ptr<IVggEnv> m_env;

  std::string                m_designSchemaFilePath;
  std::shared_ptr<RunLoop>   m_runLoop;
  std::shared_ptr<VggExec>   m_jsEngine;
  std::shared_ptr<Presenter> m_presenter;
  std::shared_ptr<Editor>    m_editor;
  std::shared_ptr<Reporter>  m_reporter;
  bool                       m_panning{ false };

  // configuration
  bool     m_isFitToViewportEnabled{ true };
  ERunMode m_mode;

  std::shared_ptr<Daruma> m_model;
  std::shared_ptr<Daruma> m_editModel;

  std::shared_ptr<Layout::Layout>       m_layout;
  std::shared_ptr<Layout::ExpandSymbol> m_expander;

  EventListener m_listener;

public:
  Controller(
    std::weak_ptr<IVggEnv>     env,
    std::shared_ptr<RunLoop>   runLoop,
    std::shared_ptr<VggExec>   jsEngine,
    std::shared_ptr<Presenter> presenter,
    std::shared_ptr<Editor>    editor,
    ERunMode                   mode = ERunMode::NORMAL_MODE);
  ~Controller() = default;

public:                                       // configure
  void setFitToViewportEnabled(bool enabled); // should be called before loading vgg file
  void setEditMode(bool editMode);            // can be called before or after loading vgg file
  void setContentMode(
    const std::string& contentMode); // can be called before or after loading vgg file

public:
  bool start(
    const std::string& filePath,
    const char*        designDocSchemaFilePath = nullptr,
    const char*        layoutDocSchemaFilePath = nullptr);
  bool start(
    std::vector<char>& buffer,
    const char*        designDocSchemaFilePath = nullptr,
    const char*        layoutDocSchemaFilePath = nullptr);

  bool edit(const std::string& filePath);
  bool edit(std::vector<char>& buffer);

  void onResize();
  void postFrame();

  std::shared_ptr<app::AppRenderable> editor();
  bool                                isEditMode()
  {
    return m_mode == ERunMode::EDIT_MODE;
  }
  bool isNormalMode()
  {
    return m_mode == ERunMode::NORMAL_MODE;
  }

  void updateDisplayContentIfNeeded();
  bool hasDirtyEditor();
  void onFirstRender();

  void setEventListener(EventListener listener);
  void listenAllEvents(bool enabled);

  bool handleMouseWheel(float x, float y);
  bool handleTouchEvent(const VTouchEvent& evt);

public:
  const std::string getFramesInfo() const;

  std::string currentFrameId() const;

  bool pushFrame(const std::string& id, const app::FrameOptions& opts);
  bool popFrame(const app::PopOptions& opts);
  bool presentFrame(const std::string& id, const app::FrameOptions& opts);
  bool dismissFrame();
  bool nextFrame();
  bool previouseFrame();

  bool setCurrentTheme(const std::string& theme);

  bool setState(
    const std::string&       instanceDescendantId,
    const std::string&       listenerId,
    const std::string&       stateMasterId,
    const app::StateOptions& options);
  bool presentState(
    const std::string&       instanceDescendantId,
    const std::string&       listenerId,
    const std::string&       stateMasterId,
    const app::StateOptions& options);
  bool dismissState(const std::string& instanceDescendantId);

  void openUrl(const std::string& url, const std::string& target);

public:
  bool updateElementFillColor(
    const std::string& id,
    const std::size_t  fillIndex,
    const double       r,
    const double       g,
    const double       b,
    const double       a);

private:
  void          initModel(const char* designDocSchemaFilePath, const char* layoutDocSchemaFilePath);
  JsonDocument* createJsonDoc();
  std::shared_ptr<JsonDocument>   wrapJsonDoc(std::shared_ptr<JsonDocument> jsonDoc);
  const std::shared_ptr<VggExec>& vggExec();

  void start();
  void observeModelState();
  void observeViewEvent();
  void handleEvent(UIEventPtr evt);

  bool              hasContent() const;
  VGG::Layout::Size currentPageSize() const;
  VGG::Layout::Size currentPageOriginalSize() const;
  VGG::Layout::Size pageOriginalSize(std::size_t pageIndex) const;

  void startEditing();
  void observeEditModelState();
  void observeEditViewEvent();

  void scaleContent();

  void layoutForEditing(std::size_t pageIndex);
  void scaleContent(std::size_t pageIndex);
  void aspectFill(Layout::Size size, std::size_t pageIndex);

  std::shared_ptr<ViewModel> generateViewModel(std::shared_ptr<Daruma> model, Layout::Size size);
  MakeJsonDocFn              createMakeJsonDocFn(const char* pJsonSchemaFilePath);

  void scaleContentAndUpdate(std::size_t pageIndex);
  void fitPage();
  void scaleContentUpdateViewModelAndFit();

  LayoutContext* layoutContext();

private:
  std::unique_ptr<LayoutContext> m_layoutContext;
};

} // namespace VGG
