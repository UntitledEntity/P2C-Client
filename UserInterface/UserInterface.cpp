#include "UserInterface.hpp"
#include "../Utils/Utils.hpp"

UI::UserInterfacePtr UserInterface = std::make_unique<UI::UserInterfaceClass>();
KeyAuth::api Auth(STR("APPNAME"), STR("OWNERID"), STR("SECRET"), STR(LOADER_VER));

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace UI
{
    // Win32 message handler
    inline LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
            return true;

        switch (msg)
        {
        case WM_LBUTTONDOWN:
        {
            m_Pos = MAKEPOINTS(lParam); // set click points
            return 0;
        }
        case WM_MOUSEMOVE:
        {
            if (wParam == MK_LBUTTON)
            {
                POINTS p = MAKEPOINTS(lParam); // get cur mousemove click points
                RECT rect;
                GetWindowRect(hWnd, &rect);
                rect.left += p.x - m_Pos.x; // get xDelta
                rect.top += p.y - m_Pos.y;  // get yDelta
                if (m_Pos.x >= 0 && m_Pos.x <= LOADERW - 20 /* cuz 20px - close btn */ && m_Pos.y >= 0 && m_Pos.y <= ImGui::GetFontSize() + ImGui::GetStyle().FramePadding.y * 2.0f)
                    SetWindowPos(hWnd, HWND_TOPMOST, rect.left, rect.top, 0, 0, SWP_SHOWWINDOW | SWP_NOSIZE | SWP_NOZORDER);
            }
            return 0;
        }
        case WM_SIZE:
            if (g_pd3dDevice != NULL && wParam != SIZE_MINIMIZED)
            {
                g_d3dpp.BackBufferWidth = LOWORD(lParam);
                g_d3dpp.BackBufferHeight = HIWORD(lParam);
                ResetDevice();
            }
            return 0;
        case WM_SYSCOMMAND:
            if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
                return 0;
            break;
        case WM_DESTROY:
            ::PostQuitMessage(0);
            return 0;
        }
        return ::DefWindowProc(hWnd, msg, wParam, lParam);
    }

	void UserInterfaceClass::Theme()
	{
        ImVec4* colors = ImGui::GetStyle().Colors;
        colors[ImGuiCol_Text] = ImVec4(0.95f, 0.96f, 0.98f, 1.00f);
        colors[ImGuiCol_TextDisabled] = ImVec4(0.36f, 0.42f, 0.47f, 1.00f);
        colors[ImGuiCol_WindowBg] = ImVec4(0.11f, 0.15f, 0.17f, 1.00f);
        colors[ImGuiCol_ChildBg] = ImVec4(0.15f, 0.18f, 0.22f, 1.00f);
        colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
        colors[ImGuiCol_Border] = ImVec4(0.08f, 0.10f, 0.12f, 1.00f);
        colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        colors[ImGuiCol_FrameBg] = ImVec4(0.20f, 0.25f, 0.29f, 1.00f);
        colors[ImGuiCol_FrameBgHovered] = ImVec4(0.12f, 0.20f, 0.28f, 1.00f);
        colors[ImGuiCol_FrameBgActive] = ImVec4(0.09f, 0.12f, 0.14f, 1.00f);
        colors[ImGuiCol_TitleBg] = ImVec4(0.09f, 0.12f, 0.14f, 0.65f);
        colors[ImGuiCol_TitleBgActive] = ImVec4(0.08f, 0.10f, 0.12f, 1.00f);
        colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
        colors[ImGuiCol_MenuBarBg] = ImVec4(0.15f, 0.18f, 0.22f, 1.00f);
        colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.39f);
        colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.20f, 0.25f, 0.29f, 1.00f);
        colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.18f, 0.22f, 0.25f, 1.00f);
        colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.09f, 0.21f, 0.31f, 1.00f);
        colors[ImGuiCol_CheckMark] = ImVec4(0.28f, 0.56f, 1.00f, 1.00f);
        colors[ImGuiCol_SliderGrab] = ImVec4(0.28f, 0.56f, 1.00f, 1.00f);
        colors[ImGuiCol_SliderGrabActive] = ImVec4(0.37f, 0.61f, 1.00f, 1.00f);
        colors[ImGuiCol_Button] = ImVec4(0.20f, 0.25f, 0.29f, 0.00f);
        colors[ImGuiCol_ButtonHovered] = ImVec4(0.28f, 0.56f, 1.00f, 1.00f);
        colors[ImGuiCol_ButtonActive] = ImVec4(0.06f, 0.53f, 0.98f, 1.00f);
        colors[ImGuiCol_Header] = ImVec4(0.20f, 0.25f, 0.29f, 0.55f);
        colors[ImGuiCol_HeaderHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
        colors[ImGuiCol_HeaderActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
        colors[ImGuiCol_Separator] = ImVec4(0.20f, 0.25f, 0.29f, 1.00f);
        colors[ImGuiCol_SeparatorHovered] = ImVec4(0.10f, 0.40f, 0.75f, 0.78f);
        colors[ImGuiCol_SeparatorActive] = ImVec4(0.10f, 0.40f, 0.75f, 1.00f);
        colors[ImGuiCol_ResizeGrip] = ImVec4(0.26f, 0.59f, 0.98f, 0.25f);
        colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
        colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
        colors[ImGuiCol_Tab] = ImVec4(0.11f, 0.15f, 0.17f, 1.00f);
        colors[ImGuiCol_TabHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
        colors[ImGuiCol_TabActive] = ImVec4(0.20f, 0.25f, 0.29f, 1.00f);
        colors[ImGuiCol_TabUnfocused] = ImVec4(0.11f, 0.15f, 0.17f, 1.00f);
        colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.11f, 0.15f, 0.17f, 1.00f);
        colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
        colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
        colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
        colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
        colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
        colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
        colors[ImGuiCol_NavHighlight] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
        colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
        colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
        colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
	}

    void UserInterfaceClass::Init()
    {
        WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, _T("Loader"), NULL };
        ::RegisterClassEx(&wc);
        int x = GetSystemMetrics(SM_CXSCREEN) / 2 - LOADERW / 2; // cetner screen x
        int y = GetSystemMetrics(SM_CYSCREEN) / 2 - LOADERH / 2; // center screen y
        HWND hwnd = ::CreateWindowExW(WS_EX_LAYERED, (LPCWSTR)wc.lpszClassName, (LPCWSTR)STR("Loader"), WS_POPUP, x, y, LOADERW, LOADERH, NULL, NULL, wc.hInstance, NULL);

        SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), 0, LWA_COLORKEY);

        // Initialize Direct3D
        if (!CreateDeviceD3D(hwnd))
        {
            CleanupDeviceD3D();
            ::UnregisterClass(wc.lpszClassName, wc.hInstance);
            return;
        }

        // Show the window
        ::ShowWindow(hwnd, SW_SHOWDEFAULT);
        ::UpdateWindow(hwnd);

        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.Fonts->AddFontFromMemoryTTF(&LogoFont, sizeof LogoFont, 15);
        ImFont* Logo = io.Fonts->AddFontFromMemoryTTF(&LogoFont, sizeof LogoFont, 18);
        ImFont* Main = io.Fonts->AddFontFromMemoryTTF(&OtherFont, sizeof OtherFont, 18);
        ImFont* Icon = io.Fonts->AddFontFromMemoryTTF(&IconFont, sizeof IconFont, 24);

        ImGui::GetStyle().WindowRounding = 0.0f;
        ImGui::GetStyle().WindowPadding = ImVec2(0.0f, 0.0f);
        ImGui::GetStyle().ChildRounding = 0.0f;
        ImGui::GetStyle().WindowBorderSize = 0.0f;
        ImGui::GetStyle().WindowTitleAlign = ImVec2(0.5, 0.5);
        ImGui::GetStyle().PopupBorderSize = 0.0f;

        // Setup Platform/Renderer bindings
        ImGui_ImplWin32_Init(hwnd);
        ImGui_ImplDX9_Init(g_pd3dDevice);

        // Load Fonts
        // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
        // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
        // - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
        // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
        // - Read 'docs/FONTS.md' for more instructions and details.
        // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
        //io.Fonts->AddFontDefault();
        //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
        //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
        //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
        //io.Fonts->AddFontFromFileTTF("../../misc/fonts/ProggyTiny.ttf", 10.0f);
        //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
        //IM_ASSERT(font != NULL);

        UserInterface->Theme();

        // Our state
        static bool opened = true;

        // Main loop
        MSG msg;
        ZeroMemory(&msg, sizeof(msg));
        while (msg.message != WM_QUIT)
        {
            // Poll and handle messages (inputs, window resize, etc.)
            // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
            // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
            // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
            // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
            if (::PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
            {
                ::TranslateMessage(&msg);
                ::DispatchMessage(&msg);
                continue;
            }

            // Start the Dear ImGui frame
            ImGui_ImplDX9_NewFrame();
            ImGui_ImplWin32_NewFrame();
            ImGui::NewFrame();

            ImGui::SetNextWindowPos(ImVec2(0, 0));
            ImGui::SetNextWindowSize(ImVec2(LOADERW, LOADERH));

            ImGui::Begin(STR("Loader"), &opened, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoDecoration); // Create a window called "Hello, world!" and append into it.
            {
                ImGui::SetCursorPos(ImVec2(0, 0));
                UserInterface->RenderUI(Logo, Main, Icon);
            }
            ImGui::End();

            ImGui::EndFrame();
            g_pd3dDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
            g_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
            g_pd3dDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
            g_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, NULL, 1.0f, 0);
            if (g_pd3dDevice->BeginScene() >= 0)
            {
                ImGui::Render();
                ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
                g_pd3dDevice->EndScene();
            }
            HRESULT result = g_pd3dDevice->Present(NULL, NULL, NULL, NULL);

            if (result == D3DERR_DEVICELOST && g_pd3dDevice->TestCooperativeLevel() == D3DERR_DEVICENOTRESET)
                ResetDevice();

            if (!opened) msg.message = WM_QUIT;
        }

        ImGui_ImplDX9_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();

        CleanupDeviceD3D();
        ::DestroyWindow(hwnd);
        ::UnregisterClass(wc.lpszClassName, wc.hInstance);
    }

    void UserInterfaceClass::RenderTabs(ImFont* Logo, ImFont* Icon)
    {
        ImGui::PushFont(Logo);
        ImGui::SetCursorPos(ImVec2(5, 30));
        ImGui::Text(STR("Intertwined"));
        ImGui::PushFont(Icon);

        if (ImGui::Button(STR("2"), ImVec2(130, 42)))
            CurrentTab = 0;

        if (ImGui::Button(STR("3"), ImVec2(130, 42)))
            CurrentTab = 1;

        ImGui::PushFont(Logo);
        ImGui::SetCursorPosY(400);

        if (ImGui::Button(STR("Close"), ImVec2(130, 42))) 
            ExitProcess(0);
       
        ImGui::PopFont();
    }

    void UserInterfaceClass::RenderUI(ImFont* Logo, ImFont* Main, ImFont* Icon)
    {
        ImGui::BeginChild(STR("TabsBar"), ImVec2(130, 450));
        {
            RenderTabs(Logo, Icon);
        }
        ImGui::EndChild();
        
       
        ImGui::PushFont(Main);
        ImGui::SetCursorPos(ImVec2(146, 40));

        switch (CurrentTab)
        {
        case 0:
        {
            ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(0, 52), ImVec2(130, 94), ImGui::ColorConvertFloat4ToU32(ImVec4(0.00f, 0.00f, 2.55f, 0.1f)));
            ImGui::BeginChild(STR("MainWindow"), ImVec2(440, 388));
            {
                if (Auth.Data.LoggedIn) {
                    ImGui::PushItemWidth(22);
                    ImGui::SetCursorPos(ImVec2(148, 10));
                    
                    ImGui::Text(STR("   Welcome %s"), Data.Username);

                    ImGui::PushItemWidth(22);
                    ImGui::SetCursorPos(ImVec2(125, 125));
                    ImGui::Text(STR("Game"));

                    const char* Games[] = { "CSGO", "CSGO (L)" };
                    ImGui::SetCursorPos(ImVec2(125, 150));
                    ImGui::PushItemWidth(200);
                    ImGui::Combo(STR("##Game"), &Data.Game, Games, IM_ARRAYSIZE(Games));

                    if (Data.Beta) {
                        ImGui::SetCursorPos(ImVec2(125, 250));
                        ImGui::Checkbox("Beta", &Data.SelectedBeta);
                    } 

                    ImGui::SetCursorPos(ImVec2(125, 300));
                    if (ImGui::Button(STR("Inject"), ImVec2(165, 40))) {

                        if (Utils::GetPid(STR("csgo.exe")))
                            Data.ShouldInject = true;
                        else
                            MessageBoxA(NULL, STR("Please open CS:GO"), STR("Intertwined Client"), MB_OK);
                    }
                }
                else {
                    ImGui::SetCursorPos(ImVec2(165, 38));
                    ImGui::PushItemWidth(22);
                    ImGui::Text(STR("  Username"));
                    ImGui::PushItemWidth(401);
                    ImGui::SetCursorPos(ImVec2(19, 60));
                    ImGui::InputText(STR("###Username"), Data.Username, IM_ARRAYSIZE(Data.Username));

                    ImGui::SetCursorPos(ImVec2(165, 100));
                    ImGui::PushItemWidth(22);
                    ImGui::Text(STR("  Password"));
                    ImGui::PushItemWidth(401);
                    ImGui::SetCursorPos(ImVec2(19, 130));
                    ImGui::InputText(STR("###Password"), Data.Password, IM_ARRAYSIZE(Data.Password), ImGuiInputTextFlags_Password);

                    ImGui::PopItemWidth();
                    ImGui::SetCursorPos(ImVec2(125, 200));

                    if (ImGui::Button(STR("Sign in"), ImVec2(165, 40)))
                    {
                        if (Auth.Login(Data.Username, Data.Password)) {
                            if (Data.SaveLogin)
                                SaveLogin(Data.Username, Data.Password);

                            Data.Admin = Auth.User.Sub == STR("admin");
                            Data.Beta = Auth.User.Sub == STR("beta") || STR("mod") || Data.Admin;
                        }

                        else 
                            ASSERT_ERROR(Auth.Data.ErrorMsg.c_str(), STR("[ERR:003B00]"))
                    }

                    ImGui::PopItemWidth();
                    ImGui::SetCursorPos(ImVec2(100, 250));

                    if (LoginDataExists() && ImGui::Button(STR("Sign in using saved data"), ImVec2(200, 40)))
                    {
                        GetLogin();

                        if (Auth.Login(Data.LoginData.first, Data.LoginData.second)) {
                            Data.Admin = Auth.User.Sub == STR("admin");
                            Data.Beta = Auth.User.Sub != STR("normal");
                        }

                        else {
                            std::filesystem::remove(STR("C:\\ProgramData\\int.data"));
                            ASSERT_ERROR(Auth.Data.ErrorMsg.c_str(), STR("[ERR:003B00]"));
                        }
                            
                    }
                }

            }
            ImGui::EndChild();

            break;
        }
        case 1: {
            ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(0, 98), ImVec2(130, 140), ImGui::ColorConvertFloat4ToU32(ImVec4(0.00f, 0.00f, 2.55f, 0.1f)));

            ImGui::BeginChild(STR("Options"), ImVec2(440, 388));
            {
                ImGui::SetCursorPos(ImVec2(25, 25));
                ImGui::Checkbox(STR("Save password"), &Data.SaveLogin);
                ImGui::SetCursorPosX(25);
                ImGui::Checkbox(STR("Automatically inject"), &Data.AutoInject);
            }
            ImGui::EndChild();

            break;
        }

        default:
            break;
        }
    
    }
}