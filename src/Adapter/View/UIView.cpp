#include "Application/UIView.hpp"

#include "Scene/Zoomer.h"

#include <SDL2/SDL.h>

using namespace VGG;
using namespace VGG::Layout;
using namespace nlohmann;

constexpr auto flip_y_factor = -1;

void UIView::onEvent(const SDL_Event& evt, Zoomer* zoomer)
{
  if (!m_event_listener)
  {
    return;
  }

  if (!m_is_editor)
  {
    m_bounds.origin.x = zoomer->offset.x;
    m_bounds.origin.y = zoomer->offset.y;

    m_contentScaleFactor = zoomer->zoom;
  }

  // todo, hittest
  for (auto& subview : m_subviews)
  {
    subview->onEvent(evt, zoomer);
  }

  // todo, capturing
  // todo, bubbling
  UIEvent::PathType target_path{ "/fake/update_background_color" };

  decltype(m_has_event_listener) has_event_listener =
    [this](const std::string& path, UIEventType type)
  {
    if (this->m_superview && this->m_superview->m_is_editor)
    {
      return true;
    }
    else
    {
      return m_has_event_listener(path, type);
    }
  };
  switch (evt.type)
  {
    case SDL_MOUSEBUTTONDOWN:
    {
      Layout::Point point{ toVggLayoutScalar(evt.button.x), toVggLayoutScalar(evt.button.y) };
      point = converPointFromWindow(point);
      auto target_view = m_root->hitTest(point,
                                         [&has_event_listener](const std::string& path) {
                                           return has_event_listener(path, UIEventType::mousedown);
                                         });
      if (target_view)
      {
        auto js_button_index{ evt.button.button - 1 };
        auto [alt, ctrl, meta, shift] = getKeyModifier(SDL_GetModState());
        m_event_listener(UIEventPtr(new MouseEvent(target_view->path(),
                                                   UIEventType::mousedown,
                                                   js_button_index,
                                                   evt.button.x,
                                                   evt.button.y,
                                                   0,
                                                   0,
                                                   alt,
                                                   ctrl,
                                                   meta,
                                                   shift)));
      }
    }
    break;

    case SDL_MOUSEMOTION:
    {
      Layout::Point point{ toVggLayoutScalar(evt.motion.x), toVggLayoutScalar(evt.motion.y) };
      point = converPointFromWindow(point);
      auto target_view = m_root->hitTest(point,
                                         [&has_event_listener](const std::string& path) {
                                           return has_event_listener(path, UIEventType::mousemove);
                                         });
      if (target_view)
      {
        auto [alt, ctrl, meta, shift] = getKeyModifier(SDL_GetModState());
        m_event_listener(UIEventPtr(new MouseEvent(target_view->path(),
                                                   UIEventType::mousemove,
                                                   0,
                                                   evt.motion.x,
                                                   evt.motion.y,
                                                   evt.motion.xrel,
                                                   evt.motion.yrel,
                                                   alt,
                                                   ctrl,
                                                   meta,
                                                   shift)));
      }
    }
    break;

    case SDL_MOUSEBUTTONUP:
    {
      Layout::Point point{ toVggLayoutScalar(evt.button.x), toVggLayoutScalar(evt.button.y) };
      point = converPointFromWindow(point);
      auto js_button_index{ evt.button.button - 1 };
      auto [alt, ctrl, meta, shift] = getKeyModifier(SDL_GetModState());

      auto target_view = m_root->hitTest(point,
                                         [&has_event_listener](const std::string& path) {
                                           return has_event_listener(path, UIEventType::mouseup);
                                         });
      if (target_view)
      {
        m_event_listener(UIEventPtr(new MouseEvent(target_view->path(),
                                                   UIEventType::mouseup,
                                                   js_button_index,
                                                   evt.button.x,
                                                   evt.button.y,
                                                   0,
                                                   0,
                                                   alt,
                                                   ctrl,
                                                   meta,
                                                   shift)));
      }

      if (js_button_index == 0)
      {
        auto target_view = m_root->hitTest(point,
                                           [&has_event_listener](const std::string& path) {
                                             return has_event_listener(path, UIEventType::click);
                                           });
        if (target_view)
        {
          m_event_listener(UIEventPtr(new MouseEvent(target_view->path(),
                                                     UIEventType::click,
                                                     js_button_index,
                                                     evt.button.x,
                                                     evt.button.y,
                                                     0,
                                                     0,
                                                     alt,
                                                     ctrl,
                                                     meta,
                                                     shift)));
        }
      }
      else
      {
        auto target_view = m_root->hitTest(point,
                                           [&has_event_listener](const std::string& path) {
                                             return has_event_listener(path, UIEventType::auxclick);
                                           });
        if (target_view)
        {
          m_event_listener(UIEventPtr(new MouseEvent(target_view->path(),
                                                     UIEventType::auxclick,
                                                     js_button_index,
                                                     evt.button.x,
                                                     evt.button.y,
                                                     0,
                                                     0,
                                                     alt,
                                                     ctrl,
                                                     meta,
                                                     shift)));
        }

        if (js_button_index == 2)
        {
          auto target_view =
            m_root->hitTest(point,
                            [&has_event_listener](const std::string& path)
                            { return has_event_listener(path, UIEventType::contextmenu); });
          if (target_view)
          {
            m_event_listener(UIEventPtr(new MouseEvent(target_view->path(),
                                                       UIEventType::contextmenu,
                                                       js_button_index,
                                                       evt.button.x,
                                                       evt.button.y,
                                                       0,
                                                       0,
                                                       alt,
                                                       ctrl,
                                                       meta,
                                                       shift)));
          }
        }
      }
    }
    break;

    case SDL_KEYDOWN:
    {
      auto [alt, ctrl, meta, shift] = getKeyModifier(SDL_GetModState());
      m_event_listener(UIEventPtr(new KeyboardEvent(target_path,
                                                    UIEventType::keydown,
                                                    evt.key.keysym.sym,
                                                    evt.key.repeat,
                                                    alt,
                                                    ctrl,
                                                    meta,
                                                    shift)));
    }
    break;

    case SDL_KEYUP:
    {
      auto [alt, ctrl, meta, shift] = getKeyModifier(SDL_GetModState());
      m_event_listener(UIEventPtr(new KeyboardEvent(target_path,
                                                    UIEventType::keyup,
                                                    evt.key.keysym.sym,
                                                    evt.key.repeat,
                                                    alt,
                                                    ctrl,
                                                    meta,
                                                    shift)));
    }
    break;

    case SDL_FINGERDOWN:
    {
      m_event_listener(UIEventPtr(new TouchEvent(target_path, UIEventType::touchstart)));
    }
    break;

    case SDL_FINGERMOTION:
    {
      m_event_listener(UIEventPtr(new TouchEvent(target_path, UIEventType::touchmove)));
    }
    break;

    case SDL_FINGERUP:
    {
      m_event_listener(UIEventPtr(new TouchEvent(target_path, UIEventType::touchend)));
    }
    break;

      // todo, more event

    default:
      break;
  }
}

std::tuple<bool, bool, bool, bool> UIView::getKeyModifier(int keyMod)
{
  bool l_alt = keyMod & KMOD_LALT;
  bool l_ctrl = keyMod & KMOD_LCTRL;
  bool l_meta = keyMod & KMOD_LGUI;
  bool l_shift = keyMod & KMOD_LSHIFT;

  bool r_alt = keyMod & KMOD_RALT;
  bool r_ctrl = keyMod & KMOD_RCTRL;
  bool r_meta = keyMod & KMOD_RGUI;
  bool r_shift = keyMod & KMOD_RSHIFT;

  return { l_alt || r_alt, l_ctrl || r_ctrl, l_meta || r_meta, l_shift || r_shift };
}

void UIView::draw(SkCanvas* canvas, Zoomer* zoomer)
{
  if (m_is_editor) // editor; zoom only subviews
  {
    m_scene->render(canvas); // render self(editor) without zoom

    // draw inner edit view
    for (auto& subview : m_subviews)
    {
      subview->draw(canvas, zoomer);
    }
  }
  else // edit view; edited document; zoom self
  {
    canvas->save();

    // setup clip & offset for edit view
    canvas->translate(m_frame.origin.x, m_frame.origin.y);
    SkRect edit_rect{ 0, 0, m_frame.size.width, m_frame.size.height };
    canvas->clipRect(edit_rect);

    zoomer->apply(canvas);

    m_scene->render(canvas);

    canvas->restore();
  }
}

void UIView::becomeEditorWithSidebar(scalar_type top,
                                     scalar_type right,
                                     scalar_type bottom,
                                     scalar_type left)
{
  m_is_editor = true;

  m_top = top;
  m_right = right;
  m_bottom = bottom;
  m_left = left;

  // todo, editor layout
}

void UIView::setupTree(const nlohmann::json& j)
{
  // todo, select artboard
  auto& artboard = j["artboard"][0];
  if (!artboard.is_object())
  {
    FAIL("no artboard in design file");
    return;
  }

  m_root = createOneLayoutView(artboard, json::json_pointer("/artboard/0"), nullptr);
}

std::shared_ptr<LayoutView> UIView::createOneLayoutView(const nlohmann::json& j,
                                                        json::json_pointer current_path,
                                                        std::shared_ptr<LayoutView> parent)
{
  if (!j.is_object())
  {
    WARN("create one layout view from json object, json is not object, return");
    return nullptr;
  }

  // check top level class
  auto class_name = j.value("class", "");
  if (class_name != "artboard" && class_name != "frame" && class_name != "group" &&
      class_name != "image" && class_name != "layer" && class_name != "path" &&
      class_name != "symbolInstance" && class_name != "symbolMaster" && class_name != "text")
  {
    return nullptr;
  }

  Layout::Rect frame;
  constexpr auto frame_key = "frame";
  const auto it = j.find(frame_key);
  if (it != j.end())
  {
    auto frame_json = j[frame_key];
    auto x = frame_json["x"];
    auto y = frame_json["y"].get<Layout::Scalar>() * flip_y_factor;
    auto width = frame_json["width"];
    auto height = frame_json["height"];

    frame = { { x, y }, { width, height } };
  }

  auto layout_view = std::make_shared<LayoutView>(current_path.to_string(), frame);
  if (parent)
  {
    parent->addChild(layout_view);
  }

  for (auto& [key, val] : j.items())
  {
    auto path = current_path;
    path /= key;

    createOneOrMoreLayoutViews(val, path, layout_view);
  }

  return layout_view;
}

void UIView::createLayoutViews(const nlohmann::json& j,
                               json::json_pointer current_path,
                               std::shared_ptr<LayoutView> parent)
{
  if (!j.is_array())
  {
    WARN("create layout views from json array, json is not array, return");
    return;
  }

  auto size = j.size();
  for (auto i = 0; i < size; ++i)
  {
    auto path = current_path;
    path /= i;

    createOneOrMoreLayoutViews(j[i], path, parent);
  }
}

void UIView::createOneOrMoreLayoutViews(const nlohmann::json& j,
                                        json::json_pointer current_path,
                                        std::shared_ptr<LayoutView> parent)
{
  if (j.is_object())
  {
    createOneLayoutView(j, current_path, parent);
  }
  else if (j.is_array())
  {
    createLayoutViews(j, current_path, parent);
  }
}

void UIView::layoutSubviews()
{
  if (m_is_editor)
  {
    for (auto& subview : m_subviews)
    {
      subview->m_frame = { { toVggLayoutScalar(m_left), toVggLayoutScalar(m_top) },
                           {
                             m_frame.size.width - m_left - m_right,
                             m_frame.size.height - m_top - m_bottom,
                           } };
    }
  }
}

Layout::Point UIView::converPointFromWindow(Layout::Point point)
{
  auto x = point.x;
  auto y = point.y;

  auto parent = this;
  while (parent)
  {
    x -= parent->m_frame.origin.x;
    y -= parent->m_frame.origin.y;

    x -= parent->m_bounds.origin.x;
    y -= parent->m_bounds.origin.y;

    parent = parent->m_superview;
  }

  return { x, y };
}