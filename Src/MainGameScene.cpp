/**
* @file MainGameScene.cpp
*/
#include "GLFWEW.h"
#include "MainGameScene.h"
#include "StatusScene.h"
#include "EventScene.h"
#include "GameOverScene.h"
#include "SkeletalMeshActor.h"
#include "EventScript.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/constants.hpp>
#include <random>
#include <iostream>

/**
* �Փ˂���������.
*
* @param a  �Փ˂����A�N�^�[���̂P.
* @param b  �Փ˂����A�N�^�[���̂Q.
* @param p  �Փˈʒu.
*/
void PlayerCollisionHandler(const ActorPtr& a, const ActorPtr& b, const glm::vec3& p)
{
  const glm::vec3 v = a->colWorld.s.center - p;
  // �Փˈʒu�Ƃ̋������߂����Ȃ������ׂ�.
  if (dot(v, v) > FLT_EPSILON) {
    // a��b�ɏd�Ȃ�Ȃ��ʒu�܂ňړ�.
    const glm::vec3 vn = normalize(v);
    float radiusSum = a->colWorld.s.r;
    switch (b->colWorld.type) {
    case Collision::Shape::Type::sphere: radiusSum += b->colWorld.s.r; break;
    case Collision::Shape::Type::capsule: radiusSum += b->colWorld.c.r; break;
    }
    const float distance = radiusSum - glm::length(v) + 0.01f;
    a->position += vn * distance;
    a->colWorld.s.center += vn * distance;
    if (a->velocity.y < 0 && vn.y >= glm::cos(glm::radians(60.0f))) {
      a->velocity.y = 0;
    }
  } else {
    // �ړ���������(�������߂�����ꍇ�̗�O����).
    const float deltaTime = static_cast<float>(GLFWEW::Window::Instance().DeltaTime());
    const glm::vec3 deltaVelocity = a->velocity * deltaTime;
    a->position -= deltaVelocity;
    a->colWorld.s.center -= deltaVelocity;
  }
}

/**
* �V�[��������������.
*
* @retval true  ����������.
* @retval false ���������s. �Q�[���i�s�s�ɂ��A�v���O�������I�����邱��.
*/
bool MainGameScene::Initialize()
{
  spriteRenderer.Init(1000, "Res/Sprite.vert", "Res/Sprite.frag");
  sprites.reserve(100);
  Sprite spr(Texture::Image2D::Create("Res/TitleBg.tga"));
  spr.Scale(glm::vec2(2));
  spr.Color(glm::vec4(0.5f, 1, 0.5f, 1));
  sprites.push_back(spr);

  fontRenderer.Init(1000);
  fontRenderer.LoadFromFile("Res/font.fnt");

  textWindow.Init("Res/TextWindow.tga", glm::vec2(0, -248), glm::vec2(48, 32), glm::vec2(0));
  textWindow.Open(L"�e�L�X�g�E�B���h�E�̎���\n����͉��s�e�X�g�B\n�R�s�ځB\n4\n5");

  meshBuffer.Init(1'000'000 * sizeof(Mesh::Vertex), 3'000'000 * sizeof(GLushort));
  meshBuffer.LoadMesh("Res/red_pine_tree.gltf");
  meshBuffer.LoadMesh("Res/black_pine_trees_3.gltf");
  meshBuffer.LoadMesh("Res/jizo_statue.gltf");
  meshBuffer.LoadSkeletalMesh("Res/bikuni.gltf");
  meshBuffer.LoadSkeletalMesh("Res/oni_small.gltf");
  meshBuffer.LoadMesh("Res/wall_stone.gltf");

  // FBO���쐬����.
  const GLFWEW::Window& window = GLFWEW::Window::Instance();
  fboMain = FrameBufferObject::Create(window.Width(), window.Height(), GL_RGBA16F);
  Mesh::FilePtr rt = meshBuffer.AddPlane("RenderTarget");
  if (rt) {
    rt->materials[0].program = Shader::Program::Create("Res/DepthOfField.vert", "Res/DepthOfField.frag");
    rt->materials[0].texture[0] = fboMain->GetColorTexture();
    rt->materials[0].texture[1] = fboMain->GetDepthTexture();
  }
  if (!rt || !rt->materials[0].program) {
    return false;
  }

  // ���𑜓x�̏c��1/2(�ʐςł�1/4)�̑傫���̃u���[���pFBO�����.
  int w = window.Width();
  int h = window.Height();
  for (int j = 0; j < sizeof(fboBloom) / sizeof(fboBloom[0]); ++j) {
    w /= 2;
    h /= 2;
    for (int i = 0; i < sizeof(fboBloom[0]) / sizeof(fboBloom[0][0]); ++i) {
      fboBloom[j][i] = FrameBufferObject::Create(w, h, GL_RGBA16F, FrameBufferType::colorOnly);
      if (!fboBloom[j][i]) {
        return false;
      }
    }
  }

  // �u���[���E�G�t�F�N�g�p�̕��ʃ|���S�����b�V�����쐬����.
  fboDepthOfField = FrameBufferObject::Create(window.Width(), window.Height(), GL_RGBA16F);
  if (Mesh::FilePtr mesh = meshBuffer.AddPlane("BrightPassFilter")) {
    Shader::ProgramPtr p = Shader::Program::Create("Res/Simple.vert", "Res/BrightPassFilter.frag");
    p->Use();
    p->SetViewProjectionMatrix(glm::mat4(1));
    mesh->materials[0].program = p;
    mesh->materials[0].texture[0] = fboDepthOfField->GetColorTexture();
  }
  if (Mesh::FilePtr mesh = meshBuffer.AddPlane("GaussianBlur9")) {
    Shader::ProgramPtr p = Shader::Program::Create("Res/Simple.vert", "Res/GaussianBlur9.frag");
    p->Use();
    p->SetViewProjectionMatrix(glm::mat4(1));
    mesh->materials[0].program = p;
  }
  if (Mesh::FilePtr mesh = meshBuffer.AddPlane("Simple")) {
    Shader::ProgramPtr p = Shader::Program::Create("Res/Simple.vert", "Res/Simple.frag");
    p->Use();
    p->SetViewProjectionMatrix(glm::mat4(1));
    mesh->materials[0].program = p;
  }
  if (glGetError()) {
    std::cout << "[�G���[]" << __func__ << ":�u���[���p���b�V���̍쐬�Ɏ��s.\n";
    return false;
  }

  // �V���h�E�pFBO���쐬����.
  {
    fboShadow = FrameBufferObject::Create(4096, 4096, GL_NONE, FrameBufferType::depthOnly);
    if (glGetError()) {
      std::cout << "[�G���[]" << __func__ << ":�V���h�E�pFBO�̍쐬�Ɏ��s.\n";
      return false;
    }
    // sampler2DShadow�̂��߂Ƀe�N�X�`����r���[�h��ݒ肷��.
    glBindTexture(GL_TEXTURE_2D, fboShadow->GetDepthTexture()->Get());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);

    // �{�[�_�[�J���[���g���āA�V���h�E�e�N�X�`���͈̔͊O�̐[�x�l���ő�(1.0)�ɂ��邱�ƂŁA
    // �͈͊O�̗̈�ɕs���R�ȉe�������Ȃ��悤�ɂ���.
    // ���s�����͈͕̔s���ɂ͑Ή��ł��Ȃ��̂ŁAnear�Afar�v���[���̐ݒ�͓K�؂ɂ��邱��.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    const glm::vec3 borderColor(1.0);
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, &borderColor.x);
    glBindTexture(GL_TEXTURE_2D, 0);
  }

  // �n�C�g�}�b�v���쐬����.
  if (!heightMap.LoadFromFile("Res/Terrain.tga", 50.0f, 0.5f)) {
    return false;
  }
  if (!heightMap.CreateMesh(meshBuffer, "Terrain")) {
    return false;
  }
  if (!heightMap.CreateWaterMesh(meshBuffer, "Water", -15)) {
    return false;
  }

  // �p�[�e�B�N���E�V�X�e��������������.
  particleSystem.Init(10000);
  {
    // �p�[�e�B�N���E�V�X�e���̃e�X�g�p�ɃG�~�b�^�[��ǉ�.
    ParticleEmitterParameter ep;
    ep.imagePath = "Res/FireParticle.tga";
    ep.position = glm::vec3(97, 0, 98);
    ep.position.y = heightMap.Height(ep.position);
    ep.emissionsPerSecond = 20.0f;
    ep.dstFactor = GL_ONE;
    ep.gravity = 0;
    ep.tiles = glm::ivec2(2, 2);
    ParticleParameter pp;
    pp.scale = glm::vec2(0.5f);
    pp.color = glm::vec4(0.8f, 0.3f, 0.1f, 1.0f);
    particleSystem.Add(ep, pp);
  }
  {
    ParticleEmitterParameter ep;
    ep.imagePath = "Res/CircleParticle.tga";
    ep.position = glm::vec3(97, 0, 100);
    ep.position.y = heightMap.Height(ep.position);
    ep.angle = glm::radians(30.0f);
    ParticleParameter pp;
    pp.lifetime = 2;
    pp.scale = glm::vec2(0.2f);
    pp.velocity = glm::vec3(0, 3, 0);
    pp.color = glm::vec4(0.1f, 0.3f, 0.8f, 1.0f);
    particleSystem.Add(ep, pp);
  }

  lightBuffer.Init(1);
  lightBuffer.BindToShader(meshBuffer.GetStaticMeshShader());
  lightBuffer.BindToShader(meshBuffer.GetSkeletalMeshShader());
  lightBuffer.BindToShader(meshBuffer.GetTerrainShader());
  lightBuffer.BindToShader(meshBuffer.GetWaterShader());

  glm::vec3 startPos(95, 0, 105);
  startPos.y = heightMap.Height(startPos);
  player = std::make_shared<PlayerActor>(&heightMap, meshBuffer, startPos);

  rand.seed(0);

  // ���C�g��z�u
  const int lightRangeMin = 80;
  const int lightRangeMax = 120;
  lights.Add(std::make_shared<DirectionalLightActor>("DLight", glm::vec3(0.15f, 0.25f, 0.2f)* 4.0f, glm::normalize(glm::vec3(1, -1, -1))));
  for (int i = 0; i < 30; ++i) {
    glm::vec3 color(1, 0.8f, 0.5f);
    glm::vec3 position(0);
    position.x = static_cast<float>(std::uniform_int_distribution<>(lightRangeMin, lightRangeMax)(rand));
    position.z = static_cast<float>(std::uniform_int_distribution<>(lightRangeMin, lightRangeMax)(rand));
    position.y = heightMap.Height(position) + 5;
    lights.Add(std::make_shared<PointLightActor>("PointLight", color * 20.0f, position));
  }
  for (int i = 0; i < 30; ++i) {
    glm::vec3 color(1, 2, 3);
    glm::vec3 direction(glm::normalize(glm::vec3(0.25f, -1, 0.25f)));
    glm::vec3 position(0);
    position.x = static_cast<float>(std::uniform_int_distribution<>(lightRangeMin, lightRangeMax)(rand));
    position.z = static_cast<float>(std::uniform_int_distribution<>(lightRangeMin, lightRangeMax)(rand));
    position.y = heightMap.Height(position) + 5;
    lights.Add(std::make_shared<SpotLightActor>("SpotLight", color * 25.0f, position, direction
      , glm::radians(20.0f), glm::radians(15.0f)));
  }
  lights.Update(0);
  lightBuffer.Update(lights, glm::vec3(0.1f, 0.05f, 0.15f));
  heightMap.UpdateLightIndex(lights);

  // ���n���l��z�u
  for (int i = 0; i < 4; ++i) {
    glm::vec3 position(0);
    position.x = static_cast<float>(std::uniform_int_distribution<>(50, 150)(rand));
    position.z = static_cast<float>(std::uniform_int_distribution<>(50, 150)(rand));
    position.y = heightMap.Height(position);
    glm::vec3 rotation(0);
    rotation.y = std::uniform_real_distribution<float>(0.0f, 3.14f * 2.0f)(rand);
    JizoActorPtr p = std::make_shared<JizoActor>(
      meshBuffer.GetFile("Res/jizo_statue.gltf"), position, i, this);
    p->scale = glm::vec3(3); // �����₷���悤�Ɋg��.
    objects.Add(p);
  }

  // �Εǂ�z�u
  {
    //  0
    // +-+
    //1| |
    // +-+
    //
    // + +-+-+-+
    // | |     |
    // + + +-+ +
    // |   |   |
    // +-+-+-+ +
    // |     | |
    // + +-+ + +
    //   |     |
    // +-+-+-+-+
    const int maze[5][5] = {
      { 0b10, 0b11, 0b01, 0b01, 0b10 },
      { 0b10, 0b00, 0b11, 0b00, 0b10 },
      { 0b11, 0b01, 0b01, 0b10, 0b10 },
      { 0b00, 0b11, 0b00, 0b00, 0b10 },
      { 0b01, 0b01, 0b01, 0b01, 0b00 },
    };
    const Mesh::FilePtr meshStoneWall = meshBuffer.GetFile("Res/wall_stone.gltf");
    const glm::vec3 basePosition(100, 0, 50);
    for (int y = 0; y < 5; ++y) {
      for (int x = 0; x < 5; ++x) {
        if (maze[y][x] & 0b01) {
          glm::vec3 position = basePosition + glm::vec3(x * 4 + 2, 0, y * 4);
          position.y = heightMap.Height(position);
          StaticMeshActorPtr p = std::make_shared<StaticMeshActor>(
            meshStoneWall, "StoneWall", 100, position, glm::vec3(0, 0, 0));
          p->colLocal = Collision::CreateOBB(glm::vec3(0, 0, 0),
            glm::vec3(1, 0, 0), glm::vec3(0, 1, 0), glm::vec3(0, 0, -1), glm::vec3(2, 2, 0.5f));
          objects.Add(p);
        }
        if (maze[y][x] & 0b10) {
          glm::vec3 position = basePosition + glm::vec3(x * 4, 0, y * 4 + 2);
          position.y = heightMap.Height(position);
          StaticMeshActorPtr p = std::make_shared<StaticMeshActor>(
            meshStoneWall, "StoneWall", 100, position, glm::vec3(0, glm::pi<float>() * 0.5f, 0));
          p->colLocal = Collision::CreateOBB(glm::vec3(0, 0, 0),
            glm::vec3(1, 0, 0), glm::vec3(0, 1, 0), glm::vec3(0, 0, -1), glm::vec3(2, 2, 0.5f));
          objects.Add(p);
        }
      }
    }
  }

  // �G��z�u.
  {
    const size_t oniCount = 10;
    enemies.Reserve(oniCount);
#if 0
    for (size_t i = 0; i < oniCount; ++i) {
      // �G�̈ʒu��(50,50)-(150,150)�͈̔͂��烉���_���ɑI��.
      glm::vec3 position(0);
      position.x = std::uniform_real_distribution<float>(50, 150)(rand);
      position.z = std::uniform_real_distribution<float>(50, 150)(rand);
      position.y = heightMap.Height(position);
      // �G�̌����������_���ɑI��.
      glm::vec3 rotation(0);
      rotation.y = std::uniform_real_distribution<float>(0, glm::pi<float>() * 2)(rand);
      const Mesh::SkeletalMeshPtr mesh = meshBuffer.GetSkeletalMesh("oni_small");
      SkeletalMeshActorPtr p = std::make_shared<SkeletalMeshActor>(
        mesh, "Kooni", 13, position, rotation);
      p->GetMesh()->Play("Run");
      p->colLocal = Collision::CreateCapsule(
        glm::vec3(0, 0.5f, 0), glm::vec3(0, 1, 0), 0.5f);
      enemies.Add(p);
    }
#endif
  }

  // �؂�z�u.
  {
    const size_t treeCount = 200;
    trees.Reserve(treeCount);
    const Mesh::FilePtr meshBlack = meshBuffer.GetFile("Res/black_pine_trees_3.gltf");
    const Mesh::FilePtr meshRed = meshBuffer.GetFile("Res/red_pine_tree.gltf");
    for (size_t i = 0; i < treeCount; ++i) {
      // �ʒu��(50,50)-(150,150)�͈̔͂��烉���_���ɑI��.
      glm::vec3 position(0);
      position.x = std::uniform_real_distribution<float>(20, 180)(rand);
      position.z = std::uniform_real_distribution<float>(20, 180)(rand);
      position.y = heightMap.Height(position);
      // �����������_���ɑI��.
      glm::vec3 rotation(0);
      rotation.y = std::uniform_real_distribution<float>(0, glm::radians(360.0f))(rand);
      rotation.x = std::uniform_real_distribution<float>(glm::radians(-10.0f), glm::radians(10.0f))(rand);
      glm::vec3 scale(std::normal_distribution<float>(0, 1)(rand) * 0.10f + 1.0f);
      StaticMeshActorPtr p = std::make_shared<StaticMeshActor>(
        (rand() % 3) >= 2 ? meshRed : meshBlack, "PineTree", 13, position, rotation, scale);
      p->colLocal = Collision::CreateCapsule(
        glm::vec3(0, 0, 0), glm::vec3(0, 3, 0), 1.0f);
      trees.Add(p);
    }
  }

  Update(0);

  EventScriptEngine::Instance().SetCamera(&camera);
  SceneStack::Instance().Push(std::make_shared<EventScene>("Res/SampleScript.txt"));

  return true;
}

/**
* �v���C���[�̓��͂���������.
*/
void MainGameScene::ProcessInput()
{
  GLFWEW::Window& window = GLFWEW::Window::Instance();

  // �v���C���[����.
  player->ProcessInput();

  if (window.GetGamePad().buttonDown & GamePad::START) {
    SceneStack::Instance().Push(std::make_shared<StatusScene>());
  } else if (window.GetGamePad().buttonDown & GamePad::X) {
    SceneStack::Instance().Replace(std::make_shared<GameOverScene>());
  }
}

/**
* �V�[�����X�V����.
*
* @param deltaTime  �O��̍X�V����̌o�ߎ���(�b).
*/
void MainGameScene::Update(float deltaTime)
{
  player->Update(deltaTime);
  enemies.Update(deltaTime);
  trees.Update(deltaTime);
  objects.Update(deltaTime);

  DetectCollision(player, enemies);
  DetectCollision(player, trees);
  DetectCollision(player, objects);

  // �v���C���[�̍U������.
  ActorPtr attackCollision = player->GetAttackCollision();
  if (attackCollision) {
    bool hit = false;
    DetectCollision(attackCollision, enemies,
      [this, &hit](const ActorPtr& a, const ActorPtr& b, const glm::vec3& p) {
        SkeletalMeshActorPtr bb = std::static_pointer_cast<SkeletalMeshActor>(b);
        bb->health -= a->health;
        if (bb->health <= 0) {
          bb->colLocal = Collision::Shape{};
          bb->health = 1;
          bb->GetMesh()->Play("Down", false);
        } else {
          bb->GetMesh()->Play("Hit", false);
        }
        hit = true;
      }
    );
    if (hit) {
      attackCollision->health = 0;
    }
  }

  lights.Update(deltaTime);

  // �J�����̏�Ԃ��X�V.
  {
    const float distance = 15.0f;
    const float angle = glm::radians(-30.0f);
    const glm::vec3 axis(1, 0, 0);
    const glm::vec3 offset(glm::rotate(glm::mat4(1), glm::radians(0.0f), glm::vec3(0, 1, 0)) * glm::rotate(glm::mat4(1), angle, axis) * glm::vec4(0, 0, distance, 1));
    camera.target = player->position + glm::vec3(0, 1.2f, 0);
    camera.position = camera.target + offset;
    camera.fNumber = 4.4f;
    camera.fov = glm::radians(30.0f);
  }

  // ���S�A�j���[�V�����̏I������G������.
  for (auto& e : enemies) {
    SkeletalMeshActorPtr enemy = std::static_pointer_cast<SkeletalMeshActor>(e);
    Mesh::SkeletalMeshPtr mesh = enemy->GetMesh();
    if (mesh->IsFinished()) {
      if (mesh->GetAnimation() == "Down") {
        enemy->health = 0;
      } else {
        mesh->Play("Wait");
      }
    }
  }

  // ���C�g�̍X�V.
  lightBuffer.Update(lights, glm::vec3(0.1f, 0.05f, 0.15f));
  for (auto e : trees) {
    const std::vector<ActorPtr> neighborhood = lights.FindNearbyActors(e->position, 20);
    std::vector<int> pointLightIndex;
    std::vector<int> spotLightIndex;
    pointLightIndex.reserve(neighborhood.size());
    spotLightIndex.reserve(neighborhood.size());
    for (auto light : neighborhood) {
      if (PointLightActorPtr p = std::dynamic_pointer_cast<PointLightActor>(light)) {
        if (pointLightIndex.size() < 8) {
          pointLightIndex.push_back(p->index);
        }
      } else if (SpotLightActorPtr p = std::dynamic_pointer_cast<SpotLightActor>(light)) {
        if (spotLightIndex.size() < 8) {
          spotLightIndex.push_back(p->index);
        }
      }
    }
    StaticMeshActorPtr p = std::static_pointer_cast<StaticMeshActor>(e);
    p->SetPointLightList(pointLightIndex);
    p->SetSpotLightList(spotLightIndex);
  }

  particleSystem.Update(Collision::CreateFrustum(camera), deltaTime);

  // �G��S�ł�������ړI�B���t���O��true�ɂ���.
  if (jizoId >= 0) {
    if (enemies.Empty()) {
      achivements[jizoId] = true;
      jizoId = -1;
    }
  }

  // �S�Ă̖ړI�B���t���O��true�ɂȂ��Ă����烁�b�Z�[�W��\��.
  int achivedCount = 0;
  for (auto e : achivements) {
    if (e) {
      ++achivedCount;
    }
  }
  if (isClear) {
    clearTimer -= deltaTime;
    if (clearTimer <= 0) {
      SceneStack::Instance().Replace(std::make_shared<GameOverScene>());
      return;
    }
  } else if (achivedCount >= 4) {
    isClear = true;
    clearTimer = 3;
  }

  player->UpdateDrawData(deltaTime);
  enemies.UpdateDrawData(deltaTime);
  trees.UpdateDrawData(deltaTime);
  objects.UpdateDrawData(deltaTime);

  spriteRenderer.BeginUpdate();
  for (const Sprite& e : sprites) {
    spriteRenderer.AddVertices(e);
  }
  spriteRenderer.EndUpdate();

  const GLFWEW::Window& window = GLFWEW::Window::Instance();
  const float w = static_cast<float>(window.Width());
  const float h = static_cast<float>(window.Height());
  const float lineHeight = fontRenderer.LineHeight();
  fontRenderer.BeginUpdate();
  fontRenderer.Color(glm::vec4(0, 1, 0, 1));
  fontRenderer.AddString(glm::vec2(-w * 0.5f + 32, h * 0.5f - lineHeight), L"���C���Q�[�����");
  fontRenderer.Color(glm::vec4(1));
  if (isClear) {
    fontRenderer.AddString(glm::vec2(-32 * 4, 0), L"�ړI��B�������I");
  }
  fontRenderer.EndUpdate();

  textWindow.Update(deltaTime);
}

/**
* ���b�V����`�悷��.
*/
void MainGameScene::RenderMesh(Mesh::DrawType drawType, const Collision::Frustum* pFrustum)
{
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);

  glm::vec3 cubePos(100, 0, 100);
  cubePos.y = heightMap.Height(cubePos);
  const glm::mat4 matModel = glm::translate(glm::mat4(1), cubePos);
  Mesh::Draw(meshBuffer.GetFile("Cube"), matModel, drawType);
  Mesh::Draw(meshBuffer.GetFile("Terrain"), glm::mat4(1), drawType);

  player->Draw(drawType);
  if (!pFrustum) {
    enemies.Draw(drawType);
    objects.Draw(drawType);
    glDisable(GL_CULL_FACE);
    trees.Draw(drawType);
  } else {
    enemies.Draw(*pFrustum, drawType);
    objects.Draw(*pFrustum, drawType);
    glDisable(GL_CULL_FACE);
    trees.Draw(*pFrustum, drawType);
  }

  glm::vec3 treePos(110, 0, 110);
  treePos.y = heightMap.Height(treePos);
  const glm::mat4 matTreeModel =
    glm::translate(glm::mat4(1), treePos) * glm::scale(glm::mat4(1), glm::vec3(3));
  Mesh::Draw(meshBuffer.GetFile("Res/red_pine_tree.gltf"), matTreeModel, drawType);

  glEnable(GL_CULL_FACE);
  if (drawType == Mesh::DrawType::color) {
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    Mesh::Draw(meshBuffer.GetFile("Water"), glm::mat4(1), drawType);
  }
}

/**
* �V�[����`�悷��.
*/
void MainGameScene::Render()
{
  const GLFWEW::Window& window = GLFWEW::Window::Instance();

  // �V���h�E�e�N�X�`���ɕ`��.
  {
    glBindFramebuffer(GL_FRAMEBUFFER, fboShadow->GetFramebuffer());
    auto tex = fboShadow->GetDepthTexture();
    glViewport(0, 0, tex->Width(), tex->Height());
    glClear(GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glDisable(GL_BLEND);

    // �f�B���N�V���i���E���C�g�̌�������e�p�̃r���[�s����쐬.
    glm::vec3 direction(0, -1, 0);
    for (auto e : lights) {
      if (auto p = std::dynamic_pointer_cast<DirectionalLightActor>(e)) {
        direction = p->direction;
        break;
      }
    }
    const glm::vec3 position = camera.target - direction * 100.0f;
    const glm::mat4 matView = glm::lookAt(position, camera.target, glm::vec3(0, 1, 0));

    // ���s���e�ɂ��v���W�F�N�V�����s����쐬.
    const float width = 100; // �`��͈͂̕�.
    const float height = 100; // �`��͈͂̍���.
    const float near = 10.0f; // �`��͈͂̎�O���̋��E.
    const float far = 200.0f; // �`��͈͂̉����̋��E.
    const glm::mat4 matProj = glm::ortho<float>(-width / 2, width / 2, -height / 2, height / 2, near, far);

    // �r���[�E�v���W�F�N�V�����s���ݒ肵�ă��b�V����`��.
    meshBuffer.SetShadowViewProjectionMatrix(matProj * matView);
    Camera shadowCamera = camera;
    shadowCamera.position = position;
    const Collision::Frustum viewFrustum(shadowCamera, -width / 2, width / 2, -height / 2, height / 2, near, far);
    RenderMesh(Mesh::DrawType::shadow, &viewFrustum);
  }

  glBindFramebuffer(GL_FRAMEBUFFER, fboMain->GetFramebuffer());
  auto texMain = fboMain->GetColorTexture();
  glViewport(0, 0, texMain->Width(), texMain->Height());

  glClearColor(0.5f, 0.6f, 0.8f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);
  glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  lightBuffer.Upload();
  lightBuffer.Bind();

  const glm::mat4 matView = glm::lookAt(camera.position, camera.target, camera.up);
  const float aspectRatio =
    static_cast<float>(window.Width()) / static_cast<float>(window.Height());
  const glm::mat4 matProj =
    glm::perspective(camera.fov, aspectRatio, camera.near, camera.far);
  meshBuffer.SetViewProjectionMatrix(matProj * matView);
  meshBuffer.SetCameraPosition(camera.position);
  meshBuffer.SetTime(window.Time());
  meshBuffer.BindShadowTexture(fboShadow->GetDepthTexture());


  const Collision::Frustum viewFrustum = Collision::CreateFrustum(camera);
  RenderMesh(Mesh::DrawType::color, &viewFrustum);
  particleSystem.Draw(matProj, matView);

  meshBuffer.UnbindShadowTexture();

  // ��ʊE�[�x.
  {
    glBindFramebuffer(GL_FRAMEBUFFER, fboDepthOfField->GetFramebuffer());
    auto tex = fboDepthOfField->GetColorTexture();
    glViewport(0, 0, tex->Width(), tex->Height());
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    camera.Update(matView);

    glDisable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    Mesh::FilePtr mesh = meshBuffer.GetFile("RenderTarget");
    Shader::ProgramPtr prog = mesh->materials[0].program;
    prog->Use();
    prog->SetViewInfo(static_cast<float>(window.Width()), static_cast<float>(window.Height()), camera.near, camera.far);
    prog->SetCameraInfo(camera.focalPlane, camera.focalLength, camera.aperture, camera.sensorSize);
    Mesh::Draw(mesh, glm::mat4(1));
  }

  // �u���[���E�G�t�F�N�g.
  {
    // ���邢�����𒊏o.
    {
      glBindFramebuffer(GL_FRAMEBUFFER, fboBloom[0][0]->GetFramebuffer());
      auto tex = fboBloom[0][0]->GetColorTexture();
      glViewport(0, 0, tex->Width(), tex->Height());
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      glDisable(GL_BLEND);
      glDisable(GL_DEPTH_TEST);
      glDisable(GL_CULL_FACE);
      Mesh::Draw(meshBuffer.GetFile("BrightPassFilter"), glm::mat4(1));
    }

    // �k���R�s�[.
    Mesh::FilePtr simpleMesh = meshBuffer.GetFile("Simple");
    for (int i = 0; i < sizeof(fboBloom) / sizeof(fboBloom[0]) - 1; ++i) {
      simpleMesh->materials[0].texture[0] = fboBloom[i][0]->GetColorTexture();
      glBindFramebuffer(GL_FRAMEBUFFER, fboBloom[i + 1][0]->GetFramebuffer());
      auto tex = fboBloom[i + 1][0]->GetColorTexture();
      glViewport(0, 0, tex->Width(), tex->Height());
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      Mesh::Draw(simpleMesh, glm::mat4(1));
    }

    // �K�E�X�ڂ���.
    Mesh::FilePtr blurMesh = meshBuffer.GetFile("GaussianBlur9");
    Shader::ProgramPtr progBlur = blurMesh->materials[0].program;
    for (int i = sizeof(fboBloom) / sizeof(fboBloom[0]) - 1; i >= 0; --i) {
      glBindFramebuffer(GL_FRAMEBUFFER, fboBloom[i][1]->GetFramebuffer());
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      auto tex = fboBloom[i][0]->GetColorTexture();
      glViewport(0, 0, tex->Width(), tex->Height());
      progBlur->Use();
      progBlur->SetBlurDirection(1.0f / static_cast<float>(tex->Width()), 0.0f);
      blurMesh->materials[0].texture[0] = tex;
      Mesh::Draw(blurMesh, glm::mat4(1));

      glBindFramebuffer(GL_FRAMEBUFFER, fboBloom[i][0]->GetFramebuffer());
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      progBlur->Use();
      progBlur->SetBlurDirection(0.0f, 1.0f / static_cast<float>(tex->Height()));
      blurMesh->materials[0].texture[0] = fboBloom[i][1]->GetColorTexture();
      Mesh::Draw(blurMesh, glm::mat4(1));
    }

    // �g�偕���Z����.
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    for (int i = sizeof(fboBloom) / sizeof(fboBloom[0]) - 1; i > 0; --i) {
      glBindFramebuffer(GL_FRAMEBUFFER, fboBloom[i - 1][0]->GetFramebuffer());
      auto tex = fboBloom[i - 1][0]->GetColorTexture();
      glViewport(0, 0, tex->Width(), tex->Height());
      simpleMesh->materials[0].texture[0] = fboBloom[i][0]->GetColorTexture();
      Mesh::Draw(simpleMesh, glm::mat4(1));
    }
  }
  
  // �S�Ă��f�t�H���g�E�t���[���o�b�t�@�ɍ����`��.
  {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, window.Width(), window.Height());

    const glm::vec2 screenSize(window.Width(), window.Height());
    spriteRenderer.Draw(screenSize);

    // �摜��`��.
    glDisable(GL_BLEND);
    Mesh::FilePtr simpleMesh = meshBuffer.GetFile("Simple");
    simpleMesh->materials[0].texture[0] = fboDepthOfField->GetColorTexture();
    Mesh::Draw(simpleMesh, glm::mat4(1));

    // �u���[����`��.
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    simpleMesh->materials[0].texture[0] = fboBloom[0][0]->GetColorTexture();
    Mesh::Draw(simpleMesh, glm::mat4(1));

    textWindow.Draw();
    fontRenderer.Draw(screenSize);
  }

#if 0
  // �f�o�b�O�p�Ƀu���[���p�t���[���o�b�t�@��\��.
  {
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ZERO);
    Mesh::FilePtr simpleMesh = meshBuffer.GetFile("Simple");
    for (int i = 0; i < 6; ++i) {
      auto tex = fboBloom[i][0]->GetColorTexture();
      glBindTexture(GL_TEXTURE_2D, tex->Get());
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      simpleMesh->materials[0].texture[0] = tex;
      glm::mat4 m = glm::scale(glm::translate(glm::mat4(1), glm::vec3(-0.75f + (float)(i%3) * 0.5f, 0.75f - (float)(i/3)*0.5f, 0)), glm::vec3(0.25f));
      Mesh::Draw(simpleMesh, m);
      glBindTexture(GL_TEXTURE_2D, tex->Get());
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glBindTexture(GL_TEXTURE_2D, 0);
    }
  }
#endif

#if 0
  {
    glDisable(GL_BLEND);
    Mesh::FilePtr simpleMesh = meshBuffer.GetFile("Simple");
    simpleMesh->materials[0].texture[0] = fboShadow->GetDepthTexture();
    glm::mat4 m = glm::scale(glm::translate(glm::mat4(1), glm::vec3(-0.45f, 0, 0)), glm::vec3(0.5f, 0.89f, 1));
    Mesh::Draw(simpleMesh, m);
  }
#endif
}

/**
* ���n���l�ɐG�ꂽ�Ƃ��̏���.
*
* @param id  ���n���l�̔ԍ�.
* @param pos ���n���l�̍��W.
*
* @retval true  ��������.
* @retval false ���łɐ퓬���Ȃ̂ŏ������Ȃ�����.
*/
bool MainGameScene::HandleJizoEffects(int id, const glm::vec3& pos)
{
  if (jizoId >= 0) {
    return false;
  }
  jizoId = id;
  const size_t oniCount = 8;
  for (size_t i = 0; i < oniCount; i++)
  {
    glm::vec3 position(pos);
    position.x += std::uniform_real_distribution<float>(-15, 15)(rand);
    position.z += std::uniform_real_distribution<float>(-15, 15)(rand);
    position.y = heightMap.Height(position);

    glm::vec3 rotation(0);
    rotation.y = std::uniform_real_distribution<float>(0, 3.14f * 2.0f)(rand);
    const Mesh::SkeletalMeshPtr mesh = meshBuffer.GetSkeletalMesh("oni_small");
    SkeletalMeshActorPtr p = std::make_shared<SkeletalMeshActor>(
      mesh, "Kooni", 13, position, rotation);
    p->GetMesh()->Play("Wait");
    p->colLocal = Collision::CreateCapsule(
      glm::vec3(0, 0.5f, 0), glm::vec3(0, 1, 0), 0.5f);
    enemies.Add(p);
  }
  return true;
}

