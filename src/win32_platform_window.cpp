
struct PlatformWindow
{
	u32 width;
	u32 height;
	bool32 isFullscreen;
	bool32 isMinimized;

	bool32 isCursorVisible = true;

	bool32 shouldClose;

	// Lol(Leo): maybe this is acceptable name.
	HWND            hwnd;
	HINSTANCE       hInstance;

	// Note(Leo): This stores previous window size before fullscreen
	WINDOWPLACEMENT windowedWindowPosition;

	struct
	{
		bool32 resized;
	} events;
};

using Win32PlatformWindow = PlatformWindow;

enum WindowMessage : u32
{
	WindowMessage_set_fullscreen = WM_USER
};

// ---------- PLATFORM API FUNCTIONS --------------------------

static void platform_window_set_fullscreen(Win32PlatformWindow * window, bool32 setFullscreen)
{
	PostMessage(window->hwnd, WindowMessage_set_fullscreen, static_cast<WPARAM>(setFullscreen), 0);
}

static void platform_window_set_cursor_visible(Win32PlatformWindow * window, bool32 visible)
{
	if (window->isCursorVisible != visible)
	{
		window->isCursorVisible = visible;
		ShowCursor(visible);
	}
}

static u32 platform_window_get_width(Win32PlatformWindow const * window)
{
	return window->width;
}

static u32 platform_window_get_height(Win32PlatformWindow const * window)
{
	return window->height;
}

static bool32 platform_window_get_fullscreen(Win32PlatformWindow const * window)
{
	return window->isFullscreen;
}


// Todo(Leo): this needs also to be asked from vulkan
static bool32 win32_window_is_drawable(Win32PlatformWindow const * window)
{
	bool32 isDrawable = (window->isMinimized == false)
						&& (window->width > 0)
						&& (window->height > 0);
	return isDrawable;
}


static Win32PlatformWindow win32_platform_window_create(void * userPointer, WNDPROC windowCallback, u32 width, u32 height)
{
	HINSTANCE hInstance = GetModuleHandle(nullptr);

	char windowClassName [] = "FriendSimulatorWindowClass";
	char windowTitle [] = "FriendSimulator";

	WNDCLASS windowClass = {};
	windowClass.style           = CS_VREDRAW | CS_HREDRAW;
	windowClass.lpfnWndProc     = windowCallback;
	windowClass.hInstance       = hInstance;
	windowClass.lpszClassName   = windowClassName;

	// Study: https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-makeintresourcew
	auto defaultArrowCursor = MAKEINTRESOURCEW(32512);
	windowClass.hCursor = LoadCursorW(nullptr, defaultArrowCursor);

	if (RegisterClass(&windowClass) == 0)
	{
		AssertRelease(false, "Failed to create window class");
	}

	HWND hwnd = CreateWindowEx(	0,
								windowClassName,
								windowTitle,
								WS_OVERLAPPEDWINDOW | WS_VISIBLE,
								CW_USEDEFAULT,
								CW_USEDEFAULT,
								width, height,
								nullptr, nullptr,
								hInstance,
								userPointer);

	AssertRelease(hwnd != nullptr, "Failed to create window");

	RECT windowRect;
	GetWindowRect(hwnd, &windowRect);

	Win32PlatformWindow window = {};

	window.width        = windowRect.right - windowRect.left;
	window.height       = windowRect.bottom - windowRect.top;
	window.hwnd         = hwnd;
	window.hInstance    = hInstance;

	return window;
}

static void win32_window_process_messages(Win32PlatformWindow & window)
{
	window.events = {};

	MSG message;
	while (PeekMessageW(&message, window.hwnd, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&message);
		DispatchMessage(&message);
	}
}