/**
* @file FrameBufferObject.cpp
*/
#include "FrameBufferObject.h"
#include <iostream>

/**
* フレームバッファオブジェクトを作成する.
*
* @param w フレームバッファオブジェクトの幅(ピクセル単位).
* @param h フレームバッファオブジェクトの高さ(ピクセル単位).
* @param internalFormat カラーバッファのピクセル・フォーマット.
* @param type           フレームバッファの種類.
*
* @return 作成したフレームバッファオブジェクトへのポインタ.
*/
FrameBufferObjectPtr FrameBufferObject::Create(int w, int h, GLenum internalFormat, FrameBufferType type)
{
  FrameBufferObjectPtr fbo = std::make_shared<FrameBufferObject>();
  if (!fbo) {
    return nullptr;
  }

  // テクスチャを作成する.
  if (type != FrameBufferType::depthOnly) {
    GLenum imageType = GL_UNSIGNED_BYTE;
    if (internalFormat == GL_RGBA16F) {
      imageType = GL_HALF_FLOAT;
    } else if (internalFormat == GL_RGBA32F) {
      imageType = GL_FLOAT;
    }
    fbo->texColor = std::make_shared<Texture::Image2D>(Texture::CreateImage2D(w, h, nullptr, GL_RGBA, imageType, internalFormat));
    fbo->texColor->SetWrapMode(GL_CLAMP_TO_EDGE);
  }
  if (type != FrameBufferType::colorOnly) {
    fbo->texDepth = std::make_shared<Texture::Image2D>(Texture::CreateImage2D(w, h, nullptr, GL_DEPTH_COMPONENT, GL_FLOAT, GL_DEPTH_COMPONENT32F));
    fbo->texDepth->SetWrapMode(GL_CLAMP_TO_EDGE);
  }

  // フレームバッファを作成する.
  glGenFramebuffers(1, &fbo->id);
  glBindFramebuffer(GL_FRAMEBUFFER, fbo->id);
  if (fbo->texColor) {
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, fbo->texColor->Target(), fbo->texColor->Get(), 0);
  }
  if (fbo->texDepth) {
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, fbo->texDepth->Target(), fbo->texDepth->Get(), 0);
  }
  if (type == FrameBufferType::depthOnly) {
    glDrawBuffer(GL_NONE);
  }
  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    std::cerr << "[エラー]" << __func__ << "フレームバッファの作成に失敗(" << w << "x" << h << ").\n";
  }
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  return fbo;
}

/**
* デストラクタ.
*/
FrameBufferObject::~FrameBufferObject()
{
  if (id) {
    glDeleteFramebuffers(1, &id);
  }
}

/**
* FBOのIDを取得する.
*
* @return FBOのID.
*/
GLuint FrameBufferObject::GetFramebuffer() const
{
  return id;
}

/**
* カラーバッファ用テクスチャを取得する.
*
* @return カラーバッファ用テクスチャ.
*/
const Texture::Image2DPtr& FrameBufferObject::GetColorTexture() const
{
  return texColor;
}

/**
* 深度バッファ用テクスチャを取得する.
*
* @return 深度バッファ用テクスチャ.
*/
const Texture::Image2DPtr& FrameBufferObject::GetDepthTexture() const
{
  return texDepth;
}

