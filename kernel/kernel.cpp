#include "Utils.h"
#include "def.h"
#include "Kernel.h"
#include "video.h"
#include "keyboard.h"
#include "mouse.h"
#include "process.h"
#include "task.h"
#include "Pe.h"
#include "satadriver.h"
#include "sectorReader.h"
#include "fat32/FAT32.h"
#include "fat32/fat32file.h" 
#include "file.h"
#include "NTFS/ntfs.h"
#include "NTFS/ntfsFile.h"
#include "pci.h"
#include "speaker.h"
#include "system.h"
#include "screenUtils.h"
#include "cmosAlarm.h"
#include "rs232.h"
#include "floppy.h"
#include "slab.h"
#include "page.h"
#include "dosProcess.h"
#include "gdi.h"
#include "coprocessor.h"
#include "Thread.h"
#include "debugger.h"
#include "descriptor.h"

//#pragma comment(linker, "/ENTRY:DllMain")
//#pragma comment(linker, "/align:512")
//#pragma comment(linker, "/merge:.data=.text")
//#pragma comment(linker, "/merge:.rdata=.text")

//problem:
// getCpuInfo and getCpuType in C language is ok but in assembly language exception,why?
//process没有真正退出？
//createthread退出错误？

LPSEGDESCRIPTOR glpLdt = 0;
LPSEGDESCRIPTOR glpGdt = 0;
LPSYSDESCRIPTOR glpIdt = 0;
DWORD gV86VMIEntry = 0;
DWORD gV86VMLeave = 0;
DWORD gKernel16;
DWORD gKernel32;
DWORD gKernelData;
DWORD gAsmTsses;

void getGdtIdt() {
	DESCRIPTOR_REG gdt;
	DESCRIPTOR_REG idt;
	__asm {
		lea eax,gdt
		sgdt [eax]

		lea eax,idt
		sidt [eax]
	}

	glpGdt = (LPSEGDESCRIPTOR)gdt.addr;

	glpIdt = (LPSYSDESCRIPTOR)idt.addr;

	int gdtcnt = (gdt.size + 1) >> 3;
	for (int i = 1; i < gdtcnt; i++)
	{
		if (glpGdt[i].attr == 0xe2 || glpGdt[i].attr == 0x82)
		{
			glpLdt = &glpGdt[i];
			initLdt(glpLdt);
			break;
		}
	}
}


//c++函数的导出函数对应函数声明的顺序，而不是函数体，函数体的参数一一对应于声明中的顺序
int __kernelEntry(LPVESAINFORMATION vesa, DWORD fontbase,DWORD v86Proc,DWORD v86Leave ,DWORD kerneldata,DWORD kernel16,DWORD kernel32,DWORD lpasmTsses) {

	int ret = 0;
	char szout[1024];

	gV86VMIEntry = v86Proc;
	gV86VMLeave = v86Leave;
	gKernelData = kerneldata;
	gKernel16 = kernel16;
	gKernel32 = kernel32;
	gAsmTsses = lpasmTsses;

	//must be first to prepare for showing
	__getVideoParams(vesa, fontbase);

	getGdtIdt();

	initMemory();

	initPage();

	__initTask();

	initDll();


	initRS232Com1();
	initRS232Com2();

	initEfer();

	initCoprocessor();

	__asm {
		sti
	}
	//__sprintf(szout, "Hello world of Liunux!\r\n");

	__createDosInFileTask(gV86VMIEntry, "V86VMIEntry");

	__kCreateProcess(VSMAINDLL_LOAD_ADDRESS,0x20000, "main.dll", "__kExplorer", 3, 0);


// 	TASKCMDPARAMS cmd;
// 	__memset((char*)&cmd, 0, sizeof(TASKCMDPARAMS));
// 	__kCreateThread((DWORD)__kSpeakerProc, (DWORD)&cmd, "__kSpeakerProc");

	//logFile("__kernelEntry\n");
	//ret = writeFat32File(LOG_FILENAME, szout, __strlen(szout), FILE_WRITE_APPEND);

	//__rmSectorReader(0, 1, szout, 1024);
	
// 	DWORD kernelMain = getAddrFromName(KERNEL_DLL_BASE, "__kKernelMain");
// 	if (kernelMain)
// 	{
// 		TASKCMDPARAMS cmd;
// 		__memset((char*)&cmd, 0, sizeof(TASKCMDPARAMS));
// 		__kCreateThread((unsigned int)kernelMain,(DWORD)&cmd, "__kKernelMain");
// 	}

	//must be after running V86VMIEntry and sti
	initFileSystem();

	initDebugger();

	__kCmosAlarmProc();

// 	floppyInit();
// 	FloppyReadSector(0,1, (unsigned char*)FLOPPY_DMA_BUFFER);

// 	ret = loadLibRunFun("c:\\liunux\\main.dll", "__kMainProcess");
// 	__printf(szout, "__kMainProcess result:%x\n", ret);
// 	__drawGraphChars((unsigned char*)szout, 0);

	while (1)
	{
		if (__findProcessFuncName("__kExplorer") == FALSE)
		{
			__kCreateProcess(VSMAINDLL_LOAD_ADDRESS, 0x20000, "main.dll", "__kExplorer", 3, 0);
		}

		__asm {
			hlt
		}
	}

	return 0;
}



void __kKernelMain(DWORD retaddr,int pid,char * filename,char * funcname,DWORD param) {

	int ret = 0;

 	char szout[1024];
	__printf(szout, "__kKernelMain task pid:%x,filename:%s,function name:%s\n", pid, filename,funcname);
	__drawGraphChars((unsigned char*)szout, 0);

// 	unsigned char sendbuf[1024];
// 	//最大不能超过14字节
// 	__strcpy((char*)sendbuf, "how are you?");
// 	ret = sendCom2Data(sendbuf, __strlen("how are you?"));
// 
// 	unsigned char recvbuf[1024];
// 	int recvlen = getCom2Data(recvbuf);
// 	if (recvlen > 0)
// 	{
// 		*(recvbuf + recvlen) = 0;
// 		
// 		__printf(szout, "recvbuf data:%s\n", recvbuf);
// 		__drawGraphChars((unsigned char*)szout, 0);
// 	}

	//setVideoMode(0x4112);
	
	while (1)
	{
		__sleep(1000);
	}

	while (1)
	{
		__asm {
			hlt
		}
	}
}


void mytest() {
	return;
}

#ifdef _USRDLL
int __stdcall DllMain( HINSTANCE hInstance,  DWORD fdwReason,  LPVOID lpvReserved) {
	return TRUE;
}
#else
int __stdcall WinMain(  HINSTANCE hInstance,  HINSTANCE hPrevInstance,  LPSTR lpCmdLine,  int nShowCmd )
{
	mytest();
	return TRUE;
}
#endif


//if word tmp = 0; __asm{push tmp} will push tmp as word,not dword,esp will sub 2 not 4
//注意二位数组在内存中的排列和结构
//sizeof(IMAGE_NT_HEADERS) == 0xf0


