#include "process.h"
#include "task.h"
#include "Utils.h"
#include "Kernel.h"
#include "video.h"
#include "Pe.h"
#include "dosProcess.h"
#include "file.h"
#include "systemTimer.h"
#include "page.h"
#include "def.h"
#include "slab.h"



TASK_LIST_ENTRY *gTasksListPtr = 0;

void __terminateTask(int tid, char * filename, char * funcname, DWORD lpparams) {
	int retvalue = 0;
	__asm {
		mov retvalue, eax
	}

	removeTaskList(tid);
	__sleep(-1);
}





TASK_LIST_ENTRY* addTaskList(int tid) {
	LPPROCESS_INFO base = (LPPROCESS_INFO)TASKS_TSS_BASE;

	TASK_LIST_ENTRY * listbase = (TASK_LIST_ENTRY*)TASKS_LIST_BASE;

	TASK_LIST_ENTRY * list = (TASK_LIST_ENTRY*)TASKS_LIST_BASE;
	for (int i = 0; i < TASK_LIMIT_TOTAL; i++)
	{
		if (list[i].valid == 0 ) {

			list[i].process = base + tid;
			list[i].process->status = TASK_RUN;
			list[i].valid = TRUE;
			addlistTail((LIST_ENTRY*)&(listbase->list), (LIST_ENTRY*)&list[i].list);
			return &list[i];
		}
	}
	return 0;
}



TASK_LIST_ENTRY* removeTaskList(int tid) {

	TASK_LIST_ENTRY * list = (TASK_LIST_ENTRY*)TASKS_LIST_BASE;
	do 
	{
		if (list->valid && list->process && list->process->tid == tid)
		{
			list->process->status = TASK_OVER;
			list->process = FALSE;
			list->valid = FALSE;

			removelist((LIST_ENTRY*)&list->list);

			return list;
		}
		list = (TASK_LIST_ENTRY *)list->list.next;
		if (list == 0)
		{
			break;
		}
	} while (list != (TASK_LIST_ENTRY*)TASKS_LIST_BASE);

	return 0;
}



int __initTask() {

	LPPROCESS_INFO tssbase = (LPPROCESS_INFO)TASKS_TSS_BASE;
	for (int i = 0; i < TASK_LIMIT_TOTAL; i++)
	{
		__memset((char*)&tssbase[i], 0, sizeof(PROCESS_INFO));
	}

	DWORD *backuptables = (DWORD*)BACKUP_PAGE_TABLES;
	DWORD * cr3 = (DWORD*)PDE_ENTRY_VALUE;
	for (int i = 0; i < ITEM_IN_PAGE; i++)
	{
		backuptables[i] = cr3[i];
	}

	LPPROCESS_INFO process0 = (LPPROCESS_INFO)CURRENT_TASK_TSS_BASE;
	__strcpy(process0->filename, KERNEL_DLL_MODULE_NAME);
	__strcpy(process0->funcname, "__kernelEntry");
	process0->tid = 0;
	process0->pid = 0;
	process0->moduleaddr = (DWORD)KERNEL_DLL_BASE;
	process0->level = 0;
	process0->counter = 0;
	//process0->tss.iomapEnd = 0xff;
	//process0->tss.iomapOffset = 136;
	//process0->tss.cr3 = PDE_ENTRY_VALUE;
	process0->status = TASK_RUN;
	process0->va = 0;
	process0->vasize = 0;
	process0->espbase = KERNEL_TASK_STACK_BASE;
	//process0->tss.esp0 = TASKS_STACK0_BASE + TASK_STACK0_SIZE - STACK_TOP_DUMMY;
	//process0->tss.ss0 = KERNEL_MODE_STACK;

	__memcpy((char*)TASKS_TSS_BASE, (char*)CURRENT_TASK_TSS_BASE, sizeof(PROCESS_INFO));

	__memset((char*)TASKS_LIST_BASE, 0, TASK_LIMIT_TOTAL * sizeof(TASK_LIST_ENTRY));

	gTasksListPtr = (TASK_LIST_ENTRY *)TASKS_LIST_BASE;
	initListEntry(&gTasksListPtr->list);
	gTasksListPtr->process = (LPPROCESS_INFO)TASKS_TSS_BASE;
	gTasksListPtr->valid = TRUE;

	__memset((char*)V86_TASKCONTROL_ADDRESS, 0, LIMIT_V86_PROC_COUNT*12);
	return 0;
}

//CF(bit 0) [Carry flag]   
//????????????????????????????????(most-significant bit)??????????????????????1????????????
//????????????????????????????????????????????????????????????????(multiple-precision arithmetic)??????

//SF(bit 7) [Sign flag]   
//??????????????????????????????????????(0????????????????????????) 

//OF(bit 11) [Overflow flag]   
//??????????????????????????????????????????????????????????????????????1????????????????????????????????????????????????

//TF(bit 8) [Trap flag]   ????????????1????????????????????????????????????

//OF????????????????????????
//OF????????????????????????????????CF??????????????????????????????????????1??????????0????????????????
//OF????????????????????????CF????????????????????
//MOV AX,858F
//SUB AX,7869

// IOPL??I/O????????????????????????????EFLAGS??????????12??13????????in??ins??out??outs??cli??sti??????CPL<= IOPL????????????
//??????????????I/O????????????????????????????????????????I/O????????????????????????????(#GP)
//????????IOPL??????????popfl??iret????????????????????????0??????????????????

//TS??????????????????????????????????????????????????TS??1??TS = 1????????????????????????????(DNA)?????? 
//MP??????WAIT??????TS = 1????????????DNA??????MP = 1??TS = 1????WAIT????????



void prepareTss(LPPROCESS_INFO tss) {
	__memset((CHAR*)tss, 0, sizeof(PROCESS_INFO));
	tss->status = TASK_SUSPEND;

	tss->tss.iomapEnd = 0xff;
	tss->tss.iomapOffset = 136;

	//tss->tss.trap = 1;
}


int __getFreeTask(LPTASKRESULT ret) {
	if (ret == 0)
	{
		return FALSE;
	}
	ret->lptss = 0;
	ret->number = 0;

	LPPROCESS_INFO tss = (LPPROCESS_INFO)TASKS_TSS_BASE;
	for (int i = 0;i < TASK_LIMIT_TOTAL; i++)
	{
		if (tss[i].status == TASK_OVER)
		{
			prepareTss(&tss[i]);

			ret->number = i;
			ret->lptss = &tss[i];
			return TRUE;
		}
	}

	return FALSE;
}



int __createDosInFileTask(DWORD addr, char * filename) {
	if (__findProcessFileName(filename))
	{
		return 0;
	}
	return __kCreateProcess(addr,0x1000, filename, filename, INFILE_DOS_PROCESS_FLAG | 3, 0);
}





TASK_LIST_ENTRY* __findProcessFuncName(char * funcname) {
	TASK_LIST_ENTRY * list = (TASK_LIST_ENTRY*)TASKS_LIST_BASE;
	do
	{
		if (__strcmp(list->process->funcname,funcname) == 0)
		{
			return list;
		}
		list = (TASK_LIST_ENTRY *)list->list.next;
		if (list == 0)
		{
			break;
		}
	} while (list != (TASK_LIST_ENTRY*)TASKS_LIST_BASE);

	return FALSE;
}

TASK_LIST_ENTRY * __findProcessFileName(char * filename) {
	TASK_LIST_ENTRY * list = (TASK_LIST_ENTRY*)TASKS_LIST_BASE;
	do
	{
		if (__strcmp(list->process->filename, filename) == 0)
		{
			return list;
		}
		list = (TASK_LIST_ENTRY *)list->list.next;
	} while (list != (TASK_LIST_ENTRY*)TASKS_LIST_BASE);

	return FALSE;
}



TASK_LIST_ENTRY* __findProcessByPid(int pid) {
	TASK_LIST_ENTRY * list = (TASK_LIST_ENTRY*)TASKS_LIST_BASE;
	do
	{
		if (list->process->pid == pid)
		{
			return list;
		}
		list = (TASK_LIST_ENTRY *)list->list.next;
	} while (list != (TASK_LIST_ENTRY*)TASKS_LIST_BASE);

	return FALSE;
}


TASK_LIST_ENTRY* __findProcessByTid(int tid) {
	TASK_LIST_ENTRY * list = (TASK_LIST_ENTRY*)TASKS_LIST_BASE;
	do
	{
		if (list->process->tid == tid)
		{
			return list;
		}
		list = (TASK_LIST_ENTRY *)list->list.next;
	} while (list != (TASK_LIST_ENTRY*)TASKS_LIST_BASE);

	return FALSE;
}


int __terminateByFileName(char * filename) {

	TASK_LIST_ENTRY* list = __findProcessFileName(filename);
	if (list)
	{
		removeTaskList(list->process->pid);
	}

	return FALSE;
}

int __terminateByFuncName(char * funcname) {

	TASK_LIST_ENTRY* list = __findProcessFuncName(funcname);
	if (list)
	{
		removeTaskList(list->process->pid);
	}

	return FALSE;
}

int __terminatePid(int pid) {

	TASK_LIST_ENTRY* list = __findProcessByPid(pid);
	if (list)
	{
		removeTaskList(list->process->pid);
	}

	return 0;
}


int __terminateTid(int tid) {

	TASK_LIST_ENTRY* list = __findProcessByTid(tid);
	if (list)
	{
		removeTaskList(list->process->pid);
	}

	return 0;
}



extern "C"  __declspec(dllexport) DWORD __kTaskSchedule(CONTEXT_REGS *regs) {

	systimerProc();

// 	LPDOS_PE_CONTROL dos_status = (LPDOS_PE_CONTROL)V86_TASKCONTROL_ADDRESS;
// 	for (int i = 0; i < LIMIT_V86_PROC_COUNT; i++)
// 	{
// 		if (dos_status[i].status == DOS_TASK_OVER)
// 		{
// 			LPPROCESS_INFO p = __findProcessByPid(dos_status[i].pid);
// 			if (p)
// 			{
// 				p->status = TASK_OVER;
// 				removeTaskList(dos_status[i].pid);
// 			}
// 		}
// 	}

	TASK_LIST_ENTRY * prev = gTasksListPtr;
	gTasksListPtr = (TASK_LIST_ENTRY*)gTasksListPtr->list.next;
	if (prev->process->tid == gTasksListPtr->process->tid)
	{
		return 0;
	}

	LPPROCESS_INFO process = (LPPROCESS_INFO)CURRENT_TASK_TSS_BASE;
	LPPROCESS_INFO tss = (LPPROCESS_INFO)TASKS_TSS_BASE;
	if (prev->process->status == TASK_RUN /*&& process->status == TASK_RUN*/)
	{
		process->tss.eax = regs->eax;
		process->tss.ecx = regs->ecx;
		process->tss.edx = regs->edx;
		process->tss.ebx = regs->ebx;

		process->tss.ebp = regs->ebp;
		process->tss.esi = regs->esi;
		process->tss.edi = regs->edi;
		process->tss.ds = regs->ds & 0xffff;
		process->tss.es = regs->es & 0xffff;
		process->tss.fs = regs->fs & 0xffff;
		process->tss.gs = regs->gs & 0xffff;

		process->tss.esp = regs->esp;
		process->tss.ss = KERNEL_MODE_STACK;

		// 	process->tss.ss = regs->iretaddr_v86.iretaddr3.ss3;
		// 	process->tss.esp = regs->iretaddr_v86.iretaddr3.esp3;
		// 	process->tss.esp0 = regs->esp;
		// 	process->tss.ss0 = regs->ss;

		// 	process->tss.eflags = regs->iretaddr_v86.iretaddr3.iretaddr0.eflags;
		// 	process->tss.cs = regs->iretaddr_v86.iretaddr3.iretaddr0.cs & 0xffff;
		// 	process->tss.eip = regs->iretaddr_v86.iretaddr3.iretaddr0.eip;

		process->counter++;
		__memcpy((char*)(tss + prev->process->tid), (char*)process, sizeof(PROCESS_INFO));
	}

	if (gTasksListPtr->process->status == TASK_RUN)
	{
		__memcpy((char*)process, (char*)(gTasksListPtr->process->tid + tss), sizeof(PROCESS_INFO));

		regs->eax = process->tss.eax;
		regs->ecx = process->tss.ecx;
		regs->edx = process->tss.edx;
		regs->ebx = process->tss.ebx;

		regs->ebp = process->tss.ebp;
		regs->esi = process->tss.esi;
		regs->edi = process->tss.edi;
		regs->ds = process->tss.ds;
		regs->es = process->tss.es;
		regs->fs = process->tss.fs;
		regs->gs = process->tss.gs;

		// 	regs->retaddr_stack.ss3 = process->tss.ss;
		// 	regs->retaddr_stack.esp3 = process->tss.esp;
		// 	regs->esp = process->tss.esp0;
		// 	regs->ss = process->tss.ss0;

		// 	regs->iretaddr_v86.iretaddr3.iretaddr0.cs = process->tss.cs;
		// 	regs->iretaddr_v86.iretaddr3.iretaddr0.eip = process->tss.eip;
		// 	regs->iretaddr_v86.iretaddr3.iretaddr0.eflags = process->tss.eflags;

		regs->ss = KERNEL_MODE_STACK;
		regs->esp = process->tss.esp;

		// 	LPTSS timertss = (LPTSS)gAsmTsses;
		// 	timertss->cr3 = nextcr3;
		DWORD nextcr3 = gTasksListPtr->process->tss.cr3;
		__asm {
			mov eax, nextcr3
			mov cr3, eax
		}
	}

	//??????????????cr3??ldt??????????????????iret????????cr3??ldt????????????????????
// 	short ldt = ((DWORD)glpLdt - (DWORD)glpGdt);
// 	__asm {
// 		mov ax,ldt
// 		lldt ax
// 	}

	//tasktest();

 	char * fenvprev = (char*)FPU_STATUS_BUFFER + (prev->process->tid << 9);
	//If a memory operand is not aligned on a 16-byte boundary, regardless of segment
	//The assembler issues two instructions for the FSAVE instruction (an FWAIT instruction followed by an FNSAVE instruction), 
	//and the processor executes each of these instructions separately.
	//If an exception is generated for either of these instructions, the save EIP points to the instruction that caused the exception.
	__asm {
		clts			//before all fpu instructions
		fwait
		mov eax, fenvprev
		FxSAVE[eax]
		//fsave [fenv]
		//FNCLEX
	}
	prev->process->tss.fpu = 1;

	if (gTasksListPtr->process->tss.fpu)
	{
		char * fenvnext = (char*)FPU_STATUS_BUFFER + (gTasksListPtr->process->tid << 9);
		__asm {
			clts
			fwait
			finit
			mov eax, fenvnext
			//frstor [fenv]
			fxrstor[eax]
		}
	}

	return TRUE;
}




void tasktest(TASK_LIST_ENTRY *gTasksListPtr, TASK_LIST_ENTRY*gPrevTasksPtr) {
	static int gTestFlag = 0;
	if (gTestFlag >= 0 && gTestFlag <= -1)
	{
		char szout[1024];
		//TSS* procinfo = (TSS*)gAsmTsses;

		//__printf(szout, "clock tick tss link:%x,eflags:%x\r\n", procinfo->link, procinfo->eflags);
		//__drawGraphChars((unsigned char*)szout, 0);

		__printf(szout,
			"saved  cr3:%x,pid:%x,name:%s,level:%u,esp0:%x,ss0:%x,eip:%x,cs:%x,esp3:%x,ss3:%x,eflags:%x,link:%x,\r\n"
			"loaded cr3:%x,pid:%x,name:%s,level:%u,esp0:%x,ss0:%x,eip:%x,cs:%x,esp3:%x,ss3:%x,eflags:%x,link:%x.\r\n\r\n",
			gPrevTasksPtr->process->tss.cr3, gPrevTasksPtr->process->pid, gPrevTasksPtr->process->filename, gPrevTasksPtr->process->level,
			gPrevTasksPtr->process->tss.esp0, gPrevTasksPtr->process->tss.ss0, gPrevTasksPtr->process->tss.eip, gPrevTasksPtr->process->tss.cs,
			gPrevTasksPtr->process->tss.esp, gPrevTasksPtr->process->tss.ss, gPrevTasksPtr->process->tss.eflags, gPrevTasksPtr->process->tss.link,

			gTasksListPtr->process->tss.cr3, gTasksListPtr->process->pid, gTasksListPtr->process->filename, gTasksListPtr->process->level,
			gTasksListPtr->process->tss.esp0, gTasksListPtr->process->tss.ss0, gTasksListPtr->process->tss.eip, gTasksListPtr->process->tss.cs,
			gTasksListPtr->process->tss.esp, gTasksListPtr->process->tss.ss, gTasksListPtr->process->tss.eflags, gTasksListPtr->process->tss.link
		);
		__drawGraphChars((unsigned char*)szout, 0);
		gTestFlag++;
	}
}