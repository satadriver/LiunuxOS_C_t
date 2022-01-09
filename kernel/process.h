#pragma once
#include "def.h"
#include "descriptor.h"
#include "page.h"
#include "slab.h"
#include "video.h"

#pragma pack(push,1)


typedef struct
{
	unsigned char	opcode;
	DWORD			offset;
	unsigned short	selector;
}JUMP32, *LPJUMP32;

typedef struct
{
	DWORD eip;
	DWORD cs;
	DWORD eflags;
}IRETADDR_RING0;

typedef struct
{
	IRETADDR_RING0 iretaddr0;
	DWORD esp3;
	DWORD ss3;
}IRETADDR_RING3;

typedef struct
{
	IRETADDR_RING3 iretaddr3;
	DWORD es;
	DWORD ds;
	DWORD fs;
	DWORD gs;
}IRETADDR_VM86;

//esp0 ss0 cr3 ldt iomap intmap are all static
typedef struct __TSS{
	DWORD link; // 保存前一个 TSS 段选择子，使用 call 指令切换寄存器的时候由CPU填写。

	DWORD esp0; //4
	DWORD ss0;  //8
	DWORD esp1; //12
	DWORD ss1;  //16
	DWORD esp2; //20
	DWORD ss2;  //24
	
	//static
	DWORD cr3;	//28

	DWORD eip;	//32
	DWORD eflags;
	DWORD eax;	//40
	DWORD ecx;
	DWORD edx;	//48
	DWORD ebx;
	DWORD esp;	//56
	DWORD ebp;
	DWORD esi;	//64
	DWORD edi;
	DWORD es;	//72
	DWORD cs;
	DWORD ss;	//80
	DWORD ds;
	DWORD fs;	//88
	DWORD gs;

	//static
	DWORD ldt;	//96
	unsigned short	trap;				//100
	unsigned short	iomapOffset;		//102
	unsigned char	intMap[32];
	unsigned char	iomap[8192];
	unsigned char	iomapEnd;			//104 + 32 + 8192

	unsigned char	fpu;
	unsigned char	unused[2];
} TSS,*LPTSS;

#pragma pack(pop)

#pragma pack(1)
typedef struct  
{
	TSS tss;

	//IRETADDR_VM86 iretaddr_v86;

	DWORD pid;

	DWORD tid;

	DWORD level;

	//物理地址而不是线性地址
	DWORD moduleaddr;
	DWORD moduleLinearAddr;

	DWORD espbase;

	DWORD va;

	//内存分配的虚拟地址偏移
	DWORD vasize;

	DWORD errorno;

	DWORD counter;

	DWORD status;

	LPWINDOWCLASS window;

	char filename[256];

	char funcname[64];

}PROCESS_INFO,*LPPROCESS_INFO;

typedef struct
{
	DWORD ss;
	DWORD gs;
	DWORD fs;
	DWORD es;
	DWORD ds;
	DWORD edi;
	DWORD esi;
	DWORD ebp;
	DWORD esp;
	DWORD ebx;
	DWORD edx;
	DWORD ecx;
	DWORD eax;
	IRETADDR_VM86 iretaddr_v86;
}CONTEXT_REGS;
#pragma pack()

#pragma pack(push,1)
typedef struct
{
	DWORD cmd;
	DWORD addr;
	DWORD filesize;
	char filename[256];
}TASKCMDPARAMS, *LPTASKCMDPARAMS;

//ring3线程发生时切换堆栈，中断堆栈中带着中断返回参数，这里的参数是第一次运行时赋值，不是为了中断返回。中断返回的参数在0级堆栈
typedef struct
{
	DWORD terminate;		//ret address
	DWORD terminate2;		//param 1
	DWORD tid;
	char * filename;
	char * funcname;
	LPTASKCMDPARAMS lpcmdparams;
	char szFileName[256];
	char szFuncName[64];

	TASKCMDPARAMS cmdparams;
}TASKPARAMS3, *LPTASKPARAMS3;

//ring0线程发生时不切换堆栈，中断堆栈中带着中断返回参数
typedef struct
{
	IRETADDR_RING0 iretaddr0;

	DWORD terminate;		//ret address
	DWORD terminate2;		//param 1
	DWORD tid;
	char * filename;
	char * funcname;
	LPTASKCMDPARAMS lpcmdparams;
	char szFileName[256];
	char szFuncName[64];

	TASKCMDPARAMS cmdparams;
}TASKPARAMS0, *LPTASKPARAMS0;


typedef struct
{
	DWORD terminate;
	DWORD pid;
	char * filename;
	char * funcname;
	DWORD addr;
	DWORD param;
	char szFileName[256];
	char szFuncName[64];
}TASKDOSPARAMS, *LPTASKDOSPARAMS;

typedef struct {
	int number;
	LPPROCESS_INFO lptss;
}TASKRESULT,*LPTASKRESULT;

#pragma pack(pop)

int __initProcess(LPPROCESS_INFO tss, int num, DWORD filedata, char * filename, char * funcname, DWORD level, DWORD runparam);

void __kFreeProcess(int pid);

#ifdef DLL_EXPORT
extern "C" __declspec(dllexport) void __terminateProcess(int pid, char * filename, char * funcname, DWORD lpparams);
extern "C" __declspec(dllexport) int __kCreateProcess(DWORD addr, int datasize,char * filename, char * funcname, int syslevel, DWORD param);
#else
extern "C" __declspec(dllimport) void __terminateProcess(int pid, char * filename, char * funcname, DWORD lpparams);
extern "C" __declspec(dllimport) int __kCreateProcess(DWORD addr, int datasize, char * filename, char * funcname, int syslevel, DWORD param);
#endif



