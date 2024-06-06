#pragma once

// When include this file, you need setup the include directory correctly.
// both SDL2 and imgui remains as its standard include structure
#include <SDL_events.h>
#include <SDL_video.h>

#include <imgui.h>
#include <backends/imgui_impl_sdl2.h>
#include <backends/imgui_impl_opengl3.h>

// NOLINTBEGIN

class ImGuiPanel
{
public:
  virtual void draw() = 0;
};

class ImGuiPanelOfficialDemo : public ImGuiPanel
{
  bool show_demo_window = true;

public:
  void draw() override
  {
    if (show_demo_window)
      ImGui::ShowDemoWindow(&show_demo_window);
  }
};

inline void initImGUI(SDL_Window* window, SDL_GLContext gl_context)
{

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO();
  (void)io;
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls

  ImGui::StyleColorsDark();
  // ImGui::StyleColorsLight();

  // Setup Platform/Renderer backends
  ImGui_ImplSDL2_InitForOpenGL(window, gl_context);

  const char* glsl_version = "#version 130";
  ImGui_ImplOpenGL3_Init(glsl_version);
}

inline void processEventImGUI(const SDL_Event& event)
{
  ImGui_ImplSDL2_ProcessEvent(&event);
}

inline void beginImGUIFrame()
{
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplSDL2_NewFrame();
  ImGui::NewFrame();
}

inline void drawImGUIFrame(ImGuiPanel* panel)
{
  if (panel)
    panel->draw();
}

inline void endImGUIFrame()
{
  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

inline void shutdownImGUI()
{
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplSDL2_Shutdown();
  ImGui::DestroyContext();
}

// NOLINTEND

// #endif
