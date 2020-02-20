#include <iostream>
#include <Windows.h>
#include <string>
#include "process_actions.h"
#include <shellapi.h>
#include "Shlwapi.h"

#pragma comment(lib, "Shlwapi.lib")
#define _CRT_SECURE_NO_WARNINGS

//Load a DLL File into a specified process. 

int main(int argc, char* argv[])
{
	std::string pname, dllpath;
	PROCESS_ACTIONS::ProcActions<DWORD> obj;

	printf("Process name: "), std::cin >> pname, cin.ignore();
	printf("DLL Path: "), std::getline(cin, dllpath);

	const char* p = pname.c_str(); const char* d = dllpath.c_str();
	if (obj.getProcessIDbyName(p) == 0) {
		cerr << "[ERROR] The process specified was not found." << std::endl;
		return -1;
	}
	//Check if the path to the file exists
	if (!PathFileExistsA(d)) {
		cerr << "\n[ERROR] Invalid file path specified." << endl;
		return -1;
	}
	else {
		//open a handle to the target process
		HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, obj.getProcessIDbyName(p));
		if (hProcess == NULL) {
			cerr << "Could not open process" << GetLastErrorAsString();
			return -1;
		}
		else {
			//allocate memory in process, save base address
			LPVOID dllP = VirtualAllocEx(hProcess, 0, strlen(d) + 1, MEM_COMMIT, PAGE_READWRITE);
			if (dllP == NULL) {
				cerr << "Failed to allocate memory in process. " << GetLastErrorAsString() << endl;
				return -1;
			}
			else {
				//write the path to the region allocated
				WriteProcessMemory(hProcess, dllP, (LPVOID)d, strlen(d) + 1, 0);
				//create thread that loads dll
				HANDLE handleLthd = CreateRemoteThread(hProcess, 0, 0, (LPTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandleA("kernel32.dll"), "LoadLibraryA"), dllP, 0, 0);
				DWORD retVal = WaitForSingleObject(handleLthd, INFINITE);
				if (handleLthd == NULL | retVal == WAIT_FAILED ) {
					cerr << "Failed to create remote thread in process." << endl;
					return -1;
				}
				printf("Path allocation address:  0x%08x\n", dllP);
				VirtualFreeEx(hProcess, dllP, strlen(d) + 1, MEM_RELEASE);
				return 0;
			}
		}
	}
}

