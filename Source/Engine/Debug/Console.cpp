#include "Console.h"
#include "SDL2/SDL.h"
#include "../Kernel.h"

Console::Record::Record(const std::string& t, ImColor c) : text(t), color(c) {}

ImColor Console::COLOR_CMD = ImColor(128, 255, 128);
ImColor Console::COLOR_ERROR = ImColor(255, 128, 128);
ImColor Console::COLOR_OUTPUT = ImColor(255, 255, 255);

Console::Console(Console::Callback processor) : mProcessor(processor) {
  for (auto i = 0; i != BUFFER_SIZE; i++) {
    mInputBuffer[i] = '\0';
  }
  mHistoryUnset = true;
  auto sprName = Kernel::SYSTEM_SPRITE_NAME;
  mSystemSprite = K_SpriteMan.GetLoadItem(sprName, sprName);
}
Console::~Console() {}

void Console::RunCommand(const std::string& command) {
  mCommandHistory.push_front(command);
  if (mCommandHistory.size() > 255) {
    mCommandHistory.pop_back();
  }
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

void Console::AddLog(const std::string& txt, Vec4 color) {
  AddLog(txt, ImColor(color.x, color.y, color.z, color.w));
}
void Console::AddLog(const std::string& txt, ImColor color) {
  mOutput.push_back(Record(txt, color));
}

void Console::Render(const std::string& title, bool* p_open) {
  ImGui::SetNextWindowSize(ImVec2(520, 600), ImGuiSetCond_FirstUseEver);
  if (!ImGui::Begin(title.c_str(), p_open)) {
    ImGui::End();
    return;
  }
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

  ImGui::PushStyleColorButton(Vec4(0, 0, 0, 0));
  ImGui::PushStyleColorButtonHovered(Vec4(0.75, 0.75, 1, 1));

  ImGui::PushID(1001);
  bool continuePressed = ImGui::SpriteButton(mSystemSprite, "Play", 0);
  ImGui::PopID();

  ImGui::SameLine();

  ImGui::PushID(1002);
  bool breakPressed = ImGui::SpriteButton(mSystemSprite, "Stop", 0);
  ImGui::PopID();

  ImGui::PopStyleColor(2);

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
  auto callback = [](ImGuiTextEditCallbackData* data) {
    Console* console = (Console*)data->UserData;
    return console->TextEditCallback(data);
  };
  if (ImGui::InputText("Input", mInputBuffer, BUFFER_SIZE,
                       ImGuiInputTextFlags_CallbackHistory |
                           // ImGuiInputTextFlags_CallbackCompletion |
                           ImGuiInputTextFlags_EnterReturnsTrue,
                       callback, this)) {
    char* input_end = mInputBuffer + strlen(mInputBuffer);
    mHistoryUnset = true;
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

  if (continuePressed) {
    Kernel::DebugResumeExecution();
  }
  if (breakPressed) {
    Kernel::DebugPauseExecution();
  }
}

int Console::TextEditCallback(ImGuiTextEditCallbackData* data) {
  switch (data->EventFlag) {
    case ImGuiInputTextFlags_CallbackHistory:
      if (mCommandHistory.size() == 0) {
        return 0;
      }

      bool changeInput = false;

      if (mHistoryUnset) {
        mHistoryPos = mCommandHistory.begin();
        mHistoryUnset = false;
        changeInput = true;
      } else {
        if (data->EventKey == ImGuiKey_UpArrow) {
          mHistoryPos++;
          changeInput = true;
          if (mHistoryPos == mCommandHistory.end()) {
            mHistoryPos--;
            changeInput = false;
          }
        } else if (data->EventKey == ImGuiKey_DownArrow) {
          if (mHistoryPos != mCommandHistory.begin()) {
            mHistoryPos--;
            changeInput = true;
          }
        }
      }

      if (changeInput) {
        data->CursorPos = data->SelectionStart = data->SelectionEnd =
            data->BufTextLen = (int)snprintf(data->Buf, (size_t)data->BufSize,
                                             "%s", mHistoryPos->c_str());
        data->BufDirty = true;
      }
  }
  return 0;
}
