#include "Security.hpp"
#include "../Utils/Utils.hpp"

SEC::RuntimeSecurityPtr RuntimeSecurity = std::make_unique<SEC::SecurityClass>();

namespace SEC {

	BOOL SecurityClass::IsAdministrator()
	{
		SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
		PSID AdministratorsGroup;

		if (!AllocateAndInitializeSid(&NtAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID,
			DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &AdministratorsGroup))
		{
			return FALSE;
		}

		BOOL IsInAdminGroup = FALSE;

		if (!CheckTokenMembership(NULL, AdministratorsGroup, &IsInAdminGroup))
		{
			IsInAdminGroup = FALSE;
		}

		FreeSid(AdministratorsGroup);
		return IsInAdminGroup;
	}

	void SecurityClass::KillTasks() {

		LI_FN(WinExec)(STR("taskkill /f /im HTTPDebuggerUI.exe >nul 2>&1"), SW_HIDE);
		LI_FN(WinExec)(STR("taskkill /f /im HTTPDebuggerSvc.exe >nul 2>&1"), SW_HIDE);
		LI_FN(WinExec)(STR("sc stop HTTPDebuggerPro >nul 2>&1"), SW_HIDE);
		LI_FN(WinExec)(STR("cmd.exe /c @RD /S /Q \"C:\\Users\\%username%\\AppData\\Local\\Microsoft\\Windows\\INetCache\\IE\" >nul 2>&1"), SW_HIDE);

	}
#pragma optimize("", off)

	void SecurityClass::CheckDebugger() {

		MUTATE_START

		for (;;) {
			BOOL DebuggerPresent = TRUE;
			LI_FN(CheckRemoteDebuggerPresent)(CurrentProc, &DebuggerPresent);

			if (LI_FN(IsDebuggerPresent)() || DebuggerPresent)
				SecurityCallback(STR("Malicious activity [Debugger present]"));

			LI_FN(Sleep)(25);
		}

		MUTATE_END
	}

	void SecurityClass::CheckDrivers() {

		MUTATE_START

		for (;;) {
			const TCHAR* handles[] = {
				_T("\\\\.\\KsDumper")
			};

			WORD iLength = sizeof(handles) / sizeof(handles[0]);
			for (int i = 0; i < iLength; i++)
			{
				HANDLE hFile = CreateFile(handles[i], GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

				if (hFile != INVALID_HANDLE_VALUE)
					SecurityCallback(STR("Malicious activity [Blacklisted driver present]"));
			}

			LI_FN(Sleep)(10);
		}

		MUTATE_END
	}

	void SecurityClass::CheckTraversedMem() {

		MUTATE_START

		for (;;) {
			const auto m = LI_FN(VirtualAlloc).Get()(nullptr, 4096, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

			PSAPI_WORKING_SET_EX_INFORMATION set{ };
			set.VirtualAddress = m;

			if (LI_FN(K32QueryWorkingSetEx).Get()(CurrentProc, &set, sizeof(set) && (set.VirtualAttributes.Valid & 0x1)))
				SecurityCallback(STR("Malicious activity [MemoryTraversed]"));

			LI_FN(Sleep)(30);
		}

	
		MUTATE_END
	}

	void SecurityClass::CheckKernelInfo() {

		MUTATE_START

		for (;;) {

			SYSTEM_KERNEL_DEBUGGER_INFORMATION Info;

			// We have to import the function
			pZwQuerySystemInformation ZwQuerySystemInformation = NULL;

			ZwQuerySystemInformation = (pZwQuerySystemInformation)GetProcAddress(NtDLL, STR("ZwQuerySystemInformation"));
			if (!ZwQuerySystemInformation)
				return;

			NTSTATUS Status = ZwQuerySystemInformation(SystemKernelDebuggerInformation, &Info, sizeof(Info), NULL);

			if (Status != 0x00000000L /* SUCCESS */ || !Info.DebuggerEnabled)
				return;

			if (!Info.DebuggerNotPresent)
				SecurityCallback(STR("Malicious activity [Debugger present]"));

			LI_FN(Sleep)(50);
		}

		MUTATE_END
	}

	void SecurityClass::CheckSession() {

		MUTATE_START

		if (!Auth.Data.Initiated)
			return;

		for (;;) {

			if (!Auth.CheckExistingSession())
				SecurityCallback(STR("Malicious activity [Invalid Session]"));

			LI_FN(Sleep)(10000);
	
		}

		MUTATE_END
	}

#pragma optimize("", on)

	bool SecurityClass::Init() 
	{
		WRAP_IF_RELEASE(

			// if we don't have admin permissions, just exit the proccess
			if (!IsAdministrator())
				return false;

			// kill http debugger and other services
			KillTasks();

			// create threads
			CreateThreads();
		);

		return true;
	}

#pragma optimize("", off)
	void SecurityClass::CreateThreads()
	{
		MUTATE_START

		std::thread DebugThread(&SecurityClass::CheckDebugger,		  this); DebugThread.detach();
		std::thread DriverThread(&SecurityClass::CheckDrivers,		  this); DriverThread.detach();
		std::thread CheckMemThread(&SecurityClass::CheckTraversedMem, this); CheckMemThread.detach();
		std::thread KernalInfoThread(&SecurityClass::CheckKernelInfo, this); KernalInfoThread.detach();
		std::thread CheckSessionThread(&SecurityClass::CheckSession,  this); CheckSessionThread.detach();

		MUTATE_END
	}
#pragma optimize("", on)

	void SecurityClass::SecurityCallback(const char* Reason) {

		static bool Triggered = false;

		if (!Triggered) {

			Auth.BanUser();

			std::ofstream File("loader.errr");
			File.write(Reason, LI_FN(strlen).Get()(Reason));
			File.close();

			LI_FN(ExitProcess)(rand() % RAND_MAX);

			Triggered = true;
		}

	}
}