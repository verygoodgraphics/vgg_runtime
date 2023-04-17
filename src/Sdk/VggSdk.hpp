#ifndef VGG_SDK_HPP
#define VGG_SDK_HPP

#include <string>

class JsonDocument;

class VggSdk
{
public:
  virtual ~VggSdk() = default;

  virtual void updateStyle();

  // IDE api
  // document
  const std::string documentJson();

  //   // path(json pointer)
  //   const std::string getElementPath(const std::string& content);
  //   const std::string getElementContainerPath(const std::string& content);

  // low level api
  void addToDocument(const std::string& json_pointer, const std::string& value);
  void replaceInDocument(const std::string& json_pointer, const std::string& value);
  void deleteFromDocument(const std::string& json_pointer);
  const std::string jsonAt(const std::string& json_pointer);

  //   // -- element
  //   virtual const std::string findElement(const std::string& content) = 0;

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

  //   // void updateTextColor(char *id, char *color);

  //   // char *getInputText(char *id);

private:
  std::shared_ptr<JsonDocument>& designDocument();
  // std::string document_json_;
};

#endif