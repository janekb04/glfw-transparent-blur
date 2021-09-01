#include <GL/glew.h>

#define GLFW_EXPOSE_NATIVE_WIN32
#include "gui.h"
#include "windows_swca.h"
#include <AcrylicCompositor.h>
#include <glfwpp/glfwpp.h>
#include <glfwpp/native.h>
#include <iostream>
#include <tuple>

glfw::Window createMainWindow() {
  glfw::WindowHints hints;
  hints.clientApi = glfw::ClientApi::OpenGl;
  hints.contextVersionMajor = 4;
  hints.contextVersionMinor = 6;
  hints.transparentFramebuffer = true;
  hints.decorated = false;
  hints.apply();
  glfw::Window wnd(800, 600, "GLFWPP ImGui integration example");

  wnd.framebufferSizeEvent.setCallback(
      [](const glfw::Window &wnd, int width, int height) {
        glViewport(0, 0, width, height);
      });

  acrylic_swca(glfw::native::getWin32Window(wnd));

  glfw::makeContextCurrent(wnd);
  if (glewInit() != GLEW_OK) {
    throw std::runtime_error("Could not initialize GLEW");
  }

  return wnd;
}

int main() {
  [[maybe_unused]] glfw::GlfwLibrary library = glfw::init();
  auto wnd = createMainWindow();

  initImgui(wnd);

  ImGuiIO &io = ImGui::GetIO();
  io.Fonts->AddFontFromFileTTF("../../res/fonts/Roboto-Regular.ttf", 14);

  while (!wnd.shouldClose()) {
    glClear(GL_COLOR_BUFFER_BIT);

    renderImgui([&]() {
      ImGui::DockSpaceOverViewport(ImGui::GetMainViewport(),
                                   ImGuiDockNodeFlags_PassthruCentralNode);

      if (ImGui::BeginMainMenuBar()) {
        {
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
        }

        {
          static bool freezeButtons = false;

          if (freezeButtons) {
            freezeButtons = false;
          } else {
            const float ItemSpacing = 0;

            static float HostButtonWidth = 100.0f;
            float pos = HostButtonWidth + ItemSpacing + 5;
            // ImGui::PushStyleColor(ImGuiColactive_)
            ImGui::SameLine(ImGui::GetWindowWidth() - pos);
            if (ImGui::BeginMenu("X")) {
              ImGui::EndMenu();
              wnd.setShouldClose(true);
              freezeButtons = true;
            }
            HostButtonWidth = ImGui::GetItemRectSize().x;

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
            ClientButtonWidth = ImGui::GetItemRectSize().x;

            static float LocalButtonWidth = 100.0f;
            pos += LocalButtonWidth + ItemSpacing;
            ImGui::SameLine(ImGui::GetWindowWidth() - pos);
            if (ImGui::BeginMenu("-")) {
              ImGui::EndMenu();
              wnd.iconify();
              freezeButtons = true;
            }
            LocalButtonWidth = ImGui::GetItemRectSize().x;
          }
        }

        ImGui::EndMainMenuBar();
      }

      {
        class WindowResizer {
          const int OP_RESIZE_LEFT = 1, OP_RESIZE_TOP = 2, OP_RESIZE_RIGHT = 4,
                    OP_RESIZE_BOTTOM = 8, OP_MOVE = 16;
          int state;
          glfw::Window &wnd;

          const int TITLE_BAR_SIZE = 15;
          const int BORDER_WIDTH = 5;
          const int MIN_HEIGHT = 20;
          const int MIN_WIDTH = 20;

        public:
          WindowResizer(glfw::Window &wnd) : state{0}, wnd{wnd} {}

          void next() {
            // Stop any op if no longer holding mouse
            if (!wnd.getMouseButton(glfw::MouseButton::Left)) {
              if (state == OP_MOVE) {
                state = 0;
              } else if (state != 0) { // resizing in some way
                state = 0;
                acrylic_swca(glfw::native::getWin32Window(wnd));
              }
            }

            const auto [mouse_x, mouse_y] = wnd.getCursorPos();
            const auto [mouse_dx, mouse_dy] = ImGui::GetMouseDragDelta();
            const auto [wnd_width, wnd_height] = wnd.getSize();

            // Potentially pick new current state
            {
              const int dst_to_left_edge = abs(mouse_x);
              const int dst_to_right_edge = abs(wnd_width - dst_to_left_edge);
              const int dst_to_top_edge = abs(mouse_y);
              const int dst_to_bottom_edge = abs(wnd_height - dst_to_top_edge);

              if (state == 0 && wnd.getMouseButton(glfw::MouseButton::Left)) {
                const bool on_title_bar = dst_to_top_edge < TITLE_BAR_SIZE;
                const bool in_drag_location =
                    !ImGui::GetIO().WantCaptureMouse || on_title_bar;
                if (in_drag_location && ImGui::IsMouseDragging(0)) {
                  state = OP_MOVE;
                } else {
                  if (dst_to_left_edge < BORDER_WIDTH) {
                    state |= OP_RESIZE_LEFT;
                  }
                  if (dst_to_right_edge < BORDER_WIDTH) {
                    state |= OP_RESIZE_RIGHT;
                  }
                  if (dst_to_bottom_edge < BORDER_WIDTH) {
                    state |= OP_RESIZE_BOTTOM;
                  }
                  if (dst_to_top_edge < BORDER_WIDTH) {
                    state |= OP_RESIZE_TOP;
                  }

                  if (state != 0) {
                    acrylic_swca_disable(glfw::native::getWin32Window(wnd));
                  }
                }
              }
            }

            const auto [wnd_x, wnd_y] = wnd.getPos();

            // Perform current op
            {
              if (state == OP_MOVE) {
                auto [x, y] = wnd.getPos();
                wnd.setPos(x + mouse_dx, y + mouse_dy);
              } else {
                if (state & OP_RESIZE_BOTTOM) {
                  const auto new_height = wnd_height + mouse_dy;
                  if (new_height >= MIN_HEIGHT)
                    wnd.setSize(wnd_width, new_height);
                }
                if (state & OP_RESIZE_RIGHT) {
                  const auto new_width = wnd_width + mouse_dx;
                  if (new_width >= MIN_WIDTH)
                    wnd.setSize(new_width, wnd_height);
                }
                if (state & OP_RESIZE_TOP) {
                  const auto new_height = wnd_height - mouse_dy;
                  if (new_height >= MIN_HEIGHT)
                    wnd.setSize(wnd_width, new_height);
                  wnd.setPos(wnd_x, wnd_y + mouse_dy);
                }
                if (state & OP_RESIZE_LEFT) {
                  const auto new_width = wnd_width - mouse_dx;
                  if (new_width >= MIN_WIDTH)
                    wnd.setSize(new_width, wnd_height);
                  wnd.setPos(wnd_x + mouse_dx, wnd_y);
                }
              }
            }

            if (state != 0)
              ImGui::ResetMouseDragDelta();
          }
        };

        static WindowResizer resizer{wnd};
        resizer.next();
      }

      ImGui::ShowDemoWindow();
    });

    glfw::pollEvents();
    wnd.swapBuffers();
  }

  cleanupImgui();
}