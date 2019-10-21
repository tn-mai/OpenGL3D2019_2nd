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
* FOURCCを作成する.
*/
#define MAKE_FOURCC(a, b, c, d) \
  static_cast<uint32_t>(a + (b << 8) + (c << 16) + (d << 24))

/**
* バイト列から数値を復元する.
*
* @param p      バイト列へのポインタ.
* @param offset 数値のオフセット.
* @param size   数値のバイト数(1～4).
*
* @return 復元した数値.
*/
uint32_t Get(const uint8_t* p, size_t offset, size_t size)
{
  uint32_t result = 0;
  p += offset;
  for (size_t i = 0; i < size; ++i) {
    result += p[i] << (i * 8);
  }
  return result;
}

/**
* DDS画像情報.
*/
struct DDSPixelFormat
{
  uint32_t size; ///< この構造体のバイト数(32).
  uint32_t flgas; ///< 画像に含まれるデータの種類を示すフラグ.
  uint32_t fourCC; ///< 画像フォーマットを示すFOURCC.
  uint32_t rgbBitCount; ///< 1ピクセルのビット数.
  uint32_t redBitMask; ///< 赤要素が使う部分を示すビット.
  uint32_t greenBitMask; ///< 緑要素が使う部分を示すビット.
  uint32_t blueBitMask; ///< 青要素が使う部分を示すビット.
  uint32_t alphaBitMask; ///< 透明要素が使う部分を示すビット.
};

/**
* バッファからDDS画像情報を読み出す.
*
* @param buf 読み出し元バッファ.
*
* @return 読み出したDDS画像情報.
*/
DDSPixelFormat ReadDDSPixelFormat(const uint8_t* buf)
{
  DDSPixelFormat tmp;
  tmp.size = Get(buf, 0, 4);
  tmp.flgas = Get(buf, 4, 4);
  tmp.fourCC = Get(buf, 8, 4);
  tmp.rgbBitCount = Get(buf, 12, 4);
  tmp.redBitMask = Get(buf, 16, 4);
  tmp.greenBitMask = Get(buf, 20, 4);
  tmp.blueBitMask = Get(buf, 24, 4);
  tmp.alphaBitMask = Get(buf, 28, 4);
  return tmp;
}

/**
* DDSファイルヘッダ.
*/
struct DDSHeader
{
  uint32_t size;  ///< この構造体のバイト数(124).
  uint32_t flags; ///< どのパラメータが有効かを示すフラグ.
  uint32_t height; ///< 画像の高さ(ピクセル数).
  uint32_t width; ///< 画像の幅(ピクセル数).
  uint32_t pitchOrLinearSize; ///< 横のバイト数または画像1枚のバイト数.
  uint32_t depth; ///< 画像の奥行き(枚数)(3次元テクスチャ等で使用).
  uint32_t mipMapCount; ///< 含まれているミップマップレベル数.
  uint32_t reserved1[11]; ///< (将来のために予約されている).
  DDSPixelFormat ddspf; ///< DDS画像情報.
  uint32_t caps[4]; ///< 含まれている画像の種類.
  uint32_t reserved2; ///< (将来のために予約されている).
};

/**
* バッファからDDSファイルヘッダを読み出す.
*
* @param buf 読み出し元バッファ.
*
* @return 読み出したDDSファイルヘッダ.
*/
DDSHeader ReadDDSHeader(const uint8_t* buf)
{
  DDSHeader tmp = {};
  tmp.size = Get(buf, 0, 4);
  tmp.flags = Get(buf, 4, 4);
  tmp.height = Get(buf, 8, 4);
  tmp.width = Get(buf, 12, 4);
  tmp.pitchOrLinearSize = Get(buf, 16, 4);
  tmp.depth = Get(buf, 20, 4);
  tmp.mipMapCount = Get(buf, 24, 4);
  tmp.ddspf = ReadDDSPixelFormat(buf + 28 + 4 * 11);
  for (int i = 0; i < 4; ++i) {
    tmp.caps[i] = Get(buf, 28 + 4 * 11 + 32 + i * 4, 4);
  }
  return tmp;
}

/**
* DDSファイルからテクスチャを作成する.
*
* @param filename DDSファイル名.
*
* @retval 0以外 作成したテクスチャID.
* @retval 0     作成失敗.
*/
GLuint LoadDDS(const char* filename)
{
  std::basic_ifstream<uint8_t> ifs(filename, std::ios_base::binary);
  if (!ifs) {
    std::cerr << "[エラー] " << filename << "を開けません.\n";
    return 0;
  }

  // １メガバイトのメモリを割り当てて、一度にたくさん読み込めるようにする
  std::vector<uint8_t> readBuffer(1'000'000);
  ifs.rdbuf()->pubsetbuf(readBuffer.data(), readBuffer.size());

  // DDSヘッダーを読み込む.
  std::vector<uint8_t> buf(128);
  ifs.read(buf.data(), 128);
  if (ifs.eof()) {
    return 0;
  }
  if (buf[0] != 'D' || buf[1] != 'D' || buf[2] != 'S' || buf[3] != ' ') {
    return 0;
  }
  const DDSHeader header = ReadDDSHeader(buf.data() + 4);
  if (header.size != 124) {
    std::cerr << "[警告] " << filename << "は未対応のDDSファイルです.\n";
    return 0;
  }

  // ファイルのDDS形式に対応するOpenGLのフォーマットを選択する.
  GLenum iformat = GL_RGBA8;
  GLenum format = GL_RGBA;
  uint32_t blockSize = 16;
  bool isCompressed = false;
  if (header.ddspf.flgas & 0x04) {
    // 圧縮フォーマット
    switch (header.ddspf.fourCC) {
    case MAKE_FOURCC('D', 'X', 'T', '1'):
      iformat = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
      blockSize = 8;
      break;
    case MAKE_FOURCC('D', 'X', 'T', '2'):
    case MAKE_FOURCC('D', 'X', 'T', '3'):
      iformat = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
      break;
    case MAKE_FOURCC('D', 'X', 'T', '4'):
    case MAKE_FOURCC('D', 'X', 'T', '5'):
      iformat = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
      break;
    case MAKE_FOURCC('B', 'C', '4', 'U'):
      iformat = GL_COMPRESSED_RED_RGTC1;
      break;
    case MAKE_FOURCC('B', 'C', '4', 'S'):
      iformat = GL_COMPRESSED_SIGNED_RED_RGTC1;
      break;
    case MAKE_FOURCC('B', 'C', '5', 'U'):
      iformat = GL_COMPRESSED_RG_RGTC2;
      break;
    case MAKE_FOURCC('B', 'C', '5', 'S'):
      iformat = GL_COMPRESSED_SIGNED_RG_RGTC2;
      break;
    default:
      std::cerr << "[警告] " << filename << "は未対応のDDSファイルです.\n";
      return 0;
    }
    isCompressed = true;
  } else if (header.ddspf.flgas & 0x40) {
    // 無圧縮フォーマット
    if (header.ddspf.redBitMask == 0xff) {
      iformat = header.ddspf.alphaBitMask ? GL_RGBA8 : GL_RGB8;
      format = header.ddspf.alphaBitMask ? GL_RGBA : GL_RGB;
    } else if (header.ddspf.blueBitMask == 0xff) {
      iformat = header.ddspf.alphaBitMask ? GL_RGBA8 : GL_RGB8;
      format = header.ddspf.alphaBitMask ? GL_BGRA : GL_BGR;
    } else {
      std::cerr << "[警告] " << filename << "は未対応のDDSファイルです.\n";
      return 0;
    }
  } else {
    std::cerr << "[警告] " << filename << "は未対応のDDSファイルです.\n";
    return 0;
  }

  // 画像枚数を取得する.
  const bool isCubemap = header.caps[1] & 0x200;
  const GLenum target = isCubemap ? GL_TEXTURE_CUBE_MAP_POSITIVE_X : GL_TEXTURE_2D;
  const int faceCount = isCubemap ? 6 : 1;

  // 画像を読み込む.
  buf.resize(header.width * header.height * 4);
  GLuint texId;
  glGenTextures(1, &texId);
  glBindTexture(isCubemap ? GL_TEXTURE_CUBE_MAP : GL_TEXTURE_2D, texId);
  for (int faceIndex = 0; faceIndex < faceCount; ++faceIndex) {
    GLsizei curWidth = header.width;
    GLsizei curHeight = header.height;
    for (int mipLevel = 0; mipLevel < static_cast<int>(header.mipMapCount);
      ++mipLevel) {
      if (isCompressed) {
        const uint32_t imageBytes = ((curWidth + 3) / 4) * ((curHeight + 3) / 4) * blockSize;
        ifs.read(buf.data(), imageBytes);
        glCompressedTexImage2D(target + faceIndex, mipLevel, iformat,
          curWidth, curHeight, 0, imageBytes, buf.data());
      } else {
        const uint32_t imageBytes = curWidth * curHeight * 4;
        ifs.read(buf.data(), imageBytes);
        glTexImage2D(target + faceIndex, mipLevel, iformat,
          curWidth, curHeight, 0, format, GL_UNSIGNED_BYTE, buf.data());
      }
      const GLenum result = glGetError();
      if (result != GL_NO_ERROR) {
        std::cerr << "[警告] " << filename << "の読み込みに失敗("
          << std::hex << result << ").\n";
      }
      curWidth = std::max(1, curWidth / 2);
      curHeight = std::max(1, curHeight / 2);
    }
  }

  // テクスチャ・パラメーターを設定する.
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, header.mipMapCount - 1);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
    header.mipMapCount <= 1 ? GL_LINEAR : GL_LINEAR_MIPMAP_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glBindTexture(GL_TEXTURE_2D, 0);

  return texId;
}

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
  const size_t len = strlen(path);
  if (_stricmp(path + len - 4, ".dds") == 0) {
    const GLuint id = LoadDDS(path);
    if (id) {
      return id;
    }
  }

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
*
*/
Image2DArrayPtr Image2DArray::Create(const std::vector<std::string>& files)
{
  std::vector<ImageData> imageDataList;
  imageDataList.resize(files.size());
  for (size_t i = 0; i < files.size(); ++i) {
    if (!LoadImage2D(files[i].c_str(), &imageDataList[i])) {
      return nullptr;
    }
  }

  const GLint width = imageDataList[0].width;
  const GLint height = imageDataList[0].height;

  GLuint id;
  glGenTextures(1, &id);
  glBindTexture(GL_TEXTURE_2D_ARRAY, id);
  glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_RGBA8, width, height, imageDataList.size());

  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  for (size_t i = 0; i < imageDataList.size(); ++i) {
    glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, width, height, 1,
      imageDataList[i].format, imageDataList[i].type, imageDataList[i].data.data());
    const GLenum result = glGetError();
    if (result != GL_NO_ERROR) {
      std::cerr << "ERROR: テクスチャの作成に失敗(0x" << std::hex << result << ").";
      glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
      glDeleteTextures(1, &id);
      return nullptr;
    }
  }
  glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

  // テクスチャのパラメータを設定する
  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_LEVEL, 0);
  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);
  // １要素の画像データの場合、(R,R,R,l)として読み取られるように設定する
  if (imageDataList[0].format == GL_RED) {
    const GLint swizzle[] = { GL_RED, GL_RED, GL_RED, GL_ONE };
    glTexParameteriv(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_SWIZZLE_RGBA, swizzle);
  }

  glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

  return std::make_shared<Image2DArray>(id);
}

/**
*
*/
Image2DArray::Image2DArray(GLuint texId)
{
  Reset(texId);
}

/**
*
*/
Image2DArray::~Image2DArray()
{
  glDeleteTextures(1, &id);
}

/**
*
*/
void Image2DArray::Reset(GLuint texId)
{
  glDeleteTextures(1, &id);
  id = texId;
  if (id) {
    // テクスチャの幅と高さを取得する.
    glBindTexture(GL_TEXTURE_2D_ARRAY, id);
    glGetTexLevelParameteriv(GL_TEXTURE_2D_ARRAY, 0, GL_TEXTURE_WIDTH, &width);
    glGetTexLevelParameteriv(GL_TEXTURE_2D_ARRAY, 0, GL_TEXTURE_HEIGHT, &height);
    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
  }
}

/**
*
*/
bool Image2DArray::IsNull() const
{
  return id == 0;
}

/**
*
*/
GLuint Image2DArray::Get() const
{
  return id;
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