#pragma once

#include "../includes.hpp"

namespace SEC {

	typedef struct _SYSTEM_KERNEL_DEBUGGER_INFORMATION {
		BOOLEAN DebuggerEnabled;
		BOOLEAN DebuggerNotPresent;
	} SYSTEM_KERNEL_DEBUGGER_INFORMATION, * PSYSTEM_KERNEL_DEBUGGER_INFORMATION;

	enum SYSTEM_INFORMATION_CLASS { SystemKernelDebuggerInformation = 35 };

	// Function Pointer Typedef for ZwQuerySystemInformation
	typedef NTSTATUS(WINAPI* pZwQuerySystemInformation)(IN SYSTEM_INFORMATION_CLASS SystemInformationClass, IN OUT PVOID SystemInformation, IN ULONG SystemInformationLength, OUT PULONG ReturnLength);


	class SecurityClass {

	protected:

		BOOL IsAdministrator();
		void KillTasks();

		void CheckDebugger();
		void CheckDrivers();
		void CheckTraversedMem();
		void CheckKernelInfo();

		void CreateThreads();

	public:

		bool Init();
		bool IsTampered = false;

		void SecurityCallback(const char* Reason);

	};

	using RuntimeSecurityPtr = std::unique_ptr<SecurityClass>;

}

extern SEC::RuntimeSecurityPtr RuntimeSecurity;
