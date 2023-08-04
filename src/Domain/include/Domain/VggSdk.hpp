#pragma once

#include "Daruma.hpp"

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

  // design document
  const std::string designDocument(IndexType index = main_or_editor_daruma_index);
  void designDocumentAddAt(const std::string& json_pointer,
                           const std::string& value,
                           IndexType index = main_or_editor_daruma_index);
  void designDocumentReplaceAt(const std::string& json_pointer,
                               const std::string& value,
                               IndexType index = main_or_editor_daruma_index);
  void designDocumentDeleteAt(const std::string& json_pointer,
                              IndexType index = main_or_editor_daruma_index);

  // event listener
  // event types: https://developer.mozilla.org/en-US/docs/Web/API/Element#events
  void addEventListener(const std::string& element_path,
                        const std::string& event_type,
                        const std::string& listener_code,
                        IndexType index = main_or_editor_daruma_index);
  void removeEventListener(const std::string& element_path,
                           const std::string& event_type,
                           const std::string& listener_code,
                           IndexType index = main_or_editor_daruma_index);
  ListenersType getEventListeners(const std::string& element_path,
                                  IndexType index = main_or_editor_daruma_index);

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
  std::shared_ptr<VGG::Daruma> getModel(IndexType index = main_or_editor_daruma_index);
};
