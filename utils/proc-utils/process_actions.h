#pragma once
#include <iostream>
#include <Windows.h>
#include <TlHelp32.h>
#include <winternl.h>

#include "getlast_error.h"

#define _CRT_SECURE_NO_WARNINGS

using namespace std;
typedef unsigned short uint;

//Important: windowProcessName is the string as specified in SetWindowText - the title of the window. 

namespace PROCESS_ACTIONS {
	template<class typeNameA>
	class ProcActions {
	public:
		DWORD getProcessIDbyName(LPCSTR windowProcessName);
		MEMORY_BASIC_INFORMATION* _QueryProcessInfo_(LPCSTR windowProcessName, DWORD addressToQuery, HANDLE exHndl = 0);
		bool _WriteProcMemory_(DWORD AddressToWriteTo, int InsertionValue, LPCSTR WindowName);
		bool _ChageProtection_(LPCSTR windowProcessName, const int DESIRED_PROTECTION);
		int _ReadProcMemory_(DWORD AddressToReadFrom, LPCSTR windowProcessName);
	private:
	};

	//Query process information
	template<class typeNameA>
	MEMORY_BASIC_INFORMATION* ProcActions<typename typeNameA>::_QueryProcessInfo_(LPCSTR windowProcesName, DWORD addressToQuery, HANDLE exHndl) {
		DWORD ProcessID;
		MEMORY_BASIC_INFORMATION mbi;
		std::string updatedProtection;

		HWND handleToWindow = FindWindowA(NULL, windowProcesName);
		if (handleToWindow == NULL) {
			cerr << "Error --- Could not find window: " << GetLastErrorAsString() << endl;
			return false;
		}
		else {
			int threadID = GetWindowThreadProcessId(handleToWindow, &ProcessID);
			HANDLE toProc = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, ProcessID);
			if (exHndl != NULL) {
				toProc = exHndl;
			}
			else {

				VirtualQueryEx(toProc, (void*)addressToQuery, &mbi, sizeof(mbi));

				/*
				switch (mbi.Protect)
				{
				case 1: mbi.Protect = 1;
					updatedProtection = "PAGE_EXECUTE";

				case 2: mbi.Protect = 0x20;
					updatedProtection = "PAGE_EXECUTE_READ";

				case 3: mbi.Protect = 0x40;
					updatedProtection = "PAGE_EXECUTE_READWRITE";

				case 4: mbi.Protect = 0x80;
					updatedProtection = "PAGE_EXECUTE_WRITECOPY";

				case 5: mbi.Protect = 0x01;
					updatedProtection = "PAGE_NOACCESS";

				case 6: mbi.Protect = 0x02;
					updatedProtection = "PAGE_READONLY";

				case 7: mbi.Protect = 0x04;
					updatedProtection = "PAGE_READWRITE";

				case 8: mbi.Protect = 0x08;
					updatedProtection = "PAGE_WRITECOPY";

				case 9: mbi.Protect = 0x40000000;
					updatedProtection = "PAGE_TARGETS_INVALID";

				case 10: mbi.Protect = 0x40000000;
					updatedProtection = "PAGE_TARGETS_INVALID";

				}
				*/


				printf("====== PROCESS INFO =====\n");
				printf("Process name: %s\n\n", windowProcesName);
				printf("Base address: %p\n", mbi.BaseAddress);
				printf("Allocation base: %p\n", mbi.AllocationBase);
				printf("Allocation protect: %d\n", mbi.AllocationProtect);
				printf("Region size: %d\n", mbi.RegionSize);
				printf("State: 0x%08x\n", mbi.State);
				printf("Protect: 0x%08x\n", mbi.Protect);
				printf("Type: 0x%08x\n", mbi.Type);

				std::cout << GetLastErrorAsString();
				CloseHandle(toProc);
				return &mbi;
			}
		}
	}


	//Read values from memory
	template<class typeNameA>
	int ProcActions<typename typeNameA>::_ReadProcMemory_(DWORD addressToReadFrom, LPCSTR windowProcessName) {
		HWND handleToWindow = FindWindowA(NULL, windowProcessName);
		if (handleToWindow == NULL) {
			cerr << "Error --- Could not find window: " << GetLastErrorAsString() << endl;
			return false;
		}
		else {
			DWORD processID;
			int readBuffer = 0;
			DWORD threadProcessID = GetWindowThreadProcessId(handleToWindow, &processID);
			HANDLE hndlToOpenProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processID);
			if (hndlToOpenProc == NULL) {
				cerr << "Error -- Could not Open this process" << GetLastErrorAsString() << endl;
			}
			else {
				bool read = ReadProcessMemory(hndlToOpenProc, (LPVOID)addressToReadFrom, &readBuffer, sizeof(readBuffer), 0);
				if (read == true) {
					printf("Successfully read value: %d at location: %p\n", readBuffer, (void*)addressToReadFrom);
				}
				else {
					cerr << "Failed to write memory." << GetLastErrorAsString() << endl;
					return false;
				}
			}
			return readBuffer;
		}
	}

	template<class typeNameA>
	bool ProcActions<typename typeNameA>::_ChageProtection_(LPCSTR windowProcessName, const int DESIRED_PROTECTION) {
		DWORD procID;
		MEMORY_BASIC_INFORMATION mbi;
		HWND handleToWindow = FindWindowA(NULL, "AssaultCube");
		if (handleToWindow == NULL) {
			cerr << "Error --- Could not find window: " << GetLastErrorAsString() << endl;
			return false;
		}
		DWORD threadProcessID = GetWindowThreadProcessId(handleToWindow, &procID);
		HANDLE hndlToOpenProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, procID); 
		if (!hndlToOpenProc)

		//Allocate virtual memory to application
		LPVOID lpbase = VirtualAllocEx(hndlToOpenProc, NULL, 4096, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
		//Change protection
		DWORD oldProtect;
		bool cvProtect = VirtualProtectEx(hndlToOpenProc, lpbase, 4096, PAGE_EXECUTE_READ, &oldProtect);
		VirtualQueryEx(hndlToOpenProc, lpbase, &mbi, sizeof(mbi));
		printf("Protection: %d\n", mbi.Protect);
		if (cvProtect == NULL) {
			cerr << "could not change protection! : " << GetLastErrorAsString();
		}
		else {
			VirtualFreeEx(hndlToOpenProc, lpbase, 4096, MEM_DECOMMIT | MEM_RELEASE);
		}
		return true;
	}

	//Write values to memory
	template<class typeNameA>
	bool ProcActions<typename typeNameA>::_WriteProcMemory_(DWORD AddressToWriteTo, int InsertionValue, LPCSTR windowProcessName) {
		HWND handleToWindow = FindWindowA(NULL, windowProcessName);
		if (handleToWindow == NULL) {
			cerr << "Error --- Could not find window: " << GetLastErrorAsString() << endl;
			return false;
		}
		else {
			DWORD processID;
			DWORD threadProcessID = GetWindowThreadProcessId(handleToWindow, &processID);
			//You may have to open with PROCESS_VM_OPERATION
			HANDLE hndlToOpenProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processID);
			if (hndlToOpenProc == NULL) {
				cerr << "Error -- Could not Open this process" << GetLastErrorAsString() << endl;
			}
			else {
				bool write = WriteProcessMemory(hndlToOpenProc, (LPVOID)AddressToWriteTo, &InsertionValue, sizeof(InsertionValue), 0);
				if (write == true) {
					printf("Successfully wrote Value: %d at Location: %p\n", InsertionValue, (void*)AddressToWriteTo);
				}
				else {
					cerr << "Failed to write memory." << GetLastErrorAsString() << endl;
					return false;
				}
			}
		}
		return true;
	}
	
	//get the process ID of a process by name not handle
	template<class typeNameA>
	DWORD ProcActions<typename typeNameA>::getProcessIDbyName(LPCSTR windowProcessName) {
		//Take a snapshot of all processes in system
		DWORD processID;
		PROCESSENTRY32W PE;
		std::string retStr;
		HANDLE snapShot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
		if (snapShot == INVALID_HANDLE_VALUE) {
			cerr << "Error: " << GetLastErrorAsString();
		}

		//Set szie of process entry;
		PE.dwSize = sizeof(PROCESSENTRY32);

		//We need to convert the WCHAR* / wchar_t (utf-16) to a std::string (utf-8) --> Windows32 API native encoding is UTF-16
		
		//Retrieves information about first process captured by snapshot(CreateToolhelp32SnapShot)
		if (Process32First(snapShot, &PE)) {
			//get the size of the string including the null character (-1 FLAG)
			int sizeRequired = WideCharToMultiByte(CP_UTF8, 0, PE.szExeFile, -1, NULL, 0, NULL, NULL);
			while (Process32Next(snapShot, &PE) != 0) {
				std::vector<char> utf8String(sizeRequired);
				int bytesCinverted = WideCharToMultiByte(CP_UTF8, 0, PE.szExeFile, -1, &utf8String[0], utf8String.size(), NULL, NULL);
				retStr = &utf8String[0];

				//Convert a std::string to const char*
				const char* c = retStr.c_str();
				//cout << c << '\n';

				if (strcmp(windowProcessName, c) == 0) {
					cout << "Process name: " << c << endl;
					cout << "Process ID: " << PE.th32ProcessID << endl;;
					break;
				}
			}
		}
		if (snapShot != 0) {
			CloseHandle(snapShot);
		}
		return PE.th32ProcessID;
	}
}
		


