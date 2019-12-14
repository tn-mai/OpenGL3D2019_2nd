/**
* @file Mesh.cpp
*/
#define NOMINMAX
#include "Mesh.h"
#include "SkeletalMesh.h"
#include "json11/json11.hpp"
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/constants.hpp>
#include <fstream>
#include <algorithm>
#include <iostream>

/**
* メッシュに関する機能を格納する名前空間.
*/
namespace Mesh {

/**
* ファイルを読み込む.
*
* @param path ファイル名.
*
* @return ファイルの内容.
*/
std::vector<char> ReadFile(const char* path)
{
  std::ifstream ifs(path, std::ios_base::binary);
  if (!ifs) {
    std::cerr << "[エラー] " << __func__ << ": " << path << "を開けません.\n";
    return {};
  }
  std::vector<char> tmp(1'000'000);
  ifs.rdbuf()->pubsetbuf(tmp.data(), tmp.size());

  ifs.seekg(0, std::ios_base::end);
  const std::streamoff size = ifs.tellg();
  ifs.seekg(0, std::ios_base::beg);

  std::vector<char> buf;
  buf.resize(static_cast<size_t>(size));
  ifs.read(&buf[0], size);

  return buf;
}

/**
* JSONの配列データをglm::vec3に変換する.
*
* @param json 変換元となる配列データ.
*
* @return jsonを変換してできたvec3の値.
*/
glm::vec3 GetVec3(const json11::Json& json)
{
  const std::vector<json11::Json>& a = json.array_items();
  if (a.size() < 3) {
    return glm::vec3(0);
  }
  return glm::vec3(a[0].number_value(), a[1].number_value(), a[2].number_value());
}

/**
* JSONの配列データをglm::quatに変換する.
*
* @param json 変換元となる配列データ.
*
* @return jsonを変換してできたquatの値.
*/
glm::quat GetQuat(const json11::Json& json)
{
  const std::vector<json11::Json>& a = json.array_items();
  if (a.size() < 4) {
    return glm::quat(0, 0, 0, 1);
  }
  return glm::quat(
    static_cast<float>(a[3].number_value()),
    static_cast<float>(a[0].number_value()),
    static_cast<float>(a[1].number_value()),
    static_cast<float>(a[2].number_value())
  );
}

/**
* JSONの配列データをglm::mat4に変換する.
*
* @param json 変換元となる配列データ.
*
* @return jsonを変換してできたmat4の値.
*/
glm::mat4 GetMat4(const json11::Json& json)
{
  const std::vector<json11::Json>& a = json.array_items();
  if (a.size() < 16) {
    return glm::mat4(1);
  }
  return glm::mat4(
    a[0].number_value(), a[1].number_value(), a[2].number_value(), a[3].number_value(),
    a[4].number_value(), a[5].number_value(), a[6].number_value(), a[7].number_value(),
    a[8].number_value(), a[9].number_value(), a[10].number_value(), a[11].number_value(),
    a[12].number_value(), a[13].number_value(), a[14].number_value(), a[15].number_value()
  );
}

/**
* アクセッサが指定するバイナリデータの位置とバイト数を取得する.
*
* @param accessor    glTFアクセッサ
* @param bufferViews バイナリデータを分割管理するためのデータ配列.
* @param binFiles    バイナリファイルの配列.
* @param pp          取得したバイナリデータの位置.
* @param pLength     取得したバイナリデータのバイト数.
* @param pStride     取得したバイナリデータのデータ幅(頂点データの定義で使用).
*/
void GetBuffer(const json11::Json& accessor, const json11::Json& bufferViews,
  const std::vector<std::vector<char>>& binFiles, const void** pp, size_t* pLength,
  int* pStride = nullptr)
{
  const int bufferViewId = accessor["bufferView"].int_value();
  const int byteOffset = accessor["byteOffset"].int_value();
  const json11::Json bufferView = bufferViews[bufferViewId];
  const int bufferId = bufferView["buffer"].int_value();
  const int baesByteOffset = bufferView["byteOffset"].int_value();
  int byteLength = bufferView["byteLength"].int_value();
  if (!accessor["count"].is_null()) {
    int unitByteSize;
    switch (accessor["componentType"].int_value()) {
    case GL_BYTE:
    case GL_UNSIGNED_BYTE:
      unitByteSize = 1;
      break;
    case GL_SHORT:
    case GL_UNSIGNED_SHORT:
      unitByteSize = 2;
      break;
    default:
      unitByteSize = 4;
      break;
    }
    const std::string& type = accessor["type"].string_value();
    static const char* const typeNameList[] = { "SCALAR", "VEC2", "VEC3", "VEC4", "MAT4" };
    static const int typeSizeList[] = { 1, 2, 3, 4, 16 };
    int typeSize = -1;
    for (size_t i = 0; i < 5; ++i) {
      if (type == typeNameList[i]) {
        typeSize = typeSizeList[i];
        break;
      }
    }
    if (typeSize < 0) {
      std::cerr << "[エラー]" << __func__ << ": アクセッサの型(" << type << ")には未対応です.\n";
      byteLength = 0;
    } else {
      byteLength = std::min(byteLength, accessor["count"].int_value() * unitByteSize * typeSize);
    }
  }

  if (pStride) {
    *pStride = bufferView["byteStride"].int_value();
  }
  *pLength = byteLength;
  *pp = binFiles[bufferId].data() + baesByteOffset + byteOffset;
}

/**
* メッシュバッファを初期化する.
*
* @param vboSize VBOのバイトサイズ.
* @param iboSize IBOのバイトサイズ.
*
* @retval true  初期化成功.
* @retval false 初期化失敗.
*/
bool Buffer::Init(GLsizeiptr vboSize, GLsizeiptr iboSize)
{
  if (!vbo.Create(GL_ARRAY_BUFFER, vboSize)) {
    return false;
  }
  if (!ibo.Create(GL_ELEMENT_ARRAY_BUFFER, iboSize)) {
    return false;
  }
  progStaticMesh = Shader::Program::Create("Res/StaticMesh.vert", "Res/StaticMesh.frag");
  if (progStaticMesh->IsNull()) {
    return false;
  }

  // スケルタルメッシュ用のシェーダーを読み込む.
  progSkeletalMesh = Shader::Program::Create(
    "Res/SkeletalMesh.vert", "Res/SkeletalMesh.frag");
  if (progSkeletalMesh->IsNull()) {
    return false;
  }
  SkeletalAnimation::BindUniformBlock(progSkeletalMesh);

  progTerrain = Shader::Program::Create("Res/Terrain.vert", "Res/Terrain.frag");
  if (progTerrain->IsNull()) {
    return false;
  }
  progWater = Shader::Program::Create("Res/Terrain.vert", "Res/Water.frag");
  if (progWater->IsNull()) {
    return false;
  }

  progShadow  = Shader::Program::Create("Res/StaticMesh.vert", "Res/Shadow.frag");
  progNonTexturedShadow  = Shader::Program::Create("Res/StaticMesh.vert", "Res/NonTexturedShadow.frag");
  progSkeletalShadow  = Shader::Program::Create("Res/SkeletalMesh.vert", "Res/Shadow.frag");
  if (progShadow->IsNull() || progNonTexturedShadow->IsNull() || progSkeletalShadow->IsNull()) {
    return false;
  }

  vboEnd = 0;
  iboEnd = 0;
  files.reserve(100);

  AddCube("Cube");

  return true;
}

/**
* 頂点データを追加する.
*
* @param data 追加するデータのポインタ.
* @param size 追加するデータのバイト数.
*
* @return データを追加した位置.
*         CreatePrimitiveのvOffsetパラメータとして使用する.
*/
GLintptr Buffer::AddVertexData(const void* data, size_t size)
{
  vbo.BufferSubData(vboEnd, size, data);
  const GLintptr tmp = vboEnd;
  vboEnd += ((size + 3) / 4) * 4;
  return tmp;
}

/**
* インデックスデータを追加する.
*
* @param data 追加するデータのポインタ.
* @param size 追加するデータのバイト数.
*
* @return データを追加した位置.
*         プリミティブのindicesパラメータとして使うことができる.
*/
GLintptr Buffer::AddIndexData(const void* data, size_t size)
{
  ibo.BufferSubData(iboEnd, size, data);
  const GLintptr tmp = iboEnd;
  iboEnd += ((size + 3) / 4) * 4;
  return tmp;
}

/**
* プリミティブを作成する.
*
* @param count    プリミティブのインデックスデータの数.
* @param type     インデックスデータの型
*                 (GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT, GL_UNSIGNED_INTのいずれか).
* @param iOffset  IBO内のインデックスデータの開始位置.
* @param vOffset  VBO内の頂点データの開始位置.
*
* @return 作成したPrimitive構造体.
*/
Primitive Buffer::CreatePrimitive(
  size_t count, GLenum type, size_t iOffset, size_t vOffset) const
{
  // プリミティブ用のVAOを作成.
  std::shared_ptr<VertexArrayObject> vao = std::make_shared<VertexArrayObject>();
  vao->Create(vbo.Id(), ibo.Id());
  vao->Bind();
  vao->VertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
    vOffset + offsetof(Vertex, position));
  vao->VertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
    vOffset + offsetof(Vertex, texCoord));
  vao->VertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
    vOffset + offsetof(Vertex, normal));
  vao->Unbind();

  // プリミティブのメンバ変数を設定.
  Primitive p;
  p.mode = GL_TRIANGLES;
  p.count = static_cast<GLsizei>(count);
  p.type = type;
  p.indices = reinterpret_cast<const GLvoid*>(iOffset);
  p.baseVertex = 0;
  p.vao = vao;
  p.material = 0; // マテリアルは0番で固定.

  return p;
}

/**
* マテリアルを作成する.
*
* @param color   マテリアルの基本色.
* @param texture マテリアルのテクスチャ.
*
* @return 作成したMaterial構造体.
*/
Material Buffer::CreateMaterial(
  const glm::vec4& color, Texture::Image2DPtr texture) const
{
  Material m;
  m.baseColor = color;
  m.texture[0] = texture;
  m.program = progStaticMesh;
  m.progSkeletalMesh = progSkeletalMesh;
  m.progShadow = progShadow;
  return m;
}

/**
* メッシュを追加する.
*
* @param name      メッシュ及びファイルの名前.
* @param primitive メッシュとして追加するプリミティブ.
* @param material  プリミティブ用のマテリアル.
*
* @retval true  追加成功.
* @retval false 追加失敗(同名のメッシュが登録済み).
*/
bool Buffer::AddMesh(
  const char* name, const Primitive& primitive, const Material& material)
{
  if (files.find(name) != files.end()) {
    std::cerr << "[警告]" << __func__ << ": " << name <<
      "という名前は既に追加されています.\n";
    return false;
  }

  FilePtr p = std::make_shared<File>();
  p->name = name;
  p->materials.push_back(material);
  p->meshes.resize(1);
  p->meshes[0].name = name;
  p->meshes[0].primitives.push_back(primitive);

  files.insert(std::make_pair(p->name, p));
  std::cout << "[情報]" << __func__ << ": メッシュ'" << name << "'を追加.\n";
  return true;
}

/**
* 頂点属性を設定する.
*
* @param prim        頂点データを設定するプリミティブ.
* @param index       設定する頂点属性のインデックス.
* @param accessor    頂点データの格納情報.
* @param bufferViews 頂点データを参照するためのバッファ・ビュー配列.
* @param binFiles    頂点データを格納しているバイナリデータ配列.
*
* @retval true  設定成功.
* @retval false 設定失敗.
*/
bool Buffer::SetAttribute(Primitive* prim, int index, const json11::Json& accessor,
  const json11::Json& bufferViews, const std::vector<std::vector<char>>& binFiles)
{
  if (accessor.is_null()) {
    std::cerr << "[エラー]" << __func__ << ": アクセッサがnullです.\n";
    return true;
  }
  static const char* const typeNameList[] = { "SCALAR", "VEC2", "VEC3", "VEC4" };
  static const int typeSizeList[] = { 1, 2, 3, 4 };
  const std::string& type = accessor["type"].string_value();
  int size = -1;
  for (size_t i = 0; i < 4; ++i) {
    if (type == typeNameList[i]) {
      size = typeSizeList[i];
      break;
    }
  }
  if (size < 0) {
    std::cerr << "[エラー]" << __func__ << ": " << type << "は頂点属性に設定できません.\n";
    return false;
  }

  const void* p;
  size_t byteLength;
  int byteStride;
  GetBuffer(accessor, bufferViews, binFiles, &p, &byteLength, &byteStride);
  const GLenum componentType = accessor["componentType"].int_value();
  prim->vao->Bind();
  prim->vao->VertexAttribPointer(index, size, componentType, GL_FALSE, byteStride, vboEnd);
  prim->vao->Unbind();

  vbo.BufferSubData(vboEnd, byteLength, p);
  vboEnd += ((byteLength + 3) / 4) * 4; // 4バイト境界に整列.
  return true;
}

/**
* glTFファイルを読み込む.
*
* @param path glTFファイル名.
*
* @retval true  読み込み成功.
* @retval false 読み込み失敗.
*/
bool Buffer::LoadMesh(const char* path)
{
  // glTFファイルを読み込む.
  std::vector<char> gltfFile = ReadFile(path);
  if (gltfFile.empty()) {
    return false;
  }
  gltfFile.push_back('\0');


  // JSON解析.
  std::string error;
  const json11::Json json = json11::Json::parse(gltfFile.data(), error);
  if (!error.empty()) {
    std::cerr << "[エラー]" << __func__ << ": " << path <<
      "の読み込みに失敗しました.\n  " << error << "\n";
    return false;
  }

  // バイナリファイルを読み込む.
  std::vector<std::vector<char>> binFiles;
  for (const json11::Json& buffer : json["buffers"].array_items()) {
    const json11::Json& uri = buffer["uri"];
    if (!uri.is_string()) {
      std::cerr << "[エラー]" << __func__ << ": " << path << "に不正なuriがあります.\n";
      return false;
    }
    const std::string binPath = std::string("Res/") + uri.string_value();
    binFiles.push_back(ReadFile(binPath.c_str()));
    if (binFiles.back().empty()) {
      return false;
    }
  }

  FilePtr pFile = std::make_shared<File>();
  File& file = *pFile;

  // 頂点データとインデックスデータをGPUメモリへ転送.
  const json11::Json& accessors = json["accessors"];
  const json11::Json& bufferViews = json["bufferViews"];
  file.meshes.reserve(json["meshes"].array_items().size());
  for (const auto& currentMesh : json["meshes"].array_items()) {
    Mesh mesh;
    mesh.name = currentMesh["name"].string_value();
    const std::vector<json11::Json>& primitives = currentMesh["primitives"].array_items();
    mesh.primitives.resize(primitives.size());
    for (size_t primId = 0; primId < primitives.size(); ++primId) {
      const json11::Json& primitive = currentMesh["primitives"][primId];

      // 頂点インデックス.
      {
        const int accessorId_index = primitive["indices"].int_value();
        const json11::Json& accessor = accessors[accessorId_index];
        if (accessor["type"].string_value() != "SCALAR") {
          std::cerr << "ERROR: インデックスデータ・タイプはSCALARでなくてはなりません \n";
          std::cerr << "  type = " << accessor["type"].string_value() << "\n";
          return false;
        }

        mesh.primitives[primId].mode = primitive["mode"].is_null() ? GL_TRIANGLES : primitive["mode"].int_value();
        mesh.primitives[primId].count = accessor["count"].int_value();
        mesh.primitives[primId].type = accessor["componentType"].int_value();
        mesh.primitives[primId].indices = reinterpret_cast<const GLvoid*>(iboEnd);

        const void* p;
        size_t byteLength;
        GetBuffer(accessor, bufferViews, binFiles, &p, &byteLength);
        ibo.BufferSubData(iboEnd, byteLength, p);
        iboEnd += ((byteLength + 3) / 4) * 4; // 次に来るのがどのデータ型でも大丈夫なように4バイト境界に整列.
      }

      // 頂点属性.
      const json11::Json& attributes = primitive["attributes"];
      const int accessorId_position = attributes["POSITION"].int_value();
      const int accessorId_normal = attributes["NORMAL"].is_null() ? -1 : attributes["NORMAL"].int_value();
      const int accessorId_texcoord = attributes["TEXCOORD_0"].is_null() ? -1 : attributes["TEXCOORD_0"].int_value();
      mesh.primitives[primId].vao = std::make_shared<VertexArrayObject>();
      mesh.primitives[primId].vao->Create(vbo.Id(), ibo.Id());
      SetAttribute(&mesh.primitives[primId], 0, accessors[accessorId_position], bufferViews, binFiles);
      SetAttribute(&mesh.primitives[primId], 1, accessors[accessorId_texcoord], bufferViews, binFiles);
      SetAttribute(&mesh.primitives[primId], 2, accessors[accessorId_normal], bufferViews, binFiles);

      mesh.primitives[primId].material = primitive["material"].int_value();
    }
    file.meshes.push_back(mesh);
  }

  // マテリアルを取得.
  {
    const std::vector<json11::Json> materials = json["materials"].array_items();
    file.materials.reserve(materials.size());
    for (const json11::Json& material : materials) {
      std::string texturePath;
      const json11::Json& pbr = material["pbrMetallicRoughness"];
      const json11::Json& index = pbr["baseColorTexture"]["index"];
      if (index.is_number()) {
        const int textureId = index.int_value();
        const json11::Json& texture = json["textures"][textureId];
        const int imageSourceId = texture["source"].int_value();
        const json11::Json& imageName = json["images"][imageSourceId]["name"];
        if (imageName.is_string()) {
          texturePath = std::string("Res/") + imageName.string_value() + ".tga";
        }
      }
      glm::vec4 col(0, 0, 0, 1);
      const std::vector<json11::Json>& baseColorFactor = pbr["baseColorFactor"].array_items();
      if (baseColorFactor.size() >= 4) {
        for (size_t i = 0; i < 4; ++i) {
          col[i] = static_cast<float>(baseColorFactor[i].number_value());
        }
      }
      Texture::Image2DPtr tex;
      if (!texturePath.empty()) {
        tex = Texture::Image2D::Create(texturePath.c_str());
      }
      file.materials.push_back(CreateMaterial(col, tex));
    }
  }

  file.name = path;
  files.insert(std::make_pair(file.name, pFile));

  std::cout << "[INFO]" << __func__ << ": '" << path << "'を読み込みました.\n";
  for (size_t i = 0; i < file.meshes.size(); ++i) {
    std::cout << "  mesh[" << i << "] = " << file.meshes[i].name << "\n";
  }

  return true;
}

/**
* ファイルを取得する.
*
* @param name 取得したいファイルの名前.
*
* @return nameと同じ名前を持つメッシュファイル.
*/
FilePtr Buffer::GetFile(const char* name) const
{
  const auto itr = files.find(name);
  if (itr == files.end()) {
    std::cerr << "[警告]" << __func__ << ": " << name <<
      "という名前のメッシュは追加されていません.\n";
    static FilePtr empty = std::make_shared<File>();
    return empty;
  }
  return itr->second;
}

/**
* 立方体を追加する.
*
* @param name 立方体のメッシュ名.
*/
void Buffer::AddCube(const char* name)
{
  //    6---7      +Y -Z
  //   /|  /|       |/
  //  / 5-/-4  -X --*-- +X
  // 3---2 /       /|
  // |/  |/     +Z -Y
  // 0---1
  const glm::vec3 basePositions[] = {
    {-1,-1, 1}, { 1,-1, 1}, { 1, 1, 1}, {-1, 1, 1},
    { 1,-1,-1}, {-1,-1,-1}, {-1, 1,-1}, { 1, 1,-1},
  };
  const glm::vec2 baseTexCoords[] = { { 0, 1}, { 0, 0}, { 1, 0}, { 1, 1} };
  const glm::vec3 normals[] = {
    { 0, 0, 1}, { 1, 0, 0}, { 0, 0,-1}, {-1, 0, 0},
    { 0, -1, 0}, { 0, 1, 0} };
  const int planes[6][4] = {
    { 0, 1, 2, 3}, { 1, 4, 7, 2}, { 4, 5, 6, 7}, { 5, 0, 3, 6},
    { 5, 4, 1, 0}, { 3, 2, 7, 6} };
  const GLubyte baseIndices[] = { 0, 1, 2, 2, 3, 0 };

  std::vector<Vertex> vertices;
  vertices.reserve(4 * 6);
  std::vector<GLubyte> indices;
  indices.reserve(6 * 6);

  // 頂点データとインデックスデータを作成.
  for (size_t plane = 0; plane < 6; ++plane) {
    for (size_t i = 0; i < 4; ++i) {
      Vertex v;
      v.position = basePositions[planes[plane][i]];
      v.texCoord = baseTexCoords[i];
      v.normal = normals[plane];
      vertices.push_back(v);
    }
    for (size_t i = 0; i < 6; ++i) {
      indices.push_back(static_cast<GLubyte>(baseIndices[i] + (plane * 4)));
    }
  }

  // メッシュを追加.
  const size_t vOffset =
    AddVertexData(vertices.data(), vertices.size() * sizeof(Vertex));
  const size_t iOffset =
    AddIndexData(indices.data(), indices.size() * sizeof(GLubyte));
  const Primitive p =
    CreatePrimitive(indices.size(), GL_UNSIGNED_BYTE, iOffset, vOffset);
  const Material m = CreateMaterial(glm::vec4(1), nullptr);
  AddMesh(name, p, m);
}

/**
* 平面を追加する.
*
* @param name 平面のメッシュ名.
*/
FilePtr Buffer::AddPlane(const char* name)
{
  const Vertex v[] = {
    {{-1,-1, 0}, { 0, 0}, { 0, 0, 1}},
    {{ 1,-1, 0}, { 1, 0}, { 0, 0, 1}},
    {{ 1, 1, 0}, { 1, 1}, { 0, 0, 1}},
    {{-1, 1, 0}, { 0, 1}, { 0, 0, 1}},
  };
  const GLubyte i[] = { 0, 1, 2, 2, 3, 0 };

  const size_t vOffset = AddVertexData(v, sizeof(v));
  const size_t iOffset = AddIndexData(i, sizeof(i));
  const Primitive p = CreatePrimitive(6, GL_UNSIGNED_BYTE, iOffset, vOffset);
  const Material m = CreateMaterial(glm::vec4(1), nullptr);
  if (!AddMesh(name, p, m)) {
    return nullptr;
  }
  return GetFile(name);
}
  
/**
* シェーダにビュー・プロジェクション行列を設定する.
*
* @param matVP ビュー・プロジェクション行列.
*/
void Buffer::SetViewProjectionMatrix(const glm::mat4& matVP) const
{
  progStaticMesh->Use();
  progStaticMesh->SetViewProjectionMatrix(matVP);
  progSkeletalMesh->Use();
  progSkeletalMesh->SetViewProjectionMatrix(matVP);
  progTerrain->Use();
  progTerrain->SetViewProjectionMatrix(matVP);
  progWater->Use();
  progWater->SetViewProjectionMatrix(matVP);
  glUseProgram(0);
}

/**
* シェーダに影用のビュー・プロジェクション行列を設定する.
*
* @param matVP 影用ビュー・プロジェクション行列.
*/
void Buffer::SetShadowViewProjectionMatrix(const glm::mat4& matVP) const
{
  progStaticMesh->Use();
  progStaticMesh->SetShadowViewProjectionMatrix(matVP);
  progSkeletalMesh->Use();
  progSkeletalMesh->SetShadowViewProjectionMatrix(matVP);
  progTerrain->Use();
  progTerrain->SetShadowViewProjectionMatrix(matVP);
  progWater->Use();
  progWater->SetShadowViewProjectionMatrix(matVP);
  progShadow->Use();
  progShadow->SetViewProjectionMatrix(matVP);
  progNonTexturedShadow->Use();
  progNonTexturedShadow->SetViewProjectionMatrix(matVP);
  progSkeletalShadow->Use();
  progSkeletalShadow->SetViewProjectionMatrix(matVP);
  glUseProgram(0);
}

/**
* シェーダーにカメラのワールド座標を設定する.
*
* @param pos カメラのワールド座標.
*/
void Buffer::SetCameraPosition(const glm::vec3& pos) const
{
  progStaticMesh->Use();
  progStaticMesh->SetCameraPosition(pos);
  progSkeletalMesh->Use();
  progSkeletalMesh->SetCameraPosition(pos);
  progTerrain->Use();
  progTerrain->SetCameraPosition(pos);
  progWater->Use();
  progWater->SetCameraPosition(pos);
  glUseProgram(0);
}

/**
* シェーダーにアプリが起動してからの経過時間を設定する.
*
* @param time アプリが起動してからの経過時間(秒).
*/
void Buffer::SetTime(double time) const
{
  const float ftime = static_cast<float>(std::fmod(time, 24 * 60 * 60));
  progStaticMesh->Use();
  progStaticMesh->SetTime(ftime);
  progSkeletalMesh->Use();
  progSkeletalMesh->SetTime(ftime);
  progTerrain->Use();
  progTerrain->SetTime(ftime);
  progWater->Use();
  progWater->SetTime(ftime);
  glUseProgram(0);
}

/**
* 影用テクスチャをGLコンテキストに割り当てる.
*
* @param tex  影用テクスチャ.
*/
void Buffer::BindShadowTexture(const Texture::InterfacePtr& texture)
{
  shadowTextureTarget = texture->Target();
  glActiveTexture(GL_TEXTURE0 + Shader::Program::shadowTextureBindingPoint);
  glBindTexture(shadowTextureTarget, texture->Get());
}

/**
* 影用テクスチャの割り当てを解除する.
*/
void Buffer::UnbindShadowTexture()
{
  if (shadowTextureTarget != GL_NONE) {
    glActiveTexture(GL_TEXTURE0 + Shader::Program::shadowTextureBindingPoint);
    glBindTexture(shadowTextureTarget, 0);
    shadowTextureTarget = GL_NONE;
  }
}

/**
* メッシュを描画する.
*
* @param file 描画するファイル.
* @param matM  描画に使用するモデル行列.
* @param type  描画する
*/
void Draw(const FilePtr& file, const glm::mat4& matM, DrawType type)
{
  if (!file || file->meshes.empty() || file->materials.empty()) {
    return;
  }

  const Mesh& mesh = file->meshes[0];
  for (const Primitive& p : mesh.primitives) {
    if (p.material < static_cast<int>(file->materials.size())) {
      if (const GLenum error = glGetError()) {
        std::cout << "[エラー]" << std::hex << error << "\n";
      }

      p.vao->Bind();
      const Material& m = file->materials[p.material];
      Shader::ProgramPtr program = m.program;
      if (type == DrawType::shadow) {
        program = m.progShadow;
      }
      program->Use();
      program->SetModelMatrix(matM);
      if (const GLenum error = glGetError()) {
        std::cout << "[エラー]" << std::hex << error << "\n";
      }

      // テクスチャがあるときは、そのテクスチャIDを設定する. ないときは0を設定する.
      for (int i = 0; i < sizeof(m.texture)/sizeof(m.texture[0]); ++i) {
        glActiveTexture(GL_TEXTURE0 + i);
        if (m.texture[i]) {
          glBindTexture(m.texture[i]->Target(), m.texture[i]->Get());
        } else {
          glBindTexture(GL_TEXTURE_2D, 0);
          glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
          glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
          glBindTexture(GL_TEXTURE_BUFFER, 0);
        }
      }
      if (const GLenum error = glGetError()) {
        std::cout << "[エラー]" << std::hex << error << "\n";
      }

      glDrawElementsBaseVertex(p.mode, p.count, p.type, p.indices, p.baseVertex);
      p.vao->Unbind();
    }
  }
//  glActiveTexture(GL_TEXTURE0);
//  glBindTexture(GL_TEXTURE_2D, 0);
  glUseProgram(0);
}

} // namespace Mesh