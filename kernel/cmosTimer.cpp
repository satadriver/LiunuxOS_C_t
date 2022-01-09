
#include "cmosTimer.h"
#include "cmosAlarm.h"
#include "Utils.h"
#include "video.h"


char * dw2str(int dw) {
	if (dw == 0)
	{
		return "Sunday";
	}else if (dw == 1)
	{
		return "Monday";
	}
	else if (dw == 2)
	{
		return "Tuesday";
	}
	else if (dw == 3)
	{
		return "Wednesday";
	}
	else if (dw == 4)
	{
		return "Thursday";
	}
	else if (dw == 5)
	{
		return "Friday";
	}
	else if (dw == 6)
	{
		return "Saturday";
	}
	else {
		return "Other";
	}
}

#define SHUTDOWN_SCREEN_DELAY 360

unsigned char bcd2binary(char bcd) {
	int low = (bcd & 0xf) ;
	int high = (bcd >> 4)*10;
	return low + high;
}


unsigned short bcd2asc(char bcd) {
	int low = (bcd & 0xf) + 0x30;
	int high = (bcd >> 4) + 0x30;
	return (low << 8) + high;
}

void __kCmosTimer() {
	char c = readCmosPort(0x32);
	char y = readCmosPort(9);
	char m = readCmosPort(8);
	char d = readCmosPort(7);
	char hour = readCmosPort(4);
	char minute = readCmosPort(2);
	char second = readCmosPort(0);

	int strc = bcd2asc(c);
	int stry = bcd2asc(y);
	int strm = bcd2asc(m);
	int strd = bcd2asc(d);
	int strhour = bcd2asc(hour);
	int strminute = bcd2asc(minute);
	int strsecond = bcd2asc(second);

	char dw = readCmosPort(6);
	char *strdw = dw2str(dw);

	char szout[256];
	
	char*  singlefmt = "%s%s/%s/%s [%s] %s:%s:%s";
	char* doublefmt =  "%s%s-%s-%s [%s] %s:%s:%s";
	char*  thirdfmt =  "%s%s\\%s\\%s [%s] %s:%s:%s";
	char * fmt = 0;

	int fontcolor = 0;

	int binsec = bcd2binary(second);
	if ((binsec % 3) == 0)
	{
		fmt = singlefmt;
		fontcolor = CMOS_TIMESTAMP_SINGLE_COLOR;
	}
	else if ((binsec % 3) == 1)
	{
		fmt = doublefmt;
		fontcolor = CMOS_TIMESTAMP_DOUBLE_COLOR;
	}else if ((binsec % 3) == 2)
	{
		fmt = thirdfmt;
		fontcolor = CMOS_TIMESTAMP_THIRD_COLOR;
	}

	__printf(szout,fmt , &strc, &stry, &strm, &strd, strdw, &strhour, &strminute, &strsecond);
	__strcpy((char*)CMOS_DATETIME_STRING, szout);

	DWORD * lptickcnt = (DWORD*)CMOS_SECONDS_TOTAL;
	(*lptickcnt)++;
	if (*lptickcnt >= SHUTDOWN_SCREEN_DELAY)
	{
		__asm {
			mov eax, 8
			int 80h
		}
		*lptickcnt = 0;
	}

	DWORD pos = (gVideoHeight - GRAPHCHAR_HEIGHT) * gVideoWidth * gBytesPerPixel;
	__drawGraphChar((unsigned char*)szout, fontcolor, pos, TASKBARCOLOR);
}