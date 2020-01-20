/**
* @file TextWindow.cpp
*/
#include "TextWindow.h"
#include "GLFWEW.h"
#include <iostream>

/**
* テキストウィンドウを初期化する.
*
* @param imagePath      ウィンドウの画像ファイル名.
* @param position       ウィンドウの表示位置(ドット).
* @param textAreaMargin ウィンドウサイズとテキスト表示領域の間隔(ドット).
* @param textAreaOffset テキスト表示領域の位置をずらす値(ドット).
*
* @retval true  初期化成功.
* @retval false 初期化失敗.
*/
bool TextWindow::Init(const char* imagePath, const glm::vec2& position,
  const glm::vec2& textAreaMargin, const glm::vec2& textAreaOffset)
{
  this->position = position;
  this->textAreaOffset = textAreaOffset;
  this->textAreaMargin = textAreaMargin;
  sprBackground = Sprite(Texture::Image2D::Create(imagePath));
  if (!spriteRenderer.Init(100, "Res/Sprite.vert", "Res/Sprite.frag")) {
    std::cerr << "[エラー]" << __func__ << ": テキストウィンドウの初期化に失敗.\n";
    return false;
  }
  if (!fontRenderer.Init(1000)) {
    std::cerr << "[エラー]" << __func__ << ": テキストウィンドウの初期化に失敗.\n";
    return false;
  }
  if (!fontRenderer.LoadFromFile("Res/font.fnt")) {
    std::cerr << "[エラー]" << __func__ << ": テキストウィンドウの初期化に失敗.\n";
    return false;
  }
  return true;
}

/**
* テキストウィンドウの状態を更新する.
*
* @param deltaTime  前回の更新からの経過時間(秒).
*/
void TextWindow::Update(float deltaTime)
{
  if (!isOpen) {
    return;
  }

  // 背景画像の更新.
  sprBackground.Position(glm::vec3(position, 0));
  spriteRenderer.BeginUpdate();
  spriteRenderer.AddVertices(sprBackground);
  spriteRenderer.EndUpdate();

  // すべての文字が表示済み(outputCountがテキスト文字数以上)なら
  // フォントを更新する必要はない.
  if (outputCount >= static_cast<int>(text.size())) {
    return;
  }

  // 文字表示間隔が0より大きいなら、1文字ずつ表示する.
  // 0以下なら一気に全部を表示する.
  if (interval > 0) {
    outputTimer += deltaTime;
    const int c = static_cast<int>(outputTimer / interval);
    if (c == 0) {
      return;
    }
    outputCount += c;
    outputTimer -= static_cast<float>(c) * interval;
  } else {
    outputCount = text.size();
  }

  // 文章表示領域のサイズを計算.
  const Texture::Image2DPtr tex = sprBackground.Texture();
  const glm::vec2 windowSize = glm::vec2(tex->Width(), tex->Height());
  const glm::vec2 textAreaSize = windowSize - textAreaMargin * 2.0f;

  // 現在までに出力するべき文字を行単位で取得.
  std::vector<std::wstring> rowList;
  float lineWidth = 0; // 行の横幅.
  int outputOffset = 0; // 処理済みの文字数.
  for (int i = 0; i < outputCount; ++i) {
    lineWidth += fontRenderer.XAdvance(text[i]);
    // 改行文字か、行の幅がテキスト表示領域の幅を超えたら1行表示して改行.
    if (text[i] == L'\n' || lineWidth > textAreaSize.x) {
      rowList.push_back(text.substr(outputOffset, i - outputOffset));
      outputOffset = i;
      lineWidth = 0;
    }
  }
  // 最後の行に文字が残っていたら、それを表示.
  if (outputOffset < outputCount) {
    rowList.push_back(text.substr(outputOffset, outputCount - outputOffset));
  }

  // テキスト領域のパラメーターを考慮して、最初の文字の位置(offset)を計算する.
  glm::vec2 offset = textAreaSize * glm::vec2(-0.5f, 0.5f);
  offset += textAreaOffset;
  offset.y -= fontRenderer.LineHeight();
  offset += position;

  // 表示開始行を決める.
  const int maxLines = static_cast<int>(textAreaSize.y / fontRenderer.LineHeight());
  int startLine = static_cast<int>(rowList.size()) - maxLines;
  if (startLine < 0) {
    startLine = 0;
  }

  // 文章を表示.
  fontRenderer.BeginUpdate();
  fontRenderer.Color(glm::vec4(0, 0, 0, 1));
  for (int i = startLine; i < static_cast<int>(rowList.size()); ++i) {
    fontRenderer.AddString(glm::vec3(offset, 0), rowList[i].c_str());
    offset.y -= fontRenderer.LineHeight();
  }
  fontRenderer.EndUpdate();
}

/**
* テキストウィンドウを描画する.
*/
void TextWindow::Draw()
{
  if (isOpen) {
    const GLFWEW::Window& window = GLFWEW::Window::Instance();
    const glm::vec2 screenSize(window.Width(), window.Height());
    spriteRenderer.Draw(screenSize);
    fontRenderer.Draw(screenSize);
  }
}

/**
* テキストウィンドウを開く.
*
* @param str ウィンドウに表示する文章.
*/
void TextWindow::Open(const wchar_t* str)
{
  SetText(str);
  isOpen = true;
}

/**
* テキストを変更する.
*
* @param str ウィンドウに表示する文章.
*/
void TextWindow::SetText(const wchar_t* str)
{
  text = str;
  outputCount = 0;
  outputTimer = 0;
}

/**
* テキストウィンドウを閉じる.
*/
void TextWindow::Close()
{
  text.clear();
  outputCount = 0;
  outputTimer = 0;
  isOpen = false;
}

/**
* テキストの表示が完了したか調べる.
*
* @retval true  すべて文字を表示した.
* @retval false まだ表示していない文字がある.
*/
bool TextWindow::IsFinished() const
{
  return isOpen && (outputCount >= static_cast<int>(text.size()));
}

