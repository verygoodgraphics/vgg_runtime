#include "Controller.hpp"

#include "Editor.hpp"

#include "Usecase/EditModel.hpp"

#include "Domain/VggExec.hpp"
#include "Domain/RawJsonDocument.hpp"
#include "Domain/SchemaValidJsonDocument.hpp"
// #include "Domain/UndoRedoJsonDocument.hpp"
#include "Domain/Daruma.hpp"
#include "Domain/DarumaContainer.hpp"
#include "Presenter.hpp"
#include "DIContainer.hpp"
#include "Log.h"
#include "Usecase/ModelChanged.hpp"
#include "Usecase/ResizeWindow.hpp"
#include "Usecase/StartRunning.hpp"

#include <cassert>

constexpr auto PSEUDO_PATH_EDIT_VIEW = "::editView";

namespace VGG
{

Controller::Controller(std::shared_ptr<RunLoop> runLoop,
                       std::shared_ptr<Presenter> presenter,
                       ERunMode mode)
  : m_runLoop(runLoop)
  , m_presenter(presenter)
  , m_mode(mode)
  , m_editor{ new Editor }
{
  assert(m_runLoop);
}

bool Controller::start(const std::string& filePath,
                       const char* designDocSchemaFilePath,
                       const char* layoutDocSchemaFilePath)
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

bool Controller::start(std::vector<char>& buffer,
                       const char* designDocSchemaFilePath,
                       const char* layoutDocSchemaFilePath)
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
  auto darumaToEdit = editModel.open(filePath);
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
  auto darumaToEdit = editModel.open(buffer);
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
  // ResizeWindow{ m_layout }.onResize(m_presenter->viewSize());
  if (m_editModel)
  {
    // todo, edited model
    // ResizeWindow{m_edit_layout}.onResize(m_presenter->editViewSize());
  }
}

void Controller::setEditMode(bool editMode)
{
  if (editMode)
  {
    if (m_mode == ERunMode::EDIT_MODE)
    {
      return;
    }

    m_mode = ERunMode::EDIT_MODE;
  }
  else
  {
    if (m_mode == ERunMode::NORMAL_MODE)
    {
      return;
    }

    m_mode = ERunMode::NORMAL_MODE;
  }

  m_presenter->setEditMode(editMode);
  m_editor->enable(editMode);
}

std::shared_ptr<app::AppScene> Controller::editor()
{
  return m_editor;
}

void Controller::initModel(const char* designDocSchemaFilePath, const char* layoutDocSchemaFilePath)
{
  if (designDocSchemaFilePath)
  {
    m_designSchemaFilePath.append(designDocSchemaFilePath);
  }

  m_model.reset(new Daruma(createMakeJsonDocFn(designDocSchemaFilePath),
                           createMakeJsonDocFn(layoutDocSchemaFilePath)));

  DarumaContainer().add(m_model);
}

void Controller::start()
{
  m_presenter->setModel(generateViewModel(m_model, m_presenter->viewSize()));

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
      auto sharedThis = weakThis.lock();
      if (!sharedThis)
      {
        return;
      }

      if (sharedThis->m_mode == ERunMode::EDIT_MODE)
      {
        if (sharedThis->m_editor)
        {
          sharedThis->m_editor->handleUIEvent(evt);
        }
        return;
      }

      auto listenersMap = sharedThis->m_model->getEventListeners(evt->path());
      std::string type = evt->type();
      if (auto it = listenersMap.find(type); it != listenersMap.end())
      {
        for (auto& listener : it->second)
        {
          // todo, evt phase // kCapturingPhase = 1, // kAtTarget = 2, // kBubblingPhase = 3
          // todo, evt PropagationStopped
          sharedThis->vggExec()->evalModule(listener, evt);
        }
      }
    });

  m_presenter->getObservable().subscribe(observer);
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

      auto listenersMap = sharedThis->m_model->getEventListeners(PSEUDO_PATH_EDIT_VIEW);
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
  return VGG::DIContainer<std::shared_ptr<VggExec>>::get();
}

JsonDocument* Controller::createJsonDoc()
{
  if (m_mode == ERunMode::NORMAL_MODE)
  {
    return new RawJsonDocument();
  }
  else
  {
    return nullptr; // new UndoRedoJsonDocument();
  }
}

JsonDocumentPtr Controller::wrapJsonDoc(std::shared_ptr<JsonDocument> jsonDoc)
{
  if (m_mode == ERunMode::NORMAL_MODE)
  {
    return jsonDoc;
  }
  else
  {
    // todo, wrap with remote doc which can save to server
    return jsonDoc;
  }
}

std::shared_ptr<ViewModel> Controller::generateViewModel(std::shared_ptr<Daruma> model,
                                                         Layout::Size size)
{
  StartRunning startRunning{ model };
  m_layout = startRunning.layout();

  // startRunning.layout(size);

  auto viewModel = std::make_shared<ViewModel>();
  viewModel->model = m_model;
  viewModel->designDoc = m_model->runtimeDesignDoc()->content();
  viewModel->layoutTree = startRunning.layoutTree();

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

    if (!jsonSchemaFilePath.empty())
    {
      SchemaValidJsonDocument::ValidatorPtr validator;
      std::ifstream schemaIfs(jsonSchemaFilePath);
      json schema = json::parse(schemaIfs);
      validator.reset(new JsonSchemaValidator);
      validator->setRootSchema(schema);

      // todo, validate doc

      jsonDocPtr = new SchemaValidJsonDocument(JsonDocumentPtr(jsonDocPtr), validator);
    }

    return wrapJsonDoc(JsonDocumentPtr(jsonDocPtr));
  };

  return lambda;
}

} // namespace VGG
