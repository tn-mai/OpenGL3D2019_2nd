/**
* @file Particle.cpp
*/
#include "Particle.h"
#include <algorithm>
#include <random>
#include <iostream>
#include <time.h>
#include <glm/gtc/matrix_transform.hpp>

namespace /* unnamed */ {

struct Vertex {
  glm::vec3 position; ///< 座標
  glm::vec4 color; ///< 色
  glm::vec2 texCoord; ///< テクスチャ座標
  glm::vec2 offset;
};

std::mt19937& Rand()
{
  static std::mt19937 instance(static_cast<int>(time(nullptr)));
  return instance;
}
 
} // unnamed namespace

/**
*
*/
void Particle::Update(float deltaTime)
{
  lifetime -= deltaTime;
  velocity += acceleration * deltaTime;
  position += velocity * deltaTime;
  color.a = lifetime / 2.0;
}

/**
*
*/
ParticleEmitter::ParticleEmitter(const char* imagePath, const glm::vec3& pos, float duration, bool loop, float eps) :
  duration(duration),
  loop(loop),
  emissionsPerSecond(eps),
  position(pos),
  interval(1.0f / eps)
{
  texture = Texture::Image2D::Create(imagePath);
}

/**
*
*/
void ParticleEmitter::AddParticle()
{
  const float rx = std::uniform_real_distribution<float>(-angle, angle)(Rand());
  const float ry = std::uniform_real_distribution<float>(0.0f, glm::two_pi<float>())(Rand());
  const glm::mat4 mvel = glm::rotate(glm::rotate(glm::mat4(1),
    ry, glm::vec3(0, 1, 0)),
    rx, glm::vec3(1, 0, 0));
  const float velocity = std::uniform_real_distribution<float>(5, 6.0f)(Rand());

  const glm::mat4 moff = glm::rotate(glm::rotate(glm::rotate(glm::mat4(1),
    rotation.y, glm::vec3(0, 1, 0)),
    rotation.z, glm::vec3(0, 0, 1)),
    rotation.x, glm::vec3(1, 0, 0));
  const float offsetX = std::uniform_real_distribution<float>(-radius, radius)(Rand());
  const glm::vec3 offset = moff * glm::vec4(offsetX, 0, 0, 1);

  Particle p(position + offset, moff * mvel * glm::vec4(0, velocity, 0, 1), 2.0f);
  p.rect.origin = { 0, 64 };
  p.rect.size = { 64, 64 };
  const float size = std::normal_distribution<float>(0, 1)(Rand()) * 0.0005f + 0.0045f;
  p.scale = glm::vec2(size);
  p.acceleration = glm::vec3(0, -gravity, 0);
  particles.push_back(p);
}

/**
*
*/
void ParticleEmitter::Update(float deltaTime)
{
  float emissionDelta = deltaTime;
  timer += deltaTime;
  if (timer >= duration) {
    if (loop) {
      timer -= duration;
    } else {
      emissionDelta -= timer - duration;
      timer = duration;
    }
  }
  emissionTimer += emissionDelta;
  for (; emissionTimer >= interval; emissionTimer -= interval) {
    AddParticle();
  }

  for (auto& e : particles) {
    e.Update(deltaTime);
  }
  particles.remove_if([](const Particle& p) { return p.IsDead(); });
}

/**
*
*/
void ParticleEmitter::Draw()
{
  if (count) {
    glBindTexture(GL_TEXTURE_2D, texture->Get());
    glDrawElementsBaseVertex(GL_TRIANGLES, count, GL_UNSIGNED_SHORT,
      reinterpret_cast<const GLvoid*>(0), baseVertex);
  }
}

/**
*
*/
bool ParticleSystem::Init(size_t maxParticleCount)
{
  vbo.Create(GL_ARRAY_BUFFER, sizeof(Vertex) * maxParticleCount * 4,
    nullptr, GL_STREAM_DRAW);

  static const GLushort baseIndices[] = { 0, 1, 2, 2, 3, 0 };
  std::vector<short> indices;
  indices.reserve(4000);
  for (int baseIndex = 0; baseIndex < 65536 - 4; baseIndex += 4) {
    for (auto i : baseIndices) {
      indices.push_back(static_cast<GLshort>(baseIndex + i));
    }
  }
  ibo.Create(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort) * indices.size(),
    indices.data(), GL_STATIC_DRAW);

  vao.Create(vbo.Id(), ibo.Id());
  vao.Bind();
  vao.VertexAttribPointer(
    0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), offsetof(Vertex, position));
  vao.VertexAttribPointer(
    1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), offsetof(Vertex, color));
  vao.VertexAttribPointer(
    2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), offsetof(Vertex, texCoord));
  vao.VertexAttribPointer(
    3, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), offsetof(Vertex, offset));
  vao.Unbind();

  program = Shader::Program::Create("Res/Particle.vert", "Res/Sprite.frag");

  return true;
}

/**
*
*/
void ParticleSystem::Add(const ParticleEmitterPtr& p)
{
  emitters.push_back(p);
}

/**
*
*/
void ParticleSystem::Clear()
{
  emitters.clear();
}

/**
*
*/
void ParticleSystem::Update(float deltaTime)
{
  for (auto& e : emitters) {
    e->Update(deltaTime);
  }
  emitters.remove_if([](const ParticleEmitterPtr& e) { return e->IsDead(); });

  std::vector<Vertex> vertices;
  vertices.reserve(1000);
  for (auto& e : emitters) {
    const glm::vec2 reciprocalSize(
      glm::vec2(1) / glm::vec2(e->texture->Width(), e->texture->Height()));

    e->baseVertex = vertices.size();
    e->count = 0;
    for (auto& particle : e->particles) {
      // 矩形を0.0～1.0の範囲に変換.
      Rect rect = particle.rect;
      rect.origin *= reciprocalSize;
      rect.size *= reciprocalSize;

      // 中心からの大きさを計算.
      const glm::vec2 halfSize = particle.rect.size * 0.5f;

      // 座標変換行列を作成.
      const glm::mat4 matT = glm::translate(glm::mat4(1), particle.position);
      const glm::mat4 matR =
        glm::rotate(glm::mat4(1), particle.rotation, glm::vec3(0, 0, 1));
      const glm::mat4 matS = glm::scale(glm::mat4(1), glm::vec3(particle.scale, 1));
      const glm::mat4 transform = matR * matS;

      Vertex v[4];

      v[0].position = particle.position;
      v[0].color = particle.color;
      v[0].texCoord = rect.origin;
      v[0].offset = transform * glm::vec4(-halfSize.x, -halfSize.y, 0, 1);

      v[1].position = particle.position;
      v[1].color = particle.color;
      v[1].texCoord = glm::vec2(rect.origin.x + rect.size.x, rect.origin.y);
      v[1].offset = transform * glm::vec4(halfSize.x, -halfSize.y, 0, 1);

      v[2].position = particle.position;
      v[2].color = particle.color;
      v[2].texCoord = rect.origin + rect.size;
      v[2].offset = transform * glm::vec4(halfSize.x, halfSize.y, 0, 1);

      v[3].position = particle.position;
      v[3].color = particle.color;
      v[3].texCoord = glm::vec2(rect.origin.x, rect.origin.y + rect.size.y);
      v[3].offset = transform * glm::vec4(-halfSize.x, halfSize.y, 0, 1);

      vertices.insert(vertices.end(), v, v + 4);

      e->count += 6;
    }
  }
  vbo.BufferSubData(0, vertices.size() * sizeof(Vertex), vertices.data());
}

/**
*
*/
void ParticleSystem::Draw(const glm::mat4& matProj, const glm::mat4& matView)
{
  vao.Bind();
  glEnable(GL_DEPTH_TEST);
  glDepthMask(false);
  glDisable(GL_CULL_FACE);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  program->Use();
  const glm::mat4 matInverseRotation = glm::mat4(glm::inverse(glm::mat3(glm::transpose(glm::inverse(matView)))));
  program->SetInverseViewRotationMatrix(matInverseRotation);
  program->SetViewProjectionMatrix(matProj * matView);
  glActiveTexture(GL_TEXTURE0);

  for (auto& e : emitters) {
    e->Draw();
  }

  glBindTexture(GL_TEXTURE_2D, 0);
  vao.Unbind();
  glDepthMask(true);

  if (const GLenum error = glGetError()) {
    std::cout << "[エラー]" << __func__ << ":(" << std::hex << error << ")\n";
  }
}

