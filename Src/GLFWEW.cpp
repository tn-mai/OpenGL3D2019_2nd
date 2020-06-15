/**
* @file GLFWEW.cpp
*/
#include "GLFWEW.h"
#include <iostream>


/// GLFW �� GLEW �����b�v���邽�߂̖��O���
namespace GLFWEW {

/**
* GLFW ����̃G���[�񍐂���������
*
* @param error	�G���[�ԍ�
* @param desc	�G���[�̓��e
*/
void ErrorCallback(int error, const char* desc)
{
  std::cerr << "ERROR: " << desc << std::endl;
}

/**
* �V���O���g���C���X�^���X���擾����
*
* @return Widow �̃V���O���g���C���X�^���X
*/
Window& Window::Instance()
{
  static Window instance;
  return instance;
}

/**
* �R���X�g���N�^
*/
Window::Window()
{
}

/**
* �f�X�g���N�^
*/
Window::~Window()
{
  if (isGLFWInitialized) {
    glfwTerminate();
  }
}

void APIENTRY DebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, GLvoid* userParam)
{
  std::cerr << message << "\n";
}

/**
* GLFW/GLEW�̏�����
*
* @param w �E�B���h�E�̕`��͈͂̕�
* @param h �E�B���h�E�̕`��͈͂̍���
* @param title �E�B���h�E�^�C�g��(UTF-8 �� �O�I�[����)
*
* @retval true ����������
* @retval false ���������s
*/
bool Window::Init(int w, int h, const char* title)
{
  if (isInitialized) {
    std::cerr << "ERROR: GLFWEW�͊��ɏ���������Ă��܂�." << std::endl;
    return false;
  }
  if (!isGLFWInitialized) {
    glfwSetErrorCallback(ErrorCallback);
    if (glfwInit() != GL_TRUE) {
      return false;
    }
    isGLFWInitialized = true;
  }

  if (!window) {
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
    window = glfwCreateWindow(w, h, title, nullptr, nullptr);
    if (!window) {
      return false;
    }
    glfwMakeContextCurrent(window);
  }

  if (glewInit() != GLEW_OK) {
    std::cerr << "ERROR: GLEW �̏������Ɏ��s���܂���." << std::endl;
    return false;
  }

  glDebugMessageCallback(DebugCallback, nullptr);

  width = w;
  height = h;

  //OpenGL �̏����R���\�[���E�B���h�E�֏o�͂���
  const GLubyte* renderer = glGetString(GL_RENDERER);
  std::cout << "Renderer: " << renderer << std::endl;

  const GLubyte* version = glGetString(GL_VERSION);
  std::cout << "Version: " << version << std::endl;

  const GLubyte* vendor = glGetString(GL_VENDOR);
  std::cout << "Vendor: " << vendor << std::endl;

  const GLubyte* shading_language_version = glGetString(GL_SHADING_LANGUAGE_VERSION);
  std::cout << "Shading_language_version: " << shading_language_version << std::endl;

  const GLubyte* extensions = glGetString(GL_EXTENSIONS);
  std::cout << "Extensions: " << extensions << std::endl;

  // �O���݊����R�A�v���t�@�C�����w�肷��ƁAGLFW, GLEW�̏������ŃG���[���񍐂����̂ŁA�����Ŗ���������.
  // ���ʁA���Q�͂Ȃ��悤�����A�����I�ɂ̓��C�u�������ŐV�łɒu��������K�v�����邾�낤.
  glGetError();

#define PrintGLInfo(name) { \
  GLint tmp; \
  glGetIntegerv(name, &tmp); \
  std::cout << #name ": " << tmp << "\n"; \
} (void)0
  PrintGLInfo(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS); // �ŏ�48(GL3.1)/96(GL4.3)
  PrintGLInfo(GL_MAX_UNIFORM_BLOCK_SIZE); // �ŏ�16384=16kb
  PrintGLInfo(GL_MAX_COMBINED_UNIFORM_BLOCKS); // �ŏ�36/70
  PrintGLInfo(GL_MAX_VERTEX_UNIFORM_BLOCKS); // �ŏ�12/14
  PrintGLInfo(GL_MAX_FRAGMENT_UNIFORM_BLOCKS); // �ŏ�12/14
  PrintGLInfo(GL_MAX_UNIFORM_BUFFER_BINDINGS); // �ŏ�36/72
  PrintGLInfo(GL_MAX_VERTEX_UNIFORM_VECTORS); // �ŏ�256/256
  PrintGLInfo(GL_MAX_FRAGMENT_UNIFORM_VECTORS); // �ŏ�256/256
#undef PrintGLInfo

  std::cout << "GLEW Version: " << glewGetString(GLEW_VERSION) << "\n";

  isInitialized = true;
  return true;
}

/**
* �E�B���h�E�����ׂ������ׂ�
*
* @retval true ����
* @retval false ���Ȃ�
*/
bool Window::ShouldClose() const
{
  return glfwWindowShouldClose(window) != 0;
}

/**
* �t�����g�o�b�t�@�ƃo�b�N�o�b�t�@��؂�ւ���
*/
void Window::SwapBuffers() const
{
  glfwPollEvents();
  glfwSwapBuffers(window);
}

/**
* �L�[��������Ă����邩���ׂ�
*
* @param key ���ׂ����L�[�E�R�[�h(GLFW_KEY_A�Ȃ�)
*
* @retval true �L�[��������Ă���
* @retval false �L�[��������Ă��Ȃ�
*
* �g�p�ł���L�[�E�R�[�h�̈ꗗ��glfw3.h�ɂ���܂�(�ukeyboard�v�Ō���)
*/
bool Window::IsKeyPressed(int key) const
{
  return glfwGetKey(window, key) == GLFW_PRESS;
}

/**
* �^�C�}�[������������
*/
void Window::InitTimer()
{
  glfwSetTime(0.0);
  previousTime = 0.0;
  deltaTime = 0.0;
}

/**
* �^�C�}�[���X�V����
*/
void Window::UpdateTimer()
{
  //�o�ߎ��Ԃ��v��
  const double currentTime = glfwGetTime();
  deltaTime = currentTime - previousTime;
  previousTime = currentTime;

  //�o�ߎ��Ԃ������Ȃ肷���Ȃ��悤�ɒ���
  const float upperLimit = 0.25f;	//�o�ߎ��ԂƂ��ċ��e�������
  if (deltaTime > upperLimit) {
    deltaTime = 1.0f / 60.0f;
  }

  UpdateGamePad();
}

/**
* �o�ߎ��Ԃ��擾����
*
* @return ���O�̂Q���updateTimer()�Ăяo���̊ԂɌo�߂�������
*/
double Window::DeltaTime() const
{
  return deltaTime;
}

/**
* ���o�ߎ��Ԃ��擾����.
*
* @return GLFW������������Ă���̌o�ߎ���(�b).
*/
double Window::Time() const
{
  return glfwGetTime();
}

/**
* �Q�[���p�b�h�̏�Ԃ��擾����.
*
* @return �Q�[���p�b�h�̏��.
*/
const GamePad& Window::GetGamePad() const
{
  return gamepad;
}

/**
* �Q�[���p�b�h�̃A�i���O���͑��uID.
+
* @note ������XBOX360�Q�[���p�b�h�.
*/
enum GAMEPAD_AXES
{
  GAMEPAD_AXES_LEFT_X,  ///< ���X�e�B�b�N��X��.
  GAMEPAD_AXES_LEFT_Y,  ///< ���X�e�B�b�N��Y��.
  GAMEPAD_AXES_TRIGGER, ///< �A�i���O�g���K�[.
  GAMEPAD_AXES_RIGHT_Y, ///< �E�X�e�B�b�N��Y��.
  GAMEPAD_AXES_RIGHT_X, ///< �E�X�e�B�b�N��X��.
};

/**
* �Q�[���p�b�h�̃f�W�^�����͑��uID.
*
* @note Xbox360�Q�[���p�b�h����.
*/
enum GAMEPAD_BUTTON {
  GAMEPAD_BUTTON_A, ///< A�{�^��.
  GAMEPAD_BUTTON_B, ///< B�{�^��.
  GAMEPAD_BUTTON_X, ///< X�{�^��.
  GAMEPAD_BUTTON_Y, ///< Y�{�^��.
  GAMEPAD_BUTTON_L, ///< L�{�^��.
  GAMEPAD_BUTTON_R, ///< R�{�^��.
  GAMEPAD_BUTTON_BACK, ///< Back�{�^��.
  GAMEPAD_BUTTON_START, ///< Start�{�^��.
  GAMEPAD_BUTTON_L_THUMB, ///< ���X�e�B�b�N�{�^��.
  GAMEPAD_BUTTON_R_THUMB, ///< �E�X�e�B�b�N�{�^��.
  GAMEPAD_BUTTON_UP, ///< ��L�[.
  GAMEPAD_BUTTON_RIGHT, ///< �E�L�[.
  GAMEPAD_BUTTON_DOWN, ///< ���L�[.
  GAMEPAD_BUTTON_LEFT, ///< ���L�[.
};

/**
* �Q�[���p�b�h�̏�Ԃ��X�V����.
*/
void Window::UpdateGamePad()
{
  // buttonDown�𐶐����邽�߂ɁA�X�V�O�̓��͏�Ԃ�ۑ����Ă���.
  const uint32_t prevButtons = gamepad.buttons;

  // �A�i���O���͂ƃ{�^�����͂��擾.
  int axesCount, buttonCount;
  const float* axes = glfwGetJoystickAxes(GLFW_JOYSTICK_1, &axesCount);
  const uint8_t* buttons = glfwGetJoystickButtons(GLFW_JOYSTICK_1, &buttonCount);

  // �����̔z��nullptr�łȂ��A�Œ���K�v�ȃf�[�^���𖞂����Ă���΁A�L���ȃQ�[���p�b�h��
  // �ڑ�����Ă���Ƃ݂Ȃ�.
  if (axes && buttons && axesCount >= 2 && buttonCount >= 8) {
    // �L���ȃQ�[���p�b�h���ڑ�����Ă���ꍇ

    // �����L�[�̓��͏�Ԃ��������āA���X�e�B�b�N�̓��͂Œu��������. 
    gamepad.buttons &=
      ~(GamePad::DPAD_UP|GamePad::DPAD_DOWN|GamePad::DPAD_LEFT|GamePad::DPAD_RIGHT);
    static const float digitalThreshold = 0.3f; // �f�W�^�����͂Ƃ݂Ȃ��������l.
    if (axes[GAMEPAD_AXES_LEFT_Y] >= digitalThreshold) {
      gamepad.buttons |= GamePad::DPAD_UP;
    } else if (axes[GAMEPAD_AXES_LEFT_Y] <= -digitalThreshold) {
      gamepad.buttons |= GamePad::DPAD_DOWN;
    }
    if (axes[GAMEPAD_AXES_LEFT_X] >= digitalThreshold) {
      gamepad.buttons |= GamePad::DPAD_LEFT;
    } else if (axes[GAMEPAD_AXES_LEFT_X] <= -digitalThreshold) {
      gamepad.buttons |= GamePad::DPAD_RIGHT;
    }

    // �z��C���f�b�N�X��GamePad�L�[�̑Ή��\.
    static const struct {
      int dataIndex;
      uint32_t gamepadBit;
    } keyMap[] = {
      { GAMEPAD_BUTTON_A, GamePad::A },
      { GAMEPAD_BUTTON_B, GamePad::B },
      { GAMEPAD_BUTTON_X, GamePad::X },
      { GAMEPAD_BUTTON_Y, GamePad::Y },
      { GAMEPAD_BUTTON_L, GamePad::L },
      { GAMEPAD_BUTTON_R, GamePad::R },
      { GAMEPAD_BUTTON_START, GamePad::START },
      { GAMEPAD_BUTTON_UP, GamePad::DPAD_UP },
      { GAMEPAD_BUTTON_DOWN, GamePad::DPAD_DOWN },
      { GAMEPAD_BUTTON_LEFT, GamePad::DPAD_LEFT },
      { GAMEPAD_BUTTON_RIGHT, GamePad::DPAD_RIGHT },
    };

    for (const auto& e : keyMap) {
      if (buttons[e.dataIndex] == GLFW_PRESS) {
        gamepad.buttons |= e.gamepadBit;
      } else if (buttons[e.dataIndex] == GLFW_RELEASE) {
        gamepad.buttons &= ~e.gamepadBit;
      }
    }
  } else {
    // �L���ȃQ�[���p�b�h���ڑ�����Ă��Ȃ��̂ŁA�L�[�{�[�h���͂ő�p.

    // �z��C���f�b�N�X��GamePad�L�[�̑Ή��\.
    static const struct {
      int keyCode;
      uint32_t gamepadBit;
    } keyMap[] = {
      { GLFW_KEY_J, GamePad::A },
      { GLFW_KEY_K, GamePad::B },
      { GLFW_KEY_U, GamePad::X },
      { GLFW_KEY_I, GamePad::Y },
      { GLFW_KEY_O, GamePad::L },
      { GLFW_KEY_L, GamePad::R },
      { GLFW_KEY_ENTER, GamePad::START },
      { GLFW_KEY_W, GamePad::DPAD_UP },
      { GLFW_KEY_S, GamePad::DPAD_DOWN },
      { GLFW_KEY_A, GamePad::DPAD_LEFT },
      { GLFW_KEY_D, GamePad::DPAD_RIGHT },
    };

    for (const auto& e : keyMap) {
      const int key = glfwGetKey(window, e.keyCode);
      if (key == GLFW_PRESS) {
        gamepad.buttons |= e.gamepadBit;
      } else if (key == GLFW_RELEASE) {
        gamepad.buttons &= ~e.gamepadBit;
      }
    }
  }

  // �O��̍X�V�ŉ�����ĂȂ��āA���񉟂���Ă���L�[�̏���buttonDown�Ɋi�[.
  gamepad.buttonDown = gamepad.buttons & ~prevButtons;
}

} // namespace GLFWEW