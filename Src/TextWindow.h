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

  bool Init(const char* imagePath, const glm::vec2& position,
    const glm::vec2& textAreaMargin, const glm::vec2& textAreaOffset);
  void Update(float deltaTime);
  void Draw();

  void Open(const wchar_t*);
  void SetText(const wchar_t*);
  void Close();
  bool IsFinished() const;
  bool IsOpen() const { return isOpen; }

private:
  glm::vec2 position = glm::vec3(0); // ウィンドウ左上の位置.
  glm::vec2 textAreaMargin = glm::vec2(8);
  glm::vec2 textAreaOffset = glm::vec2(0);

  std::wstring text; // 表示するテキスト.
  int outputCount = 0; // 出力済みの文字数.
  bool isOpen = false; // ウィンドウが開いていたらtrue.
  float interval = 0.01f; // 文字の表示間隔(秒).
  float outputTimer = 0; // 文字表示タイマー(秒).

  FontRenderer fontRenderer;
  SpriteRenderer spriteRenderer;
  Sprite sprBackground;
};

#endif // TEXTWINDOW_H_INCLUDED