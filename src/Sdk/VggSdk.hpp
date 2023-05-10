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

  //   // code
  //   void addCode(const std::string& json_pointer, const std::string& value);
  //   void updateCode(const std::string& json_pointer, const std::string& value);
  //   void deleteCodeAt(const std::string& json_pointer);
  //   const std::string codeAt(const std::string& json_pointer);

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