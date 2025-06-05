
#include <Windows.h>
#include <winerror.h>

#include <stdio.h>

#include <iostream>
using namespace std;

void wmain()
{
	wprintf(L"wmain()\n");
	//std::unique_ptr<BYTE[]> ptrcFni(new BYTE[_MAX_PATH * sizeof(TCHAR) + sizeof(FILE_FULL_DIR_INFO)]);
	//FILE_FULL_DIR_INFO* pFni = reinterpret_cast<FILE_FULL_DIR_INFO*>(ptrcFni.get());
	BYTE byBuff[1024];
	SecureZeroMemory(byBuff, 1024);
	FILE_NAME_INFO* pFni = reinterpret_cast<FILE_NAME_INFO*>(byBuff);
	HANDLE hFile = CreateFile(L"D:\\Test\\Test\\CPPMIPSDKTestFiles\\Protected\\food.ptxt",
		GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	//BOOL b = GetFileInformationByHandleEx(hFile, FileFullDirectoryInfo, pFni, sizeof(FILE_FULL_DIR_INFO) + (_MAX_PATH * sizeof(TCHAR)));
	BOOL b = GetFileInformationByHandleEx(hFile, FileNameInfo, pFni, 1024);
	CloseHandle(hFile);

}
