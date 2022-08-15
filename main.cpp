#include <iostream>
#include <vector>

#include "includes.hpp"

#include "UserInterface/UserInterface.hpp"
#include "Utils/Utils.hpp"

#include <Registry.hpp>

/*
NOTES:
Themida macros - may error if optimization is on for that function
Refrence: https://www.oreans.com/help/tm/hm_faq_when-i-use-macros-directly-aro.htm

Themida is included into this source, which requires this library in the same directory
https://cdn.discordapp.com/attachments/1003540719895592981/1008544405625516062/SecureEngineSDK32.rar
This includes SecureEngineSDK32.dll, which can be found in Themida\ThemidaSDK (InstallDir\ThemidaSDK)

*/

int WINAPI WinMain(HINSTANCE, HINSTANCE, PSTR, int)
{
    CurrentProc = LI_FN(GetCurrentProcess).Get()();
    NtDLL = LI_FN(LoadLibraryA).Get()(STR("ntdll.dll"));

    // Initiate auth.
    if (!Auth.Init() && !Auth.Data.InvalidVer) 
        ASSERT_ERROR(Auth.Data.ErrorMsg.c_str(), STR("[ERR:001A00]"));

    if (Auth.CheckBlacklisted()) 
        RuntimeSecurity->SecurityCallback(STR("Blacklisted."));
    
    // hide console
    LI_FN(AllocConsole)();
    LI_FN(ShowWindow)(FindWindowA(STR("ConsoleWindowClass"), NULL), false);
 
    FileReader File;

    if (!File.Start(STR("SecureEngineSDK32.dll")))
        ASSERT_ERROR(STR("Unable to find needed DLLs, please reinstall. If this happens repeatedly contact an administrator."), STR("[ERR:003A00]")); 

    if (!RuntimeSecurity->Init())
        ASSERT_ERROR(STR("Unable to initialize."), STR("[ERR:004A00]"));

    // Initiate UI.
    std::thread UserInterfaceThread([&] { UserInterface->Init(); }); UserInterfaceThread.detach();
    
    auto Inject = [&]() -> void 
    {
        DWORD PID = Utils::GetPid(STR("csgo.exe"));
        if (!PID)
            ASSERT_ERROR(STR("Unable to find CS:GO."), STR("[ERR:001B00]"));

        HANDLE hProc = LI_FN(OpenProcess).Get()(PROCESS_ALL_ACCESS, FALSE, PID);

        ByteArray Internal = UserInterface->Data.Game ? Auth.RecieveLegacyData(UserInterface->Data.SelectedBeta) : Auth.RecieveData(UserInterface->Data.SelectedBeta);
        if (!ManualMap(hProc, &Internal[0]))
            ASSERT_ERROR(STR("Unable to inject."), STR("[ERR:002B00]"));

        Internal.clear();
    };

    // Allow the user to login.
    while (!Auth.Data.LoggedIn) { LI_FN(Sleep)(1); }

    if (UserInterface->Data.AutoInject)
    {
        while (!Utils::GetPid(STR("csgo.exe"))) { LI_FN(Sleep)(1); }

        LI_FN(Sleep)(1000);

        Inject();
    }

    while (!UserInterface->Data.ShouldInject) { LI_FN(Sleep)(1); }

    Inject();

    LI_FN(ExitProcess)(1);
    return 1;
}
