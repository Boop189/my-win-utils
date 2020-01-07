#pragma once
#include <iostream>
#include <Windows.h>
#include "getlast_error.h"

using namespace std;
typedef unsigned short uint;

//A set of functions for working with processes - TO BE expaneded upon.

namespace PROCESS_ACTIONS {
	class ReadWrite {
	public:
		MEMORY_BASIC_INFORMATION* _QueryProcessInfo_(LPCSTR windowProcessName, DWORD addressToQuery);
		bool _WriteProcMemory_(DWORD AddressToWriteTo, int InsertionValue, LPCSTR WindowName);
		int _ReadProcMemory_(DWORD AddressToReadFrom, LPCSTR WindowName);
	private:
	};

	MEMORY_BASIC_INFORMATION* ReadWrite::_QueryProcessInfo_(LPCSTR windowProcesName, DWORD addressToQuery) {
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
	
	//Simply write values into locations in memory
	int ReadWrite::_ReadProcMemory_(DWORD addressToReadFrom, LPCSTR WindowName) {
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

	//Read values from locations in memory
	bool ReadWrite::_WriteProcMemory_(DWORD AddressToWriteTo, int InsertionValue, LPCSTR WindowName) {
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
}
