#pragma once


// DW | Byte3 | Byte2 | Byte1 | Byte0 | Addr
// -- - +-------------------------------------------------------- - +---- -
// 0 | Device ID | Vendor ID | 00
// -- - +-------------------------------------------------------- - +---- -
// 1 | Status　　　　　 | Command　　　　　　 | 04
// -- - +-------------------------------------------------------- - +---- -
// 2 | Class Code　　　　　　　　 | Revision ID　 | 08
// -- - +-------------------------------------------------------- - +---- -
// 3 | BIST　　 | Header Type | Latency Timer | Cache Line | 0C
// -- - +-------------------------------------------------------- - +---- -
// 4 | Base Address 0 | 10
// -- - +-------------------------------------------------------- - +---- -
// 5 | Base Address 1 | 14
// -- - +-------------------------------------------------------- - +---- -
// 6 | Base Address 2 | 18
// -- - +-------------------------------------------------------- - +---- -
// 7 | Base Address 3 | 1C
// -- - +-------------------------------------------------------- - +---- -
// 8 | Base Address 4 | 20
// -- - +-------------------------------------------------------- - +---- -
// 9 | Base Address 5 | 24
// -- - +-------------------------------------------------------- - +---- -
// 10 | CardBus CIS pointer　　　　　　　　　 | 28
// -- - +-------------------------------------------------------- - +---- -
// 11 | Subsystem Device ID　　 | Subsystem Vendor ID　　 | 2C
// -- - +-------------------------------------------------------- - +---- -
// 12 | Expansion ROM Base Address　　　　　　　　 | 30
// -- - +-------------------------------------------------------- - +---- -
// 13 | Reserved(Capability List) | 34
// -- - +-------------------------------------------------------- - +---- -
// 14 | Reserved　　　　　　　　　　　　　 | 38
// -- - +-------------------------------------------------------- - +---- -
// 15 | Max_Lat　 | Min_Gnt　 | IRQ Pin　 | IRQ Line　　 | 3C
// ------------------------------------------------------------------ -


//BAR恷朔匯了葎0燕幣宸頁啌符議IO坪贋??葎1頁燕幣宸頁IO 極笥??輝頁IO坪贋議扮昨1-2了燕幣坪贋議窃侏??
//bit 2葎1燕幣寡喘64了仇峽??葎0燕幣寡喘32了仇峽。bit1葎1燕幣曝寂寄弌階狛1M??葎0燕幣音階狛1M.bit3燕幣頁倦屶隔辛圓函

//1111 1111 1111 1111 1111 1111 1111 1111
#include "def.h"



int getBasePort(DWORD * baseregs, WORD devClsVender, DWORD * dev, int * irqpin);

int getNetcard(DWORD * regs,DWORD * dev,int * irq);

int getSvga(DWORD * regs,DWORD * dev,int * irq);

int getSdcard(DWORD * regs,DWORD * dev,int * irq);
int getSmbus(DWORD * regs,DWORD * dev,int * irq);

int getUsb(DWORD * regs,DWORD * dev,int * irq);

#ifdef DLL_EXPORT

extern "C"  __declspec(dllexport) int showPciDevs();
#else
extern "C"  __declspec(dllimport) int showPciDevs();
#endif

int listpci(char *dst);
void showAllPciDevs();