#include <Core/FontManager.h>
namespace VGG
{

ResourceFontCollection* getDefaultFontCollection()
{
  static sk_sp<ResourceFontCollection> fontCollection =
    sk_make_sp<ResourceFontCollection>(std::vector{ fs::path("/usr/share/fonts/TTF") });
  return fontCollection.get();
}

void FontManager::initFontManager(const std::vector<fs::path>& fontDirs)
{
  if (m_init)
    return;
  m_init = true;
}

void FontManager::registerFontDirectory(const fs::path& fontDir)
{
}

void FontManager::registerFontFile(const std::string& fontName)
{
}
FontManager::FontManager()
{
}

sk_sp<ResourceFontCollection> FontManager::defaultFontCollection()
{
  return m_defaultFontCollection;
}

} // namespace VGG
