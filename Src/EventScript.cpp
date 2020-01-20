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
* スクリプトエンジンのシングルトン・インスタンスを取得する.
*
* @return スクリプトエンジンのシングルトン・インスタンス.
*/
EventScriptEngine& EventScriptEngine::Instance()
{
  static EventScriptEngine instance;
  return instance;
}

/**
* スクリプトエンジンを初期化する.
*
* @retval true  初期化成功.
* @retval false 初期化失敗.
*/
bool EventScriptEngine::Init(int maxVariableCount)
{
  if (isInitialized) {
    std::cerr << "[エラー] EventScriptEngineは既に初期化されています.\n";
    return false;
  }

  variables.resize(maxVariableCount, 0);
  filename.reserve(256);
  script.reserve(2048);
  if (!textWindow.Init("Res/TextWindow.tga", glm::vec2(0, -248), glm::vec2(48, 32), glm::vec2(0))) {
    std::cerr << "[エラー]" << __func__ << ":スクリプトエンジンの初期化に失敗.\n";
    return false;
  }
  isInitialized = true;
  return true;
}

/**
* イベント・スクリプトを実行する.
*
* @param filename スクリプト・ファイル名.
*
* @retval true  実行に成功.
* @retval false 実行に失敗.
*/
bool EventScriptEngine::RunScript(const char* filename)
{
  if (!isInitialized) {
    return false;
  }

  std::ifstream ifs(filename);
  if (!ifs) {
    std::cerr << "[エラー]" << __func__ << ":スクリプトファイル" << filename << "を読み込めません.\n";
    return false;
  }
  std::stringstream ss;
  ss << ifs.rdbuf();
  std::string tmp = ss.str();
  setlocale(LC_CTYPE, "ja-JP");
  const size_t size = mbstowcs(nullptr, tmp.c_str(), 0);
  script.resize(size + 1);
  mbstowcs(&script[0], tmp.c_str(), size);

  this->filename = filename;
  isFinished = false;
  textWindow.Open(script.c_str());

  std::cout << "[INFO]" << __func__ << ":スクリプトファイル" << filename << "を実行.\n";
  return true;
}

/**
* スクリプトエンジンの状態を更新する.
*
* @param deltaTime  前回の更新からの経過時間(秒).
*/
void EventScriptEngine::Update(float deltaTime)
{
  if (!isInitialized) {
    return;
  }

  if (textWindow.IsFinished()) {
    const GamePad gamepad = GLFWEW::Window::Instance().GetGamePad();
    if (gamepad.buttonDown & (GamePad::A | GamePad::B | GamePad::START)) {
      textWindow.Close();
      isFinished = true;
    }
  }
  textWindow.Update(deltaTime);
}

/**
* スクリプトエンジンを描画する.
*/
void EventScriptEngine::Draw()
{
  if (!isInitialized) {
    return;
  }

  textWindow.Draw();
}

/**
* スクリプト変数に値を設定する.
*
* @param no    変数番号(0～初期化時に設定した最大数).
* @param value 設定する値.
*/
void EventScriptEngine::SetVariable(int no, double value)
{
  if (no < 0 || no >= static_cast<int>(variables.size())) {
    return;
  }
  variables[no] = value;
}

/**
* スクリプト変数の値を取得する.
*
* @param no    変数番号(0～初期化時に設定した最大数).
*
* @return no番の変数に設定されている値.
*/
double EventScriptEngine::GetVariable(int no) const
{
  if (no < 0 || no >= static_cast<int>(variables.size())) {
    return 0;
  }
  return variables[no];
}

/**
* スクリプトの実行が完了したか調べる.
*
* @retval true  実行完了.
* @retval false 実行中、またはスクリプトが読み込まれていない.
*/
bool EventScriptEngine::IsFinished() const
{
  return isFinished;
}

