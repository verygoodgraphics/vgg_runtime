#pragma once

#include "Daruma.hpp"

#ifdef EMSCRIPTEN
#include <emscripten/val.h>
#endif

#include <string>
#include <memory>

class JsonDocument;

class VggSdk
{
public:
#ifdef EMSCRIPTEN
  using ListenersType = emscripten::val;
#else
  using ListenersType = VGG::Daruma::ListenersType;
#endif

  virtual ~VggSdk() = default;

  // design document
  const std::string designDocument();
  void designDocumentAddAt(const std::string& json_pointer, const std::string& value);
  void designDocumentReplaceAt(const std::string& json_pointer, const std::string& value);
  void designDocumentDeleteAt(const std::string& json_pointer);

  // event listener
  // event types: https://developer.mozilla.org/en-US/docs/Web/API/Element#events
  void addEventListener(const std::string& element_path,
                        const std::string& event_type,
                        const std::string& listener_code);
  void removeEventListener(const std::string& element_path,
                           const std::string& event_type,
                           const std::string& listener_code);
  ListenersType getEventListeners(const std::string& element_path);

  //   // ---
  //   void undo();
  //   void redo();

  //   // Production api

  //   // IDE & Production api

  //   // void openUrl(char *url);

  //   // void showView(char *id);
  //   // void hideView(char *id);

  //   // char *getInputText(char *id);

private:
  std::shared_ptr<JsonDocument> getDesignDocument();
  std::shared_ptr<VGG::Daruma> getModel();
};
