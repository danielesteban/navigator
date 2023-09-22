#include "core/http.hpp"
#include "core/script.hpp"
#include "core/vm.hpp"
#include "core/window.hpp"

int main(int argc, char* argv[]) {
  WindowContext ctx;
  ImGuiIO& io = ImGui::GetIO();
  GLfloat lastTick = 0.0;
  HTTP http;
  Script script(&http);
  VM vm(&http, &ctx);
  GLFWwindow *window = ctx.window;

  if (argc == 2) {
    std::string url = argv[1];
    if (!HTTP::isValidURL(url)) {
      url = "file://" + url;
    }
    ctx.setURL(url);
  } else {
    ctx.setURL("about://welcome");
  }

  glfwShowWindow(window);
  while (!glfwWindowShouldClose(window)) {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    for (; !ctx.events.empty(); ctx.events.pop_front()) {
      const WindowEvent& event = ctx.events.front();
      switch (event.type) {
        case WINDOW_EVENT_RELOAD:
          script.reload();
          break;
        case WINDOW_EVENT_SAVE_EDITOR:
          if (script.loadFromEditor()) {
            vm.load(script.getSource());
          }
          break;
        case WINDOW_EVENT_SET_URL:
          script.setURL(event.data, event.flags);
          break;
      }
    }

    http.update();
    if (script.update()) {
      vm.load(script.getSource());
    }
    vm.loop();

    ctx.mouse.buttons.primaryDown = ctx.mouse.buttons.primaryUp = ctx.mouse.buttons.secondaryDown = ctx.mouse.buttons.secondaryUp = false;
    ctx.mouse.movement = glm::vec2(0.0, 0.0);
    ctx.mouse.wheel = 0.0;

    GLfloat consoleHeight = 0.0;
    if (ctx.showUI && !ctx.mouse.locked) {
      script.addressBar();

      if (!vm.errors.empty() || !vm.messages.empty()) {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 8.0);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(16.0, 16.0));
        ImGui::Begin("##console", nullptr, ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0, 0.0, 0.0, 1.0));
        for (const auto& text : vm.errors) {
          ImGui::Text("%s", text.c_str());
        }
        ImGui::PopStyleColor();
        for (const auto& message : vm.messages) {
          ImGui::Markdown(message.c_str(), message.length(), ctx.markdown);
        }
        consoleHeight = fmin(ImGui::GetCursorPos().y + 16.0, 144.0);
        ImGui::SetWindowPos(ImVec2(8.0, io.DisplaySize.y - consoleHeight - 8.0));
        ImGui::SetWindowSize(ImVec2(io.DisplaySize.x - 16.0, consoleHeight));
        ImGui::End();
        ImGui::PopStyleVar(2);
      }
    }

    if (!vm.tooltips.empty()) {
      ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 4.0);
      for (int i = 0; !vm.tooltips.empty(); vm.tooltips.pop_front(), i++) {
        const VMTooltip& tooltip = vm.tooltips.front();
        ImGui::SetNextWindowPos(ImVec2(
          tooltip.position.x * io.DisplaySize.x - 50.0 + tooltip.offset.x,
          tooltip.position.y * io.DisplaySize.y - 12.0 + tooltip.offset.y
        ));
        ImGui::SetNextWindowSize(ImVec2(100.0, 24.0));
        ImGui::Begin(("##tooltip" + std::to_string(i + 1)).c_str(), nullptr, ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar);
        const ImVec2 s = ImGui::CalcTextSize(tooltip.message.c_str());
        const ImVec2 p = ImGui::GetCursorPos();
        ImGui::SetCursorPos(ImVec2(p.x + (84.0 - s.x) * 0.5, p.y));
        ImGui::Text("%s", tooltip.message.c_str());
        ImGui::End();
      }
      ImGui::PopStyleVar();
    }

    if (ctx.showEditor && !ctx.mouse.locked) {
      ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 8.0);
      ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0, 0.0));
      ImGui::SetNextWindowBgAlpha(0.8);
      const GLfloat y = ctx.showUI ? 46.0 : 8.0;
      const GLfloat height = io.DisplaySize.y - (consoleHeight > 0.0 ? (consoleHeight + 8.0) : 0.0) - y - 8.0;
      ImGui::SetNextWindowPos(ImVec2(8.0, y));
      ImGui::SetNextWindowSize(ImVec2(512, 0), ImGuiCond_Once);
      ImGui::SetNextWindowSizeConstraints(ImVec2(fmin(io.DisplaySize.x - 16, 256), height), ImVec2(io.DisplaySize.x - 16, height));
      ImGui::Begin("##editor", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoTitleBar);
      script.editor.Render("TextEditor");
      ImGui::End();
      ImGui::PopStyleVar(2);
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  ALCcontext* audioCtx = alcGetCurrentContext();
  if (audioCtx != nullptr) {
    ALCdevice* audioDevice = alcGetContextsDevice(audioCtx);
    alcMakeContextCurrent(nullptr);
    alcDestroyContext(audioCtx);
    if (audioDevice != nullptr) {
      alcCloseDevice(audioDevice);
    }
  }

  glfwDestroyWindow(window);
  glfwTerminate();
  exit(EXIT_SUCCESS);
}
