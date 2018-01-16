#include "Console.h"
#include "SDL2/SDL.h"

Console::Record::Record(const std::string& t, ImColor c) : text(t), color(c) {}

ImColor Console::COLOR_CMD = ImColor(128, 255, 128);
ImColor Console::COLOR_ERROR = ImColor(255, 128, 128);
ImColor Console::COLOR_OUTPUT = ImColor(255, 255, 255);

Console::Console(Console::Callback processor) : mProcessor(processor) {
  for (auto i = 0; i != BUFFER_SIZE; i++) {
    mInputBuffer[i] = '\0';
  }
}
Console::~Console() {}

void Console::RunCommand(const std::string& command) {
  mCommandHistory.push_back(command);
  AddLog(command, COLOR_CMD);

  ImColor color;
  std::string output;
  try {
    output = mProcessor(command);
    color = COLOR_OUTPUT;
  } catch (LEngineException e) {
    output = e.what();
    color = COLOR_ERROR;
  }
  AddLog(output, color);
}

void Console::ClearOutput() { mOutput.clear(); }

void Console::AddLog(const std::string& txt, ImColor color) {
  mOutput.push_back(Record(txt, color));
}

void Console::Render(const std::string& title, bool* p_open) {
  ImGui::SetNextWindowSize(ImVec2(520, 600), ImGuiSetCond_FirstUseEver);
  if (!ImGui::Begin(title.c_str(), p_open)) {
    ImGui::End();
    return;
  }
  //SDL_StartTextInput();

  // As a specific feature guaranteed by the library, after calling Begin()
  // the last Item represent the title bar. So e.g. IsItemHovered() will
  // return true when hovering the title bar.
  // Here we create a context menu only available from the title bar.
  /*
  if (ImGui::BeginPopupContextItem()) {
    if (ImGui::MenuItem("Close")) {
      *p_open = false;
    }
    ImGui::EndPopup();
  }
  */

  ImGui::TextWrapped(
      "Enter 'HELP' for help, press TAB to use text completion.");

  if (ImGui::SmallButton("Add Dummy Text")) {
    AddLog("%d some text", COLOR_CMD);
    AddLog("some more text", COLOR_CMD);
    AddLog("display very important message here!", COLOR_CMD);
  }
  ImGui::SameLine();
  if (ImGui::SmallButton("Add Dummy Error")) {
    AddLog("[error] something went wrong", COLOR_ERROR);
  }
  ImGui::SameLine();
  if (ImGui::SmallButton("Clear")) {
    ClearOutput();
  }
  ImGui::SameLine();
  bool copy_to_clipboard = ImGui::SmallButton("Copy");
  ImGui::SameLine();
  bool ScrollToBottom = ImGui::SmallButton("Scroll to bottom");
  // static float t = 0.0f; if (ImGui::GetTime() - t > 0.02f) { t =
  // ImGui::GetTime(); AddLog("Spam %f", t); }

  ImGui::Separator();

  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
  static ImGuiTextFilter filter;
  filter.Draw("Filter (\"incl,-excl\") (\"error\")", 180);
  ImGui::PopStyleVar();
  ImGui::Separator();

  const float footer_height_to_reserve =
      ImGui::GetStyle().ItemSpacing.y +
      // ImGui::GetFrameHeightWithSpacing();  // 1 separator, 1 input text
      ImGui::BeginChild(
          "ScrollingRegion", ImVec2(0, -footer_height_to_reserve), false,
          ImGuiWindowFlags_HorizontalScrollbar);  // Leave room for 1 separator
                                                  // +
                                                  // 1 InputText
  if (ImGui::BeginPopupContextWindow()) {
    if (ImGui::Selectable("Clear")) {
      ClearOutput();
    }
    ImGui::EndPopup();
  }

  // Display every line as a separate entry so we can change their color or
  // add custom widgets. If you only want raw text you can use
  // ImGui::TextUnformatted(log.begin(), log.end());
  // NB- if you have thousands of entries this approach may be too inefficient
  // and may require user-side clipping to only process visible items.
  // You can seek and display only the lines that are visible using the
  // ImGuiListClipper helper, if your elements are evenly spaced and you have
  // cheap random access to the elements.
  // To use the clipper we could replace the 'for (int i = 0; i < mOutput.Size;
  // i++)' loop with:
  //     ImGuiListClipper clipper(mOutput.Size);
  //     while (clipper.Step())
  //         for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
  // However take note that you can not use this code as is if a filter is
  // active because it breaks the 'cheap random-access' property. We would
  // need random-access on the post-filtered list.
  // A typical application wanting coarse clipping and filtering may want to
  // pre-compute an array of indices that passed the filtering test,
  // recomputing this array when user changes the filter,
  // and appending newly elements as they are inserted. This is left as a task
  // to the user until we can manage to improve this example code!
  // If your items are of variable size you may want to implement code similar
  // to what ImGuiListClipper does. Or split your data into fixed height items
  // to allow random-seeking into your list.
  ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,
                      ImVec2(4, 1));  // Tighten spacing
  if (copy_to_clipboard) {
    // SDL_SetClipboardText(text.c_str());
  }
  for (auto record : mOutput) {
    ImGui::PushStyleColor(ImGuiCol_Text, record.color);
    ImGui::TextUnformatted(record.text.c_str());
    ImGui::PopStyleColor();
  }
  if (copy_to_clipboard) {
    ImGui::LogFinish();
  }
  if (ScrollToBottom) {
    ImGui::SetScrollHere();
  }
  ScrollToBottom = false;
  ImGui::PopStyleVar();
  ImGui::EndChild();
  ImGui::Separator();

  // Command-line
  if (ImGui::InputText("Input", mInputBuffer, BUFFER_SIZE,
                       // ImGuiInputTextFlags_CallbackHistory |
                       // ImGuiInputTextFlags_CallbackCompletion |
                       ImGuiInputTextFlags_EnterReturnsTrue, NULL, NULL)) {
    char* input_end = mInputBuffer + strlen(mInputBuffer);
    while (input_end > mInputBuffer && input_end[-1] == ' ') {
      input_end--;
    }
    *input_end = 0;
    if (mInputBuffer[0]) {
      RunCommand(std::string(mInputBuffer));
    }
    strcpy(mInputBuffer, "");
  }

  // Demonstrate keeping auto focus on the input box
  if (ImGui::IsItemHovered() ||
      (ImGui::IsWindowFocused() && !ImGui::IsAnyItemActive() &&
       !ImGui::IsMouseClicked(0))) {
    ImGui::SetKeyboardFocusHere(-1);  // Auto focus previous widget
  }

  ImGui::End();
}

int Console::TextEditCallback(ImGuiTextEditCallbackData* data) {
  return 0;
}
