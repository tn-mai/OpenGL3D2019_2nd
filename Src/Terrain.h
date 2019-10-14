/**
* @file Terrain.h
*/
#ifndef TERRAIN_H_INCLUDED
#define TERRAIN_H_INCLUDED
#include "Mesh.h"
#include "Texture.h"
#include "Light.h"
#include <glm/glm.hpp>
#include <string>
#include <vector>

namespace Terrain {

/**
* 高さマップ.
*
* 1. LoadFromFile()で画像ファイルから高さ情報を読み込む.
* 2. CreateMesh()で読み込んだ高さ情報から地形メッシュを作成する.
* 3. ある地点の高さを調べるにはHeight()を使う.
*/
class HeightMap {
public:
  HeightMap() = default;
  ~HeightMap() = default;

  bool LoadFromFile(const char* path, float scale, float baseLevel);
  float Height(const glm::vec3& pos) const;
  const glm::ivec2& Size() const;
  bool CreateMesh(Mesh::Buffer& meshBuffer,
    const char* meshName, const char* texName = nullptr) const;
  void UpdateLightIndex(const ActorList& lights);

private:
  std::string name;                ///< 元になった画像ファイル名.
  glm::ivec2 size = glm::ivec2(0); ///< ハイトマップの大きさ.
  std::vector<float> heights;      ///< 高さデータ.
  Texture::BufferPtr lightIndex[2];

  glm::vec3 CalcNormal(int x, int z) const;
};

} // namespace Terrain

#endif // TERRAIN_H_INCLUDED