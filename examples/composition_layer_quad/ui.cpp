
#include "ui.h"

#include "vsgImGui/imgui.h"

SampleImGUIComponent::SampleImGUIComponent() {}

bool SampleImGUIComponent::operator()()
{


  ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
  ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
  
  ImGui::Begin("Test Window", &_windowOpen, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);

  // In VR text should be large
  ImGui::SetWindowFontScale(5.0f);

  ImGui::Text("Some useful message here.");
  ImGui::SliderFloat("float", &someFloatVal, 0.0f, 1.0f);

  if (ImGui::Button("Button"))
  {
    buttonPressed++;
  }

  ImGui::SameLine();
  ImGui::Text("counter = %d", buttonPressed);

  // TODO: vsgExamples/imgui uses the following to get fps, but for some reason that gives us only 60fps,
  //       when the app is definitely rendering at 90
  // ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

  ImGui::Text("Frame %u", _frameI++);
  ImGui::End();

  return true;
}
