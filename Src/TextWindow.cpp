/**
* @file TextWindow.cpp
*/
#include "TextWindow.h"
#include "GLFWEW.h"
#include <iostream>

/**
*
*/
bool TextWindow::Init(const char* imagePath, const glm::vec2& position, const glm::vec2& textAreaMargin, const glm::vec2& textAreaOffset)
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
*
*/
void TextWindow::ProcessInput()
{
  if (waitForInput) {
    const GamePad gamepad = GLFWEW::Window::Instance().GetGamePad();
    if (gamepad.buttonDown & GamePad::A) {
      waitForInput = false;
    }
  }
}

/**
*
*/
void TextWindow::Update(float deltaTime)
{
  if (!isOpen) {
    return;
  }

  sprBackground.Position(glm::vec3(position, 0));
  spriteRenderer.BeginUpdate();
  spriteRenderer.AddVertices(sprBackground);
  spriteRenderer.EndUpdate();

  if (outputCount > 0 && outputCount >= static_cast<int>(text.size())) {
    return;
  }
  if (waitForInput) {
    return;
  }

  if (interval > 0) {
    outputTimer += deltaTime;
    const int c = static_cast<int>(outputTimer / interval);
    outputCount += c;
    outputTimer -= static_cast<float>(c) * interval;
  } else {
    outputCount = text.size();
  }

  const Texture::Image2DPtr tex = sprBackground.Texture();
  const glm::vec2 windowSize = glm::vec2(tex->Width(), tex->Height());
  const glm::vec2 textAreaSize = windowSize - textAreaMargin * 2.0f;
  glm::vec2 offset = textAreaSize * glm::vec2(-0.5f, 0.5f);
  offset += textAreaOffset;
  offset.y -= fontRenderer.LineHeight();

  fontRenderer.BeginUpdate();
  fontRenderer.Color(glm::vec4(0, 0, 0, 1));
  offset += position;
  float lineWidth = 0;
  int outputOffset = 0;
  for (int i = 0; i < outputCount; ++i) {
    lineWidth += fontRenderer.XAdvance(text[i]);
    if (text[i] == L'\n' || lineWidth > textAreaSize.x) {
      const std::wstring tmp = text.substr(outputOffset, i - outputOffset);
      fontRenderer.AddString(glm::vec3(offset, 0), tmp.c_str());
      offset.y -= fontRenderer.LineHeight();
      outputOffset = i;
      lineWidth = 0;
    }
  }
  if (outputOffset < outputCount) {
    const std::wstring tmp = text.substr(outputOffset, outputCount - outputOffset);
    fontRenderer.AddString(glm::vec3(offset, 0), tmp.c_str());
  }
  fontRenderer.EndUpdate();
}

/**
*
*/
void TextWindow::Draw()
{
  const GLFWEW::Window& window = GLFWEW::Window::Instance();
  const glm::vec2 screenSize(window.Width(), window.Height());
  spriteRenderer.Draw(screenSize);
  fontRenderer.Draw(screenSize);
}

/**
*
*/
void TextWindow::Open(const wchar_t* str)
{
  text = str;
  outputCount = 0;
  outputTimer = 0;
  waitForInput = false;
  fontRenderer.BeginUpdate();
  fontRenderer.EndUpdate();
  isOpen = true;
}

/**
*
*/
void TextWindow::Close()
{
  text.clear();
  outputCount = 0;
  outputTimer = 0;
  waitForInput = false;
  fontRenderer.BeginUpdate();
  fontRenderer.EndUpdate();
  isOpen = false;
}

