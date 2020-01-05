#include <iostream>
#include <Windows.h>
#include "getleasstr.h"

using namespace std;
typedef unsigned short uint;

//Simple utility functions for reading and writing specified values within desired process.
//May be expanded upon in a commandline utility.


class ReadWrite {
public:
	bool WriteProcMemory(DWORD AddressToWriteTo, int InsertionValue, LPCSTR WindowName);
	bool ReadProcMemory(DWORD AddressToReadFrom, LPCSTR WindowName);
private:
};

bool ReadWrite::ReadProcMemory(DWORD addressToReadFrom, LPCSTR WindowName) {
	HWND handleToWindow = FindWindowA(NULL, WindowName);
	if (handleToWindow == NULL) {
		cerr << "Error --- Could not find window: " << GetLastErrorAsString() << endl;
		return false;
	}
	else {
		DWORD processID;
		int readBuffer;
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
	}
	return true;
}

bool ReadWrite::WriteProcMemory(DWORD AddressToWriteTo, int InsertionValue, LPCSTR WindowName) {
	HWND handleToWindow = FindWindowA(NULL, WindowName);
		if (handleToWindow == NULL) {
			cerr << "Error --- Could not find window: " << GetLastErrorAsString() << endl;
			return false;
		} else {
			DWORD processID;
			DWORD threadProcessID = GetWindowThreadProcessId(handleToWindow, &processID);
			HANDLE hndlToOpenProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processID);
			if (hndlToOpenProc == NULL) {
				cerr << "Error -- Could not Open this process" << GetLastErrorAsString() << endl;
			}
			else {
				bool write = WriteProcessMemory(hndlToOpenProc, (LPVOID)AddressToWriteTo, &InsertionValue, sizeof(InsertionValue), 0);
				if (write == true) {
					printf("Successfully wrote Value: %d at Location: %p\n", InsertionValue, (void*)AddressToWriteTo);
				} else {
					cerr << "Failed to write memory." << GetLastErrorAsString() << endl;
					return false;
				}
			}
		}
		return true;
}
int main()
{
	return 0;
}
