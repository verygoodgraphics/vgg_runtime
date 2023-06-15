#pragma once

#include "Model/Saver/Saver.hpp"
#include "Model/VggWork.hpp"

#include <memory>

namespace VGG
{

// todo, not edit mode
class Editor
{
  std::shared_ptr<VggWork> m_model;
  std::shared_ptr<Model::Saver> m_saver;

public:
  Editor(std::shared_ptr<VggWork> model, std::shared_ptr<Model::Saver> saver)
    : m_model{ model }
    , m_saver{ saver }
  {
  }

  void save()
  {
    m_model->visit(m_saver.get());
  }
};

} // namespace VGG