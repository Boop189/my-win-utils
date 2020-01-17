#pragma once
#include <iostream>
#include <Windows.h>
#include <TlHelp32.h>
#include <winternl.h>

#include "getlast_error.h"

#define _CRT_SECURE_NO_WARNINGS

using namespace std;
typedef unsigned short uint;

//A set of functions for working with processes and reverse engineering
//Todo: Add virtual protect function to modify pages

namespace PROCESS_ACTIONS {
	template<class typeNameA>
	class ProcActions {
	public:
		DWORD getProcessIDbyName(LPCSTR procName);
		MEMORY_BASIC_INFORMATION* _QueryProcessInfo_(LPCSTR windowProcessName, DWORD addressToQuery = NULL);
		bool _WriteProcMemory_(DWORD AddressToWriteTo, int InsertionValue, LPCSTR WindowName);
		int _ReadProcMemory_(DWORD AddressToReadFrom, LPCSTR WindowName);
	private:
	};

	//Query process information
	template<class typeNameA>
	MEMORY_BASIC_INFORMATION* ProcActions<typename typeNameA>::_QueryProcessInfo_(LPCSTR windowProcesName, DWORD addressToQuery) {
		DWORD ProcessID;
		MEMORY_BASIC_INFORMATION mbi;

		HWND handleToWindow = FindWindowA(NULL, windowProcesName);
		if (handleToWindow == NULL) {
			cerr << "Error --- Could not find window: " << GetLastErrorAsString() << endl;
			return false;
		}
		else {
			int threadID = GetWindowThreadProcessId(handleToWindow, &ProcessID);
			HANDLE toProc = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, ProcessID);

			//Query process using virtual query
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

			std::cout << GetLastErrorAsString();
			CloseHandle(toProc);
			return &mbi;
		}
	}

	//Read values from memory
	template<class typeNameA>
	int ProcActions<typename typeNameA>::_ReadProcMemory_(DWORD addressToReadFrom, LPCSTR WindowName) {
		HWND handleToWindow = FindWindowA(NULL, WindowName);
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

	//Write values to memory
	template<class typeNameA>
	bool ProcActions<typename typeNameA>::_WriteProcMemory_(DWORD AddressToWriteTo, int InsertionValue, LPCSTR WindowName) {
		HWND handleToWindow = FindWindowA(NULL, WindowName);
		if (handleToWindow == NULL) {
			cerr << "Error --- Could not find window: " << GetLastErrorAsString() << endl;
			return false;
		}
		else {
			DWORD processID;
			DWORD threadProcessID = GetWindowThreadProcessId(handleToWindow, &processID);
			HANDLE hndlToOpenProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processID);
			if (hndlToOpenProc == NULL) {
				cerr << "Error -- Could not Open this process" << GetLastErrorAsString() << endl;
			}
			else {
				bool write = WriteProcessMemory(hndlToOpenProc, (LPVOID)AddressToWriteTo, &InsertionValue, sizeof(InsertionValue), 0);
				if (write == true) {
					//printf("Successfully wrote Value: %d at Location: %p\n", InsertionValue, (void*)AddressToWriteTo);
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
	DWORD ProcActions<typename typeNameA>::getProcessIDbyName(LPCSTR processName) {
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

				if (strcmp(processName, c) == 0) {
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
		


