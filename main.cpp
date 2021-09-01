#define GLFW_EXPOSE_NATIVE_WIN32
#include "windows_swca.h"
#include <AcrylicCompositor.h>
#include <GL/glew.h>
#include <glfwpp/glfwpp.h>
#include <glfwpp/native.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <iostream>
#include <tuple>

void cleanupImgui() {
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
}

template <typename Func> void renderImgui(Func &&guiRenderFunc_) {
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();

  guiRenderFunc_();

  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

  if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
    glfw::Window &backupCurrentContext = glfw::getCurrentContext();
    ImGui::UpdatePlatformWindows();
    ImGui::RenderPlatformWindowsDefault();
    glfw::makeContextCurrent(backupCurrentContext);
  }
}

void initImgui(const glfw::Window &wnd) {
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
  io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

  ImGuiStyle &style = ImGui::GetStyle();
  style.WindowRounding = 0.0f;
  style.Colors[ImGuiCol_WindowBg].w = 1.0f;

  ImGui_ImplGlfw_InitForOpenGL(wnd, true);
  ImGui_ImplOpenGL3_Init();
}

auto createAcrylicBackdrop(glfw::Window &source) {
  auto [width, height] = source.getSize();
  glfw::WindowHints{.decorated = false, .transparentFramebuffer = true}.apply();
  static glfw::Window backdrop{width, height, ""};

  static AcrylicCompositor compositor{0};

  AcrylicCompositor::AcrylicEffectParameter param;
  param.blurAmount = 40;
  param.saturationAmount = 2;
  param.tintColor = D2D1::ColorF(0.0f, 0.0f, 0.0f, .30f);
  param.fallbackColor = D2D1::ColorF(0.10f, 0.10f, 0.10f, 1.0f);

  compositor.SetAcrylicEffect(glfw::native::getWin32Window(backdrop),
                              AcrylicCompositor::BACKDROP_SOURCE_HOSTBACKDROP,
                              param);

  source.posEvent.setCallback([&](glfw::Window &source, int x, int y) {
    backdrop.setPos(x, y);
    compositor.Sync(glfw::native::getWin32Window(backdrop), WM_WINDOWPOSCHANGED,
                    0, 0, false);
  });
  source.sizeEvent.setCallback([&](glfw::Window &source, int x, int y) {
    backdrop.setSize(x, y);
    compositor.Sync(glfw::native::getWin32Window(backdrop), WM_WINDOWPOSCHANGED,
                    0, 0, false);
  });
  source.focusEvent.setCallback([&](glfw::Window &source, bool active) {
    compositor.Sync(glfw::native::getWin32Window(backdrop), WM_ACTIVATE, 0, 0,
                    active);
  });
  source.closeEvent.setCallback([&](glfw::Window &source) {
    compositor.Sync(glfw::native::getWin32Window(backdrop), WM_CLOSE, 0, 0, 0);
  });

  return std::tuple{std::ref(backdrop), std::ref(compositor)};
}

int main() {
  [[maybe_unused]] glfw::GlfwLibrary library = glfw::init();

  glfw::WindowHints hints;
  hints.clientApi = glfw::ClientApi::OpenGl;
  hints.contextVersionMajor = 4;
  hints.contextVersionMinor = 6;
  hints.transparentFramebuffer = true;
  hints.decorated = false;
  hints.apply();
  glfw::Window wnd(800, 600, "GLFWPP ImGui integration example");

  acrylic_swca(glfw::native::getWin32Window(wnd));

  glfw::makeContextCurrent(wnd);
  if (glewInit() != GLEW_OK) {
    throw std::runtime_error("Could not initialize GLEW");
  }

  wnd.mouseButtonEvent.setCallback([&](glfw::Window &wnd, glfw::MouseButton,
                                       glfw::MouseButtonState,
                                       glfw::ModifierKeyBit) {

  });

  initImgui(wnd);

  ImGuiIO &io = ImGui::GetIO();
  io.Fonts->AddFontFromFileTTF("../../res/fonts/Roboto-Regular.ttf", 14);

  while (!wnd.shouldClose()) {
    glClear(GL_COLOR_BUFFER_BIT);

    renderImgui([&]() {
      ImGui::DockSpaceOverViewport(ImGui::GetMainViewport(),
                                   ImGuiDockNodeFlags_PassthruCentralNode);

      if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
          ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Edit")) {
          if (ImGui::MenuItem("Undo", "CTRL+Z")) {
          }
          if (ImGui::MenuItem("Redo", "CTRL+Y", false, false)) {
          } // Disabled item
          ImGui::Separator();
          if (ImGui::MenuItem("Cut", "CTRL+X")) {
          }
          if (ImGui::MenuItem("Copy", "CTRL+C")) {
          }
          if (ImGui::MenuItem("Paste", "CTRL+V")) {
          }
          ImGui::EndMenu();
        }

        {
          static bool freezeButtons = false;

          if (freezeButtons) {
            freezeButtons = false;
          } else {
            const float ItemSpacing = 0;

            static float HostButtonWidth =
                100.0f; // The 100.0f is just a guess size for the first frame.
            float pos = HostButtonWidth + ItemSpacing + 5;
            // ImGui::PushStyleColor(ImGuiColactive_)
            ImGui::SameLine(ImGui::GetWindowWidth() - pos);
            if (ImGui::BeginMenu("X")) {
              ImGui::EndMenu();
              wnd.setShouldClose(true);
              freezeButtons = true;
            }
            HostButtonWidth = ImGui::GetItemRectSize()
                                  .x; // Get the actual width for next frame.

            static float ClientButtonWidth = 100.0f;
            pos += ClientButtonWidth + ItemSpacing;
            ImGui::SameLine(ImGui::GetWindowWidth() - pos);
            if (ImGui::BeginMenu("[  ]")) {
              ImGui::EndMenu();
              static bool maximized = false;
              if (maximized) {
                wnd.restore();
                maximized = false;
              } else {
                wnd.maximize();
                maximized = true;
              }
              freezeButtons = true;
            }
            ClientButtonWidth = ImGui::GetItemRectSize()
                                    .x; // Get the actual width for next frame.

            static float LocalButtonWidth = 100.0f;
            pos += LocalButtonWidth + ItemSpacing;
            ImGui::SameLine(ImGui::GetWindowWidth() - pos);
            if (ImGui::BeginMenu("_")) {
              ImGui::EndMenu();
              wnd.iconify();
              freezeButtons = true;
            }
            LocalButtonWidth = ImGui::GetItemRectSize()
                                   .x; // Get the actual width for next frame.
          }
        }

        ImGui::EndMainMenuBar();
      }

      const int op_resize_top = 1, op_resize_bottom = 2, op_resize_left = 4,
                op_resize_right = 8, op_move = 16;
      bool on_title_bar = false;
      static int current_op = 0;
      {
        static auto [last_mouse_x, last_mouse_y] = wnd.getCursorPos();
        const auto [mouse_x, mouse_y] = wnd.getCursorPos();
        const auto mouse_dx = mouse_x - last_mouse_x,
                   mouse_dy = mouse_y - last_mouse_y;

        auto [left_edge, top_edge] = wnd.getPos();
        auto [width, height] = wnd.getSize();
        auto right_edge = left_edge + width, bottom_edge = top_edge + height;

        int dst_to_left_edge = abs(mouse_x);
        int dst_to_right_edge = abs(width - dst_to_left_edge);
        int dst_to_top_edge = abs(mouse_y);
        int dst_to_bottom_edge = abs(height - dst_to_top_edge);

        // ImGui::Begin("Min distances");
        // ImGui::Value("Dst to left", dst_to_left_edge);
        // ImGui::Value("Dst to right", dst_to_right_edge);
        // ImGui::Value("Dst to top", dst_to_top_edge);
        // ImGui::Value("Dst to bottom", dst_to_bottom_edge);
        // ImGui::End();

        // ImDrawList *draw_list = ImGui::GetWindowDrawList();
        // draw_list->AddRectFilled({float(left_edge), float(top_edge)},
        //                          {float(right_edge), float(bottom_edge)},
        //                          0xFFFFFFFF);

        auto min_dst = min(min(dst_to_left_edge, dst_to_right_edge),
                           min(dst_to_top_edge, dst_to_bottom_edge));

        const int border_size = 10;
        const int top_border_size = 10;

        if (dst_to_top_edge < 20) {
          on_title_bar = true;
        }
        if (current_op == 0 && wnd.getMouseButton(glfw::MouseButton::Left)) {
          if (dst_to_left_edge < border_size) {
            current_op |= op_resize_left;
          }
          if (dst_to_right_edge < border_size) {
            current_op |= op_resize_right;
          }
          if (dst_to_bottom_edge < border_size) {
            current_op |= op_resize_bottom;
          }
          if (dst_to_top_edge < top_border_size) {
            current_op |= op_resize_top;
          }
        }

        const auto [wnd_width, wnd_height] = wnd.getSize();
        const auto [wnd_x, wnd_y] = wnd.getPos();
        if (current_op & op_resize_bottom) {
          wnd.setSize(wnd_width, wnd_height + mouse_dy);
        }
        if (current_op & op_resize_right) {
          wnd.setSize(wnd_width + mouse_dx, wnd_height);
        }

        last_mouse_x = mouse_x;
        last_mouse_y = mouse_y;
      }

      bool notFocused = !ImGui::GetIO().WantCaptureMouse || on_title_bar;
      if (notFocused && ImGui::IsMouseDragging(0)) {
        auto [x, y] = wnd.getPos();
        auto [dx, dy] = ImGui::GetMouseDragDelta();
        wnd.setPos(x + dx, y + dy);
        ImGui::ResetMouseDragDelta();
      }
      ImGui::ShowDemoWindow();
    });

    glfw::pollEvents();
    wnd.swapBuffers();
  }

  cleanupImgui();
}