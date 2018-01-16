#ifndef LENGINE_DEBUG_CONSOLE
#define LENGINE_DEBUG_CONSOLE

#include "../gui/imgui_LEngine.h"
#include "../gui/imgui.h"
#include "../Exceptions.h"

#include <vector>
#include <list>
#include <string>
#include <functional>

class Console {
  /// Takes an input command string and returns the result
  typedef std::function<std::string(const std::string&)> Callback;
  struct Record {
    Record(const std::string& t, ImColor c);
    std::string text;
    ImColor color;
  };

 public:
  Console(Callback processor);
  ~Console();

  /// Will run a command
  void RunCommand(const std::string& command);

  /// Will render a window with the title passed
  /// a pointer is passed to a bool, so that the window can close itself
  void Render(const std::string& title, bool* p_open);
  void ClearOutput();

 private:
  const int BUFFER_SIZE = 512;
  void AddLog(const std::string& txt, ImColor color);
  int TextEditCallback(ImGuiTextEditCallbackData* data);
  char mInputBuffer[512];
  Callback mProcessor;
  std::vector<Record> mOutput;

  std::list<std::string> mCommandHistory;
  std::list<std::string>::iterator mHistoryPos;
  bool mHistoryUnset;

  static ImColor COLOR_CMD;
  static ImColor COLOR_ERROR;
  static ImColor COLOR_OUTPUT;
};

#endif
