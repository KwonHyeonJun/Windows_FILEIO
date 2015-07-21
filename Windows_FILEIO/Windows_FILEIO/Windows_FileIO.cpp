#include <stdio.h>
#include <Windows.h>
#include <stdint.h>
#include <crtdbg.h>
#include <strsafe.h>

bool read_file_using_memory_map()
{
	// current directory 를 구다.
	wchar_t *buf=NULL;
	uint32_t buflen = 0;
	buflen = GetCurrentDirectoryW(buflen, buf);
	char Wr_text[]="'노용환멘토님 만세~ I can give my word.'";
	wchar_t temp_text[256] = {0,};
	char utf_text[256] = {0,};
	char readfile[256]={0,};
	DWORD Wr_result;
	DWORD Rd_result;
	LARGE_INTEGER curPtr;


	if (0 == buflen)
	{
		printf("err, GetCurrentDirectoryW() failed. gle = 0x%08x", GetLastError());
		return false;
	}

	buf = (PWSTR) malloc(sizeof(WCHAR) * buflen);

	if (0 == GetCurrentDirectoryW(buflen, buf))
	{
		printf("err, GetCurrentDirectoryW() failed. gle = 0x%08x", GetLastError());
		free(buf);
		return false;
	}

	// current dir \\ test.txt 파일명 생성
	wchar_t file_name[260];
	if (!SUCCEEDED(StringCbPrintfW(
		file_name, 
		sizeof(file_name), 
		L"%ws\\bob.txt", 
		buf)))
	{  
		printf("err, can not create file name");
		free(buf);
		return false;
	}




	/*if (true == is_file_existsW(file_name))
	{
	::DeleteFileW(file_name);
	}*/




	HANDLE file_handle = CreateFileW(
		(LPCWSTR)file_name, 
		GENERIC_READ | GENERIC_WRITE,
		NULL,
		NULL,
		CREATE_ALWAYS, 
		FILE_ATTRIBUTE_NORMAL, 
		NULL
		);
	if (INVALID_HANDLE_VALUE == file_handle)
	{
		printf("err, CreateFile(%ws) failed, gle = %u", file_name, GetLastError());
		return false;
	}



	MultiByteToWideChar( CP_ACP,  0, Wr_text, -1, temp_text, 256);
	WideCharToMultiByte( CP_UTF8, 0, temp_text, -1, utf_text, 256, NULL, NULL );

	// File Write
	WriteFile(
		file_handle,
		utf_text,
		strlen(utf_text),
		&Wr_result,
		NULL);

	printf("Write Complete\nReading Start");


	// check file size 
	// 
	LARGE_INTEGER fileSize;
	if (TRUE != GetFileSizeEx(file_handle, &fileSize))
	{
		printf("err, GetFileSizeEx(%ws) failed, gle = %u", file_name, GetLastError());
		CloseHandle(file_handle);
		return false;
	}

	// [ WARN ]
	// 
	// 4Gb 이상의 파일의 경우 MapViewOfFile()에서 오류가 나거나 
	// 파일 포인터 이동이 문제가 됨
	// FilIoHelperClass 모듈을 이용해야 함
	// 
	_ASSERTE(fileSize.HighPart == 0);
	if (fileSize.HighPart > 0) 
	{
		printf("file size = %I64d (over 4GB) can not handle. use FileIoHelperClass",
			fileSize.QuadPart);
		CloseHandle(file_handle);
		return false;
	}

	DWORD file_size = (DWORD)fileSize.QuadPart;
	HANDLE file_map = CreateFileMapping(
		file_handle, 
		NULL, 
		PAGE_READONLY, 
		0, 
		0, 
		NULL
		);
	if (NULL == file_map)
	{
		printf("err, CreateFileMapping(%ws) failed, gle = %u", file_name, GetLastError());
		CloseHandle(file_handle);
		return false;
	}

	PCHAR file_view = (PCHAR) MapViewOfFile(
		file_map, 
		FILE_MAP_READ,
		0, 
		0, 
		0
		);
	if(file_view == NULL)
	{
		printf("err, MapViewOfFile(%ws) failed, gle = %u", file_name, GetLastError());

		CloseHandle(file_map);
		CloseHandle(file_handle);
		return false;
	}   

	// do some io
	
	MultiByteToWideChar( CP_UTF8, 0, file_view, -1, temp_text, 256 );
    WideCharToMultiByte( CP_ACP,  0, temp_text, -1, readfile, 256, NULL, NULL);
	printf("MMIO 방식 : %s\n", readfile);
	
	curPtr.QuadPart = 0;
	SetFilePointerEx(file_handle,curPtr, NULL, FILE_BEGIN);
	
	ReadFile(file_handle, readfile, strlen(utf_text), &Rd_result, NULL);

	MultiByteToWideChar( CP_UTF8, 0, readfile, -1, temp_text, 256 );
    WideCharToMultiByte( CP_ACP,  0, temp_text, -1, readfile,    256, NULL, NULL);

	printf("ReadFile API 방식 : %s\n", readfile);


	wchar_t Cp_file_name[260];
	if (!SUCCEEDED(StringCbPrintfW(
		Cp_file_name, 
		sizeof(Cp_file_name), 
		L"%ws\\bob2.txt", 
		buf)))
	{  
		printf("err, can not create file name");
		free(buf);
		return false;
	}

	CloseHandle(file_handle);
	CopyFile(file_name, Cp_file_name, false);

	HANDLE Cp_file_handle = CreateFileW(
		(LPCWSTR)Cp_file_name, 
		GENERIC_READ,
		NULL,
		NULL,
		OPEN_EXISTING, 
		FILE_ATTRIBUTE_NORMAL, 
		NULL
		);
	if (INVALID_HANDLE_VALUE == Cp_file_handle)
	{
		printf("err, CreateFile(%ws) failed, gle = %u", Cp_file_name, GetLastError());
		return false;
	}

	// close all
	UnmapViewOfFile(file_view);
	CloseHandle(file_map);
	CloseHandle(file_handle);
	CloseHandle(Cp_file_handle);

	free(buf); buf = NULL;

	printf("생성된 파일 모두 삭제!!!\n");
	DeleteFileW(file_name);
	DeleteFileW(Cp_file_name);
	printf("CLEAR!!!! 과제 종료!!\n");
	return true;
}
int main(){
	read_file_using_memory_map();
	return 0;
}