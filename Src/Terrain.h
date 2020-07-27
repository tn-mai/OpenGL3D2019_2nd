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
* �����}�b�v.
*
* 1. LoadFromFile()�ŉ摜�t�@�C�����獂������ǂݍ���.
* 2. CreateMesh()�œǂݍ��񂾍�����񂩂�n�`���b�V�����쐬����.
* 3. ����n�_�̍����𒲂ׂ�ɂ�Height()���g��.
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
  bool CreateWaterMesh(Mesh::Buffer& meshBuffer,
    const char* meshName, float waterLevel) const;
  void SetupGrassShader(const Mesh::Buffer& meshBuffer, const char* meshName) const;
  void UpdateLightIndex(const ActorList& lights);
  void UpdateGrass(const Collision::Frustum& frustum);
  size_t GetGrassInstanceCount() const { return grassInstanceCount; }

private:
  std::string name;                ///< ���ɂȂ����摜�t�@�C����.
  glm::ivec2 size = glm::ivec2(0); ///< �n�C�g�}�b�v�̑傫��.
  std::vector<float> heights;      ///< �����f�[�^.
  struct GrassInfo {
    uint8_t grassHeight; // ����.
    float height;        // �n�ʂ̍���.
  };
  std::vector<GrassInfo> grassHeights;
  Texture::BufferPtr lightIndex[2];
  Texture::Image2DPtr texHeightMap;
  Texture::Image2DPtr texGrassHeightMap; ///< ����}�b�v�e�N�X�`��.
  Texture::BufferPtr texGrassInstanceData;
  size_t grassInstanceCount = 0;

  glm::vec3 CalcNormal(int x, int z) const;
};

} // namespace Terrain

#endif // TERRAIN_H_INCLUDED