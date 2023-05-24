#ifndef VGG_SDK_HPP
#define VGG_SDK_HPP

#include <string>
#include <memory>

class JsonDocument;

class VggSdk
{
public:
  virtual ~VggSdk() = default;

  // design document
  const std::string designDocument();
  void designDocumentAddAt(const std::string& json_pointer, const std::string& value);
  void designDocumentReplaceAt(const std::string& json_pointer, const std::string& value);
  void designDocumentDeleteAt(const std::string& json_pointer);

  // code
  // event types: https://developer.mozilla.org/en-US/docs/Web/API/Element#events
  void addEventListener(const std::string& json_pointer,
                        const std::string& type,
                        const std::string& code);
  void removeEventListener(const std::string& json_pointer,
                           const std::string& type,
                           const std::string& code);
  const std::vector<std::string> getEventListeners(const std::string& json_pointer,
                                                   const std::string& type);

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
  std::shared_ptr<JsonDocument>& getDesignDocument();
};

#endif