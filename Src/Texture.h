/**
* @file Texture.h
*/
#ifndef TEXTURE_H_INCLUDED
#define TEXTURE_H_INCLUDED
#include <GL/glew.h>
#include "BufferObject.h"
#include <glm/glm.hpp>
#include <vector>
#include <string>
#include <memory>

namespace Texture {

class Interface;
using InterfacePtr = std::shared_ptr<Interface>;
class Image2D;
using Image2DPtr = std::shared_ptr<Image2D>;
class Image2DArray;
using Image2DArrayPtr = std::shared_ptr<Image2DArray>;
class Buffer;
using BufferPtr = std::shared_ptr<Buffer>;
class Cube;
using CubePtr = std::shared_ptr<Cube>;

/**
* 画像データ.
*/
class ImageData {
public:
  glm::vec4 GetColor(int x, int y) const;

public:
  GLint width = 0; ///< 横のピクセル数.
  GLint height = 0; ///< 縦のピクセル数.
  GLenum format = GL_NONE; ///< 記録されている色の種類.
  GLenum type = GL_NONE; ///< それぞれの色情報のビット配置.
  std::vector<uint8_t> data; ///< バイトデータ.
};

GLuint CreateImage2D(GLsizei width, GLsizei height, const GLvoid* data,
  GLenum format, GLenum type);
GLuint LoadImage2D(const char* path);
bool LoadImage2D(const char* path, ImageData* imageData);

/**
* テクスチャ操作インターフェイス.
*/
class Interface
{
public:
  Interface() = default;
  virtual ~Interface() = default;
  virtual bool IsNull() const = 0;
  virtual GLuint Get() const = 0;
  virtual GLint Width() const = 0;
  virtual GLint Height() const = 0;
  virtual GLenum Target() const = 0;
};

/**
* 2Dテクスチャ
*/
class Image2D : public Interface
{
public:
  static Image2DPtr Create(const char*);
  Image2D() = default;
  explicit Image2D(GLuint texId);
  virtual ~Image2D();

  void Reset(GLuint texId);
  virtual bool IsNull() const override;
  virtual GLuint Get() const override;
  virtual GLint Width() const override { return width; }
  virtual GLint Height() const override { return height; }
  virtual GLenum Target() const override { return GL_TEXTURE_2D; }

private:
  GLuint id = 0;
  GLint width = 0;
  GLint height = 0;
};

/**
* 2D配列テクスチャ
*/
class Image2DArray : public Interface
{
public:
  static Image2DArrayPtr Create(const std::vector<std::string>&);
  Image2DArray() = default;
  explicit Image2DArray(GLuint texId);
  virtual ~Image2DArray();

  void Reset(GLuint texId);
  virtual bool IsNull() const override;
  virtual GLuint Get() const override;
  virtual GLint Width() const override { return width; }
  virtual GLint Height() const override { return height; }
  virtual GLenum Target() const override { return GL_TEXTURE_2D_ARRAY; }

private:
  GLuint id = 0;
  GLint width = 0;
  GLint height = 0;
};

/**
* バッファ・テクスチャ.
*/
class Buffer : public Interface
{
public:
  static BufferPtr Create(GLenum internalFormat, GLsizeiptr size,
    const GLvoid* data = nullptr, GLenum usage = GL_STATIC_DRAW);
  Buffer() = default;
  virtual ~Buffer();

  virtual bool IsNull() const override { return !id; }
  virtual GLuint Get() const override { return id; }
  virtual GLint Width() const override { return bo.Size(); }
  virtual GLint Height() const override { return 1; }
  virtual GLenum Target() const override { return GL_TEXTURE_BUFFER; }

  bool BufferSubData(GLintptr offset, GLsizeiptr size, const GLvoid* data);
  GLuint BufferId() const { return bo.Id(); }
  GLsizeiptr Size() const { return bo.Size(); }

private:
  GLuint id = 0;
  BufferObject bo;
};

/**
* キューブマップ・テクスチャ.
*/
class Cube : public Interface
{
public:
  static CubePtr Create(const std::vector<std::string>&);
  Cube() = default;
  virtual ~Cube();

  virtual bool IsNull() const override { return !id; }
  virtual GLuint Get() const override { return id; }
  virtual GLint Width() const override { return width; }
  virtual GLint Height() const override { return height; }
  virtual GLenum Target() const override { return GL_TEXTURE_CUBE_MAP; }

private:
  GLuint id = 0;
  GLint width = 0;
  GLint height = 0;
};

} // namespace Texture

#endif // TEXTURE_H_INCLUDED