/**
* @file Particle.cpp
*/
#include "Particle.h"
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <random>
#include <iostream>
#include <time.h>

namespace /* unnamed */ {

/// パーティクル用の頂点データ型.
struct Vertex {
  glm::vec3 center; ///< 中心の座標
  glm::vec4 color; ///< 色
  glm::vec2 texCoord; ///< テクスチャ座標
  glm::vec2 offset; ///< 中心からの距離.
};

/// パーティクル用の乱数エンジン.
std::mt19937 randomEngine(static_cast<int>(time(nullptr)));

/**
* int型の乱数を生成する.
*
* @param min 生成する乱数の最小値.
* @param max 生成する乱数の最大値.
*/
int RandomInt(int min, int max)
{
  return std::uniform_int_distribution<>(min, max)(randomEngine);
}
 
/**
* float型の乱数を生成する.
*
* @param min 生成する乱数の最小値.
* @param max 生成する乱数の最大値.
*/
float RandomFloat(float min, float max)
{
  return std::uniform_real_distribution<float>(min, max)(randomEngine);
}
 
} // unnamed namespace

/**
* コンストラクタ.
*
* @param  pp  パーティクルのパラメーター.
* @param  pos パーティクルの初期座標.
* @param  r   パーティクルのテクスチャ表示範囲を示す矩形.
*/
Particle::Particle(const ParticleParameter& pp, const glm::vec3& pos, const Rect& r) :
  rect(r),
  position(pos),
  lifetime(pp.lifetime),
  velocity(pp.velocity),
  acceleration(pp.acceleration),
  scale(pp.scale),
  rotation(pp.rotation),
  color(pp.color)
{
}

/**
* パーティクルの状態を更新する.
*
* @param deltaTime  前回の更新からの経過時間(秒).
*/
void Particle::Update(float deltaTime)
{
  lifetime -= deltaTime;
  velocity += acceleration * deltaTime;
  position += velocity * deltaTime;
}

/**
* コンストラクタ.
*
* @param  ep エミッターの初期化パラメータ.
* @param  pp パーティクルの初期化パラメータ.
*/
ParticleEmitter::ParticleEmitter(const ParticleEmitterParameter& ep, const ParticleParameter& pp) :
  ep(ep),
  pp(pp),
  interval(1.0f / ep.emissionsPerSecond)
{
  texture = Texture::Image2D::Create(ep.imagePath.c_str());
}

/**
* パーティクルを追加する.
*/
void ParticleEmitter::AddParticle()
{
  const float rx = RandomFloat(-ep.angle, ep.angle);
  const float ry = RandomFloat(0.0f, glm::two_pi<float>());
  const glm::mat4 mvel = glm::rotate(glm::rotate(glm::mat4(1),
    ry, glm::vec3(0, 1, 0)),
    rx, glm::vec3(1, 0, 0));

  const glm::mat4 moff = glm::rotate(glm::rotate(glm::rotate(glm::mat4(1),
    ep.rotation.y, glm::vec3(0, 1, 0)),
    ep.rotation.z, glm::vec3(0, 0, 1)),
    ep.rotation.x, glm::vec3(1, 0, 0));
  const glm::vec3 offset = moff * mvel * glm::vec4(0, ep.radius, 0, 1);

  ParticleParameter tmpPP = pp;
  tmpPP.velocity = moff * mvel * glm::vec4(pp.velocity, 1);

  Rect rect;
  rect.origin = glm::vec2(0);
  if (ep.tiles.x > 1) {
    const int tx = RandomInt(0, ep.tiles.x - 1);
    rect.origin.x = static_cast<float>(tx) / static_cast<float>(ep.tiles.x);
  }
  if (ep.tiles.y > 1) {
    const int ty = RandomInt(0, ep.tiles.y - 1);
    rect.origin.y = static_cast<float>(ty) / static_cast<float>(ep.tiles.y);
  }
  rect.size = glm::vec2(1) / glm::vec2(ep.tiles);

  Particle p(tmpPP, ep.position + offset, rect);
  particles.push_back(p);
}

/**
* エミッターの管理下にあるパーティクルの状態を更新する.
*
* @param deltaTime  前回の更新からの経過時間(秒).
*/
void ParticleEmitter::Update(float deltaTime)
{
  float emissionDelta = deltaTime;
  timer += deltaTime;
  if (timer >= ep.duration) {
    if (ep.loop) {
      timer -= ep.duration;
    } else {
      emissionDelta -= timer - ep.duration;
      timer = ep.duration;
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
* エミッターの管理下にあるパーティクルを描画する.
*/
void ParticleEmitter::Draw()
{
  if (count) {
    glBlendFunc(ep.srcFactor, ep.dstFactor);
    glBindTexture(GL_TEXTURE_2D, texture->Get());
    glDrawElementsBaseVertex(GL_TRIANGLES, count, GL_UNSIGNED_SHORT,
      reinterpret_cast<const GLvoid*>(0), baseVertex);
  }
}

/**
* パーティクル・システムを初期化する.
*
* @param maxParticleCount 表示可能なパーティクルの最大数.
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
    0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), offsetof(Vertex, center));
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
* エミッターを追加する.
*
* @param  ep  エミッターの初期化パラメータ.
* @param  pp  パーティクルの初期化パラメータ.
*/
void ParticleSystem::Add(const ParticleEmitterParameter& ep, const ParticleParameter& pp)
{
  ParticleEmitterPtr p = std::make_shared<ParticleEmitter>(ep, pp);
  emitters.push_back(p);
}

/**
* 指定されたIDを持つエミッターを検索する.
*
* @param id  検索するID.
*
* @return 引数idと一致するIDを持つエミッター.
*/
ParticleEmitterPtr ParticleSystem::Find(int id) const
{
  auto itr = std::find_if(emitters.begin(), emitters.end(), [id](const ParticleEmitterPtr& p) { return p->Id() == id; });
  if (itr != emitters.end()) {
    return *itr;
  }
  return nullptr;
}

/**
* 指定されたエミッターを削除する.
*/
void ParticleSystem::Remove(const ParticleEmitterPtr& p)
{
  emitters.remove(p);
}

/**
* すべてのエミッターを削除する.
*/
void ParticleSystem::Clear()
{
  emitters.clear();
}

/**
* パーティクルの状態を更新する.
*
* @param deltaTime  前回の更新からの経過時間(秒).
* @param matView    ビュー行列.
*/
void ParticleSystem::Update(float deltaTime, const glm::mat4& matView)
{
  for (auto& e : emitters) {
    e->Update(deltaTime);
  }
  emitters.remove_if([](const ParticleEmitterPtr& e) { return e->IsDead(); });
  struct A {
    float z;
    ParticleEmitterPtr p;
  };
  std::vector<A> sortedList;
  sortedList.reserve(emitters.size());
  for (auto& e : emitters) {
    const glm::vec3 pos = matView * glm::vec4(e->Position(), 1);
    sortedList.push_back({ pos.z, e });
  }
  std::sort(sortedList.begin(), sortedList.end(), [](const A& a, const A& b) { return a.z < b.z; });

  std::vector<Vertex> vertices;
  vertices.reserve(10000);
  for (auto& e : sortedList) {
    const glm::vec2 reciprocalSize(
      glm::vec2(1) / glm::vec2(e.p->texture->Width(), e.p->texture->Height()));

    e.p->baseVertex = vertices.size();
    e.p->count = 0;
    for (auto& particle : e.p->particles) {
      // 矩形を0.0～1.0の範囲に変換.
      Rect rect = particle.rect;

      // 中心からの大きさを計算.
      const glm::vec2 halfSize = particle.rect.size * 0.5f;

      // 座標変換行列を作成.
      const glm::mat4 matT = glm::translate(glm::mat4(1), particle.position);
      const glm::mat4 matR =
        glm::rotate(glm::mat4(1), particle.rotation, glm::vec3(0, 0, 1));
      const glm::mat4 matS = glm::scale(glm::mat4(1), glm::vec3(particle.scale, 1));
      const glm::mat4 transform = matR * matS;

      Vertex v[4];

      v[0].center = particle.position;
      v[0].color = particle.color;
      v[0].texCoord = rect.origin;
      v[0].offset = transform * glm::vec4(-1, -1, 0, 1);

      v[1].center = particle.position;
      v[1].color = particle.color;
      v[1].texCoord = glm::vec2(rect.origin.x + rect.size.x, rect.origin.y);
      v[1].offset = transform * glm::vec4(1, -1, 0, 1);

      v[2].center = particle.position;
      v[2].color = particle.color;
      v[2].texCoord = rect.origin + rect.size;
      v[2].offset = transform * glm::vec4(1, 1, 0, 1);

      v[3].center = particle.position;
      v[3].color = particle.color;
      v[3].texCoord = glm::vec2(rect.origin.x, rect.origin.y + rect.size.y);
      v[3].offset = transform * glm::vec4(-1, 1, 0, 1);

      vertices.insert(vertices.end(), v, v + 4);

      e.p->count += 6;
    }
  }
  vbo.BufferSubData(0, vertices.size() * sizeof(Vertex), vertices.data());
}

/**
* パーティクルを描画する.
*
* @param  matProj 描画に使用するプロジェクション行列.
* @param  matView 描画に使用するビュー行列.
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

