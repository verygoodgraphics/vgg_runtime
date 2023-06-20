#include "Core/TextNode.h"

namespace VGG
{
class TextNode__pImpl
{
  VGG_DECL_API(TextNode)
public:
  TextNode__pImpl(TextNode* api)
    : q_ptr(api)
  {
  }
  std::string text;
  ETextLayoutMode mode;
  std::vector<TextStyleStub> styles;
};
} // namespace VGG
