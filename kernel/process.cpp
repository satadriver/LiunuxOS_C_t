#include "process.h"
#include "def.h"
#include "video.h"
#include "Utils.h"
#include "task.h"
#include "Pe.h"
#include "file.h"
#include "dosProcess.h"
#include "pevirtual.h"
#include "Kernel.h"
#include "memory.h"
#include "slab.h"
#include "page.h"
#include "ListEntry.h"
#include "window.h"




void __kFreeProcess(int pid) {

	freeProcessMemory();

	//do not need to free stack esp 0,because it must be existed in head of 100M
	freeProcessPages();

	//clearcr3();

	//destroyWindows();
}


//1 ��ֹͣ���룬Ȼ���ͷ��ڴ棬˳���ܷ�
//2 ��ֹͣ�����̣߳�Ȼ��ֹͣ���̣߳�˳���ܷ���
void __terminateProcess(int vpid, char * filename, char * funcname, DWORD lpparams) {

	int pid = vpid & 0x7fffffff;

	char szout[1024];

	int retvalue = 0;
	__asm {
		mov retvalue, eax
	}

	LPPROCESS_INFO tss = (LPPROCESS_INFO)CURRENT_TASK_TSS_BASE;
	if (tss->pid != pid)
	{
		__printf(szout, "__terminateProcess pid:%x,filename:%s,funcname:%s,current pid:%x not equal\n", pid, filename, funcname, tss->pid);
		__drawGraphChars((unsigned char*)szout, 0);
	}
	else {
		__printf(szout, "__terminateProcess pid:%x,filename:%s,funcname:%s\n", pid, filename, funcname);
		__drawGraphChars((unsigned char*)szout, 0);
	}

	LPPROCESS_INFO process = 0;
	TASK_LIST_ENTRY * p = 0;
	do 
	{
		p = __findProcessByPid(pid);
		if (p == 0)
		{
			break;
		}
		else if (p->process->pid == p->process->tid && p->process->pid == pid)
		{
			process = p->process;
		}
		else
		{
			TASK_LIST_ENTRY * list = removeTaskList(p->process->tid);
		}
	} while (p);

	__kFreeProcess(tss->pid);

	removeTaskList(process->pid);

	tss->status = TASK_OVER;

	__printf(szout, "__terminateProcess pid:%d,filename:%s,funcname:%s\n", pid, filename, funcname);
	__drawGraphChars((unsigned char*)szout, 0);

	if (vpid & 0x80000000)
	{
		__sleep(0);
	}
	else {
		__sleep(-1);
	}
}



int __initProcess(LPPROCESS_INFO tss, int pid, DWORD filedata, char * filename, char * funcname,DWORD level, DWORD runparam) 
{
	int result = 0;

	char szout[1024];
	

	DWORD imagesize = getSizeOfImage((char*)filedata);
	DWORD alignsize = 0;
	tss->va = USER_SPACE_START;
	tss->vasize = 0;
	DWORD vaddr = tss->va + tss->vasize;
	DWORD pemap = (DWORD)__kProcessMalloc(imagesize,&alignsize,pid, vaddr);
	if (pemap <= 0) {
		tss->status = TASK_OVER;
		__printf(szout, "__initProcess %s %s __kProcessMalloc ERROR\n", funcname, filename);
		__drawGraphChars((unsigned char*)szout, 0);
		return FALSE;
	}
	tss->vasize += alignsize;

	tss->moduleaddr = pemap;
	tss->moduleLinearAddr = USER_SPACE_START;

	tss->tss.trap = 1;

	tss->tss.ldt = ((DWORD)glpLdt - (DWORD)glpGdt);

	// 	__printf(szout, "membase:%x,va size:%x,va:%x\n",pemap,tss->vasize,tss->va);
	// 	__drawGraphChars((unsigned char*)szout, 0);

	mapFile((char*)filedata, (char*)pemap);

	DWORD entry = 0;
	DWORD type = getType((DWORD)pemap);
	if (type & 0x2000)
	{
		//getAddrFromName �Ѿ�����һ��pemap�����Է���ֵ�����ȥ��
		entry = getAddrFromName((DWORD)pemap, funcname);
		if (entry == FALSE) {
			__printf(szout, "__kCreateTask not found export function:%s in:%s\n", funcname, filename);
			__drawGraphChars((unsigned char*)szout, 0);

			__kFree(pemap);
			tss->status = TASK_OVER;
			return FALSE;
		}
		else {
			entry = entry - pemap + USER_SPACE_START;
		}
	}
	else {
		entry = getEntry((char*)pemap) + USER_SPACE_START;
	}
	tss->tss.eip = entry;

	relocTableV((char*)pemap, USER_SPACE_START);

	importTable((DWORD)pemap);

	setImageBaseV((char*)pemap, USER_SPACE_START);

	tss->tss.cr3 = initCr3((DWORD)pemap);
	if (tss->tss.cr3 == 0)
	{
		clearCr3((DWORD*)tss->tss.cr3);
		__kFreeProcess(pid);
		tss->status = TASK_OVER;
		return FALSE;
	}
	
	if (level & 3)
	{
		copyBackupTables(0, USER_SPACE_START, (DWORD*)tss->tss.cr3);
		//copyBackupTables(0, MEMMORY_HEAP_BASE, (DWORD*)tss->tss.cr3);
	}
	else {
		//copyBackupTables(0, MEMMORY_HEAP_BASE, (DWORD*)tss->tss.cr3);
		copyBackupTables(0, USER_SPACE_START, (DWORD*)tss->tss.cr3);
	}
	copyBackupTables(USER_SPACE_END, 0 - USER_SPACE_END, (DWORD*)tss->tss.cr3);

	DWORD syslevel = level & 3;
	tss->level = syslevel;

	DWORD eflags = 0x210;	//if = 1,et = 1
	if (syslevel)
	{
		eflags |= (syslevel<<12);	//iopl = 3
	}
	//eflags |= 0x4000;		//nt == 1
	tss->tss.eflags = eflags;

	tss->tss.eax = 0;
	tss->tss.ecx = 0;
	tss->tss.edx = 0;
	tss->tss.ebx = 0;
	tss->tss.esi = 0;
	tss->tss.edi = 0;

	//����ӳ�䵽cr3
	tss->tss.esp0 = TASKS_STACK0_BASE + (pid + 1) * TASK_STACK0_SIZE - STACK_TOP_DUMMY;
	tss->tss.ss0 = KERNEL_MODE_STACK;

	DWORD espsize = 0;
	vaddr = tss->va + tss->vasize;
	
	if (syslevel == 0)
	{
		tss->tss.ds = KERNEL_MODE_DATA |4;
		tss->tss.es = KERNEL_MODE_DATA | 4;
		tss->tss.fs = KERNEL_MODE_DATA | 4;
		tss->tss.gs = KERNEL_MODE_DATA | 4;
		tss->tss.cs = KERNEL_MODE_CODE | 4;
		tss->tss.ss = KERNEL_MODE_STACK | 4;

		tss->espbase = __kProcessMalloc(KTASK_STACK_SIZE, &espsize,pid, vaddr);
		if (tss->espbase == FALSE)
		{
			__kFreeProcess(tss->pid);
			tss->status = TASK_OVER;
			return FALSE;
		}

		result = phy2linear(vaddr, tss->espbase, KTASK_STACK_SIZE, (DWORD*)tss->tss.cr3);
		if (result == FALSE)
		{
			__kFreeProcess(tss->pid);
			tss->status = TASK_OVER;
			return FALSE;
		}

		LPTASKPARAMS0 params = (LPTASKPARAMS0)(tss->espbase + KTASK_STACK_SIZE  - STACK_TOP_DUMMY - sizeof(TASKPARAMS0));
		params->iretaddr0.eflags = tss->tss.eflags;
		params->iretaddr0.cs = tss->tss.cs;
		params->iretaddr0.eip = tss->tss.eip;

		params->terminate = (DWORD)__terminateProcess;
		params->terminate2 = (DWORD)__terminateProcess;
		params->tid = pid;
		__strcpy(params->szFileName, filename);
		params->filename = params->szFileName;
		__strcpy(params->szFuncName, funcname);
		params->funcname = params->szFuncName;
		params->lpcmdparams = &params->cmdparams;
		if (runparam)
		{
			__memcpy((char*)params->lpcmdparams, (char*)runparam, sizeof(TASKCMDPARAMS));
		}

		tss->tss.esp = (DWORD)vaddr + KTASK_STACK_SIZE - STACK_TOP_DUMMY - sizeof(TASKPARAMS0);
		tss->tss.ebp = tss->tss.esp;
	}
	else {
		tss->tss.ds = USER_MODE_DATA | syslevel | 4;
		tss->tss.es = USER_MODE_DATA | syslevel | 4;
		tss->tss.fs = USER_MODE_DATA | syslevel | 4;
		tss->tss.gs = USER_MODE_DATA | syslevel | 4;
		tss->tss.cs = USER_MODE_CODE | syslevel | 4;
		tss->tss.ss = USER_MODE_STACK | syslevel | 4;

		tss->espbase = __kProcessMalloc(UTASK_STACK_SIZE,&espsize,pid, vaddr);
		if (tss->espbase == FALSE)
		{
			__kFreeProcess(tss->pid);
			tss->status = TASK_OVER;
			return FALSE;
		}

		result = phy2linear(vaddr, tss->espbase, UTASK_STACK_SIZE, (DWORD*)tss->tss.cr3);
		if (result == FALSE)
		{
			__kFreeProcess(tss->pid);
			tss->status = TASK_OVER;
			return FALSE;
		}

		LPTASKPARAMS3 params = (LPTASKPARAMS3)(tss->espbase + UTASK_STACK_SIZE - STACK_TOP_DUMMY - sizeof(TASKPARAMS3));
		params->terminate = (DWORD)__terminateProcess;
		params->terminate2 = (DWORD)__terminateProcess;
		params->tid = pid;
		__strcpy(params->szFileName, filename);
		params->filename = params->szFileName;
		__strcpy(params->szFuncName, funcname);
		params->funcname = params->szFuncName;
		params->lpcmdparams = &params->cmdparams;
		if (runparam)
		{
			__memcpy((char*)params->lpcmdparams, (char*)runparam, sizeof(TASKCMDPARAMS));
		}

		DWORD esp3 = (DWORD)vaddr + UTASK_STACK_SIZE - STACK_TOP_DUMMY - sizeof(TASKPARAMS3);
		tss->tss.ebp = esp3;

		IRETADDR_RING3 * iretaddr3 = (IRETADDR_RING3*)(tss->tss.esp0 - sizeof(IRETADDR_RING3));
		iretaddr3->esp3 = esp3;
		iretaddr3->ss3 = tss->tss.ss;
		iretaddr3->iretaddr0.cs = tss->tss.cs;
		iretaddr3->iretaddr0.eip = tss->tss.eip;
		iretaddr3->iretaddr0.eflags = tss->tss.eflags;

		tss->tss.esp =(DWORD) iretaddr3;	
	}
	tss->vasize += espsize;

	tss->counter = 0;
	tss->errorno = 0;

	tss->pid = pid;
	tss->tid = pid;
	__strcpy(tss->filename, filename);
	__strcpy(tss->funcname, funcname);

	tss->window = 0;

// 	__printf(szout, "imagebase:%x,imagesize:%x,map base:%x,entry:%x,cr3:%x,esp:%x\n",
// 		getImageBase((char*)pemap), imagesize, pemap, entry, tss->tss.cr3,tss->espbase);
// 	__drawGraphChars((unsigned char*)szout, 0);

	addTaskList(tss->tid);

	return TRUE;
}



int __kCreateProcess(DWORD filedata, int filesize, char * filename, char * funcname, int syslevel, DWORD params) {

	int ret = 0;
	char szout[1024];

	//ͬһ�����̲������Ի���������ַ�������Է��ʡ�
	//��һ�����̰ѵ�ǰ���̵����Ե�ַת��Ϊ������ַ�����ݸ���һ�����̣�
	//��������һ����������̺���������������ַ��ʱ����Ҫ����ӳ���
	//��˼򵥵���ͨ�����ʹ�ͬ��������ַʵ�ֹ����ǲ��е�

// 	DWORD filedata = linear2phy(lpfiledata);
// 	char * filename = (char *)linear2phy((DWORD)fn);
// 	char * funcname = (char *)linear2phy((DWORD)functionname);
// 	DWORD params = linear2phy(paramlist);
	int mode = syslevel & 0xfffffffc;
	DWORD level = syslevel & 3;

	if (filedata == 0 && filename == 0)
	{
		__printf(szout, "__kCreateProcess filedata:%x or filename:%s error\n", filedata, filename);
		__drawGraphChars((unsigned char*)szout, 0);
		return FALSE;
	}

	if (filedata == 0 && filename != 0)
	{
		filesize = readFileTo(filename);
		if (filesize <= 0)
		{
			__printf(szout, "__kCreateProcess readFileTo:%s error\n", filename);
			__drawGraphChars((unsigned char*)szout, 0);
			return FALSE;
		}

		filedata = FILE_BUFFER_ADDRESS;
	}

	TASKRESULT result;
	ret = __getFreeTask(&result);
	if (ret == FALSE)
	{
		__printf(szout, "__kCreateProcess filename:%s function:%s __getFreeTask error\n", filename, funcname);
		__drawGraphChars((unsigned char*)szout, 0);
		return FALSE;
	}

	if (mode & INFILE_DOS_PROCESS_FLAG)
	{
		ret = __initDosTss(result.lptss, result.number, filedata, filename, funcname, mode + (level | 3), params);
		return ret;
	}


	int petype = getPeType(filedata);
	if (petype == 1 || petype == 0)
	{
		if (filesize == 0)
		{
			return FALSE;
		}
		else {
			filedata = __initDosExe(filedata, filesize, result.number);
			if (filedata)
			{
				ret = __initDosTss(result.lptss, result.number, filedata, filename, funcname, 3, params);
				return ret;
			}
			else {
				__printf(szout, "__kCreateProcess __initDosTss:%s error\n", filename);
				__drawGraphChars((unsigned char*)szout, 0);
				return FALSE;
			}
		}
	}
	else if (petype == 2)
	{
		DWORD type = getType((DWORD)filedata);
		if (type & 0x2000)
		{
			if (funcname == 0)
			{
				__printf(szout, "__kCreateProcess run dll without function name\n");
				__drawGraphChars((unsigned char*)szout, 0);
				return FALSE;
			}
		}
		ret = __initProcess(result.lptss, result.number, filedata, filename, funcname, level, params);
		return ret;
	}
	else {
		ret = FALSE;
	}

	return ret;
}

