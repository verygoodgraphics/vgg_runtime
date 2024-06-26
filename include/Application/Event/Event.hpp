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
#include "Keycode.hpp"
#include "Scancode.hpp"

enum EButtonState : uint8_t
{
  VGG_RELEASE,
  VGG_PRESSED
};

enum EEventType
{
  VGG_FIRSTEVENT = 0, /**< Unused (do not remove) */

  /* Application events */
  VGG_QUIT = 0x100, /**< User-requested quit */
  VGG_APP_INIT,

  VGG_APP_TERMINATING,

  VGG_APP_LOWMEMORY,
  VGG_APP_WILLENTERBACKGROUND, /**< The application is about to enter the background
                                   Called on iOS in applicationWillResignActive()
                                   Called on Android in onPause()
                              */
  VGG_APP_DIDENTERBACKGROUND,  /**< The application did enter the background and may not get CPU
                                */
  VGG_APP_WILLENTERFOREGROUND, /**< The application is about to enter the foreground
                                   Called on iOS in applicationWillEnterForeground()
                                   Called on Android in onResume()
                              */
  VGG_APP_DIDENTERFOREGROUND,  /**< The application is now interactive
                                    Called on iOS in applicationDidBecomeActive()
                                    Called on Android in onResume()
                               */

  VGG_LOCALECHANGED, /**< The user's locale preferences have changed. */

  /* Display events */
  VGG_DISPLAYEVENT = 0x150, /**< Display state change */

  /* Window events */
  VGG_WINDOWEVENT = 0x200, /**< Window state change */
  VGG_SYSWMEVENT,          /**< System specific event */

  /* Keyboard events */
  VGG_KEYDOWN = 0x300, /**< Key pressed */
  VGG_KEYUP,           /**< Key released */
  VGG_TEXTEDITING,     /**< Keyboard text editing (composition) */
  VGG_TEXTINPUT,       /**< Keyboard text input */
  VGG_KEYMAPCHANGED,   /**< Keymap changed due to a system event such as an
                            input language or keyboard layout change.
                       */
  VGG_TEXTEDITING_EXT, /**< Extended keyboard text editing (composition) */

  /* Mouse events */
  VGG_MOUSEMOTION = 0x400, /**< Mouse moved */
  VGG_MOUSEBUTTONDOWN,     /**< Mouse button pressed */
  VGG_MOUSEBUTTONUP,       /**< Mouse button released */
  VGG_MOUSEWHEEL,          /**< Mouse wheel motion */

  /* Touch events */
  VGG_TOUCHDOWN = 0x700,
  VGG_TOUCHUP,
  VGG_TOUCHMOTION,

  /* Clipboard events */
  VGG_CLIPBOARDUPDATE = 0x900, /**< The clipboard or primary selection changed */

  /* Drag and drop events */
  VGG_DROPFILE = 0x1000, /**< The system requests a file open */
  VGG_DROPTEXT,          /**< text/plain drag-and-drop event */
  VGG_DROPBEGIN,         /**< A new set of drops is beginning (NULL filename) */
  VGG_DROPCOMPLETE,      /**< Current set of drops is now complete (NULL filename) */

  VGG_PAINT = 0x1100,

  VGG_USEREVENT = 0x8000,

  /**
   *  This last event is only for bounding internal arrays
   */
  VGG_LASTEVENT = 0xFFFF
};

/**
 *  \brief Window state change event data (event.window.*)
 */

struct VKeysym
{
  EVGGScancode scancode; /**< VGG physical key code - see ::SDL_Scancode for details */
  EVGGKeyCode  sym;      /**< VGG virtual key code - see ::SDL_Keycode for details */
  EVGGKeymod   mod;      /**< current key modifiers */
  uint32_t     unused;
};

/**
 *  \brief Keyboard button event structure (event.key.*)
 */
struct VKeyboardEvent
{
  uint32_t     type;      /**< ::VGG_KEYDOWN or ::VGG_KEYUP */
  uint32_t     timestamp; /**< In milliseconds, populated using VGG_GetTicks() */
  EButtonState state;     /**< ::VGG_Pressed or ::VGG_Release */
  uint8_t      repeat;    /**< Non-zero if this is a key repeat */
  uint8_t      padding2;
  uint8_t      padding3;
  VKeysym      keysym; /**< The key that was pressed or released */
};

#define VGG_TEXTEDITINGEVENT_TEXT_SIZE (32)
/**
 *  \brief Keyboard text editing event structure (event.edit.*)
 */
struct VTextEditingEvent
{
  uint32_t type;      /**< ::VGG_TEXTEDITING */
  uint32_t timestamp; /**< In milliseconds, populated using VGG_GetTicks() */
  char     text[VGG_TEXTEDITINGEVENT_TEXT_SIZE]; /**< The editing text */
  int32_t  start;                                /**< The start cursor of selected editing text */
  int32_t  length;                               /**< The length of selected editing text */
};

/**
 *  \brief Extended keyboard text editing event structure (event.editExt.*) when text would be
 *  truncated if stored in the text buffer VGG_TextEditingEvent
 */
struct VTextEditingExtEvent
{
  uint32_t type;      /**< ::VGG_TEXTEDITING_EXT */
  uint32_t timestamp; /**< In milliseconds, populated using VGG_GetTicks() */
  char* text; /**< The editing text, which should be freed with VGG_free(), and will not be NULL */
  int32_t start;  /**< The start cursor of selected editing text */
  int32_t length; /**< The length of selected editing text */
};

#define VGG_TEXTINPUTEVENT_TEXT_SIZE (32)
/**
 *  \brief Keyboard text input event structure (event.text.*)
 */
struct VTextInputEvent
{
  uint32_t type;      /**< ::VGG_TEXTINPUT */
  uint32_t timestamp; /**< In milliseconds, populated using VGG_GetTicks() */
  char     text[VGG_TEXTINPUTEVENT_TEXT_SIZE]; /**< The input text */
};

/**
 *  \brief Mouse motion event structure (event.motion.*)
 */
struct VMouseMotionEvent
{
  uint32_t     type;       /**< ::VGG_MOUSEMOTION */
  uint32_t     timestamp;  /**< In milliseconds, populated using VGG_GetTicks() */
  uint32_t     which;      /**< The mouse instance id, or VGG_TOUCH_MOUSEID */
  EButtonState state;      /**< ::VGG_PRESSED or ::VGG_RELEASED */
  int32_t      windowX;    /**< X coordinate, relative to window */
  int32_t      windowY;    /**< Y coordinate, relative to window */
  int32_t      xrel;       /**< The relative motion in the X direction */
  int32_t      yrel;       /**< The relative motion in the Y direction */
  int32_t      canvasXRel; /**< The relative motion in the X direction in canvas space*/
  int32_t      canvasYRel; /**< The relative motion in the Y direction in canvas space*/
  int32_t      canvasX;    /*mouse position on canvas means that the mapped position on the drawable
                                     texture beacuse of high dpi screen, its value should be given correctly
                                     by the app, same with mouse position by default*/
  int32_t      canvasY;
};

/**
 *  \brief Mouse button event structure (event.button.*)
 */
struct VMouseButtonEvent
{
  uint32_t     type;      /**< ::VGG_MOUSEBUTTONDOWN or ::VGG_MOUSEBUTTONUP */
  uint32_t     timestamp; /**< In milliseconds, populated using VGG_GetTicks() */
  uint32_t     which;     /**< The mouse instance id, or VGG_TOUCH_MOUSEID */
  uint8_t      button;    /**< The mouse button index */
  EButtonState state;     /**< ::VGG_PRESSED or ::VGG_RELEASED */
  uint8_t      clicks;    /**< 1 for single-click, 2 for double-click, etc. */
  uint8_t      padding1;
  int32_t      windowX; /**< X coordinate, relative to window */
  int32_t      windowY; /**< Y coordinate, relative to window */
  int32_t      canvasX;
  int32_t      canvasY;
};

/**
 *  \brief Mouse wheel event structure (event.wheel.*)
 */
struct VMouseWheelEvent
{
  uint32_t type;      /**< ::VGG_MOUSEWHEEL */
  uint32_t timestamp; /**< In milliseconds, populated using VGG_GetTicks() */
  uint32_t which;     /**< The mouse instance id, or VGG_TOUCH_MOUSEID */
  int32_t
    x; /**< The amount scrolled horizontally, positive to the right and negative to the left */
  int32_t  y; /**< The amount scrolled vertically, positive away from the user and negative toward
                the user */
  uint32_t direction; /**< Set to one of the VGG_MOUSEWHEEL_* defines. When FLIPPED the values in
                         X and Y will be opposite. Multiply by -1 to change them back */
  float   preciseX; /**< The amount scrolled horizontally, positive to the right and negative to the
                       left, with float precision (added in 2.0.18) */
  float   preciseY; /**< The amount scrolled vertically, positive away from the user and negative
                       toward the user, with float precision (added in 2.0.18) */
  int32_t mouseX;   /**< X coordinate, relative to window (added in 2.26.0) */
  int32_t mouseY;   /**< Y coordinate, relative to window (added in 2.26.0) */
  int32_t canvasX;  /**<X coordiante, relative to canvas  */
  int32_t canvasY;  /**<Y coordiante, relative to canvas  */
};

struct VTouchEvent
{
  uint32_t type;
  float    windowX; /**< X coordinate, relative to window. Normalized in the range 0...1 */
  float    windowY; /**< Y coordinate, relative to window. Normalized in the range 0...1 */
  float    xrel;    /**< The relative motion in the X direction. Normalized in the range -1...1 */
  float    yrel;    /**< The relative motion in the Y direction. Normalized in the range -1...1 */
};

enum EWindowEventID
{
  VGG_WINDOWEVENT_NONE,         /**< Never used */
  VGG_WINDOWEVENT_SHOWN,        /**< Window has been shown */
  VGG_WINDOWEVENT_HIDDEN,       /**< Window has been hidden */
  VGG_WINDOWEVENT_EXPOSED,      /**< Window has been exposed and should be
                                     redrawn */
  VGG_WINDOWEVENT_MOVED,        /**< Window has been moved to data1, data2
                                 */
  VGG_WINDOWEVENT_RESIZED,      /**< Window has been resized to data1xdata2 */
  VGG_WINDOWEVENT_SIZE_CHANGED, /**< The window size has changed, either as
                                     a result of an API call or through the
                                     system or user changing the window size. */
  VGG_WINDOWEVENT_MINIMIZED,    /**< Window has been minimized */
  VGG_WINDOWEVENT_MAXIMIZED,    /**< Window has been maximized */
  VGG_WINDOWEVENT_RESTORED,     /**< Window has been restored to normal size
                                     and position */
  VGG_WINDOWEVENT_ENTER,        /**< Window has gained mouse focus */
  VGG_WINDOWEVENT_LEAVE,        /**< Window has lost mouse focus */
  VGG_WINDOWEVENT_FOCUS_GAINED, /**< Window has gained keyboard focus */
  VGG_WINDOWEVENT_FOCUS_LOST,   /**< Window has lost keyboard focus */
  VGG_WINDOWEVENT_CLOSE,        /**< The window manager requests that the window be closed */
  VGG_WINDOWEVENT_TAKE_FOCUS, /**< Window is being offered a focus (should SetWindowInputFocus() on
                                 itself or a subwindow, or ignore) */
  VGG_WINDOWEVENT_HIT_TEST,   /**< Window had a hit test that wasn't SDL_HITTEST_NORMAL. */
  VGG_WINDOWEVENT_ICCPROF_CHANGED, /**< The ICC profile of the window's display has changed. */
  VGG_WINDOWEVENT_DISPLAY_CHANGED  /**< Window has been moved to display data1. */
};

struct VWindowEvent
{
  uint32_t       type;          /**< ::VGG_WINDOWEVENT */
  uint32_t       timestamp;     /**< In milliseconds, populated using VGG_GetTicks() */
  uint32_t       windowID;      /**< The associated window */
  EWindowEventID event;         /**< ::VGG_WindowEventID */
  int32_t        data1;         /**< event dependent data */
  int32_t        data2;         /**< event dependent data */
  int32_t        drawableWidth; /*window drawable size width: */
  int32_t        drawableHeight;
};

/**
 *  \brief An event used to request a file open by the system (event.drop.*)
 *         This event is enabled by default, you can disable it with VGG_EventState().
 *  \note If this event is enabled, you must free the filename in the event.
 */
struct VDropEvent
{
  uint32_t type; /**< ::VGG_DROPBEGIN or ::VGG_DROPFILE or ::SDL_DROPTEXT or ::SDL_DROPCOMPLETE */
  uint32_t timestamp; /**< In milliseconds, populated using VGG_GetTicks() */
  char*
    file; /**< The file name, which should be freed with VGG_free(), is NULL on begin/complete */
};

/**
 *  \brief Sensor event structure (event.sensor.*)
 */

/**
 *  \brief The "quit requested" event
 */
struct VQuitEvent
{
  uint32_t type;      /**< ::VGG_QUIT */
  uint32_t timestamp; /**< In milliseconds, populated using VGG_GetTicks() */
};

/**
 *  \brief A user-defined event type (event.user.*)
 */
struct VUserEvent
{
  uint32_t type;      /**< ::VGG_USEREVENT through ::VGG_LASTEVENT-1 */
  uint32_t timestamp; /**< In milliseconds, populated using VGG_GetTicks() */
  int32_t  code;      /**< User defined event code */
  void*    data1;     /**< User defined data pointer */
  void*    data2;     /**< User defined data pointer */
};

struct VPaintEvent
{
  uint32_t type;
  uint32_t timestamp;
  void*    data;
  float    dpi;
};

struct VAppInitEvent
{
  uint32_t type;
  uint32_t timestamp;
  uint32_t windowWidth;
  uint32_t windowHeight;
  uint32_t drawableWidth;
  uint32_t drawableHeight;
  int      argc;
  char**   argv;
};

/**
 *  \brief General event structure
 */
union UEvent
{
  uint32_t             type; /**< Event type, shared with all events */
  VAppInitEvent        init;
  VKeyboardEvent       key;
  VWindowEvent         window;
  VTextEditingEvent    edit;
  VTextEditingExtEvent editExt;
  VTextInputEvent      text;
  VMouseMotionEvent    motion;
  VMouseButtonEvent    button;
  VMouseWheelEvent     wheel;
  VTouchEvent          touch;
  VQuitEvent           quit;
  VUserEvent           user;
  VDropEvent           drop;
  VPaintEvent          paint;
};
