#pragma once
#include "Map.hpp"
#include "../includes.hpp"

void __stdcall Shellcode(MANUAL_MAPPING_DATA* pData);

bool ManualMap(HANDLE hProc, std::uint8_t* szDllFile)
{
	BYTE* pSourceData = nullptr;
	IMAGE_NT_HEADERS* pOldNtHeader = nullptr;
	IMAGE_OPTIONAL_HEADER* pOldOptionalHeader = nullptr;
	IMAGE_FILE_HEADER* pOldFileHeader = nullptr;
	BYTE* pTargetBase = nullptr;

	//We are at the end of the file, so we know the size already.  
	auto FileSize = sizeof(szDllFile);
	//Let's get the byte array of the file.  
	pSourceData = (BYTE*)szDllFile;
	//If we couldn't allocate memory for this data we failed
	if (!pSourceData)
	{
		return false;
	}

	if (reinterpret_cast<IMAGE_DOS_HEADER*>(pSourceData)->e_magic != 0x5A4D)
	{
		delete[] pSourceData;
		return false;
	}
	//Save the old NT Header
	pOldNtHeader = reinterpret_cast<IMAGE_NT_HEADERS*>(pSourceData + reinterpret_cast<IMAGE_DOS_HEADER*>(pSourceData)->e_lfanew);
	//Save the old optional header
	pOldOptionalHeader = &pOldNtHeader->OptionalHeader;
	//Save the old file header
	pOldFileHeader = &pOldNtHeader->FileHeader;
	//Handle X86 and X64
#ifdef _WIN64
	//If the machine type is not the current file type we fail
	if (pOldFileHeader->Machine != IMAGE_FILE_MACHINE_AMD64)
	{
		delete[] pSourceData;
		return false;
	}
#else
	//If the machine type is not the current file type we fail
	if (pOldFileHeader->Machine != IMAGE_FILE_MACHINE_I386)
	{
		delete[] pSourceData;
		return false;
	}
#endif
	//Get the target base address to allocate memory in the target process
	//Try to load at image base of the old optional header, the size of the optional header image, commit = make , reserve it, execute read write to write the memory
	pTargetBase = reinterpret_cast<BYTE*>(LI_FN(VirtualAllocEx).Get()(hProc, reinterpret_cast<void*>(pOldOptionalHeader->ImageBase), pOldOptionalHeader->SizeOfImage, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE));
	if (!pTargetBase)
	{
		//We can't allocate it, lets initialize it instead?
		//Forget the image base, just use nullptr, if this fails we cannot allocate memory in the target process.  
		pTargetBase = reinterpret_cast<BYTE*>(LI_FN(VirtualAllocEx).Get()(hProc, nullptr, pOldOptionalHeader->SizeOfImage, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE));
		if (!pTargetBase)
		{
			delete[] pSourceData;
			return false;
		}
	}
	//Declare data to map
	MANUAL_MAPPING_DATA data{ 0 };
	//Declare function prototype
	data.pLoadLibraryA = LI_FN(LoadLibraryA).Get();
	//Declare function prototype
	data.pGetProcAddress = reinterpret_cast<f_GetProcAddress>(LI_FN(GetProcAddress).Get());

	//Get the section header
	auto* pSectionHeader = IMAGE_FIRST_SECTION(pOldNtHeader);
	//Loop the file header sections for section data, we only care about the raw data in here, it contains other data that is used after runtime which we dont care about.  
	for (UINT i = 0; i != pOldFileHeader->NumberOfSections; ++i, ++pSectionHeader)
	{
		//If it's raw data
		if (pSectionHeader->SizeOfRawData)
		{
			//Try to write our source data into the process, mapping.
			if (!LI_FN(WriteProcessMemory).Get()(hProc, pTargetBase + pSectionHeader->VirtualAddress, pSourceData + pSectionHeader->PointerToRawData, pSectionHeader->SizeOfRawData, nullptr))
			{
				//We couldn't allocate memory 
		//		printf("Failed to allocate memory: 0x%x\n", GetLastError());
				delete[] pSourceData;
				VirtualFreeEx(hProc, pTargetBase, 0, MEM_RELEASE);
				return false;
			}
		}
	}

	LI_FN(memcpy)(pSourceData, &data, sizeof(data));
	LI_FN(WriteProcessMemory)(hProc, pTargetBase, pSourceData, 0x1000, nullptr);

	void* pShellcode = LI_FN(VirtualAllocEx).Get()(hProc, nullptr, 0x1000, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	if (!pShellcode)
	{
		//printf("Memory allocation failed (1) (ex) 0x%X\n", GetLastError());
		LI_FN(VirtualFreeEx)(hProc, pTargetBase, 0, MEM_RELEASE);
		return false;
	}

	LI_FN(WriteProcessMemory).Get()(hProc, pShellcode, Shellcode, 0x1000, nullptr);

	HANDLE hThread = LI_FN(CreateRemoteThread).Get()(hProc, nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(pShellcode), pTargetBase, 0, nullptr);
	if (!hThread)
	{
		//	printf("Thread creation failed 0x%X\n", GetLastError());
		LI_FN(VirtualFreeEx).Get()(hProc, pTargetBase, 0, MEM_RELEASE);
		LI_FN(VirtualFreeEx).Get()(hProc, pShellcode, 0, MEM_RELEASE);
		return false;
	}

	LI_FN(CloseHandle)(hThread);

	HINSTANCE hCheck = NULL;
	while (!hCheck)
	{
		MANUAL_MAPPING_DATA data_checked{ 0 };
		LI_FN(ReadProcessMemory)(hProc, pTargetBase, &data_checked, sizeof(data_checked), nullptr);
		hCheck = data_checked.hMod;
		LI_FN(Sleep)(10);
	}

	LI_FN(VirtualFreeEx)(hProc, pShellcode, 0, MEM_RELEASE);

	return true;
}

#define RELOC_FLAG32(RelInfo) ((RelInfo >> 0x0C) == IMAGE_REL_BASED_HIGHLOW)
#define RELOC_FLAG64(RelInfo) ((RelInfo >> 0x0C) == IMAGE_REL_BASED_DIR64)

#ifdef _WIN64
#define RELOC_FLAG RELOC_FLAG64
#else
#define RELOC_FLAG RELOC_FLAG32
#endif

void __stdcall Shellcode(MANUAL_MAPPING_DATA* pData)
{
	if (!pData)
		return;
	//Process base
	BYTE* pBase = reinterpret_cast<BYTE*>(pData);
	//Optional data
	auto* pOptionalHeader = &reinterpret_cast<IMAGE_NT_HEADERS*>(pBase + reinterpret_cast<IMAGE_DOS_HEADER*>(pData)->e_lfanew)->OptionalHeader;

	auto _LoadLibraryA = pData->pLoadLibraryA;
	auto _GetProcAddress = pData->pGetProcAddress;
	auto _DllMain = reinterpret_cast<f_DLL_ENTRY_POINT>(pBase + pOptionalHeader->AddressOfEntryPoint);

	BYTE* LocationDelta = pBase - pOptionalHeader->ImageBase;
	if (LocationDelta)
	{
		if (!pOptionalHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size)
			return;

		auto* pRelocData = reinterpret_cast<IMAGE_BASE_RELOCATION*>(pBase + pOptionalHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress);
		while (pRelocData->VirtualAddress)
		{
			UINT AmountOfEntries = (pRelocData->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(WORD);
			WORD* pRelativeInfo = reinterpret_cast<WORD*>(pRelocData + 1);

			for (UINT i = 0; i != AmountOfEntries; ++i, ++pRelativeInfo)
			{
				if (RELOC_FLAG(*pRelativeInfo))
				{
					UINT_PTR* pPatch = reinterpret_cast<UINT_PTR*>(pBase + pRelocData->VirtualAddress + ((*pRelativeInfo) & 0xFFF));
					*pPatch += reinterpret_cast<UINT_PTR>(LocationDelta);
				}
			}
			pRelocData = reinterpret_cast<IMAGE_BASE_RELOCATION*>(reinterpret_cast<BYTE*>(pRelocData) + pRelocData->SizeOfBlock);
		}
	}

	if (pOptionalHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size)
	{
		auto* pImportDescr = reinterpret_cast<IMAGE_IMPORT_DESCRIPTOR*>(pBase + pOptionalHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);
		while (pImportDescr->Name)
		{
			char* szMod = reinterpret_cast<char*>(pBase + pImportDescr->Name);
			HINSTANCE hDll = _LoadLibraryA(szMod);

			ULONG_PTR* pThunkRef = reinterpret_cast<ULONG_PTR*>(pBase + pImportDescr->OriginalFirstThunk);
			ULONG_PTR* pFuncRef = reinterpret_cast<ULONG_PTR*>(pBase + pImportDescr->FirstThunk);

			if (!pThunkRef)
				pThunkRef = pFuncRef;

			for (; *pThunkRef; ++pThunkRef, ++pFuncRef)
			{
				if (IMAGE_SNAP_BY_ORDINAL(*pThunkRef))
				{
					*pFuncRef = _GetProcAddress(hDll, reinterpret_cast<char*>(*pThunkRef & 0xFFFF));
				}
				else
				{
					auto* pImport = reinterpret_cast<IMAGE_IMPORT_BY_NAME*>(pBase + (*pThunkRef));
					*pFuncRef = _GetProcAddress(hDll, pImport->Name);
				}
			}
			++pImportDescr;
		}
	}

	if (pOptionalHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS].Size)
	{
		auto* pTLS = reinterpret_cast<IMAGE_TLS_DIRECTORY*>(pBase + pOptionalHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS].VirtualAddress);
		auto* pCallback = reinterpret_cast<PIMAGE_TLS_CALLBACK*>(pTLS->AddressOfCallBacks);
		for (; pCallback && *pCallback; ++pCallback)
			(*pCallback)(pBase, DLL_PROCESS_ATTACH, nullptr);
	}
	//Execute dll main
	_DllMain(pBase, DLL_PROCESS_ATTACH, nullptr);

	pData->hMod = reinterpret_cast<HINSTANCE>(pBase);
}