#pragma once

#include "Domain/Saver.hpp"
#include "Domain/Daruma.hpp"

#include <memory>

namespace VGG
{

// todo, not edit mode
class Editor
{
  std::shared_ptr<Daruma> m_model;
  std::shared_ptr<Model::Saver> m_saver;

public:
  Editor(std::shared_ptr<Daruma> model, std::shared_ptr<Model::Saver> saver)
    : m_model{ model }
    , m_saver{ saver }
  {
  }

  void save()
  {
    m_model->accept(m_saver.get());
  }
};

} // namespace VGG