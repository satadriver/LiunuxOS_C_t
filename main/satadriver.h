#pragma once
#include "def.h"

//#define LIUNUX_DEBUG_FLAG

#define HD_READ_COMMAND			0X20
#define HD_WRITE_COMMAND		0X30

#define HD_LBA48READ_COMMAND	0X24
#define HD_LBA48WRITE_COMMAND	0X34

//multiple read command can read more than one sector with instruction rep insw/insb/insd
#define HD_MUTIPLEREAD_COMMAND		0X29
#define HD_MUTIPLEWRITE_COMMAND		0X39

#define DEVICE_MASTER			0XE0
#define DEVICE_SLAVE			0XF0

#define ONCE_READ_LIMIT			128

#pragma pack(1)

typedef struct
{
	unsigned char len;
	unsigned char reserved;
	unsigned short seccnt;
	unsigned int segoff;
	unsigned int secnolow;
	unsigned int secnohigh;
}INT13PAT, *LPINT13PAT;


typedef struct
{
	unsigned char bwork;	
	unsigned char intno;
	unsigned int reax;		//2
	unsigned int recx;		//6
	unsigned int redx;		//a
	unsigned int rebx;		//e
	unsigned int resi;		//12
	unsigned int redi;		//16
	unsigned short res;		//1a
	unsigned short rds;		//1c
	unsigned int result;	//1e
}V86VMIPARAMS, *LPV86VMIPARAMS;

#pragma pack()


int readSectorLBA48(unsigned int secnoLow, unsigned int secnoHigh, unsigned char seccnt, char * buf,int device);

int writeSectorLBA48(unsigned int secnoLow, unsigned int secnoHigh, unsigned char seccnt, char * buf, int device);


int readSectorLBA24(unsigned int secno, unsigned char seccnt, char * buf, int device);

int writeSectorLBA24(unsigned int secno, unsigned char seccnt, char * buf, int device);


int readSectorLBA24Mimo(unsigned int secno, unsigned char seccnt, char * buf, int device);

int readSectorLBA48Mimo(unsigned int secnoLow, unsigned int secnoHigh, unsigned char seccnt, char * buf, int device);

int waitFree(WORD port);
int waitComplete(WORD port);

int testHdPort(unsigned short port);

int testHdPortMimo(unsigned short port);

void __initStatusPort(unsigned char master_slave);

int getHdPort();

int readPortSector(unsigned int secno, DWORD secnohigh, unsigned int seccnt, char * buf);
int writePortSector(unsigned int secno, DWORD secnohigh, unsigned int seccnt, char * buf);

// int vm86ReadBlock(unsigned int secno, DWORD secnohigh, unsigned short seccnt, char * buf,int disk);
// int vm86WriteBlock(unsigned int secno, DWORD secnohigh, unsigned short seccnt, char * buf,int disk);
int vm86ReadSector(unsigned int secno, DWORD secnohigh, unsigned int seccnt, char * buf);
int vm86WriteSector(unsigned int secno, DWORD secnohigh, unsigned int seccnt, char * buf);

#ifdef DLL_EXPORT
extern "C"  __declspec(dllexport)  int vm86ReadBlock(unsigned int secno, DWORD secnohigh, unsigned short seccnt, char * buf, int disk,int sectorsize);
extern "C"  __declspec(dllexport)  int vm86WriteBlock(unsigned int secno, DWORD secnohigh, unsigned short seccnt, char * buf, int disk, int sectorsize);
extern "C"  __declspec(dllexport)  int(__cdecl * readSector)(unsigned int secnolow,DWORD secnohigh, unsigned int seccnt, char * buf);
extern "C"  __declspec(dllexport)  int(__cdecl * writeSector)(unsigned int secnolow,DWORD secnohigh, unsigned int seccnt, char * buf);
#else
extern "C"  __declspec(dllimport)  int vm86ReadBlock(unsigned int secno, DWORD secnohigh, unsigned short seccnt, char * buf, int disk, int sectorsize);
extern "C"  __declspec(dllimport)  int vm86WriteBlock(unsigned int secno, DWORD secnohigh, unsigned short seccnt, char * buf, int disk, int sectorsize);
extern "C"  __declspec(dllimport)  int(__cdecl * readSector)(unsigned int secnolow, DWORD secnohigh, unsigned int seccnt, char * buf);
extern "C"  __declspec(dllimport)  int(__cdecl * writeSector)(unsigned int secnolow, DWORD secnohigh, unsigned int seccnt, char * buf);
#endif

//1f7 general output is 50h
//bit0 ֮ǰ����������� if is 1
//bit1 ����ÿתһ�ܵ���1
//bit2 ecc check correctly to read sector data if is 1
//bit3 work complete if is 1
//bit4 ��ͷͣ���ڴŵ��� if is 1
//bit5	write error if 1
//bit6 ready to work if is 1
//bit7 be busy if is 1



//1f6
//bit7:1
//bit6:1 is lba,0 is chs
//bit5:1
//bit4:0 is master,1 is slave
//bit0-bit3:if bit6 is 1,the sector no of 24-27,if bit6 is 0,header number

//���棨cylinder������ͷ��head��,sector
//LBA(�߼�������)=��ͷ�� �� ÿ�ŵ������� �� ��ǰ��������� + ÿ�ŵ������� �� ��ǰ���ڴ�ͷ�� + ��ǰ���������� �C 1
//CHS=0/0/1������ݹ�ʽLBA=255 �� 63 �� 0 + 63 �� 0 + 1 �C 1= 0
//CHSģʽ֧�ֵ�Ӳ�� ��8bit���洢��ͷ��ַ����10bit���洢�����ַ����6bit���洢������ַ��
//��һ����������512Byte������ʹ��CHSѰַһ��Ӳ���������Ϊ256 * 1024 * 63 * 512B = 8064 MB



 


