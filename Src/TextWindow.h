/**
* @file TextWindow.h
*/
#ifndef TEXTWINDOW_H_INCLUDED
#define TEXTWINDOW_H_INCLUDED
#include "Sprite.h"
#include "Font.h"
#include <string>

/**
* テキスト表示用ウィンドウ.
*
* 指定されたテキストを表示する.
*/
class TextWindow
{
public:
  TextWindow() = default;
  ~TextWindow() = default;

  bool Init(const char* imagePath, const glm::vec2& scale, const glm::vec2& textAreaOffset, const glm::vec2& textAreaSize);
  void ProcessInput();
  void Update(float deltaTime);
  void Draw();

  void Open(const wchar_t*);
  void Close();

private:
  glm::vec3 position = glm::vec3(0); // ウィンドウ左上の位置.
  glm::vec2 scale = glm::vec2(1); // ウィンドウのサイズ.
  glm::vec2 textAreaOffset = glm::vec2(-0.1f, -0.2f);
  glm::vec2 textAreaSize = glm::vec2(0.8f);

  std::wstring text; // 表示するテキスト.
  int outputCount = 0; // 出力済みの文字数.
  bool isOpen = false; // ウィンドウが開いていたらtrue.
  bool waitForInput = false; // 入力待ちならtrue. 

  float interval = 0.1f; // 文字の表示間隔(秒).
  float outputTimer = 0; // 文字表示タイマー(秒).

  SpriteRenderer spriteRenderer;
  Sprite sprBackground;

  FontRenderer fontRenderer;
};

#endif // TEXTWINDOW_H_INCLUDED