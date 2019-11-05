/**
* @file FrameBufferObject.cpp
*/
#include "FrameBufferObject.h"

/**
* フレームバッファオブジェクトを作成する.
*
* @param w フレームバッファオブジェクトの幅(ピクセル単位).
* @param h フレームバッファオブジェクトの高さ(ピクセル単位).
*
* @return 作成したフレームバッファオブジェクトへのポインタ.
*/
FrameBufferObjectPtr FrameBufferObject::Create(int w, int h)
{
  FrameBufferObjectPtr fbo = std::make_shared<FrameBufferObject>();
  if (!fbo) {
    return nullptr;
  }

  // テクスチャを作成する.
  fbo->texColor = std::make_shared<Texture::Image2D>(Texture::CreateImage2D(w, h, nullptr, GL_RGBA, GL_UNSIGNED_BYTE));
  fbo->texDepth = std::make_shared<Texture::Image2D>(Texture::CreateImage2D(w, h, nullptr, GL_DEPTH_COMPONENT, GL_FLOAT, GL_DEPTH_COMPONENT32F));

  // フレームバッファを作成する.
  glGenFramebuffers(1, &fbo->id);
  glBindFramebuffer(GL_FRAMEBUFFER, fbo->id);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, fbo->texColor->Target(), fbo->texColor->Get(), 0);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, fbo->texDepth->Target(), fbo->texDepth->Get(), 0);
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

