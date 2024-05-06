/*
 * Copyright 2023-2024 VeryGoodGraphics LTD <bd@verygoodgraphics.com>
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
  struct ImageOptions
  {
    std::string type{ "png" }; // png, jpg, webp, raw
    int         quality{ 100 };
  };

  struct AnimationOptions
  {
    double      duration{ 0.3 };
    std::string type{ "dissolve" };
    std::string timingFunction{ "linear" };
  };

public:
  virtual ~ISdk() = default;

  // configure
  virtual void setFitToViewportEnabled(bool enabled) = 0;

  /*
    "topLeft"
    "scaleAspectFill"
    "scaleAspectFillTopCenter"
    "scaleAspectFit"
  */
  virtual void setContentMode(const std::string& newModel) = 0;

  // -
  virtual std::string designDocument() = 0;
  virtual std::string designDocumentValueAt(const std::string& jsonPointer) = 0;

  virtual std::string getElement(const std::string& id) = 0;
  virtual void updateElement(const std::string& id, const std::string& contentJsonString) = 0;

  // -
  virtual std::string getFramesInfo() const = 0;
  virtual std::string currentFrameId() const = 0;

  // configure
  virtual const std::string launchFrameId() const = 0;
  virtual bool              setLaunchFrameById(const std::string& id) = 0;
  virtual const std::string currentTheme() const = 0;
  virtual bool              setCurrentTheme(const std::string& theme) = 0;

  // frame
  virtual bool setCurrentFrameById(const std::string& id, bool resetScrollPosition = true) = 0;
  virtual bool setCurrentFrameByIdAnimated(
    const std::string&      id,
    bool                    resetScrollPosition,
    const AnimationOptions& option) = 0;
  virtual bool presentFrameById(const std::string& id, bool resetScrollPosition = true) = 0;
  virtual bool dismissFrame() = 0;
  virtual bool goBack(bool resetScrollPosition = true, bool resetState = true) = 0;
  virtual bool nextFrame() = 0;
  virtual bool previousFrame() = 0;

  // instance
  virtual bool setState(
    const std::string& instanceDescendantId,
    const std::string& listenerId,
    const std::string& newMasterId,
    bool               resetScrollPosition = true) = 0;
  virtual bool presentState(
    const std::string& instanceDescendantId,
    const std::string& listenerId,
    const std::string& newStateMasterId,
    bool               resetScrollPosition = true) = 0;
  virtual bool dismissState(const std::string& instanceDescendantId) = 0;

  // element
  virtual bool updateElementFillColor(
    const std::string& id,
    const std::size_t  fillIndex,
    const double       r,
    const double       g,
    const double       b,
    const double       a) = 0;

  // misc
  virtual std::string requiredFonts() const = 0;
  virtual bool        addFont(const uint8_t* data, size_t size, const char* defaultName) = 0;

  virtual std::vector<uint8_t>     vggFileBuffer() = 0;
  virtual std::vector<std::string> texts() = 0;

  virtual std::vector<uint8_t> makeImageSnapshot(const ImageOptions& options) = 0;

  virtual void openUrl(const std::string& url, const std::string& target) = 0;
};

} // namespace VGG
