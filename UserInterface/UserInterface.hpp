#pragma once
#include "../includes.hpp"
#include "../auth/Auth.hpp"

namespace UI {

	inline static LPDIRECT3D9              g_pD3D = NULL;
	inline static LPDIRECT3DDEVICE9        g_pd3dDevice = NULL;
	inline static D3DPRESENT_PARAMETERS    g_d3dpp = {};

	inline bool CreateDeviceD3D(HWND hWnd)
	{
		if ((g_pD3D = Direct3DCreate9(D3D_SDK_VERSION)) == NULL)
			return false;

		// Create the D3DDevice
		ZeroMemory(&g_d3dpp, sizeof(g_d3dpp));
		g_d3dpp.Windowed = TRUE;
		g_d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
		g_d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
		g_d3dpp.EnableAutoDepthStencil = TRUE;
		g_d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
		g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;           // Present with vsync
		//g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;   // Present without vsync, maximum unthrottled framerate
		if (g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &g_d3dpp, &g_pd3dDevice) < 0)
			return false;

		return true;
	}

	inline void CleanupDeviceD3D()
	{
		if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = NULL; }
		if (g_pD3D) { g_pD3D->Release(); g_pD3D = NULL; }
	}

	inline void ResetDevice()
	{
		ImGui_ImplDX9_InvalidateDeviceObjects();
		HRESULT hr = g_pd3dDevice->Reset(&g_d3dpp);
		if (hr == D3DERR_INVALIDCALL)
			IM_ASSERT(0);
		ImGui_ImplDX9_CreateDeviceObjects();
	}

	inline POINTS m_Pos; // store user click pos

	inline void SaveLogin(const char* User, const char* Pass) {
		std::fstream DataFile;
		DataFile.open(STR("C:\\ProgramData\\loader.data"), std::ios::out);

		// encrypt this
		DataFile << User << std::endl << Pass << std::endl;

		DataFile.close();
	}

	inline bool LoginDataExists() {
		std::fstream DataFile;
		// decrypt this
		DataFile.open(STR("C:\\ProgramData\\loader.data"), std::ios::in);

		return !DataFile.fail();
	}

	struct UserData {

		char Username[255];
		char Password[255];

		std::pair<std::string, std::string> LoginData;

		bool Admin = false;
		bool Beta = false;
		bool SelectedBeta = false;
		bool ShouldInject = false;
		bool SaveLogin = false;
		bool AutoInject = false;

		int Game = 1;
	};

	class UserInterfaceClass
	{
	public:
		UserData Data;

		void Init();
		void Theme();
		void RenderUI(ImFont* Logo, ImFont* Main, ImFont* Icon);

	private:

		void RenderTabs(ImFont* Logo, ImFont* Icon);

		inline void GetLogin() {

			std::string User, Pass;

			std::fstream DataFile;
			DataFile.open(std::string(STR("C:\\ProgramData\\int.data")), std::ios::in);

			DataFile >> User;
			DataFile >> Pass;

			DataFile.close();

			Data.LoginData = std::make_pair(User, Pass);
		}

		int CurrentTab = 0;
	};

	using UserInterfacePtr = std::unique_ptr<UserInterfaceClass>;

}

extern UI::UserInterfacePtr UserInterface;
extern KeyAuth::api Auth;

#define ASSERT_ERROR(Error, Code, ...) { char Buffer[1024 * 16]; sprintf_s(Buffer, sizeof Buffer, Error, __VA_ARGS__); char Buffer2[1024 * 16]; sprintf_s(Buffer2, sizeof Buffer2, Code, __VA_ARGS__); MessageBoxA(NULL, Buffer, Buffer2, MB_SYSTEMMODAL | MB_ICONERROR); ExitProcess(0); }