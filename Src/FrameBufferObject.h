/**
* @file FrameBufferObject.h
*/
#ifndef FRAMEBUFFEROBJECT_H_INCLUDED
#define FRAMEBUFFEROBJECT_H_INCLUDED
#include <GL/glew.h>
#include "Texture.h"
#include <memory>

class FrameBufferObject;
typedef std::shared_ptr<FrameBufferObject> FrameBufferObjectPtr;

/**
* フレームバッファの種類.
*/
enum class FrameBufferType {
  colorAndDepth,
  colorOnly,
  depthOnly,
};

/**
* フレームバッファオブジェクト.
*/
class FrameBufferObject
{
public:
  static FrameBufferObjectPtr Create(int w, int h, GLenum internalFormat = GL_RGBA8, FrameBufferType type = FrameBufferType::colorAndDepth);
  FrameBufferObject() = default;
  ~FrameBufferObject();

  GLuint GetFramebuffer() const;
  const Texture::Image2DPtr& GetColorTexture() const;
  const Texture::Image2DPtr& GetDepthTexture() const;

private:
  Texture::Image2DPtr texColor; ///< フレームバッファ用テクスチャ.
  Texture::Image2DPtr texDepth; ///< フレームバッファ用テクスチャ.
  GLuint id= 0; ///< フレームバッファオブジェクト.
};

#endif // FRAMEBUFFEROBJECT_H_INCLUDED
