/**
* @file EventScript.cpp
*/
#define _CRT_SECURE_NO_WARNINGS
#include "EventScript.h"
#include "GLFWEW.h"
#include <fstream>
#include <sstream>
#include <iostream>

/**
*
*/
EventScriptEngine& EventScriptEngine::Instance()
{
  static EventScriptEngine instance;
  return instance;
}

/**
*
*/
bool EventScriptEngine::Init(int maxVariableCount)
{
  variables.resize(maxVariableCount, 0);
  filename.clear();
  script.clear();
  waitForInput = false;
  if (!textWindow.Init("Res/TextWindow.tga", glm::vec2(0, -248), glm::vec2(48, 32), glm::vec2(0))) {
    std::cerr << "[エラー]" << __func__ << ":スクリプトエンジンの初期化に失敗.\n";
    return false;
  }
  return true;
}

/**
*
*/
bool EventScriptEngine::Execute(const char* filename)
{
  std::ifstream ifs(filename);
  if (!ifs) {
    std::cerr << "[エラー]" << __func__ << ":スクリプトファイル" << filename << "をロードできません.\n";
    return false;
  }
  this->filename = filename;
  std::stringstream ss;
  ss << ifs.rdbuf();
  std::string tmp = ss.str();
  setlocale(LC_CTYPE, "japanese");
  const size_t size = mbstowcs(nullptr, tmp.c_str(), 0);
  script.resize(size + 1);
  mbstowcs(&script[0], tmp.c_str(), size);

  waitForInput = true;
  textWindow.Open(script.c_str());

  std::cout << "[INFO]" << __func__ << ":スクリプトファイル" << filename << "をロード.\n";
  return true;
}

/**
*
*/
void EventScriptEngine::Update(float deltaTime)
{
  textWindow.ProcessInput();
  if (textWindow.IsFinished()) {
    const GamePad gamepad = GLFWEW::Window::Instance().GetGamePad();
    if (gamepad.buttonDown & (GamePad::A | GamePad::B | GamePad::START)) {
      textWindow.Close();
      waitForInput = false;
    }
  }
  textWindow.Update(deltaTime);
}

/**
*
*/
void EventScriptEngine::Draw()
{
  textWindow.Draw();
}

/**
*
*/

void EventScriptEngine::SetVariable(int no, double value)
{
  if (no < 0 || no >= static_cast<int>(variables.size())) {
    return;
  }
  variables[no] = value;
}

/**
*
*/
double EventScriptEngine::GetVariable(int no) const
{
  if (no < 0 || no >= static_cast<int>(variables.size())) {
    return 0;
  }
  return variables[no];
}

/**
*
*/
bool EventScriptEngine::IsFinished() const
{
  return !waitForInput;
}

