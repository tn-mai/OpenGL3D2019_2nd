/**
* @file Texture.cpp
*/
#define NOMINMAX
#include "Texture.h"
#include <stdint.h>
#include <vector>
#include <fstream>
#include <iostream>
#include <algorithm>

/// テクスチャ関連の関数やクラスを格納する名前空間
namespace Texture {

/**
* 色データを取得する.
*
* @param x  X座標.
* @param y  Y座標.
*
* @return 座標(x, y)の色を0.0～1.0で表した値.
*         色要素がデータに存在しない場合、RGBは0.0、Aは1.0になる.
*/
glm::vec4 ImageData::GetColor(int x, int y) const
{
  // 座標を画像の範囲に制限.
  x = std::max(0, std::min(x, width - 1));
  y = std::max(0, std::min(y, height - 1));

  if (type == GL_UNSIGNED_BYTE) {
    // 各色１バイトのデータ.
    glm::vec4 color(0, 0, 0, 255);
    if (format == GL_BGRA) {
      // BGRAの順番で１バイトずつ、合計４バイト格納されている.
      const uint8_t* p = &data[x * 4 + y * (width * 4)];
      color.b = p[0];
      color.g = p[1];
      color.r = p[2];
      color.a = p[3];
    } else if (format == GL_BGR) {
      // BGRの順番で１バイトずつ、合計３バイト格納されている.
      const uint8_t* p = &data[x * 3 + y * (width * 3)];
      color.b = p[0];
      color.g = p[1];
      color.r = p[2];
    } else if (format == GL_RED) {
      // 赤色だけ、合計１バイト格納されている.
      color.r = data[x + y * width];
    }
    return color / 255.0f;
  } else if (type == GL_UNSIGNED_SHORT_1_5_5_5_REV) {
    // 色が２バイトに詰め込まれたデータ.
    glm::vec4 color(0, 0, 0, 1);
    const uint8_t* p = &data[x * 2 + y * (width * 2)];
    const uint16_t c = p[0] + p[1] * 0x100; // 2つのバイトを結合.
    if (format == GL_BGRA) {
      // 16ビットのデータから各色を取り出す.
      color.b = static_cast<float>((c & 0b0000'0000'0001'1111));
      color.g = static_cast<float>((c & 0b0000'0011'1110'0000) >> 5);
      color.r = static_cast<float>((c & 0b0111'1100'0000'0000) >> 10);
      color.a = static_cast<float>((c & 0b1000'0000'0000'0000) >> 15);
    }
    return color / glm::vec4(31.0f, 31.0f, 31.0f, 1.0f);
  }
return glm::vec4(0, 0, 0, 1);
}

/**
* 2D テクスチャを作成する
*
* @param width	テクスチャの幅(ピクセル数)
* @param height	テクスチャの高さ(ピクセル数)
* @param data	テクスチャデータへのポインタ
* @param format	転送元画像のデータ形式
* @param type	転送元画像のデータ格納形式
*
* @retval 0 以外	作成したテクスチャ・オブジェクトのID
* @retval 0			テクスチャの作成に失敗
*/
	GLuint CreateImage2D(GLsizei width, GLsizei height, const GLvoid* data,
		GLenum format, GLenum type) 
{
	GLuint id;
	glGenTextures(1, &id);
	glBindTexture(GL_TEXTURE_2D, id);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexImage2D(GL_TEXTURE_2D,
		0, GL_RGBA8, width, height, 0, format, type, data);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	const GLenum result = glGetError();
	if (result != GL_NO_ERROR) {
		std::cerr << "ERROR: テクスチャの作成に失敗(0x" << std::hex << result << ").";
		glBindTexture(GL_TEXTURE_2D, 0);
		glDeleteTextures(1, &id);
		return 0;
	}

	// テクスチャのパラメータを設定する
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// １要素の画像データの場合、(R,R,R,l)として読み取られるように設定する
	if (format == GL_RED) {
		const GLint swizzle[] = { GL_RED, GL_RED, GL_RED, GL_ONE };
		glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzle);
	}
		
	glBindTexture(GL_TEXTURE_2D, 0);
		
	return id;
	}

/**
* ファイルから 2D テクスチャを読み込む
*
* @param path 2D テクスチャとして読み込むファイル名
*
* @retval 0 以外 作成したテクスチャ・オブジェクトのID
*		  0		 テクスチャの作成に失敗
*/
GLuint LoadImage2D(const char* path)
{
  ImageData imageData;
  if (!LoadImage2D(path, &imageData)) {
    return 0;
  }
  return CreateImage2D(imageData.width, imageData.height, imageData.data.data(),
    imageData.format, imageData.type);
}

/**
* ファイルから画像データを読み込む.
*
* @param path      画像として読み込むファイルのパス.
* @param imageData 画像データを格納する構造体.
*
* @retval true  読み込み成功.
* @retval false 読み込み失敗.
*/
bool LoadImage2D(const char* path, ImageData* imageData)
{
// TGAヘッダを読み込む
	std::basic_ifstream<uint8_t> ifs;
	ifs.open(path, std::ios_base::binary);
	//以下2行　１メガバイトのメモリを割り当てて、一度にたくさん読み込めるようにしている
	std::vector<uint8_t> readBuffer(1'000'000);
	ifs.rdbuf()->pubsetbuf(readBuffer.data(), readBuffer.size());
	uint8_t tgaHeader[18];
	ifs.read(tgaHeader, 18);

	// イメージIDを飛ばす
	ifs.ignore(tgaHeader[0]);

	// カラーマップを飛ばす
	if (tgaHeader[1]) {
		const int colorMapLength = tgaHeader[5] + tgaHeader[6] * 0x100;
		const int colorMapEntrySize = tgaHeader[7];
		const int colorMapSize = colorMapLength * colorMapEntrySize / 8;
		ifs.ignore(colorMapSize);
	}

	// 画像データを読み込む
	const int width = tgaHeader[12] + tgaHeader[13] * 0x100;
	const int height = tgaHeader[14] + tgaHeader[15] * 0x100;
	const int pixelDepth = tgaHeader[16];
	const int imageSize = width * height * pixelDepth / 8;
	std::vector<uint8_t> buf(imageSize);
	ifs.read(buf.data(), imageSize);

	// 画像データが「上から下」で格納されている場合、上下を入れ替える
	if (tgaHeader[17] & 0x20) {
		const int lineSize = width * pixelDepth / 8;
		std::vector<uint8_t> tmp(imageSize);
		std::vector<uint8_t>::iterator source = buf.begin();
		std::vector<uint8_t>::iterator destination = tmp.end();
		for (int i = 0; i < height; ++i) {
			destination -= lineSize;
			std::copy(source, source + lineSize, destination);
			source += lineSize;
		}
		buf.swap(tmp);
	}

	GLenum type = GL_UNSIGNED_BYTE;
	GLenum format = GL_BGRA;
	if (tgaHeader[2] == 3) {
		format = GL_RED;
	}
	if (tgaHeader[16] == 24) {
		format = GL_BGR;
	}
	else if (tgaHeader[16] == 16) {
		type = GL_UNSIGNED_SHORT_1_5_5_5_REV;
	}

	// 読み込んだ画像データからテクスチャを作成する
  imageData->width = width;
  imageData->height = height;
  imageData->format = format;
  imageData->type = type;
  imageData->data.swap(buf);
  return true;
}

/**
* コンストラクタ
*
* @param texId テクスチャ・オブジェクトのID.
*/
Image2D::Image2D(GLuint texId) {
	Reset(texId);
}

/**
* デストラクタ
*/
Image2D::~Image2D()
{
	glDeleteTextures(1, &id);
}

/**
* テクスチャ・オブジェクトを設定する.
*
* @param texId テクスチャ・オブジェクトのID.
*/
void Image2D::Reset(GLuint texId) 
{
	glDeleteTextures(1, &id);
	id = texId;
  if (id) {
    // テクスチャの幅と高さを取得する.
    glBindTexture(GL_TEXTURE_2D, id);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);
    glBindTexture(GL_TEXTURE_2D, 0);
  }
}

/**
* テクスチャ・オブジェクトが設定されているか調べる.
*
* @retval false	設定されている.
* @retval true	設定されていない.
*/
bool Image2D::IsNull() const
{
	return id == 0;
}

/**
* テクスチャ・オブジェクトを取得する.
*
* @return テクスチャ・オブジェクトのID.
*/
GLuint Image2D::Get() const {
	return id;
}

/**
* 2Dテクスチャを作成する.
*
* @param path テクスチャファイル名.
*
* @return 作成したテクスチャオブジェクト.
*/
Image2DPtr Image2D::Create(const char* path)
{
  return std::make_shared<Image2D>(LoadImage2D(path));
}

/**
* バッファ・テクスチャを作成する.
*
* @param internalFormat バッファのデータ形式.
* @param size           バッファのサイズ.
* @param data           バッファに転送するデータ.
* @param usage          バッファのアクセスタイプ.
*
* @return 作成したテクスチャオブジェクト.
*/
BufferPtr Buffer::Create(GLenum internalFormat, GLsizeiptr size,
  const GLvoid* data, GLenum usage)
{
  std::shared_ptr<Buffer> buffer = std::make_shared<Buffer>();
  if (!buffer->bo.Create(GL_TEXTURE_BUFFER, size, data, usage)) {
    return false;
  }
  glGenTextures(1, &buffer->id);
  glBindTexture(GL_TEXTURE_BUFFER, buffer->id);
  glTexBuffer(GL_TEXTURE_BUFFER, internalFormat, buffer->bo.Id());
  glBindTexture(GL_TEXTURE_BUFFER, 0);

  return buffer;
}

/**
* デストラクタ.
*/
Buffer::~Buffer()
{
  glDeleteTextures(1, &id);
}

/**
* バッファにデータを転送する.
*
* @param offset 転送開始位置(バイト単位).
* @param size   転送するバイト数.
* @param data   転送するデータへのポインタ.
*
* @retval true  転送成功.
* @retval false 転送失敗.
*/
bool Buffer::BufferSubData(GLintptr offset, GLsizeiptr size, const GLvoid* data)
{
  return bo.BufferSubData(offset, size, data);
}

} // namespace Texture