#pragma once
namespace ReEngine
{
	typedef enum class KeyCode:uint16_t
	{
		// From glfw3.h
		Space = 32,
		Apostrophe = 39, /* ' */
		Comma = 44, /* , */
		Minus = 45, /* - */
		Period = 46, /* . */
		Slash = 47, /* / */

		D0 = 48, /* 0 */
		D1 = 49, /* 1 */
		D2 = 50, /* 2 */
		D3 = 51, /* 3 */
		D4 = 52, /* 4 */
		D5 = 53, /* 5 */
		D6 = 54, /* 6 */
		D7 = 55, /* 7 */
		D8 = 56, /* 8 */
		D9 = 57, /* 9 */

		Semicolon = 59, /* ; */
		Equal = 61, /* = */

		A = 65,
		B = 66,
		C = 67,
		D = 68,
		E = 69,
		F = 70,
		G = 71,
		H = 72,
		I = 73,
		J = 74,
		K = 75,
		L = 76,
		M = 77,
		N = 78,
		O = 79,
		P = 80,
		Q = 81,
		R = 82,
		S = 83,
		T = 84,
		U = 85,
		V = 86,
		W = 87,
		X = 88,
		Y = 89,
		Z = 90,

		LeftBracket = 91,  /* [ */
		Backslash = 92,  /* \ */
		RightBracket = 93,  /* ] */
		GraveAccent = 96,  /* ` */

		World1 = 161, /* non-US #1 */
		World2 = 162, /* non-US #2 */

		/* Function keys */
		Escape = 256,
		Enter = 257,
		Tab = 258,
		Backspace = 259,
		Insert = 260,
		Delete = 261,
		Right = 262,
		Left = 263,
		Down = 264,
		Up = 265,
		PageUp = 266,
		PageDown = 267,
		Home = 268,
		End = 269,
		CapsLock = 280,
		ScrollLock = 281,
		NumLock = 282,
		PrintScreen = 283,
		Pause = 284,
		F1 = 290,
		F2 = 291,
		F3 = 292,
		F4 = 293,
		F5 = 294,
		F6 = 295,
		F7 = 296,
		F8 = 297,
		F9 = 298,
		F10 = 299,
		F11 = 300,
		F12 = 301,
		F13 = 302,
		F14 = 303,
		F15 = 304,
		F16 = 305,
		F17 = 306,
		F18 = 307,
		F19 = 308,
		F20 = 309,
		F21 = 310,
		F22 = 311,
		F23 = 312,
		F24 = 313,
		F25 = 314,

		/* Keypad */
		KP0 = 320,
		KP1 = 321,
		KP2 = 322,
		KP3 = 323,
		KP4 = 324,
		KP5 = 325,
		KP6 = 326,
		KP7 = 327,
		KP8 = 328,
		KP9 = 329,
		KPDecimal = 330,
		KPDivide = 331,
		KPMultiply = 332,
		KPSubtract = 333,
		KPAdd = 334,
		KPEnter = 335,
		KPEqual = 336,

		LeftShift = 340,
		LeftControl = 341,
		LeftAlt = 342,
		LeftSuper = 343,
		RightShift = 344,
		RightControl = 345,
		RightAlt = 346,
		RightSuper = 347,
		Menu = 348
	} Key;

	typedef enum class MouseCode : uint16_t
	{
		// From glfw3.h
		Button0 = 0,
		Button1 = 1,
		Button2 = 2,
		Button3 = 3,
		Button4 = 4,
		Button5 = 5,
		Button6 = 6,
		Button7 = 7,

		ButtonLast = Button7,
		ButtonLeft = Button0,
		ButtonRight = Button1,
		ButtonMiddle = Button2
	} Mouse;

	inline std::ostream& operator<<(std::ostream& os, KeyCode keyCode)
	{
		os << static_cast<int32_t>(keyCode);
		return os;
	}

	inline std::ostream& operator<<(std::ostream& os, MouseCode mouseCode)
	{
		os << static_cast<int32_t>(mouseCode);
		return os;
	}
}



// From glfw3.h
#define RE_KEY_SPACE           ::ReEngine::Key::Space
#define RE_KEY_APOSTROPHE      ::ReEngine::Key::Apostrophe    /* ' */
#define RE_KEY_COMMA           ::ReEngine::Key::Comma         /* , */
#define RE_KEY_MINUS           ::ReEngine::Key::Minus         /* - */
#define RE_KEY_PERIOD          ::ReEngine::Key::Period        /* . */
#define RE_KEY_SLASH           ::ReEngine::Key::Slash         /* / */
#define RE_KEY_0               ::ReEngine::Key::D0
#define RE_KEY_1               ::ReEngine::Key::D1
#define RE_KEY_2               ::ReEngine::Key::D2
#define RE_KEY_3               ::ReEngine::Key::D3
#define RE_KEY_4               ::ReEngine::Key::D4
#define RE_KEY_5               ::ReEngine::Key::D5
#define RE_KEY_6               ::ReEngine::Key::D6
#define RE_KEY_7               ::ReEngine::Key::D7
#define RE_KEY_8               ::ReEngine::Key::D8
#define RE_KEY_9               ::ReEngine::Key::D9
#define RE_KEY_SEMICOLON       ::ReEngine::Key::Semicolon     /* ; */
#define RE_KEY_EQUAL           ::ReEngine::Key::Equal         /* = */
#define RE_KEY_A               ::ReEngine::Key::A
#define RE_KEY_B               ::ReEngine::Key::B
#define RE_KEY_C               ::ReEngine::Key::C
#define RE_KEY_D               ::ReEngine::Key::D
#define RE_KEY_E               ::ReEngine::Key::E
#define RE_KEY_F               ::ReEngine::Key::F
#define RE_KEY_G               ::ReEngine::Key::G
#define RE_KEY_H               ::ReEngine::Key::H
#define RE_KEY_I               ::ReEngine::Key::I
#define RE_KEY_J               ::ReEngine::Key::J
#define RE_KEY_K               ::ReEngine::Key::K
#define RE_KEY_L               ::ReEngine::Key::L
#define RE_KEY_M               ::ReEngine::Key::M
#define RE_KEY_N               ::ReEngine::Key::N
#define RE_KEY_O               ::ReEngine::Key::O
#define RE_KEY_P               ::ReEngine::Key::P
#define RE_KEY_Q               ::ReEngine::Key::Q
#define RE_KEY_R               ::ReEngine::Key::R
#define RE_KEY_S               ::ReEngine::Key::S
#define RE_KEY_T               ::ReEngine::Key::T
#define RE_KEY_U               ::ReEngine::Key::U
#define RE_KEY_V               ::ReEngine::Key::V
#define RE_KEY_W               ::ReEngine::Key::W
#define RE_KEY_X               ::ReEngine::Key::X
#define RE_KEY_Y               ::ReEngine::Key::Y
#define RE_KEY_Z               ::ReEngine::Key::Z
#define RE_KEY_LEFT_BRACKET    ::ReEngine::Key::LeftBracket   /* [ */
#define RE_KEY_BACKSLASH       ::ReEngine::Key::Backslash     /* \ */
#define RE_KEY_RIGHT_BRACKET   ::ReEngine::Key::RightBracket  /* ] */
#define RE_KEY_GRAVE_ACCENT    ::ReEngine::Key::GraveAccent   /* ` */
#define RE_KEY_WORLD_1         ::ReEngine::Key::World1        /* non-US #1 */
#define RE_KEY_WORLD_2         ::ReEngine::Key::World2        /* non-US #2 */

/* Function keys */
#define RE_KEY_ESCAPE          ::ReEngine::Key::Escape
#define RE_KEY_ENTER           ::ReEngine::Key::Enter
#define RE_KEY_TAB             ::ReEngine::Key::Tab
#define RE_KEY_BACKSPACE       ::ReEngine::Key::Backspace
#define RE_KEY_INSERT          ::ReEngine::Key::Insert
#define RE_KEY_DELETE          ::ReEngine::Key::Delete
#define RE_KEY_RIGHT           ::ReEngine::Key::Right
#define RE_KEY_LEFT            ::ReEngine::Key::Left
#define RE_KEY_DOWN            ::ReEngine::Key::Down
#define RE_KEY_UP              ::ReEngine::Key::Up
#define RE_KEY_PAGE_UP         ::ReEngine::Key::PageUp
#define RE_KEY_PAGE_DOWN       ::ReEngine::Key::PageDown
#define RE_KEY_HOME            ::ReEngine::Key::Home
#define RE_KEY_END             ::ReEngine::Key::End
#define RE_KEY_CAPS_LOCK       ::ReEngine::Key::CapsLock
#define RE_KEY_SCROLL_LOCK     ::ReEngine::Key::ScrollLock
#define RE_KEY_NUM_LOCK        ::ReEngine::Key::NumLock
#define RE_KEY_PRINT_SCREEN    ::ReEngine::Key::PrintScreen
#define RE_KEY_PAUSE           ::ReEngine::Key::Pause
#define RE_KEY_F1              ::ReEngine::Key::F1
#define RE_KEY_F2              ::ReEngine::Key::F2
#define RE_KEY_F3              ::ReEngine::Key::F3
#define RE_KEY_F4              ::ReEngine::Key::F4
#define RE_KEY_F5              ::ReEngine::Key::F5
#define RE_KEY_F6              ::ReEngine::Key::F6
#define RE_KEY_F7              ::ReEngine::Key::F7
#define RE_KEY_F8              ::ReEngine::Key::F8
#define RE_KEY_F9              ::ReEngine::Key::F9
#define RE_KEY_F10             ::ReEngine::Key::F10
#define RE_KEY_F11             ::ReEngine::Key::F11
#define RE_KEY_F12             ::ReEngine::Key::F12
#define RE_KEY_F13             ::ReEngine::Key::F13
#define RE_KEY_F14             ::ReEngine::Key::F14
#define RE_KEY_F15             ::ReEngine::Key::F15
#define RE_KEY_F16             ::ReEngine::Key::F16
#define RE_KEY_F17             ::ReEngine::Key::F17
#define RE_KEY_F18             ::ReEngine::Key::F18
#define RE_KEY_F19             ::ReEngine::Key::F19
#define RE_KEY_F20             ::ReEngine::Key::F20
#define RE_KEY_F21             ::ReEngine::Key::F21
#define RE_KEY_F22             ::ReEngine::Key::F22
#define RE_KEY_F23             ::ReEngine::Key::F23
#define RE_KEY_F24             ::ReEngine::Key::F24
#define RE_KEY_F25             ::ReEngine::Key::F25

/* Keypad */
#define RE_KEY_KP_0            ::ReEngine::Key::KP0
#define RE_KEY_KP_1            ::ReEngine::Key::KP1
#define RE_KEY_KP_2            ::ReEngine::Key::KP2
#define RE_KEY_KP_3            ::ReEngine::Key::KP3
#define RE_KEY_KP_4            ::ReEngine::Key::KP4
#define RE_KEY_KP_5            ::ReEngine::Key::KP5
#define RE_KEY_KP_6            ::ReEngine::Key::KP6
#define RE_KEY_KP_7            ::ReEngine::Key::KP7
#define RE_KEY_KP_8            ::ReEngine::Key::KP8
#define RE_KEY_KP_9            ::ReEngine::Key::KP9
#define RE_KEY_KP_DECIMAL      ::ReEngine::Key::KPDecimal
#define RE_KEY_KP_DIVIDE       ::ReEngine::Key::KPDivide
#define RE_KEY_KP_MULTIPLY     ::ReEngine::Key::KPMultiply
#define RE_KEY_KP_SUBTRACT     ::ReEngine::Key::KPSubtract
#define RE_KEY_KP_ADD          ::ReEngine::Key::KPAdd
#define RE_KEY_KP_ENTER        ::ReEngine::Key::KPEnter
#define RE_KEY_KP_EQUAL        ::ReEngine::Key::KPEqual

#define RE_KEY_LEFT_SHIFT      ::ReEngine::Key::LeftShift
#define RE_KEY_LEFT_CONTROL    ::ReEngine::Key::LeftControl
#define RE_KEY_LEFT_ALT        ::ReEngine::Key::LeftAlt
#define RE_KEY_LEFT_SUPER      ::ReEngine::Key::LeftSuper
#define RE_KEY_RIGHT_SHIFT     ::ReEngine::Key::RightShift
#define RE_KEY_RIGHT_CONTROL   ::ReEngine::Key::RightControl
#define RE_KEY_RIGHT_ALT       ::ReEngine::Key::RightAlt
#define RE_KEY_RIGHT_SUPER     ::ReEngine::Key::RightSuper
#define RE_KEY_MENU            ::ReEngine::Key::Menu

#define RE_MOUSE_BUTTON_0      ::ReEngine::Mouse::Button0
#define RE_MOUSE_BUTTON_1      ::ReEngine::Mouse::Button1
#define RE_MOUSE_BUTTON_2      ::ReEngine::Mouse::Button2
#define RE_MOUSE_BUTTON_3      ::ReEngine::Mouse::Button3
#define RE_MOUSE_BUTTON_4      ::ReEngine::Mouse::Button4
#define RE_MOUSE_BUTTON_5      ::ReEngine::Mouse::Button5
#define RE_MOUSE_BUTTON_6      ::ReEngine::Mouse::Button6
#define RE_MOUSE_BUTTON_7      ::ReEngine::Mouse::Button7
#define RE_MOUSE_BUTTON_LAST   ::ReEngine::Mouse::ButtonLast
#define RE_MOUSE_BUTTON_LEFT   ::ReEngine::Mouse::ButtonLeft
#define RE_MOUSE_BUTTON_RIGHT  ::ReEngine::Mouse::ButtonRight
#define RE_MOUSE_BUTTON_MIDDLE ::ReEngine::Mouse::ButtonMiddle
