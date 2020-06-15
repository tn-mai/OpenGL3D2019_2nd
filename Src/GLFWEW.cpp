/**
* @file GLFWEW.cpp
*/
#include "GLFWEW.h"
#include <iostream>


/// GLFW と GLEW をラップするための名前空間
namespace GLFWEW {

/**
* GLFW からのエラー報告を処理する
*
* @param error	エラー番号
* @param desc	エラーの内容
*/
void ErrorCallback(int error, const char* desc)
{
  std::cerr << "ERROR: " << desc << std::endl;
}

/**
* シングルトンインスタンスを取得する
*
* @return Widow のシングルトンインスタンス
*/
Window& Window::Instance()
{
  static Window instance;
  return instance;
}

/**
* コンストラクタ
*/
Window::Window()
{
}

/**
* デストラクタ
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
* GLFW/GLEWの初期化
*
* @param w ウィンドウの描画範囲の幅
* @param h ウィンドウの描画範囲の高さ
* @param title ウィンドウタイトル(UTF-8 の ０終端文字)
*
* @retval true 初期化成功
* @retval false 初期化失敗
*/
bool Window::Init(int w, int h, const char* title)
{
  if (isInitialized) {
    std::cerr << "ERROR: GLFWEWは既に初期化されています." << std::endl;
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
    std::cerr << "ERROR: GLEW の初期化に失敗しました." << std::endl;
    return false;
  }

  glDebugMessageCallback(DebugCallback, nullptr);

  width = w;
  height = h;

  //OpenGL の情報をコンソールウィンドウへ出力する
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

  // 前方互換性コアプロファイルを指定すると、GLFW, GLEWの初期化でエラーが報告されるので、ここで無理やり消す.
  // 当面、実害はないようだが、将来的にはライブラリを最新版に置き換える必要があるだろう.
  glGetError();

#define PrintGLInfo(name) { \
  GLint tmp; \
  glGetIntegerv(name, &tmp); \
  std::cout << #name ": " << tmp << "\n"; \
} (void)0
  PrintGLInfo(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS); // 最小48(GL3.1)/96(GL4.3)
  PrintGLInfo(GL_MAX_UNIFORM_BLOCK_SIZE); // 最小16384=16kb
  PrintGLInfo(GL_MAX_COMBINED_UNIFORM_BLOCKS); // 最小36/70
  PrintGLInfo(GL_MAX_VERTEX_UNIFORM_BLOCKS); // 最小12/14
  PrintGLInfo(GL_MAX_FRAGMENT_UNIFORM_BLOCKS); // 最小12/14
  PrintGLInfo(GL_MAX_UNIFORM_BUFFER_BINDINGS); // 最小36/72
  PrintGLInfo(GL_MAX_VERTEX_UNIFORM_VECTORS); // 最小256/256
  PrintGLInfo(GL_MAX_FRAGMENT_UNIFORM_VECTORS); // 最小256/256
#undef PrintGLInfo

  std::cout << "GLEW Version: " << glewGetString(GLEW_VERSION) << "\n";

  isInitialized = true;
  return true;
}

/**
* ウィンドウを閉じるべきか調べる
*
* @retval true 閉じる
* @retval false 閉じない
*/
bool Window::ShouldClose() const
{
  return glfwWindowShouldClose(window) != 0;
}

/**
* フロントバッファとバックバッファを切り替える
*/
void Window::SwapBuffers() const
{
  glfwPollEvents();
  glfwSwapBuffers(window);
}

/**
* キーが押されていいるか調べる
*
* @param key 調べたいキー・コード(GLFW_KEY_Aなど)
*
* @retval true キーが押されている
* @retval false キーが押されていない
*
* 使用できるキー・コードの一覧はglfw3.hにあります(「keyboard」で検索)
*/
bool Window::IsKeyPressed(int key) const
{
  return glfwGetKey(window, key) == GLFW_PRESS;
}

/**
* タイマーを初期化する
*/
void Window::InitTimer()
{
  glfwSetTime(0.0);
  previousTime = 0.0;
  deltaTime = 0.0;
}

/**
* タイマーを更新する
*/
void Window::UpdateTimer()
{
  //経過時間を計測
  const double currentTime = glfwGetTime();
  deltaTime = currentTime - previousTime;
  previousTime = currentTime;

  //経過時間が長くなりすぎないように調整
  const float upperLimit = 0.25f;	//経過時間として許容される上限
  if (deltaTime > upperLimit) {
    deltaTime = 1.0f / 60.0f;
  }

  UpdateGamePad();
}

/**
* 経過時間を取得する
*
* @return 直前の２回のupdateTimer()呼び出しの間に経過した時間
*/
double Window::DeltaTime() const
{
  return deltaTime;
}

/**
* 総経過時間を取得する.
*
* @return GLFWが初期化されてからの経過時間(秒).
*/
double Window::Time() const
{
  return glfwGetTime();
}

/**
* ゲームパッドの状態を取得する.
*
* @return ゲームパッドの状態.
*/
const GamePad& Window::GetGamePad() const
{
  return gamepad;
}

/**
* ゲームパッドのアナログ入力装置ID.
+
* @note 順序はXBOX360ゲームパッド基準.
*/
enum GAMEPAD_AXES
{
  GAMEPAD_AXES_LEFT_X,  ///< 左スティックのX軸.
  GAMEPAD_AXES_LEFT_Y,  ///< 左スティックのY軸.
  GAMEPAD_AXES_TRIGGER, ///< アナログトリガー.
  GAMEPAD_AXES_RIGHT_Y, ///< 右スティックのY軸.
  GAMEPAD_AXES_RIGHT_X, ///< 右スティックのX軸.
};

/**
* ゲームパッドのデジタル入力装置ID.
*
* @note Xbox360ゲームパッド準拠.
*/
enum GAMEPAD_BUTTON {
  GAMEPAD_BUTTON_A, ///< Aボタン.
  GAMEPAD_BUTTON_B, ///< Bボタン.
  GAMEPAD_BUTTON_X, ///< Xボタン.
  GAMEPAD_BUTTON_Y, ///< Yボタン.
  GAMEPAD_BUTTON_L, ///< Lボタン.
  GAMEPAD_BUTTON_R, ///< Rボタン.
  GAMEPAD_BUTTON_BACK, ///< Backボタン.
  GAMEPAD_BUTTON_START, ///< Startボタン.
  GAMEPAD_BUTTON_L_THUMB, ///< 左スティックボタン.
  GAMEPAD_BUTTON_R_THUMB, ///< 右スティックボタン.
  GAMEPAD_BUTTON_UP, ///< 上キー.
  GAMEPAD_BUTTON_RIGHT, ///< 右キー.
  GAMEPAD_BUTTON_DOWN, ///< 下キー.
  GAMEPAD_BUTTON_LEFT, ///< 左キー.
};

/**
* ゲームパッドの状態を更新する.
*/
void Window::UpdateGamePad()
{
  // buttonDownを生成するために、更新前の入力状態を保存しておく.
  const uint32_t prevButtons = gamepad.buttons;

  // アナログ入力とボタン入力を取得.
  int axesCount, buttonCount;
  const float* axes = glfwGetJoystickAxes(GLFW_JOYSTICK_1, &axesCount);
  const uint8_t* buttons = glfwGetJoystickButtons(GLFW_JOYSTICK_1, &buttonCount);

  // 両方の配列がnullptrでなく、最低限必要なデータ数を満たしていれば、有効なゲームパッドが
  // 接続されているとみなす.
  if (axes && buttons && axesCount >= 2 && buttonCount >= 8) {
    // 有効なゲームパッドが接続されている場合

    // 方向キーの入力状態を消去して、左スティックの入力で置き換える. 
    gamepad.buttons &=
      ~(GamePad::DPAD_UP|GamePad::DPAD_DOWN|GamePad::DPAD_LEFT|GamePad::DPAD_RIGHT);
    static const float digitalThreshold = 0.3f; // デジタル入力とみなすしきい値.
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

    // 配列インデックスとGamePadキーの対応表.
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
    // 有効なゲームパッドが接続されていないので、キーボード入力で代用.

    // 配列インデックスとGamePadキーの対応表.
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

  // 前回の更新で押されてなくて、今回押されているキーの情報をbuttonDownに格納.
  gamepad.buttonDown = gamepad.buttons & ~prevButtons;
}

} // namespace GLFWEW