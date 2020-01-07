/*=============================================================================
Leo Tamminen

Windows platform layer for mazegame
=============================================================================*/

#include <WinSock2.h>
#include <Windows.h>
#include <xinput.h>

#include <objbase.h>
#include <mmdeviceapi.h>
#include <audioclient.h>

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>

#include <stdexcept>
#include <vector>
#include <fstream>
#include <chrono>

// TODO(Leo): Make sure that arrays for getting extensions ana layers are large enough
// TOdo(Leo): Combine to fewer functions and remove throws, instead returning specific enum value

/* TODO(Leo) extra: Use separate queuefamily thing for transfering between vertex
staging buffer and actual vertex buffer. https://vulkan-tutorial.com/en/Vertex_buffers/Staging_buffer */

/* STUDY(Leo):
    http://asawicki.info/news_1698_vulkan_sparse_binding_-_a_quick_overview.html
*/

#include "MazegamePlatform.hpp"

// Todo(Leo): hack to get two controller on single pc and use only 1st controller when online
global_variable int globalXinputControllerIndex;

#include "winapi_WindowMessages.cpp"
#include "winapi_VulkanDebugStrings.cpp"
#include "winapi_WinSocketDebugStrings.cpp"
#include "winapi_ErrorStrings.hpp"
#include "winapi_Mazegame.hpp"

// Todo(Leo): Vulkan implementation depends on this, not cool
using BinaryAsset = std::vector<uint8>;
BinaryAsset
ReadBinaryFile (const char * fileName)
{
    std::ifstream file (fileName, std::ios::ate | std::ios::binary);

    if (file.is_open() == false)
    {
        throw std::runtime_error("failed to open file");
    }

    size_t fileSize = file.tellg();
    BinaryAsset result (fileSize);

    file.seekg(0);
    file.read(reinterpret_cast<char*>(result.data()), fileSize);

    file.close();
    return result;    
}

enum : int32
{
    DESCRIPTOR_SET_LAYOUT_SCENE_UNIFORM = 0,
    DESCRIPTOR_SET_LAYOUT_MATERIAL      = 1,
    DESCRIPTOR_SET_LAYOUT_MODEL_UNIFORM = 2
};


// Note(Leo): make unity build
#include "winapi_VulkanCommandBuffers.cpp"
#include "winapi_Vulkan.cpp"
#include "winapi_Audio.cpp"
#include "winapi_Network.cpp"
#include "winapi_Input.cpp"

namespace winapi
{
    internal void
    SetWindowFullScreen(HWND winWindow, winapi::State * state, bool32 setFullscreen)
    {
        // Study(Leo): https://devblogs.microsoft.com/oldnewthing/20100412-00/?p=14353
        DWORD style = GetWindowLongPtr(winWindow, GWL_STYLE);
        bool32 isFullscreen = !(style & WS_OVERLAPPEDWINDOW);

        if (isFullscreen == false && setFullscreen)
        {
            // Todo(Leo): Actually use this value to check for potential errors
            bool32 success;

            success = GetWindowPlacement(winWindow, &state->windowedWindowPosition); 

            HMONITOR monitor = MonitorFromWindow(winWindow, MONITOR_DEFAULTTOPRIMARY);
            MONITORINFO monitorInfo = { sizeof(MONITORINFO) };
            success = GetMonitorInfoW(monitor, &monitorInfo);

            DWORD fullScreenStyle = (style & ~WS_OVERLAPPEDWINDOW);
            SetWindowLongPtrW(winWindow, GWL_STYLE, fullScreenStyle);

            // Note(Leo): Remember that height value starts from top and grows towards bottom
            LONG left = monitorInfo.rcMonitor.left;
            LONG top = monitorInfo.rcMonitor.top;
            LONG width = monitorInfo.rcMonitor.right - left;
            LONG heigth = monitorInfo.rcMonitor.bottom - top;
            SetWindowPos(winWindow, HWND_TOP, left, top, width, heigth, SWP_NOOWNERZORDER | SWP_FRAMECHANGED);

            state->windowIsFullscreen = true;
        }

        else if (isFullscreen && setFullscreen == false)
        {
            SetWindowLongPtr(winWindow, GWL_STYLE, style | WS_OVERLAPPEDWINDOW);
            SetWindowPlacement(winWindow, &state->windowedWindowPosition);
            SetWindowPos(   winWindow, NULL, 0, 0, 0, 0,
                            SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_FRAMECHANGED);

            state->windowIsFullscreen = false;
        }
    }

    internal FILETIME
    GetFileLastWriteTime(const char * fileName)
    {
        WIN32_FILE_ATTRIBUTE_DATA fileInfo = {};
        if (GetFileAttributesExA(fileName, GetFileExInfoStandard, &fileInfo))
        {
        }
        else
        {
            // Todo(Leo): Now what??? Getting file time failed --> file does not exist??
        }    
        FILETIME result = fileInfo.ftLastWriteTime;
        return result;
    }

    internal State * 
    GetStateFromWindow(HWND winWindow)
    {
        State * state = reinterpret_cast<State *> (GetWindowLongPtrW(winWindow, GWLP_USERDATA));
        return state;
    }

    // Note(Leo): CALLBACK specifies calling convention for 32-bit applications
    // https://stackoverflow.com/questions/15126454/what-does-the-callback-keyword-mean-in-a-win-32-c-application
    internal LRESULT CALLBACK
    MainWindowCallback (HWND window, UINT message, WPARAM wParam, LPARAM lParam)
    {
        LRESULT result = 0;
        using namespace winapi;

        switch (message)
        {
            case WM_KEYDOWN:
                process_keyboard_input(GetStateFromWindow(window), wParam, true);
                break;

            case WM_KEYUP:
                process_keyboard_input(GetStateFromWindow(window), wParam, false);
                break;

            case WM_CREATE:
            {
                CREATESTRUCTW * pCreate = reinterpret_cast<CREATESTRUCTW *>(lParam);
                winapi::State * state = reinterpret_cast<winapi::State *>(pCreate->lpCreateParams);
                SetWindowLongPtrW (window, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(state));
            } break;

            case WM_SIZE:
            {
                winapi::State * state                   = winapi::GetStateFromWindow(window);
                state->gamePlatformInfo.windowWidth     = LOWORD(lParam);
                state->gamePlatformInfo.windowHeight    = HIWORD(lParam);

                switch (wParam)
                {
                    case SIZE_RESTORED:
                    case SIZE_MAXIMIZED:
                        state->windowIsMinimized = false;
                        break;

                    case SIZE_MINIMIZED:
                        state->windowIsMinimized = true;
                        break;
                }

            } break;

            case WM_CLOSE:
            {
                winapi::State * state = winapi::GetStateFromWindow(window);
                state->isRunning = false;
                std::cout << "window callback: WM_CLOSE\n";
            } break;

            // case WM_EXITSIZEMOVE:
            // {
            // } break;

            default:
                result = DefWindowProcW(window, message, wParam, lParam);
        }
        return result;
    }

    internal void
    ProcessPendingMessages(winapi::State * state, HWND winWindow)
    {
        // Note(Leo): Apparently Windows requires us to do this.

        MSG message;
        while (PeekMessageW(&message, winWindow, 0, 0, PM_REMOVE))
        {
            switch (message.message)
            {
                default:
                    TranslateMessage(&message);
                    DispatchMessage(&message);
            }
        }
    }
}

internal void
Run(HINSTANCE winInstance)
{
    winapi::State state = {};
    load_xinput();

    // ---------- INITIALIZE PLATFORM ------------
    HWND winWindow;
    VulkanContext context;
    {
        wchar windowClassName [] = L"MazegameWindowClass";
        wchar windowTitle [] = L"Mazegame";

        WNDCLASSW windowClass = {};
        windowClass.style           = CS_VREDRAW | CS_HREDRAW;
        windowClass.lpfnWndProc     = winapi::MainWindowCallback;
        windowClass.hInstance       = winInstance;
        windowClass.lpszClassName   = windowClassName;

        // Study: https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-makeintresourcew
        auto defaultArrowCursor = MAKEINTRESOURCEW(32512);
        windowClass.hCursor = LoadCursorW(nullptr, defaultArrowCursor);

        if (RegisterClassW(&windowClass) == 0)
        {
            // Todo(Leo): Logging and backup plan for class invalidness
            std::cout << "Failed to register window class\n";
            return;
        }

        int32 defaultWindowWidth = 960;
        int32 defaultWindowHeight = 540;

        winWindow = CreateWindowExW (
            0, windowClassName, windowTitle, WS_OVERLAPPEDWINDOW | WS_VISIBLE,
            CW_USEDEFAULT, CW_USEDEFAULT, defaultWindowWidth, defaultWindowHeight,
            nullptr, nullptr, winInstance, &state);

        if (winWindow == nullptr)
        {
            // Todo[Logging] (Leo): Log this and make backup plan
            std::cout << "Failed to create window\n";
            return;
        }

        context = winapi::VulkanInitialize(winInstance, winWindow);
        state.gamePlatformInfo.graphicsContext = &context;

        state.gamePlatformInfo.set_window_fullscreen = [&winWindow, &state](bool32 fullscreen)
        {
            winapi::SetWindowFullScreen(winWindow, &state, fullscreen);
            state.gamePlatformInfo.windowIsFullscreen = state.windowIsFullscreen;
        };
    }

    // ------- MEMORY ---------------------------
    game::Memory gameMemory = {};
    MemoryArena platformTransientMemoryArena;
    {
        // TODO [MEMORY] (Leo): Properly measure required amount
        // TODO [MEMORY] (Leo): Think of alignment
        gameMemory.persistentMemorySize = Gigabytes(1);
        gameMemory.transientMemorySize  = Gigabytes(1);

        uint64 totalGameMemorySize = gameMemory.persistentMemorySize + gameMemory.transientMemorySize;

        uint64 platformTransientMemorySize = Megabytes(1);
        uint64 totalMemorySize = totalGameMemorySize + platformTransientMemorySize;
   
        // TODO [MEMORY] (Leo): Check support for large pages
        // TODO [MEMORY] (Leo): specify base address for development builds
    #if MAZEGAME_DEVELOPMENT
        void * memoryBlock = VirtualAlloc((void*)Terabytes(2), totalMemorySize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    #else
        void * memoryBlock = VirtualAlloc(nullptr, totalMemorySize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    #endif

        gameMemory.persistentMemory     = memoryBlock;
        gameMemory.transientMemory      = (byte *)memoryBlock + gameMemory.persistentMemorySize;

        platformTransientMemoryArena = make_memory_arena((byte *)memoryBlock + totalGameMemorySize, platformTransientMemorySize);

    }

   // -------- GPU MEMORY ---------------------- 
   {
        // TODO [MEMORY] (Leo): Properly measure required amount
        // TODO[memory] (Leo): Log usage
        uint64 staticMeshPoolSize       = Megabytes(500);
        uint64 stagingBufferPoolSize    = Megabytes(100);
        uint64 modelUniformBufferSize   = Megabytes(100);
        uint64 sceneUniformBufferSize   = Megabytes(100);
        uint64 guiUniformBufferSize     = Megabytes(100);

        // TODO[MEMORY] (Leo): This will need guarding against multithreads once we get there
        // Static mesh pool
        vulkan::CreateBufferResource(   &context, staticMeshPoolSize,
                                        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                                        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                        &context.staticMeshPool);
        // Staging buffer
        vulkan::CreateBufferResource(   &context, stagingBufferPoolSize,
                                        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                        &context.stagingBufferPool);

        // Uniform buffer for model matrices. This means every model including scenery.
        vulkan::CreateBufferResource(   &context, modelUniformBufferSize,
                                        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                        &context.modelUniformBuffer);

        // Uniform buffer for scene data, ie. camera, lights etc.
        vulkan::CreateBufferResource(   &context, sceneUniformBufferSize,
                                        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
                                        &context.sceneUniformBuffer);

        // Uniform buffer for gui elements
        vulkan::CreateBufferResource(   &context, guiUniformBufferSize,
                                        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
                                        &context.guiUniformBuffer);
    }

    // -------- DRAWING ---------
    {
        // Note(Leo): After buffer creations!!
        context.descriptorSets      = CreateModelDescriptorSets(&context);
        context.sceneDescriptorSets = CreateSceneDescriptorSets(&context);
        context.textureSampler      = CreateTextureSampler(&context);
        CreateCommandBuffers(&context);
    }
    int32 currentLoopingFrameIndex = 0;

    game::RenderInfo gameRenderInfo = {};
    vulkan::RenderInfo platformRenderInfo = {};
    {
        /* Todo(Leo): We are currently pushing for worst case. It is ok because
        push_array is super cheap and memory is available anyway. However we may
        want to be more exact */
        platformRenderInfo.renderedObjects = push_array<Matrix44, RenderedObjectHandle>(
                                            &platformTransientMemoryArena, VULKAN_MAX_MODEL_COUNT);

        // Todo(Leo): define these somewhere else as proper functions and not lamdas??
        gameRenderInfo.draw = [&context, &platformRenderInfo](RenderedObjectHandle handle, Matrix44 transform)
        {
            platformRenderInfo.renderedObjects[handle] = transform;

            // Todo(Leo): Single mapping is really enough, offsets can be used here too
            uint32 startUniformBufferOffset = vulkan::GetModelUniformBufferOffsetForSwapchainImages(&context, context.currentDrawFrameIndex);

            Matrix44 * pModelMatrix;
            vkMapMemory(context.device, context.modelUniformBuffer.memory, 
                        startUniformBufferOffset + context.loadedRenderedObjects[handle].uniformBufferOffset,
                        sizeof(pModelMatrix), 0, (void**)&pModelMatrix);

            *pModelMatrix = transform;
            vkUnmapMemory(context.device, context.modelUniformBuffer.memory);


            vulkan::record_draw_command(&context, handle, transform);
        };


        gameRenderInfo.start_drawing = [&context]()
        {
            vulkan::start_drawing(&context, context.currentDrawFrameIndex);
        };
        gameRenderInfo.finish_drawing = [&context]()
        {
            vulkan::finish_drawing(&context);
        };


        gameRenderInfo.set_camera = [&context, &platformRenderInfo](Matrix44 view, Matrix44 perspective)
        {
            // Note (Leo): map vulkan memory directly to right type so we can easily avoid one (small) memcpy per frame
            VulkanCameraUniformBufferObject * pUbo;
            vkMapMemory(context.device, context.sceneUniformBuffer.memory,
                        vulkan::GetSceneUniformBufferOffsetForSwapchainImages(&context, context.currentDrawFrameIndex),
                        sizeof(VulkanCameraUniformBufferObject), 0, (void**)&pUbo);

            pUbo->view          = view;
            pUbo->perspective   = perspective;

            vkUnmapMemory(context.device, context.sceneUniformBuffer.memory);
        };

        gameRenderInfo.draw_line = [&context](Vector3 start, Vector3 end, Vector3 color)
        {
            vulkan::record_line_draw_command(&context, start, end, color);
        };
    }

    // -------- INITIALIZE NETWORK ---------
    bool32 networkIsRuined = false;
    WinApiNetwork network = winapi::CreateNetwork();
    game::Network gameNetwork = {};
    real64 networkSendDelay     = 1.0 / 20;
    real64 networkNextSendTime  = 0;
    
    /// --------- INITIALIZE AUDIO ----------------
    WinApiAudio audio = winapi::CreateAudio();
    winapi::StartPlaying(&audio);                

    /// --------- TIMING ---------------------------
    auto startTimeMark          = std::chrono::high_resolution_clock::now();
    real64 lastFrameStartTime   = 0;

    MAZEGAME_NO_INIT real64 targetFrameTime;
    MAZEGAME_NO_INIT uint32 deviceMinSchedulerGranularity;
    {
        TIMECAPS timeCaps;
        timeGetDevCaps(&timeCaps, sizeof(timeCaps));
        deviceMinSchedulerGranularity = timeCaps.wPeriodMin;

        // TODO(LEO): Make proper settings :)
        /* NOTE(Leo): Change to lower framerate if on battery. My development
        laptop at least changes to slower processing speed when not on power,
        though this probably depends on power settings. These are just some
        arbitrary values that happen to work on development laptop. */
        SYSTEM_POWER_STATUS powerStatus;
        GetSystemPowerStatus (&powerStatus);

        real64 targetFrameTimeYesPower = 60;
        real64 targetFrameTimeNoPower = 30;

        constexpr BYTE AC_ONLINE = 1;
        real64 targetFrameTimeThreshold = (powerStatus.ACLineStatus == AC_ONLINE) ? targetFrameTimeYesPower : targetFrameTimeNoPower;

        if (powerStatus.ACLineStatus == AC_ONLINE)
        {
            std::cout << "Power detected\n";
        }
        else
        {
            std::cout << "No power detected\n";
        }

        int32 gameUpdateRate = targetFrameTimeThreshold;
        {
            HDC deviceContext = GetDC(winWindow);
            int monitorRefreshRate = GetDeviceCaps(deviceContext, VREFRESH);
            ReleaseDC(winWindow, deviceContext);

            if (monitorRefreshRate > 1)
            {  
                gameUpdateRate = monitorRefreshRate;

                while((gameUpdateRate / 2.0f) > targetFrameTimeThreshold)
                {
                    gameUpdateRate /= 2.0f;
                }
            }
        }
        targetFrameTime = 1.0f / gameUpdateRate;
        
        std::cout << "Target frame time = " << targetFrameTime << ", (fps = " << 1.0 / targetFrameTime << ")\n";
    }

    /// ---------- LOAD GAME CODE ----------------------
    // Note(Leo): Only load game dynamically in development.
    winapi::Game game = {};
    FILETIME dllWriteTime;
    {
        CopyFileA(GAMECODE_DLL_FILE_NAME, GAMECODE_DLL_FILE_NAME_TEMP, false);
        game.dllHandle = LoadLibraryA(GAMECODE_DLL_FILE_NAME_TEMP);
        if (game.dllHandle != nullptr)
        {
            game.Update = reinterpret_cast<winapi::Game::UpdateFunc *> (GetProcAddress(game.dllHandle, GAMECODE_UPDATE_FUNC_NAME));
            dllWriteTime = winapi::GetFileLastWriteTime(GAMECODE_DLL_FILE_NAME);
        }
    }

    ////////////////////////////////////////////////////
    ///             MAIN LOOP                        ///
    ////////////////////////////////////////////////////
    state.isRunning = true;
    game::Input gameInput = {};
    while(state.isRunning)
    {

        /// ----- START TIME -----
        auto currentTimeMark = std::chrono::high_resolution_clock::now();
        real64 frameStartTime = std::chrono::duration<real64, std::chrono::seconds::period>(currentTimeMark - startTimeMark).count();

        /// ----- RELOAD GAME CODE -----
        FILETIME dllLatestWriteTime = winapi::GetFileLastWriteTime(GAMECODE_DLL_FILE_NAME);
        if (CompareFileTime(&dllLatestWriteTime, &dllWriteTime) > 0)
        {
            FreeLibrary(game.dllHandle);
            game.dllHandle =  nullptr;

            CopyFileA(GAMECODE_DLL_FILE_NAME, GAMECODE_DLL_FILE_NAME_TEMP, false);
            game.dllHandle = LoadLibraryA(GAMECODE_DLL_FILE_NAME_TEMP);
            if(game.IsLoaded())
            {
                game.Update = reinterpret_cast<winapi::Game::UpdateFunc *>(GetProcAddress(game.dllHandle, GAMECODE_UPDATE_FUNC_NAME));
                dllWriteTime = dllLatestWriteTime;
            }

        }

        /// ----- HANDLE INPUT -----
        {
            // Note(Leo): this is not input only...
            winapi::ProcessPendingMessages(&state, winWindow);

            HWND foregroundWindow = GetForegroundWindow();
            bool32 windowIsActive = winWindow == foregroundWindow;

            /* Note(Leo): Only get input from a single controller, locally this is a single
            player game. Use global controller index depending on network status to test
            multiplayering */
           /* TODO [Input](Leo): Test controller and index behaviour when being connected and
            disconnected */

            XINPUT_STATE xinputState;
            bool32 xinputReceived = XInputGetState(globalXinputControllerIndex, &xinputState) == ERROR_SUCCESS;
            bool32 xinputUsed = xinputReceived && xinput_is_used(&state, &xinputState);

            if (windowIsActive == false)
            {
                update_unused_input(&gameInput);
            }
            else if (xinputUsed)
            {
                update_controller_input(&gameInput, &xinputState);
            }
            else if (state.keyboardInputIsUsed)
            {
                update_keyboard_input(&gameInput, &state.keyboardInput);
                state.keyboardInputIsUsed = false;
            }
            else
            {
                update_unused_input(&gameInput);
            }

            gameInput.elapsedTime = targetFrameTime;
        }

        /// ----- PRE-UPDATE NETWORK PASS -----
        {
            // Todo(Leo): just quit now if some unhandled network error occured
            if (networkIsRuined)
            {
                auto error = WSAGetLastError();
                std::cout << "NETWORK failed: " << WinSocketErrorString(error) << " (" << error << ")\n"; 
                break;
            }

            if (network.isListening)
            {
                winapi::NetworkListen(&network);
            }
            else if (network.isConnected)
            {
                winapi::NetworkReceive(&network, &gameNetwork.inPackage);
            }
            gameNetwork.isConnected = network.isConnected;
        }

        /// --------- UPDATE GAME -------------
        /* Note(Leo): We previously had only drawing in this if-block, but due to need
        to havee imageindex available during game update because of rendering, it is now moved up here.
        I do not know yet what all this might do...*/
        if (state.isRunning && state.windowIsDrawable())
        {
            // Todo(Leo): Study fences
            vkWaitForFences(context.device, 1, &context.syncObjects.inFlightFences[currentLoopingFrameIndex],
                            VK_TRUE, VULKAN_NO_TIME_OUT);

            uint32 imageIndex;
            VkResult getNextImageResult = vkAcquireNextImageKHR(context.device, context.swapchainItems.swapchain, MaxValue<uint64>,
                                                    context.syncObjects.imageAvailableSemaphores[currentLoopingFrameIndex],
                                                    VK_NULL_HANDLE, &imageIndex);
            
            context.currentDrawFrameIndex = imageIndex;
            {   
                if(game.IsLoaded())
                {
                    game::SoundOutput gameSoundOutput = {};
                    winapi::GetAudioBuffer(&audio, &gameSoundOutput.sampleCount, &gameSoundOutput.samples);

                    bool32 gameIsAlive = game.Update(   &gameInput, &gameMemory, &state.gamePlatformInfo,
                                                        &gameNetwork, &gameSoundOutput, &gameRenderInfo);

                    if (gameIsAlive == false)
                    {
                        state.isRunning = false;
                    }
                    winapi::ReleaseAudioBuffer(&audio, gameSoundOutput.sampleCount);
                }

            }

            /// ---- DRAW -----    
            /*
            Note(Leo): Only draw image if we have window that is not minimized. Vulkan on windows MUST not
            have framebuffer size 0, which is what minimized window is.

            'currentLoopingFrameIndex' does not need to be incremented if we do not draw since it refers to
            next available swapchain frame/image, and we do not use one if we do not draw.
            */
        
            switch (getNextImageResult)
            {
                case VK_SUCCESS:
                    vulkan::draw_frame(&context, imageIndex, currentLoopingFrameIndex);
                    currentLoopingFrameIndex = (currentLoopingFrameIndex + 1) % MAX_FRAMES_IN_FLIGHT;
                    break;

                case VK_SUBOPTIMAL_KHR:
                case VK_ERROR_OUT_OF_DATE_KHR:
                    vulkan::RecreateSwapchain(&context, state.GetFrameBufferSize());
                    break;

                default:
                    throw std::runtime_error("Failed to acquire swap chain image");
            }

            // Todo(Leo): should this be in switch case where we draw succesfully???
        }

        // ----- POST-UPDATE NETWORK PASS ------
        // TODO[network](Leo): Now that we have fixed frame rate, we should count frames passed instead of time
        if (network.isConnected && frameStartTime > networkNextSendTime)
        {
            networkNextSendTime = frameStartTime + networkSendDelay;
            winapi::NetworkSend(&network, &gameNetwork.outPackage);
        }

        /// ----- CLEAR MEMORY ------
        flush_memory_arena(&platformTransientMemoryArena);

        /// ----- WAIT FOR TARGET FRAMETIME -----
        {
            auto currentTimeMark = std::chrono::high_resolution_clock::now();
            real64 currentTimeSeconds = std::chrono::duration<real64, std::chrono::seconds::period>(currentTimeMark - startTimeMark).count();

            real64 elapsedSeconds = currentTimeSeconds - frameStartTime;
            real64 timeToSleepSeconds = targetFrameTime - elapsedSeconds;

            /* TODO[time](Leo): It seems okay to sleep 0 milliseconds in case the time to sleep ends up being
            less than 1 millisecond on floating point representation. Also we may want to do a busy loop over
            remainder time after sleep. This is due to windows scheduler granularity, that is at best on 
            one millisecond scale. */
            if (timeToSleepSeconds > 0)
            {
                DWORD timeToSleepMilliSeconds = static_cast<DWORD>(1000 * timeToSleepSeconds);
                // std::cout << "Sleep for " << timeToSleepMilliSeconds << " ms\n";
                Sleep(timeToSleepMilliSeconds);

            }
        }
    }
    ///////////////////////////////////////
    ///         END OF MAIN LOOP        ///
    ///////////////////////////////////////


    /// -------- CLEANUP ---------
    winapi::StopPlaying(&audio);
    winapi::ReleaseAudio(&audio);
    winapi::CloseNetwork(&network);

    /// ------- CLEANUP VULKAN -----
    {
        // Note(Leo): All draw frame operations are asynchronous, must wait for them to finish
        vkDeviceWaitIdle(context.device);
        Cleanup(&context);

        std::cout << "[VULKAN]: shut down\n";
    }
    /// ----- Cleanup Windows
    {
        // Note(Leo): Windows will mostly clean up after us once process exits :)


        /* TODO(Leo): If there are multiple of these it means we had crash previously
        and want to reset this. This should be handled betterly though.... :( */
        timeEndPeriod(deviceMinSchedulerGranularity);
        std::cout << "[WINDOWS]: shut down\n";
    }
}


int CALLBACK
WinMain(
    HINSTANCE   winInstance,
    HINSTANCE   previousWinInstance,
    LPSTR       cmdLine,
    int         showCommand)
{
    try {
        Run(winInstance);
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}