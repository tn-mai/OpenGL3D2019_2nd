/**
* @file TextWindow.cpp
*/
#include "TextWindow.h"
#include "GLFWEW.h"
#include <iostream>

/**
*
*/
bool TextWindow::Init(const char* imagePath, const glm::vec2& scale, const glm::vec2& textAreaOrigin, const glm::vec2& textAreaSize)
{
  this->scale = scale;
  this->textAreaOffset = textAreaOffset;
  this->textAreaSize = textAreaSize;
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

  const GLFWEW::Window& window = GLFWEW::Window::Instance();
  const Texture::Image2DPtr tex = sprBackground.Texture();
  const glm::vec2 screenScale = scale * (static_cast<float>(window.Width()) / tex->Width());

  sprBackground.Position(position);
  sprBackground.Scale(screenScale);
  spriteRenderer.BeginUpdate();
  spriteRenderer.AddVertices(sprBackground);
  spriteRenderer.EndUpdate();

  if (outputCount >= static_cast<int>(text.size())) {
    return;
  }
  if (waitForInput) {
    return;
  }

  outputTimer += deltaTime;
  const int c = static_cast<int>(outputTimer / interval);
  outputCount += c;
  outputTimer -= static_cast<float>(c) * interval;

  fontRenderer.BeginUpdate();
  fontRenderer.Color(glm::vec4(1));
  const std::wstring tmp = text.substr(0, outputCount);
  glm::vec2 offset = glm::vec2(tex->Width(), tex->Height()) * glm::vec2(-0.5f, 0.5f);
  offset *= screenScale + textAreaOffset;
  offset.y -= fontRenderer.LineHeight();
  fontRenderer.AddString(position + glm::vec3(offset, 0), tmp.c_str());
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

