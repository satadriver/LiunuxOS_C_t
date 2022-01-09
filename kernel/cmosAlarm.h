#pragma once

#include "def.h"

unsigned char readCmosPort(unsigned char port);

void writeCmosPort(unsigned char port, unsigned char value);

unsigned char bcd2b(unsigned char bcd);
unsigned char b2bcd(unsigned char b);

unsigned short makehalf(unsigned char low, unsigned char high);

extern "C" __declspec(dllexport) void __kCmosAlarmProc();

extern "C" __declspec(dllexport) void __doAlarmTask(DWORD retaddr,DWORD pid, char * fname,char * funcname,DWORD param);

int isLeapYear(int year);

int getDayOfMonth(int year, int month);

#define CMOS_ALARM_MINUTE_INTERVAL 10

#define CMOS_ALARM_SECOND_INTERVAL 1