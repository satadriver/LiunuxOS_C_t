#include "cmosAlarm.h"
#include "hardware.h"
#include "def.h"
#include "Utils.h"
#include "video.h"
#include "window.h"
#include "keyboard.h"
#include "mouse.h"
#include "task.h"
#include "Pe.h"
#include "slab.h"
#include "process.h"
#include "Thread.h"

int gAlarmTaskFlag = FALSE;

unsigned char readCmosPort(unsigned char port) {
	__asm {
		//in al,70h
		//and al,80h
		//or al,port

		mov al,port
		out 70h,al

		in al,71h
		movzx eax,al
	}
}

void writeCmosPort(unsigned char port, unsigned char value) {
	__asm {
		//in al, 70h
		//and al, 80h
		//or al, port

		mov al,port
		out 70h, al

		mov al, value
		out 71h, al
	}
}

int isLeapYear(int year) {
	int ret = year % 100;
	if (ret == 0)
	{
		return FALSE;
	}

	ret = year % 4;
	if (ret )
	{
		return FALSE;
	}
	return TRUE;
}

int getDayOfMonth(int year, int month) {
	if ( month == 2)
	{
		if (isLeapYear(year)) {
			return 29;
		}
		else {
			return 28;
		}
	}

	if (month == 1 || month == 3 || month == 5 || month == 7 || month == 8 || month == 10 || month == 12)
	{
		return 31;
	}
	else {
		return 30;
	}
}



unsigned char bcd2b(unsigned char bcd) {
	unsigned char low = bcd & 0xf;
	unsigned char h = (bcd & 0xf0) >> 4;
	return h * 10 + low;
}

unsigned char b2bcd(unsigned char b) {
	int h = b / 10;
	int l = b % 10;
	return (h << 4) + l;
}

unsigned short makehalf(unsigned char low, unsigned char high) {
	return (high << 8) + low;
}

void __kCmosAlarmProc() {

	__asm {
		cli
	}
	int ret = 0;

	unsigned char bcentury = readCmosPort(0x32);
	unsigned char byear = readCmosPort(9);
	unsigned int cy = ((unsigned int)bcd2b(bcentury) * 100) + (unsigned int)bcd2b(byear);

	unsigned char bmonth = readCmosPort(8);
	unsigned char month = bcd2b(bmonth);

	unsigned char bday = readCmosPort(7);
	unsigned char bhour = readCmosPort(4);
	unsigned char bminute = readCmosPort(2);
	unsigned char bsecond = readCmosPort(0);

	unsigned char day = bcd2b(bday);
	unsigned char hour = bcd2b(bhour);
	unsigned char minute = bcd2b(bminute);
	
	unsigned char dstday = bday;
	unsigned char dsthour = bhour;
	unsigned char dstmin = bminute;

	dstmin = minute + CMOS_ALARM_MINUTE_INTERVAL;
	if (dstmin >= 60)
	{
		dstmin -= 60;
		dstmin = b2bcd(dstmin);

		dsthour = hour + 1;
		if (dsthour >= 24)
		{
			dsthour -= 24;
			dsthour = b2bcd(dsthour);

			dstday = day + 1;
			unsigned char maxMonthDay = getDayOfMonth(cy, month);
			if (dstday > maxMonthDay)
			{
				dstday = 1;
				dstday = b2bcd(dstday);
			}
			else {
				dstday = b2bcd(dstday);
			}
		}
		else
		{
			dsthour = b2bcd(dsthour);
		}
	}
	else {
		dstmin = b2bcd(dstmin);
	}

	writeCmosPort(0x0d, dstday);
	writeCmosPort(0x05, dsthour);
	writeCmosPort(0x03, dstmin);
	writeCmosPort(0x01, 0);

	__asm {
		sti
	}

// 	char szout[1024];
// 	__printf(szout, "set alarm at:%d/%d/%d %d:%d:%d\n",cy, month, day, hour, minute, 0);
// 	__drawGraphChars((unsigned char*)szout, 0);

	if (gAlarmTaskFlag == TRUE)
	{
		TASKCMDPARAMS cmd;
		__memset((char*)&cmd, 0, sizeof(TASKCMDPARAMS));
		DWORD task = getAddrFromName(KERNEL_DLL_BASE, "__doAlarmTask");
		__kCreateThread((unsigned int)task, KERNEL_DLL_BASE,(DWORD)&cmd, "__doAlarmTask" );
	}
}



void __doAlarmTask(DWORD retaddr,DWORD pid,char * filename,char * funcname,DWORD param) {

	//gAlarmTaskFlag = TRUE;

	DWORD backsize = gBytesPerPixel*(gVideoWidth)*(gVideoHeight);

	DWORD backGround = __kMalloc(backsize);

	DWORD windowid = addWindow(FALSE, 0, 0, 0,"__cmosAlarm");

	POINT p;
	p.x = 0;
	p.y = 0;

	int color = 0;

	__drawRectangle(&p, gVideoWidth, gVideoHeight, color, (unsigned char*)backGround);

	while (1)
	{
		unsigned int ck = __kGetKbd(windowid);
		//unsigned int ck = __getchar(windowid);
		unsigned int asc = ck & 0xff;
		if (asc == 0x1b)
		{
			gAlarmTaskFlag = FALSE;

			__restoreRectangle(&p, gVideoWidth, gVideoHeight, (unsigned char*)backGround);
			removeWindow(windowid);

			__kFree(backGround);

			//__terminatePid(pid);
			return;
		}

		__sleep(0);

		color += 0x00010f;
		__drawRectangle(&p, gVideoWidth, gVideoHeight, color, 0);
	}
}