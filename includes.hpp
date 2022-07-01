#pragma once
#include <d3d9.h>
#pragma comment(lib, "d3d9.lib")

#define DIRECTINPUT_VERSION 0x0800

#define LOADERW 600
#define LOADERH 450

#include <dinput.h>
#include <tchar.h>
#include <dwmapi.h>
#include <windows.h>
#include <string>
#include <thread>
#include <libloaderapi.h>
#include <cstdio>
#include <processthreadsapi.h>
#include <iostream>
#include <fstream>
#include <Lmcons.h>
#include <stdio.h>
#include <random>
#include <Psapi.h>
#include <winternl.h>
#include <shlobj.h>
#include <filesystem>
#include <codecvt>

#include <ThemidaSDK.h>

#include "xorstr.h"

#include "Utils/FileReader.hpp"

#include "UserInterface/ImGui/stringimgui.h"
#include "UserInterface/ImGui/imgui.h"
#include "UserInterface/ImGui/imgui_impl_dx9.h"
#include "UserInterface/ImGui/imgui_impl_win32.h"
#include "UserInterface/ImGui/imgui_internal.h"
#include "UserInterface/ImGui/imstb_rectpack.h"

#include "UserInterface/Fonts/Icons.h"
#include "UserInterface/Fonts/Font.h"
#include "UserInterface/Fonts/SecondFont.h"

#include "UserInterface/UserInterface.hpp"

#include "ManualMap/Map.hpp"

#include "Security/LazyImporter.hpp"
#include "Security/Security.hpp"

#pragma comment(lib, "dwmapi.lib")

#define LOADER_VER "1.01"

#define NULLSTR ("NULL")

inline HANDLE CurrentProc;
inline HMODULE NtDLL;

#define WRAP_IF_RELEASE( s ) { s; }
#define WRAP_IF_DEBUG( s )