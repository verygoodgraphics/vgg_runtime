/*
 * Copyright (C) 2021 Chaoya Li <harry75369@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef __FILE_MANAGER_HPP__
#define __FILE_MANAGER_HPP__

#include <nlohmann/json.hpp>
#include <miniz-cpp/zip_file.hpp>

#include "Entity/EntityManager.hpp"
#include "Utils/SketchImporter.hpp"
#include "Utils/FileUtils.hpp"
#include "Utils/TextureManager.hpp"

namespace VGG
{

using json = nlohmann::json;

struct FileManager
{
  struct FilePage
  {
    std::string name{ "Page" };
    EntityContainer container;
  };
  struct FileInfo
  {
    static inline int untitledCounter{ 0 };
    std::string name; // depends on path if exists
    std::optional<std::string> path;
    std::vector<FilePage> pages;
    size_t currPageIdx{ 0 };

    FileInfo()
      : name("Untitled")
    {
      if (untitledCounter > 0)
      {
        name += std::to_string(untitledCounter);
      }
      untitledCounter += 1;
      pages.push_back(FilePage{});
    }

    FileInfo(const std::string& name, std::optional<std::string> fp, std::vector<FilePage>&& pages)
      : name(name)
      , path(fp)
      , pages(pages)
    {
      if (pages.size() < 1)
      {
        WARN("No page provided. Adding empty page...");
        pages.push_back(FilePage{});
      }
    }

    inline void updatePath(const std::string& fp)
    {
      path = fp;
      name = FileManager::getFileName(fp);
    }

    inline void loadCurrPage()
    {
      ASSERT(pages.size() > 0);
      ASSERT(currPageIdx < pages.size());
      EntityManager::setEntities(&(pages[currPageIdx].container));
    }

    inline void prevPage()
    {
      currPageIdx = (currPageIdx + pages.size() - 1) % pages.size();
      loadCurrPage();
    }

    inline void nextPage()
    {
      currPageIdx = (currPageIdx + 1) % pages.size();
      loadCurrPage();
    }

    inline void gotoPage(size_t i)
    {
      currPageIdx = i % pages.size();
      loadCurrPage();
    }

    inline void newPage()
    {
      pages.push_back(FilePage{});
      currPageIdx = pages.size() - 1;
      loadCurrPage();
    }

    inline void delPage(size_t i)
    {
      if (pages.size() <= 1)
      {
        WARN("Cannot delete the last page!");
        return;
      }

      pages.erase(pages.begin() + i);
      if (currPageIdx >= pages.size())
      {
        currPageIdx = pages.size() - 1;
      }
      loadCurrPage();
    }
  };

public: // public members
  FileInfo* currFile{ nullptr };
  std::vector<FileInfo> files;

public: // public methods
  inline size_t fileCount() const
  {
    return files.size();
  }

  inline int getCurrFileIdx() const
  {
    if (!currFile)
    {
      return -1;
    }
    for (int i = 0; i < files.size(); i++)
    {
      if (&(files[i]) == currFile)
      {
        return i;
      }
    }
    return -1;
  }

  bool switchToFile(size_t idx)
  {
    if (idx >= files.size())
    {
      return false;
    }
    currFile = &(files[idx]);
    currFile->loadCurrPage();
    return true;
  }

protected: // protected methods
  FileManager()
  {
  }

public: // public static methods
  static FileManager* getInstance()
  {
    static FileManager fm;
    return &fm;
  }

  static inline std::string getFileName(const std::string& fp)
  {
    return FileUtils::getFileName(fp);
  }

  static inline std::string getLoweredFileExt(const std::string& fp)
  {
    return FileUtils::getLoweredFileExt(fp);
  }

  static inline bool newFile()
  {
    auto fm = getInstance();
    ASSERT(fm);
    fm->files.push_back(FileInfo{});
    return fm->switchToFile(fm->fileCount() - 1);
  }

  static inline bool loadFile(const std::string& fp)
  {
    auto ext = getLoweredFileExt(fp);
    miniz_cpp::zip_file file;

    if (ext == "sketch")
    {
      try
      {
        file.load(fp);
      }
      catch (const std::runtime_error& err)
      {
        FAIL("Failed to load sketch file: %s", err.what());
        return false;
      }
      return loadSketchFile(file, getFileName(fp));
    }
    if (ext == "vgg")
    {
      try
      {
        file.load(fp);
      }
      catch (const std::runtime_error& err)
      {
        FAIL("Failed to load vgg file: %s", err.what());
        return false;
      }
      return loadVGGFile(file, getFileName(fp), fp);
    }
    FAIL("Unsuported file type: %s", ext.c_str());
    return false;
  }

  static inline bool loadFileFromMem(const std::vector<unsigned char>& buf, const std::string& name)
  {
    auto ext = getLoweredFileExt(name);
    miniz_cpp::zip_file file;

    if (ext == "sketch")
    {
      try
      {
        file.load(buf);
      }
      catch (const std::runtime_error& err)
      {
        FAIL("Failed to load sketch file: %s", err.what());
        return false;
      }
      return loadSketchFile(file, getFileName(name));
    }
    else if (ext == "vgg")
    {
      try
      {
        file.load(buf);
      }
      catch (const std::runtime_error& err)
      {
        FAIL("Failed to load vgg file: %s", err.what());
        return false;
      }
      return loadVGGFile(file, getFileName(name));
    }
    else
    {
      try
      {
        file.load(buf);
      }
      catch (const std::runtime_error& err)
      {
        FAIL("Failed to load file: %s", err.what());
        return false;
      }
      if (loadSketchFile(file, name))
      {
        return true;
      }
      if (loadVGGFile(file, name))
      {
        return true;
      }
    }
    FAIL("Unsuported file type: %s", ext.c_str());
    return false;
  }

  static inline bool saveCurrentFileAs(const std::string& fp)
  {
    auto fm = FileManager::getInstance();
    ASSERT(fm);
    if (fm->currFile)
    {
      return saveFileAs(fm->getCurrFileIdx(), fp);
    }
    return false;
  }

  static inline bool saveFileAs(int idx, const std::string& fp)
  {
    auto fm = FileManager::getInstance();
    ASSERT(fm);
    if (idx < 0 || idx >= fm->fileCount())
    {
      WARN("Invalid file idx for saving file: %d", idx);
      return false;
    }
    miniz_cpp::zip_file file;
    if (saveVGGFile(file, idx, fp))
    {
      file.save(fp);
      return true;
    }
    return false;
  }

  static inline bool saveFile(size_t idx)
  {
    auto fm = FileManager::getInstance();
    ASSERT(fm);
    if (idx < 0 || idx >= fm->fileCount())
    {
      WARN("Invalid file idx for saving file: %lu", idx);
      return false;
    }
    auto& fi = fm->files[idx];
    miniz_cpp::zip_file file;
    if (fi.path.has_value() && saveVGGFile(file, idx))
    {
      file.save(fi.path.value());
      return true;
    }
    WARN("No path given to save file: %lu", idx);
    return false;
  }

  static inline bool saveFileToMem(int idx, std::vector<unsigned char>& buf)
  {
    auto fm = FileManager::getInstance();
    ASSERT(fm);
    if (idx < 0)
    {
      idx = fm->getCurrFileIdx();
    }
    if (idx < 0 || idx >= fm->fileCount())
    {
      WARN("Invalid file idx for saving file: %d", idx);
      return false;
    }
    miniz_cpp::zip_file file;
    if (saveVGGFile(file, idx))
    {
      file.save(buf);
      return true;
    }
    return false;
  }

  static inline bool hasPath(size_t idx)
  {
    auto fm = FileManager::getInstance();
    ASSERT(fm);
    if (idx < 0 || idx >= fm->fileCount())
    {
      WARN("Invalid file idx for checking path: %lu", idx);
      return false;
    }
    return fm->files[idx].path.has_value();
  }

  static inline std::string getPath(int idx)
  {
    auto fm = FileManager::getInstance();
    ASSERT(fm);
    if (idx < 0 || idx >= fm->fileCount())
    {
      return "";
    }
    return fm->files[idx].path.value_or("");
  }

  static inline std::string getName(int idx)
  {
    auto fm = FileManager::getInstance();
    ASSERT(fm);
    if (idx < 0 || idx >= fm->fileCount())
    {
      ASSERT(fm->currFile);
      return fm->currFile->name;
    }
    return fm->files[idx].name;
  }

  static inline bool closeFile(size_t idx)
  {
    auto fm = FileManager::getInstance();
    ASSERT(fm);
    if (idx < 0 || idx >= fm->fileCount())
    {
      WARN("Invalid file idx to close: %lu", idx);
      return false;
    }
    ASSERT(fm->currFile);
    size_t currIdx = fm->getCurrFileIdx();
    bool closeCurrent = (currIdx == idx);
    fm->files.erase(fm->files.begin() + idx);
    if (fm->fileCount() == 0)
    {
      newFile();
    }
    else if (fm->fileCount() <= idx)
    {
      fm->switchToFile(std::min(currIdx, fm->fileCount() - 1));
    }
    else
    {
      if (closeCurrent)
      {
        if (idx < fm->fileCount())
        {
          fm->switchToFile(idx);
        }
        else
        {
          ASSERT(idx > 0);
          fm->switchToFile(idx - 1);
        }
      }
      else
      {
        if (idx < currIdx)
        {
          ASSERT(currIdx > 0);
          fm->switchToFile(currIdx - 1);
        }
      }
    }
    return true;
  }

  static inline void prevPage()
  {
    auto fm = getInstance();
    ASSERT(fm);
    if (fm->currFile)
    {
      fm->currFile->prevPage();
    }
  }

  static inline void nextPage()
  {
    auto fm = getInstance();
    ASSERT(fm);
    if (fm->currFile)
    {
      fm->currFile->nextPage();
    }
  }

protected: // protected static methods
  static bool loadSketchFile(miniz_cpp::zip_file& file,
                             const std::string& name,
                             const std::optional<std::string>& fp = std::nullopt)
  {
    for (auto entry : file.namelist())
    {
      if (entry == "VGG")
      {
        return false;
      }
    }

    std::vector<FilePage> pages;
    for (auto entry : file.namelist())
    {
      if (entry.rfind("pages/", 0) == 0)
      {
        auto json = json::parse(file.read(entry));
        if (auto containerOpt = SketchImporter::fromSketchPage(json))
        {
          auto container = containerOpt.value();
          pages.push_back(FilePage{
            .name = json.value("name", "Page"),
            .container = container,
          });
        }
        else
        {
          return false;
        }
      }
    }

    for (auto entry : file.namelist())
    {
      if (entry.rfind("images/", 0) == 0)
      {
        std::string data = file.read(entry);
        TextureManager::loadImageBlob(entry, data);
      }
    }

    if (auto fm = getInstance())
    {
      fm->files.push_back(FileInfo(name, fp, std::move(pages)));
      if (!(fm->switchToFile(fm->fileCount() - 1)))
      {
        WARN("Failed to switch to file: %s", name.c_str());
        fm->files.pop_back();
        return false;
      }
      return true;
    }

    return false;
  }

  static bool loadVGGFile(miniz_cpp::zip_file& file,
                          const std::string& name,
                          const std::optional<std::string>& fp = std::nullopt)
  {
    std::vector<FilePage> pages;
    for (auto entry : file.namelist())
    {
      if (entry.rfind("pages/", 0) == 0)
      {
        auto json = json::parse(file.read(entry));
        FilePage p;
        json.at("name").get_to(p.name);
        json.at("container").get_to(p.container);
        pages.push_back(p);
      }
    }

    for (auto entry : file.namelist())
    {
      if (entry.rfind("images/", 0) == 0)
      {
        std::string data = file.read(entry);
        TextureManager::loadImageBlob(entry, data);
      }
    }

    if (auto fm = getInstance())
    {
      // TODO same-file check for vgg files
      fm->files.push_back(FileInfo(name, fp, std::move(pages)));
      if (!(fm->switchToFile(fm->fileCount() - 1)))
      {
        WARN("Failed to switch to file: %s", name.c_str());
        fm->files.pop_back();
        return false;
      }
      return true;
    }

    return false;
  }

  static bool saveVGGFile(miniz_cpp::zip_file& file,
                          int idx,
                          const std::optional<std::string>& filePath = std::nullopt)
  {
    auto fm = FileManager::getInstance();
    ASSERT(fm);
    ASSERT(idx >= 0 && idx < fm->fileCount());
    auto& fi = fm->files[idx];

    file.reset();

    for (size_t i = 0; i < fi.pages.size(); i++)
    {
      auto& page = fi.pages[i];
      std::vector<std::string> imgNamesInPage;
      page.container.collectFillImageNames(imgNamesInPage);
      for (auto& name : imgNamesInPage)
      {
        // NOTE we convert all images to WebP format for best file size
        const std::string fp = "images/" + name + ".webp";
        if (file.has_file(fp))
        {
          continue;
        }
        if (auto img = TextureManager::getSkiaImage(name))
        {
          // TODO provide saving option for lossless or lossy webp
          if (auto data = img->encodeToData(SkEncodedImageFormat::kWEBP, 100)) // loseless webp
          {
            auto pt = (char*)data->data();
            auto sz = data->size();
            file.writestr(fp, std::string(pt, pt + sz));
          }
          else
          {
            WARN("Failed to save image: %s", name.c_str());
          }
        }
      }
    }

    for (size_t i = 0; i < fi.pages.size(); i++)
    {
      auto& page = fi.pages[i];
      nlohmann::json j = {
        { "name", page.name },
        { "container", page.container },
      };
      file.writestr(strfmt("pages/%lu.json", i), j.dump());
    }

    // write VGG file tag
    file.writestr("VGG", "");

    if (filePath.has_value())
    {
      fi.updatePath(filePath.value());
    }

    return true;
  }
};

}; // namespace VGG

#endif // __FILE_MANAGER_HPP__
