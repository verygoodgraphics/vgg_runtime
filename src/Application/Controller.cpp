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
#include "Controller.hpp"

#include "Editor.hpp"
#include "Presenter.hpp"
#include "Reporter.hpp"
#include "RunLoop.hpp"

#include "Domain/Daruma.hpp"
#include "Domain/DarumaContainer.hpp"
#include "Domain/RawJsonDocument.hpp"
#include "Domain/SchemaValidJsonDocument.hpp"
#include "Domain/UndoRedoJsonDocument.hpp"
#include "Domain/VggExec.hpp"
#include "Utility/Log.hpp"
#include "UseCase/EditModel.hpp"
#include "UseCase/ModelChanged.hpp"
#include "UseCase/ResizeWindow.hpp"
#include "UseCase/StartRunning.hpp"

#include "VGGVersion_generated.h"

#include <cassert>

constexpr auto PSEUDO_PATH_EDIT_VIEW = "::editView";

#define MIN_DOC_VERSION VGG_PARSE_FORMAT_VER_STR

namespace
{
// Method to compare two versions.
// Returns 1 if v2 is smaller, -1
// if v1 is smaller, 0 if equal
int versionCompare(const std::string& v1, const std::string& v2)
{
  // vnum stores each numeric
  // part of version
  int vnum1 = 0, vnum2 = 0;

  // loop until both string are
  // processed
  for (int i = 0, j = 0; (i < v1.length() || j < v2.length());)
  {
    // storing numeric part of
    // version 1 in vnum1
    while (i < v1.length() && v1[i] != '.')
    {
      vnum1 = vnum1 * 10 + (v1[i] - '0');
      i++;
    }

    // storing numeric part of
    // version 2 in vnum2
    while (j < v2.length() && v2[j] != '.')
    {
      vnum2 = vnum2 * 10 + (v2[j] - '0');
      j++;
    }

    if (vnum1 > vnum2)
      return 1;
    if (vnum2 > vnum1)
      return -1;

    // if equal, reset variables and
    // go for next numeric part
    vnum1 = vnum2 = 0;
    i++;
    j++;
  }
  return 0;
}

} // namespace

namespace VGG
{

Controller::Controller(
  std::shared_ptr<RunLoop>   runLoop,
  std::shared_ptr<VggExec>   jsEngine,
  std::shared_ptr<Presenter> presenter,
  std::shared_ptr<Editor>    editor,
  ERunMode                   mode)
  : m_runLoop(runLoop)
  , m_jsEngine{ jsEngine }
  , m_presenter(presenter)
  , m_editor{ editor }
  , m_mode(mode)
  , m_reporter{ new Reporter{ jsEngine } }
{
  assert(m_runLoop);

  if (m_editor)
  {
    m_editor->setListener(m_reporter);
  }
}

bool Controller::start(
  const std::string& filePath,
  const char*        designDocSchemaFilePath,
  const char*        layoutDocSchemaFilePath)
{
  initModel(designDocSchemaFilePath, layoutDocSchemaFilePath);
  auto ret = m_model->load(filePath);
  if (ret)
  {
    start();
  }
  else
  {
    FAIL("#controller, load file failed, %s", filePath.c_str());
  }
  return ret;
}

bool Controller::start(
  std::vector<char>& buffer,
  const char*        designDocSchemaFilePath,
  const char*        layoutDocSchemaFilePath)
{
  initModel(designDocSchemaFilePath, layoutDocSchemaFilePath);
  auto ret = m_model->load(buffer);
  if (ret)
  {
    start();
  }
  else
  {
    FAIL("#controller, load buffer failed");
  }
  return ret;
}

bool Controller::edit(const std::string& filePath)
{
  EditModel editModel{ m_designSchemaFilePath };
  auto      darumaToEdit = editModel.open(filePath);
  if (darumaToEdit)
  {
    m_editModel = darumaToEdit;
    startEditing();
    return true;
  }
  else
  {
    return false;
  }
}

bool Controller::edit(std::vector<char>& buffer)
{
  EditModel editModel{ m_designSchemaFilePath };
  auto      darumaToEdit = editModel.open(buffer);
  if (darumaToEdit)
  {
    m_editModel = darumaToEdit;
    startEditing();
    return true;
  }
  else
  {
    return false;
  }
}

void Controller::onResize()
{
  if (isNormalMode())
  {
    m_presenter->resetForRunning();
    scaleContent(m_presenter->viewSize());
  }
  else
  {
    fitPageForEditing();
  }

  if (m_editModel)
  {
    // todo, edited model
    // ResizeWindow{m_edit_layout}.onResize(m_presenter->editViewSize());
  }
}

void Controller::setEditMode(bool editMode)
{
  auto newMode = editMode ? ERunMode::EDIT_MODE : ERunMode::NORMAL_MODE;
  if (newMode == m_mode)
  {
    return;
  }

  m_mode = newMode;
  m_presenter->setEditMode(editMode);
  m_editor->enable(editMode);

  if (!m_layout)
  {
    return;
  }

  if (isNormalMode())
  {
    m_presenter->resetForRunning();
    scaleContent(m_presenter->viewSize()); // windows size
  }
  else
  {
    fitPageForEditing();
  }
}

std::shared_ptr<app::AppRenderable> Controller::editor()
{
  return m_editor;
}

void Controller::initModel(const char* designDocSchemaFilePath, const char* layoutDocSchemaFilePath)
{
  if (designDocSchemaFilePath)
  {
    m_designSchemaFilePath.append(designDocSchemaFilePath);
  }

  m_model.reset(new Daruma(
    createMakeJsonDocFn(designDocSchemaFilePath),
    createMakeJsonDocFn(layoutDocSchemaFilePath)));

  DarumaContainer().add(m_model);
}

void Controller::start()
{
  m_presenter->setModel(generateViewModel(m_model, m_presenter->viewSize()));

  const auto& modelFileVersion = m_model->docVersion();
  if (versionCompare(modelFileVersion, MIN_DOC_VERSION) < 0)
  {
    WARN(
      "The loaded file version %s is lower than the minimum required version %s",
      modelFileVersion.c_str(),
      MIN_DOC_VERSION);
  }

  observeModelState();
  observeViewEvent();
}

void Controller::startEditing()
{
  m_presenter->setEditModel(generateViewModel(m_editModel, m_presenter->editViewSize()));

  observeEditModelState();
  observeEditViewEvent();

  DarumaContainer().add(m_editModel, DarumaContainer::KeyType::Edited);
}

void Controller::observeModelState()
{
  auto weakThis = weak_from_this();
  m_model->getObservable()
    .observe_on(m_runLoop->thread())
    .map(
      [weakThis](VGG::ModelEventPtr event)
      {
        auto sharedThis = weakThis.lock();
        if (!sharedThis)
        {
          return VGG::ModelEventPtr{};
        }
        // todo, layout
        // todo, layout thread?
        ModelChanged().onChange(sharedThis->m_model);

        return event;
      })
    .subscribe(m_presenter->getModelObserver());
}

void Controller::observeEditModelState()
{
  auto weakThis = weak_from_this();
  m_editModel->getObservable()
    .observe_on(m_runLoop->thread())
    .map(
      [weakThis](VGG::ModelEventPtr event)
      {
        auto sharedThis = weakThis.lock();
        if (!sharedThis)
        {
          return VGG::ModelEventPtr{};
        }
        // todo, layout
        // todo, layout thread?

        ModelChanged().onChange(sharedThis->m_editModel);

        return event;
      })
    .subscribe(m_presenter->getEditModelObserver());
}

void Controller::observeViewEvent()
{
  auto weakThis = weak_from_this();
  auto observer = rxcpp::make_observer_dynamic<UIEventPtr>(
    [weakThis](UIEventPtr evt)
    {
      if (auto sharedThis = weakThis.lock())
      {
        sharedThis->handleEvent(evt);
      }
    });

  m_presenter->getObservable().subscribe(observer);
  m_presenter->setEditorEventListener(
    [weakThis](UIEventPtr event, std::weak_ptr<LayoutNode> targetNode)
    {
      if (auto sharedThis = weakThis.lock())
      {
        sharedThis->m_editor->handleUIEvent(event, targetNode);
      }
    });
}

void Controller::handleEvent(UIEventPtr evt)
{
  if (isEditMode())
  {
    return;
  }

  auto        listenersMap = m_model->getEventListeners(evt->path());
  std::string type = evt->type();
  if (auto it = listenersMap.find(type); it != listenersMap.end())
  {
    for (auto& listener : it->second)
    {
      // todo, evt phase // kCapturingPhase = 1, // kAtTarget = 2, // kBubblingPhase = 3
      // todo, evt PropagationStopped
      vggExec()->evalModule(listener, evt);
    }
  }

  if (m_listener)
  {
    m_listener(evt);
  }
}

void Controller::observeEditViewEvent()
{
  auto weakThis = weak_from_this();
  auto observer = rxcpp::make_observer_dynamic<UIEventPtr>(
    [weakThis](UIEventPtr evt)
    {
      auto sharedThis = weakThis.lock();
      if (!sharedThis)
      {
        return;
      }

      auto        listenersMap = sharedThis->m_model->getEventListeners(PSEUDO_PATH_EDIT_VIEW);
      std::string type = evt->type();
      if (auto it = listenersMap.find(type); it != listenersMap.end())
      {
        for (auto& listener : it->second)
        {
          sharedThis->vggExec()->evalModule(listener, evt);
        }
      }
    });

  m_presenter->getEditObservable().subscribe(observer);
}

const std::shared_ptr<VggExec>& Controller::vggExec()
{
  return m_jsEngine;
}

JsonDocument* Controller::createJsonDoc()
{
  if (isNormalMode())
  {
    return new RawJsonDocument();
  }
  else
  {
    // todo: use UndoRedoJsonDocument
    return new RawJsonDocument();
  }
}

JsonDocumentPtr Controller::wrapJsonDoc(std::shared_ptr<JsonDocument> jsonDoc)
{
  if (isNormalMode())
  {
    return jsonDoc;
  }
  else
  {
    // todo, wrap with remote doc which can save to server
    return jsonDoc;
  }
}

std::shared_ptr<ViewModel> Controller::generateViewModel(
  std::shared_ptr<Daruma> model,
  Layout::Size            size)
{
  StartRunning startRunning{ model };
  m_layout = startRunning.layout();

  if (isNormalMode())
  {
    m_presenter->resetForRunning();
    scaleContent(size);
  }
  else
  {
    fitPageForEditing();
  }

  auto viewModel = std::make_shared<ViewModel>();
  viewModel->model = m_model;
  viewModel->layout = m_layout;

  return viewModel;
}

MakeJsonDocFn Controller::createMakeJsonDocFn(const char* pJsonSchemaFilePath)
{
  std::string tmpJsonSchemaFilePath;
  if (pJsonSchemaFilePath)
  {
    tmpJsonSchemaFilePath.append(pJsonSchemaFilePath);
  }

  // `jsonSchemaFilePath` is used in the returned lambda, so copy it.
  auto lambda = [&, jsonSchemaFilePath = tmpJsonSchemaFilePath](const json& designJson)
  {
    auto jsonDocPtr = createJsonDoc();
    jsonDocPtr->setContent(designJson);

    // if (!jsonSchemaFilePath.empty())
    // {
    //   SchemaValidJsonDocument::ValidatorPtr validator;
    //   std::ifstream schemaIfs(jsonSchemaFilePath);
    //   json schema = json::parse(schemaIfs);
    //   validator.reset(new JsonSchemaValidator);
    //   validator->setRootSchema(schema);

    //   // todo, validate doc

    //   jsonDocPtr = new SchemaValidJsonDocument(JsonDocumentPtr(jsonDocPtr), validator);
    // }

    return wrapJsonDoc(JsonDocumentPtr(jsonDocPtr));
  };

  return lambda;
}

bool Controller::hasDirtyEditor()
{
  return isEditMode() && m_editor->isDirty();
}

void Controller::resetEditorDirty()
{
  m_editor->setDirty(false);
}

void Controller::fitPageForEditing()
{
  auto pageSize = m_layout->pageSize(m_presenter->currentPageIndex());
  m_layout->layout(pageSize);
  m_presenter->fitForEditing(pageSize);
}

void Controller::onFirstRender()
{
  ASSERT(m_reporter);
  m_reporter->onFirstRender();
}

void Controller::setEventListener(EventListener listener)
{
  m_listener = listener;
  m_presenter->setListenAllEvents(m_listener ? true : false);
}

void Controller::listenAllEvents(bool enabled)
{
  EventListener listener;
  if (enabled)
  {
    std::weak_ptr<Reporter> weakReporter = m_reporter;
    listener = [weakReporter](UIEventPtr event)
    {
      if (auto reporter = weakReporter.lock())
      {
        reporter->onEvent(event);
      }
    };
  }

  setEventListener(listener);
}

void Controller::scaleContent(Layout::Size size)
{
  if (!m_layout)
  {
    return;
  }

  m_layout->layout(size); // scale to fill
  // todo, aspect fill & scroll
  // aspectFill(size.width);
  // aspectFit(size);
}

void Controller::aspectFill(int width)
{
  auto         pageSize = m_layout->pageSize(m_presenter->currentPageIndex());
  auto         scaleFactor = width / pageSize.width;
  Layout::Size size = { pageSize.width * scaleFactor, pageSize.height * scaleFactor };
  m_layout->layout(size);
}

void Controller::aspectFit(Layout::Size size)
{
  auto pageSize = m_layout->pageSize(m_presenter->currentPageIndex());

  auto xScale = size.width / pageSize.width;
  auto yScale = size.height / pageSize.height;
  auto scale = std::min(xScale, yScale);

  m_layout->layout({ pageSize.width * scale, pageSize.height * scale });
}

} // namespace VGG
