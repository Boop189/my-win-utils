#pragma once
#include <iostream>
#include <Windows.h>
#include <TlHelp32.h>
#include <winternl.h>
#include <vector>

#include "getlast_error.h"

#define _CRT_SECURE_NO_WARNINGS

using namespace std;
typedef unsigned short uint;

namespace PROCESS_ACTIONS {
	template<class typeNameA>
	class ProcActions {
	public:
		DWORD getProcessIDbyName(LPCSTR processName);
		void* _ChageProtection_(LPCSTR windowProcessName, const int DESIRED_PROTECTION, int nBytes);
		void writeCall(DWORD* callLoc, DWORD* newFuncLoc, HANDLE &handleToProcess);
		MEMORY_BASIC_INFORMATION* _QueryProcessInfo_(LPCSTR windowProcessName, DWORD addressToQuery, HANDLE exHndl = 0);
		bool _WriteProcMemory_(DWORD AddressToWriteTo, typeNameA InsertionValue, LPCSTR WindowName);
		int _ReadProcMemory_(DWORD AddressToReadFrom, LPCSTR windowProcessName);
	private:
	};

	//Modify a call to call another function
	template<class typeNameA>
	void ProcActions<typename typeNameA>::writeCall(DWORD* baseAddressOfCall, DWORD* addressOfNewFunction, HANDLE& handleToProcess) {
		
		//Write location begins at 0xE8 + 1 Byte
		DWORD writeLocation = (DWORD)(long)*baseAddressOfCall + 1;
		cout << LPVOID(writeLocation) << endl;

		//Each call: 0xE3 0x00 0x00 0x00 0x00 - is 5 bytes
		DWORD* offset = (DWORD*)(((unsigned)*addressOfNewFunction) - ((unsigned)*baseAddressOfCall + 5));
		cout << offset << endl;

		if (WriteProcessMemory(handleToProcess, (LPVOID)writeLocation, &offset , 4, NULL) == false) {
			std::cerr << "could not write to proc" << GetLastErrorAsString();
			return;
		}
		else {
			printf("Wrote to address: %p\n", writeLocation);
			return;
		}
	}

	//Query process information
	template<class typeNameA>
	MEMORY_BASIC_INFORMATION* ProcActions<typename typeNameA>::_QueryProcessInfo_(LPCSTR windowProcesName, DWORD addressToQuery, HANDLE exHndl) {
		DWORD ProcessID;
		MEMORY_BASIC_INFORMATION mbi;
		std::string updatedProtection;

		//Get a handle to the window
		HWND handleToWindow = FindWindowA(NULL, windowProcesName);
		if (handleToWindow == NULL) {
			cerr << "Error --- Could not find window: " << GetLastErrorAsString() << endl;
			return false;
		}
		else {
			//Get thread PID and Handle to process
			int threadID = GetWindowThreadProcessId(handleToWindow, &ProcessID);
			HANDLE toProc = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, ProcessID);

			//Query the process and format output
			VirtualQueryEx(toProc, (void*)addressToQuery, &mbi, sizeof(mbi));

			printf("====== PROCESS INFO =====\n");
			printf("Process name: %s\n\n", windowProcesName);
			printf("Base address: %p\n", mbi.BaseAddress);
			printf("Allocation base: %p\n", mbi.AllocationBase);
			printf("Allocation protect: %d\n", mbi.AllocationProtect);
			printf("Region size: %d\n", mbi.RegionSize);
			printf("State: 0x%08x\n", mbi.State);
			printf("Protect: 0x%08x\n", mbi.Protect);
			printf("Type: 0x%08x\n", mbi.Type);

			//Get last error and close handle
			std::cout << GetLastErrorAsString();
			CloseHandle(toProc);
			return &mbi;
		}
	}


	//Read from memory
	template<class typeNameA>
	int ProcActions<typename typeNameA>::_ReadProcMemory_(DWORD addressToReadFrom, LPCSTR windowProcessName) {

		//Get a handle to the window
		HWND handleToWindow = FindWindowA(NULL, windowProcessName);
		if (handleToWindow == NULL) {
			cerr << "Error --- Could not find window: " << GetLastErrorAsString() << endl;
			return false;
		}
		else {
			DWORD processID;
			int readBuffer = 0;

			//get the thread PID assoicated with the window
			DWORD threadProcessID = GetWindowThreadProcessId(handleToWindow, &processID);

			//get a handle to the process with all access rights
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
					return 0;
				}
			}
			return readBuffer;
		}
	}

	template<class typeNameA>
	void* ProcActions<typename typeNameA>::_ChageProtection_(LPCSTR windowProcessName, const int DESIRED_PROTECTION, int nBytes) {
		DWORD procID;
		DWORD oldProtect;
		MEMORY_BASIC_INFORMATION mbi;

		//Find the window and return the hadle to the window
		HWND handleToWindow = FindWindowA(NULL, windowProcessName);
		if (handleToWindow == NULL) { cerr << "Error --- Could not find window: " << GetLastErrorAsString() << endl; exit(1); }

		//Get the thread process ID of the window and open a handle to the process
		DWORD threadProcessID = GetWindowThreadProcessId(handleToWindow, &procID);
		HANDLE hndlToOpenProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, procID);

		//Allocate memory in the remote processes address space (1 page) and make query to the updated protection 
		LPVOID lpbase = VirtualAllocEx(hndlToOpenProc, NULL, 4096, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
		VirtualQueryEx(hndlToOpenProc, lpbase, &mbi, sizeof(mbi));
		printf("Old protection: 0x%08x\n", mbi.Protect);

		//change protection on pages in processes virtual address space
		if (VirtualProtectEx(hndlToOpenProc, lpbase, nBytes, DESIRED_PROTECTION, &oldProtect) != NULL) {
			VirtualQueryEx(hndlToOpenProc, lpbase, &mbi, sizeof(mbi));
			printf("Updated Protection at : 0x%08x : Updated protection : 0x%08x", lpbase, mbi.Protect);
			return lpbase;
		}
		else {
			cerr << "Could not update protection!" << endl;
			CloseHandle(hndlToOpenProc);
			exit(EXIT_FAILURE);
		}
	}


	template<class typeNameA>
	bool ProcActions<typename typeNameA>::_WriteProcMemory_(DWORD AddressToWriteTo, typeNameA InsertionValue, LPCSTR windowProcessName) {
		//Get a handle to the window
		HWND handleToWindow = FindWindowA(NULL, windowProcessName);
		if (handleToWindow == NULL) {
			cerr << "Error --- Could not find window: " << GetLastErrorAsString() << endl;
			return false;
		}
		else {
			DWORD processID;

			//get the threadID of the process
			DWORD threadProcessID = GetWindowThreadProcessId(handleToWindow, &processID);

			//Open handle to the process
			HANDLE hndlToOpenProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processID);
			if (hndlToOpenProc == NULL) {
				cerr << "Error -- Could not Open this process" << GetLastErrorAsString() << endl;
				return false;
			}
			else {
				bool write = WriteProcessMemory(hndlToOpenProc, (LPVOID)AddressToWriteTo, &InsertionValue, sizeof(InsertionValue), 0);
				if (write == true) {
					printf("Successfully wrote Value: %d at Location: %p\n", InsertionValue, (void*)AddressToWriteTo);
					return true;
				}
				else {
					cerr << "Failed to write memory." << GetLastErrorAsString() << endl;
					return false;
				}
			}
		}
		return true;
	}

	//get the PID by name
	template<class typeNameA>
	DWORD ProcActions<typename typeNameA>::getProcessIDbyName(LPCSTR processName) {
		bool fFlag = false;
		DWORD processID;
		PROCESSENTRY32W PE;
		std::string retStr;

		//Take a snapshot of all processes in system
		HANDLE snapShot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
		if (snapShot == INVALID_HANDLE_VALUE) {
			cerr << "Error: " << GetLastErrorAsString();
		}

		//Set szie of process entry;
		PE.dwSize = sizeof(PROCESSENTRY32);

		//Retrieves information about first process captured by snapshot(CreateToolhelp32SnapShot)
		if (Process32First(snapShot, &PE)) {
			//get the size of the string including the null character (-1 FLAG)
			int sizeRequired = WideCharToMultiByte(CP_UTF8, 0, PE.szExeFile, -1, NULL, 0, NULL, NULL);
			while (Process32Next(snapShot, &PE) != 0) {
				std::vector<char> utf8String(sizeRequired);

				//Convert UTF-16 to UFT-8
				int bytesConvert = WideCharToMultiByte(CP_UTF8, 0, PE.szExeFile, -1, &utf8String[0], utf8String.size(), NULL, NULL);
				retStr = &utf8String[0];

				//Convert a std::string to const char*
				const char* c = retStr.c_str();

				//Compare each entity in the process list
				if (strcmp(processName, c) == 0) {
					fFlag = true;
					//cout << "Process name: " << c << '\n';
					//cout << "Process ID: " << PE.th32ProcessID << '\n';
					return PE.th32ProcessID;
				}
			}
			if (fFlag == false) {
				cerr << "Error: could not find process - \n";
				return 0;
			}
		}
		//close handle and return the pid
		if (snapShot != 0) {
			CloseHandle(snapShot);
			return PE.th32ProcessID;
		}
	}
}
