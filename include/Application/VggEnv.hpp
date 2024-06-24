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

#include <memory>
#include <sstream>
#include <string>
#include <unordered_map>
#include "Application/Controller.hpp"
#include "Domain/DarumaContainer.hpp"
#include "Domain/IVggEnv.hpp"
namespace VGG
{
class Presenter;
class UIApplication;
} // namespace VGG

namespace VGG
{

class VggEnv : public IVggEnv
{
  std::string m_containerKey{ "vggInstances" };
  std::string m_instanceKey{ "instance" };
  std::string m_listenerKey{ "listener" };

  DarumaContainer m_darumaContainer;

  UIApplication*            m_application;
  std::weak_ptr<Controller> m_controller;
  std::weak_ptr<Presenter>  m_presenter;

public:
  VggEnv();

  virtual std::string getEnv() override
  {
    const void*       address = static_cast<const void*>(this);
    std::stringstream ss;
    ss << "_" << m_tag << "_" << address;
    return ss.str();
  }

  virtual std::string getContainerKey() override
  {
    return m_containerKey;
  }
  virtual void setContainerKey(const std::string& containerKey) override
  {
    m_containerKey = containerKey;
  }

  virtual std::string getInstanceKey() override
  {
    return m_instanceKey;
  }

  virtual void setInstanceKey(const std::string& instanceKey) override
  {
    m_instanceKey = instanceKey;
  }

  virtual std::string getListenerKey() override
  {
    return m_listenerKey;
  }

  virtual void setListenerKey(const std::string& listenerKey) override
  {
    m_listenerKey = listenerKey;
  }

  virtual DarumaContainer& darumaContainer() override
  {
    return m_darumaContainer;
  }

  UIApplication* application()
  {
    return m_application;
  }
  void setApplication(UIApplication* application)
  {
    m_application = application;
  }

  std::shared_ptr<Controller> controller()
  {
    return m_controller.lock();
  }
  void setController(std::shared_ptr<Controller> controller)
  {
    m_controller = controller;
  }

  std::shared_ptr<Presenter> presenter()
  {
    return m_presenter.lock();
  }
  void setPresenter(std::shared_ptr<Presenter> presenter)
  {
    m_presenter = presenter;
  }

public:
  static std::weak_ptr<VggEnv> getDefault();
  static std::weak_ptr<VggEnv> get(const std::string& key);
  static void                  set(const std::string& key, std::weak_ptr<VggEnv> env);
  static void                  erase(const std::string& key);

private:
  static std::unordered_map<std::string, std::weak_ptr<VggEnv>>& getRepo();

  int64_t m_tag = 0;
};

} // namespace VGG