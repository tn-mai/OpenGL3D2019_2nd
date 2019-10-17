/**
* @file Terrain.cpp
*/
#include "Terrain.h"
#include "Texture.h"
#include <iostream>
#include <algorithm>

/// 地形に関するクラス等を格納する名前空間.
namespace Terrain {

/**
* 画像ファイルから地形データを読み込む.
*
* @param path      画像ファイル名.
* @param scale     高さに掛ける係数.
* @param baseLevel 高さ0とみなす高さ値(色データ0.0～1.0のどこを高さ0とするか).
*
* @retval true  読み込み成功.
* @retval false 読み込み失敗.
*
* 画像の赤要素を高さデータとみなして読み込む.
*/
bool HeightMap::LoadFromFile(const char* path, float scale, float baseLevel)
{
  // 画像ファイルを読み込む.
  Texture::ImageData imageData;
  if (!Texture::LoadImage2D(path, &imageData)) {
    std::cerr << "[エラー]" << __func__ << ": ハイトマップを読み込めませんでした.\n";
    return false;
  }

  name = path;

  // 画像の大きさを保存.
  size = glm::ivec2(imageData.width, imageData.height);

  // 画像データは下から上に向かって格納されているので、上下反転しながら高さデータに変換.
  heights.resize(imageData.data.size());
  for (int y = 0; y < size.y; ++y) {
    const int offsetY = (size.y - 1) - y; // 上下反転.
    for (int x = 0; x < size.x; ++x) {
      const glm::vec4 color = imageData.GetColor(x, y);
      heights[offsetY * size.x + x] = (color.r - baseLevel) * scale;
    }
  }

  for (int i = 0; i < 2; ++i) {
    lightIndex[i] = Texture::Buffer::Create(GL_RGBA8I, size.x * size.y * 4, nullptr, GL_DYNAMIC_DRAW);
    if (!lightIndex[i]) {
      return false;
    }
  }

  return true;
}

/**
* 高さを取得する.
*
* @param pos 高さを取得する座標.
*
* @return posの位置の高さ.
*
* ハイトマップはXZ平面に対応し、原点(0, 0)からプラス方向に定義される.
*/
float HeightMap::Height(const glm::vec3& pos) const
{
  // 座標が高さマップの範囲内に収まるように切り上げ、または切り捨てる.
  const glm::vec2 fpos = glm::clamp(
    glm::vec2(pos.x, pos.z), glm::vec2(0.0f), glm::vec2(size) - glm::vec2(1));

  // 「高さ」はfposが含まれる正方形の左上頂点を基準として計算するので、左上の座標を求める.
  // マップは右下がプラス方向で、正方形の大きさは1.0x1.0とする.
  // そうすることで、左上の頂点座標を求める計算は、単にfposの小数部を切り捨てるだけにる.
  // ただし、右端と下端の1列は、その先に正方形がないことから左上座標として使えないため、
  // 左上頂点座標はsize-2に制限する. 座標fposは
  //   (index.x, index.y), (index.x+1, index.y),
  //   (index.x, index.y+1), (index.x+1, index.y+1)
  // の4点からなる正方形の内側に存在することになる.
  const glm::ivec2 index = glm::min(glm::ivec2(fpos), size - glm::ivec2(2));

  // 左上頂点からの相対座標を計算.
  const glm::vec2 offset = fpos - glm::vec2(index);

  // 地形は4頂点からなる正方形であり、正方形は下記のように2つの三角形として定義される.
  //     -Y
  //    +--+
  // -X |／| +X
  //    +--+
  //     +Y
  // 奥が-Y方向、手前が+Y方向で、1x1の正方形であることから、
  // 「offset.x + offset.y < 0」ならば、offsetは左上の三角形領域に存在する.
  // そうでなければ右下の三角形領域に存在する.
  if (offset.x + offset.y < 1) {
    const float h0 = heights[index.y * size.x       + index.x];
    const float h1 = heights[index.y * size.x       + (index.x + 1)];
    const float h2 = heights[(index.y + 1) * size.x + index.x];
    return h0 + (h1 - h0) * offset.x + (h2 - h0) * offset.y;
  } else {
    const float h0 = heights[(index.y + 1) * size.x + (index.x + 1)];
    const float h1 = heights[(index.y + 1) * size.x + index.x];
    const float h2 = heights[index.y * size.x       + (index.x + 1)];
    return h0 + (h1 - h0) * (1.0f - offset.x) + (h2 - h0) * (1.0f - offset.y);
  }
}

/**
* 高さマップからメッシュを作成する.
*
* @param meshBuffer メッシュ作成先となるメッシュバッファ.
* @param meshName   作成するメッシュの名前.
* @param texName    メッシュに貼り付けるテクスチャファイル名.
*
* @retval true  メッシュの作成に成功.
* @retval false メッシュを作成できなかった.
*
* 三角形は以下の形になる.
*   d--c
*   |／|
*   a--b
*/
bool HeightMap::CreateMesh(
  Mesh::Buffer& meshBuffer, const char* meshName, const char* texName) const
{
  if (heights.empty()) {
    std::cerr << "[エラー]" << __func__ << ": ハイトマップが読み込まれていません.\n";
    return false;
  }

  // 頂点データを作成.
  Mesh::Vertex v;
  std::vector<Mesh::Vertex> vertices;
  vertices.reserve(size.x * size.y);
  for (int z = 0; z < size.y; ++z) {
    for (int x = 0; x < size.x; ++x) {
      // テクスチャ座標は上がプラスなので、向きを逆にする必要がある.
      v.position = glm::vec3(x, heights[z * size.x + x], z);
      v.texCoord = glm::vec2(x, (size.y - 1) - z) / (glm::vec2(size) - 1.0f);
      v.normal = CalcNormal(x, z);
      vertices.push_back(v);
    }
  }
  const size_t vOffset =
    meshBuffer.AddVertexData(vertices.data(), vertices.size() * sizeof(Mesh::Vertex));

  // インデックスデータを作成.
  std::vector<GLuint> indices;
  indices.reserve(size.x * size.y);
  for (int z = 0; z < size.y - 1; ++z) {
    for (int x = 0; x < size.x - 1; ++x) {
      const GLuint a = (z + 1) * size.x + x;
      const GLuint b = (z + 1) * size.x + (x + 1);
      const GLuint c = z       * size.x + (x + 1);
      const GLuint d = z       * size.x + x;
      indices.push_back(a);
      indices.push_back(b);
      indices.push_back(c);

      indices.push_back(c);
      indices.push_back(d);
      indices.push_back(a);
    }
  }
  const size_t iOffset =
    meshBuffer.AddIndexData(indices.data(), indices.size() * sizeof(GLuint));

  // 頂点データとインデックスデータからメッシュを作成.
  Mesh::Primitive p =
    meshBuffer.CreatePrimitive(indices.size(), GL_UNSIGNED_INT, iOffset, vOffset);
  Mesh::Material m = meshBuffer.CreateMaterial(glm::vec4(1), nullptr);
  m.texture[0] = Texture::Image2D::Create("Res/Terrain_Ratio.tga");
  m.texture[1] = Texture::Image2D::Create("Res/Terrain_Soil.tga");
  m.texture[2] = Texture::Image2D::Create("Res/Terrain_Rock.tga");
  m.texture[3] = Texture::Image2D::Create("Res/Terrain_Plant.tga");
  m.texture[4] = lightIndex[0];
  m.texture[5] = lightIndex[1];
  m.program = meshBuffer.GetTerrainShader();
  meshBuffer.AddMesh(meshName, p, m);

  return true;
}

/**
* ライトインデックスを更新する.
*
* @param lights ライトアクターのリスト.
*/
void HeightMap::UpdateLightIndex(const ActorList& lights)
{
  std::vector<glm::i8vec4> pointLightIndex;
  std::vector<glm::i8vec4> spotLightIndex;
  pointLightIndex.resize(size.x * size.y, glm::i8vec4(-1));
  spotLightIndex.resize(size.x * size.y, glm::i8vec4(-1));
  for (int y = 0; y < size.y; ++y) {
    for (int x = 0; x < size.x; ++x) {
      std::vector<ActorPtr> neiborhood = lights.FindNearbyActors(glm::vec3(x + 0.5f, 0, y + 0.5f), 20);
      int pointLightCount = 0;
      glm::i8vec4& pointLight = pointLightIndex[y * size.x + x];
      int spotLightCount = 0;
      glm::i8vec4& spotLight = spotLightIndex[y * size.x + x];
      for (auto light : neiborhood) {
        if (PointLightActorPtr p = std::dynamic_pointer_cast<PointLightActor>(light)) {
          if (pointLightCount < 4) {
            pointLight[pointLightCount] = p->index;
            ++pointLightCount;
          }
        } else if (SpotLightActorPtr p = std::dynamic_pointer_cast<SpotLightActor>(light)) {
          if (spotLightCount < 4) {
            spotLight[spotLightCount] = p->index;
            ++spotLightCount;
          }
        }
      }
    }
  }
  lightIndex[0]->BufferSubData(0, pointLightIndex.size() * 4, pointLightIndex.data());
  lightIndex[1]->BufferSubData(0, spotLightIndex.size() * 4, spotLightIndex.data());
}

/**
* 高さ情報から法線を計算する.
*
* @param centerX 計算対象のX座標.
* @param centerr 計算対象のZ座標.
*
* @return (centerX, centerZ)の位置の法線.
*/
glm::vec3 HeightMap::CalcNormal(int centerX, int centerZ) const
{
  const glm::ivec2 offsetList[] = {
    { 0,-1}, { 1,-1}, { 1, 0}, { 0, 1}, {-1, 1}, {-1, 0}, { 0,-1}
  };
  const glm::vec3 center(centerX, heights[centerZ * size.x + centerX], centerZ);
  glm::vec3 sum(0);
  for (size_t i = 0; i < 6; ++i) {
    glm::vec3 p0(centerX + offsetList[i].x, 0, centerZ + offsetList[i].y);
    if (p0.x < 0 || p0.x >= size.x || p0.z < 0 || p0.z >= size.y) {
      continue;
    }
    p0.y = heights[static_cast<size_t>(p0.z * size.x + p0.x)];

    glm::vec3 p1(centerX + offsetList[i + 1].x, 0, centerZ + offsetList[i + 1].y);
    if (p1.x < 0 || p1.x >= size.x || p1.z < 0 || p1.z >= size.y) {
      continue;
    }
    p1.y = heights[static_cast<size_t>(p1.z * size.x + p1.x)];

    sum += normalize(cross(p1 - center, p0 - center));
  }
  return normalize(sum);
}

} // namespace Terrain