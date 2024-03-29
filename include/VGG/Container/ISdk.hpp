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

#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace VGG
{

class ISdk
{
public:
  virtual ~ISdk() = default;

  virtual std::string designDocument() = 0;
  virtual std::string designDocumentValueAt(const std::string& jsonPointer) = 0;
  virtual void designDocumentAddAt(const std::string& jsonPointer, const std::string& value) = 0;
  virtual void designDocumentReplaceAt(
    const std::string& jsonPointer,
    const std::string& value) = 0;
  virtual void designDocumentDeleteAt(const std::string& jsonPointer) = 0;

  virtual std::string getElement(const std::string& id) = 0;
  virtual void updateElement(const std::string& id, const std::string& contentJsonString) = 0;

  virtual std::string getFramesInfo() const = 0;
  virtual int         currentFrameIndex() const = 0;
  virtual bool        setCurrentFrame(const std::string& name) = 0;
  virtual int         launchFrameIndex() const = 0;
  virtual bool        setLaunchFrame(const std::string& name) = 0;

  virtual std::string requiredFonts() const = 0;
  virtual bool        addFont(const uint8_t* data, size_t size, const char* defaultName) = 0;

  virtual std::vector<uint8_t>     vggFileBuffer() = 0;
  virtual std::vector<std::string> texts() = 0;
};

} // namespace VGG
