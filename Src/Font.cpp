/**
* @file Font.cpp
*/
#define  _CRT_SECURE_NO_WARNINGS
#include "Font.h"
#include <memory>
#include <iostream>
#include <stdio.h>

/**
* フォント描画オブジェクトを初期化する.
*
* @param maxCharacter  最大描画文字数.
*
* @retval true  初期化成功.
* @retval false 初期化失敗.
*/
bool FontRenderer::Init(size_t maxCharacter)
{
  return spriteRenderer.Init(maxCharacter, "Res/Sprite.vert", "Res/Sprite.frag");
}

/**
* フォントファイルを読み込む.
*
* @param filename フォントファイル名.
*
* @retval true  読み込み成功.
* @retval false 読み込み失敗.
*/
bool FontRenderer::LoadFromFile(const char* filename)
{
  // ファイルを開く.
  std::unique_ptr<FILE, decltype(&fclose)> fp(fopen(filename, "r"), &fclose);
  if (!fp) {
    std::cerr << "[エラー] " << __func__ << ": " << filename << "を開けません.\n";
    return false;
  }

  // info行を読み込む.
  int line = 1; // 読み込む行番号(エラー表示用).
  int spacing[2]; // １行目の読み込みチエック用.
  int ret = fscanf(fp.get(),
    "info face=\"%*[^\"]\" size=%*d bold=%*d italic=%*d charset=%*s unicode=%*d"
    " stretchH=%*d smooth=%*d aa=%*d padding=%*d,%*d,%*d,%*d spacing=%d,%d%*[^\n]",
    &spacing[0], &spacing[1]);
  if (ret < 2) {
    std::cerr << "[エラー]" << __func__ << ": " << filename << "の読み込みに失敗(" <<
      line << "行目).\n";
    return false;
}
  ++line;

  // common行を読み込む.
  float scaleH;
  ret = fscanf(fp.get(),
    " common lineHeight=%f base=%f scaleW=%*d scaleH=%f pages=%*d packed=%*d%*[^\n]",
    &lineHeight, &base, &scaleH);
  if (ret < 3) {
    std::cerr << "[エラー]" << __func__ << ": " << filename << "の読み込みに失敗(" <<
      line << "行目).\n";
    return false;
  }
  ++line;

  // page行を読み込む.
  std::vector<std::string> texNameList;
  texNameList.reserve(16);
  for (;;) {
    int id;
    char tex[256];
    ret = fscanf(fp.get(), " page id=%d file=\"%255[^\"]\"", &id, tex);
    if (ret < 2) {
      break;
    }
    tex[sizeof(tex) / sizeof(tex[0]) - 1] = '\0'; // 0終端を保証する.
    if (texNameList.size() <= static_cast<size_t>(id)) {
      texNameList.resize(id + 1);
    }
    texNameList[id] = std::string("Res/") + tex;
    ++line;
  }
  if (texNameList.empty()) {
    std::cerr << "[エラー]" << __func__ << ": " << filename << "の読み込みに失敗(" <<
      line << "行目).\n";
    return false;
  }

  // chars行を読み込む.
  int charCount; // char行の数.
  ret = fscanf(fp.get(), " chars count=%d", &charCount);
  if (ret < 1) {
    std::cerr << "[エラー]" << __func__ << ": " << filename << "の読み込みに失敗(" <<
      line << "行目).\n";
    return false;
  }
  ++line;

  // char行を読み込む.
  characterInfoList.clear();
  characterInfoList.resize(65536); // 16bitで表せる範囲を確保.
  for (int i = 0; i < charCount; ++i) {
    CharacterInfo info;
    ret = fscanf(fp.get(),
      " char id=%d x=%f y=%f width=%f height=%f xoffset=%f yoffset=%f xadvance=%f"
      " page=%d chnl=%*d",
      &info.id, &info.uv.x, &info.uv.y, &info.size.x, &info.size.y,
      &info.offset.x, &info.offset.y, &info.xadvance, &info.page);
    if (ret < 9) {
      std::cerr << "[エラー]" << __func__ << ": " << filename << "の読み込みに失敗(" <<
        line << "行目).\n";
      return false;
    }

    // フォントファイルは左上が原点なので、OpenGLの座標系(左下が原点)に変換.
    info.uv.y = scaleH - info.uv.y - info.size.y;

    if (info.id < static_cast<int>(characterInfoList.size())) {
      characterInfoList[info.id] = info;
    }
    ++line;
  }

  // テクスチャを読み込む.
  textures.clear();
  textures.reserve(texNameList.size());
  for (const std::string& e : texNameList) {
    Texture::Image2DPtr tex = Texture::Image2D::Create(e.c_str());
    if (!tex) {
      return false;
    }
    textures.push_back(tex);
  }
  return true;
}

/**
* 文字列の追加を開始する.
*/
void FontRenderer::BeginUpdate()
{
  spriteRenderer.BeginUpdate();
}

/**
* 文字列を追加する.
*
* @param position 表示開始座標(Y座標はフォントのベースライン).
* @param str      追加するUTF-16文字列.
*
* @retval true  追加成功.
* @retval false 追加失敗.
*/
bool FontRenderer::AddString(const glm::vec2& position, const wchar_t* str)
{
  glm::vec2 pos = position;
  for (const wchar_t* itr = str; *itr; ++itr) {
    // 改行判定.
    if (*itr == L'\n') {
      pos.x = position.x;
      pos.y -= lineHeight;
      continue;
    }
    const CharacterInfo& info = characterInfoList[*itr];
    if (info.id >= 0 && info.size.x && info.size.y) {
      // スプライトの座標は画像の中心を指定するが、フォントは左上を指定する.
      // そこで、その差を打ち消すための補正値を計算する.
      const float baseX = info.size.x * 0.5f + info.offset.x;
      const float baseY = base - info.size.y * 0.5f - info.offset.y;
      const glm::vec3 spritePos = glm::vec3(pos + glm::vec2(baseX, baseY), 0);

      Sprite sprite(textures[info.page]);
      sprite.Position(spritePos);
      sprite.Rectangle({ info.uv, info.size });
      sprite.Color(color);
      if (!spriteRenderer.AddVertices(sprite)) {
        return false;
      }
    }
    pos.x += info.xadvance; // 次の表示位置へ移動.
  }
  return true;
}

/**
* 文字列の追加を終了する.
*/
void FontRenderer::EndUpdate()
{
  spriteRenderer.EndUpdate();
}

/**
* フォントを描画する.
*
* @param screenSize 画面サイズ.
*/
void FontRenderer::Draw(const glm::vec2& screenSize) const
{
  spriteRenderer.Draw(screenSize);
}

/**
* 行の高さを取得する.
*
* @return 行の高さ(ピクセル数).
*/
float FontRenderer::LineHeight() const
{
  return lineHeight;
}
