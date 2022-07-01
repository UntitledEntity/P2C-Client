#pragma once
#include "../includes.hpp"

#pragma warning(disable : 4996)

inline	nlohmann::json response_decoder;

namespace Utils {

	inline std::string RandomString(const int len) {
		const std::string alpha_numeric(STR("ABCDEFGHIJKLMNOPRSTUVZabcdefghijklmnoprstuvz123456789"));
		std::default_random_engine generator{ std::random_device{}() };
		const std::uniform_int_distribution< std::string::size_type > distribution{ 0, alpha_numeric.size() - 1 };
		std::string str(len, 0);
		for (auto& it : str) {
			it = alpha_numeric[distribution(generator)];
		}

		return str;
	}

	inline int RandomInt(int min, int max)
	{
		int range = max - min + 1;
		return LI_FN(rand).Get()() % range + min;
	}

	inline DWORD_PTR GetPid(const std::string process_name) {
		PROCESSENTRY32 process_info;
		process_info.dwSize = sizeof(process_info);

		HANDLE snapshot = LI_FN(CreateToolhelp32Snapshot).Get()(TH32CS_SNAPPROCESS, NULL);
		if (snapshot == INVALID_HANDLE_VALUE)
			return 0;

		LI_FN(Process32First).Get()(snapshot, &process_info);
		if (!process_name.compare((const char*)process_info.szExeFile)) {
			LI_FN(CloseHandle).Get()(snapshot);
			return process_info.th32ProcessID;
		}

		while (LI_FN(Process32Next).Get()(snapshot, &process_info)) {
			if (!process_name.compare((const char*)process_info.szExeFile)) {
				LI_FN(CloseHandle).Get()(snapshot);
				return process_info.th32ProcessID;
			}
		}

		LI_FN(CloseHandle).Get()(snapshot);

		return 0;
	}

	inline void AdminPrint(const char* message, ...)
	{
		if (!UserInterface->Data.Admin)
			return;

		std::string to_print = message; to_print.append(STR("\n"));
		printf(to_print.c_str());
	}
	
	inline std::wstring StringToWString(const std::string& str) {
		std::wstring_convert<std::codecvt_utf8<wchar_t>> myconv;
		return myconv.from_bytes(str);
	}

	inline std::string GetPcName() {
		char acUserName[100];
		DWORD nUserName = sizeof(acUserName);
		LI_FN(GetUserNameA)(acUserName, &nUserName);
		return acUserName;
	}

	#define SELF_REMOVE_STRING  TEXT("cmd.exe /C ping 1.1.1.1 -n 1 -w 3000 > Nul & Del /f /q \"%s\"")

	inline void Seppuku() {
		TCHAR szModuleName[MAX_PATH];
		TCHAR szCmd[2 * MAX_PATH];
		STARTUPINFO si = { 0 };
		PROCESS_INFORMATION pi = { 0 };

		GetModuleFileName(NULL, szModuleName, MAX_PATH);

		StringCbPrintf(szCmd, 2 * MAX_PATH, STR(SELF_REMOVE_STRING), szModuleName);

		CreateProcess(NULL, szCmd, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi);

		LI_FN(CloseHandle)(pi.hThread);
		LI_FN(CloseHandle)(pi.hProcess);
	}
}