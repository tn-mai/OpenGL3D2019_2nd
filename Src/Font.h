/**
* @file Font.h
*/
#ifndef FONT_H_INCLUDED
#define FONT_H_INCLUDED
#include <GL/glew.h>
#include "Sprite.h"
#include <glm/glm.hpp>
#include <vector>
#include <string>

/**
* ビットマップフォント描画クラス.
*/
class FontRenderer
{
public:
  FontRenderer() = default;
  ~FontRenderer() = default;
  FontRenderer(const FontRenderer&) = delete;
  FontRenderer& operator=(const FontRenderer&) = delete;

  bool Init(size_t maxChar);
  bool LoadFromFile(const char* path);
  void BeginUpdate();
  bool AddString(const glm::vec2&, const wchar_t*);
  void EndUpdate();
  void Draw(const glm::vec2&) const;
  float LineHeight() const;

  // フォント色の設定と取得.
  void Color(const glm::vec4& c) { color = c; }
  const glm::vec4& Color() const { return color; }

private:
  SpriteRenderer spriteRenderer;  ///< 描画に使用するスプライトレンダラー.
  std::vector<Texture::Image2DPtr> textures; ///< フォントテクスチャリスト.
  float lineHeight = 0;           ///< 行の高さ.
  float base = 0;                 ///< 行の上部からベースラインまでの距離.
  glm::vec4 color = glm::vec4(1); ///< フォントの色.

  /// 文字情報.
  struct CharacterInfo {
    int id = -1;                     ///< 文字コード.
    int page = 0;                    ///< 文字が含まれるフォントテクスチャの番号.
    glm::vec2 uv = glm::vec2(0);     ///< 文字のテクスチャ座標.
    glm::vec2 size = glm::vec2(0);   ///< 文字の表示サイズ.
    glm::vec2 offset = glm::vec2(0); ///< 表示位置の補正値.
    float xadvance = 0;              ///< 次の文字を表示する位置.
  };
  std::vector<CharacterInfo> characterInfoList; ///< 文字情報のリスト.
};

#endif // FONT_H_INCLUDED