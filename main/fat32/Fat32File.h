#pragma once

#include "../def.h"

#include "FAT32Utils.h"
#include "FAT32.h"
#include "fileutils.h"


#ifdef DLL_EXPORT
//��2Ϊ��ʶ���ļ���ģʽ,00������ļ���01д�ļ���10дĿ¼
int openFile(const char* fn, int mode, LPFAT32DIRECTORY dir,int *dirinsec);

int openFileWrite(char * curpath, char * leastpath, int mode, int clusternumber,LPFAT32DIRECTORY lpdirectory, 
	LPFAT32DIRECTORY outdir,int *dirinsec);

int readFat32File(char * filename,char * buf);

int writeFat32File(char * filename, char * buf,int size,int createmode,int wirtemode);
#else
extern "C" __declspec(dllimport) int openFile(const char* fn, int mode, LPFAT32DIRECTORY dir, int *dirinsec);

extern "C" __declspec(dllimport) int openFileWrite(char * curpath, char * leastpath, int mode, int clusternumber, LPFAT32DIRECTORY lpdirectory,
	LPFAT32DIRECTORY outdir, int *dirinsec);

extern "C" __declspec(dllimport) int readFat32File(char * filename, char * buf);

extern "C" __declspec(dllimport) int writeFat32File(char * filename, char * buf, int size, int createmode, int wirtemode);
#endif


