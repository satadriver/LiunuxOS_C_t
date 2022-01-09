
#include "pci.h"
#include "video.h"
#include "Utils.h"

//bit31:valid
//bit 16-23:bus no		device=1 is pci bridge
//bit 11-15:device no
//bit 8-10:function no
//bit 2-6: register no
//bit0-2:0
int getBasePort(DWORD * baseregs, WORD devClsVender,DWORD * dev,int * irqpin) {

	int cnt = 0;
	__asm {
		mov edi, baseregs
		mov esi, dev
		mov ebx, irqpin

		cld
		mov eax, 0x80000008		//offset 8,read class type,vender type

		_searchPciDev :
		push eax
		mov dx, 0xcf8
		out dx, eax
		mov dx, 0xcfc
		in  eax, dx
		shr eax, 16
		cmp ax, word ptr devClsVender		//0101h
		jnz _notfindPciDevice

		pop eax
		push eax
		
		mov [esi], eax

		add eax, 8		//offset 10h,read base reg
		mov ecx, 6		//pci device most has 6 base port register
		_ReadPciDevBaseReg :
		push eax
		mov dx, 0cf8h
		out dx, eax
		mov dx, 0cfch
		in eax, dx
		stosd
		inc dword ptr cnt
		pop eax
		add eax, 4
		loop _ReadPciDevBaseReg		//offset 40

		add eax, 18h			//offset 64,IRQ number
		mov dx, 0cf8h
		out dx, eax
		mov dx, 0cfch
		in eax, dx
		mov [ebx],eax

		_notfindPciDevice:
		pop eax
		add eax, 0x100
		cmp eax, 0x80010008
		jb _searchPciDev
			
		mov eax,cnt
	}
}




int listpci(char *dst) {
	int cnt = 0;
	__asm {
		mov edi, dst
		cld
		mov eax, 0x80000008		//offset 8,read class type,vender type

		_searchPciDev :
		push eax
		mov dx, 0xcf8
		out dx, eax
		mov dx, 0xcfc
		in  eax, dx
		cmp eax,0
		jz _notfindPciDevice
		cmp eax,0xffffffff
		jz _notfindPciDevice

		shr eax, 16
		stosd

		pop eax
		push eax
		stosd
		inc dword ptr cnt

		_notfindPciDevice:
		pop eax
		add eax, 0x100
		cmp eax, 0x80010008
		jb _searchPciDev
			
		mov eax,cnt
	}
}


void showAllPciDevs() {
	unsigned int devbuf[4096];
	int cnt = listpci((char*)devbuf);
	if (cnt > 0)
	{
		for (int i = 0; i < cnt; )
		{
			char szout[1024];
			__printf(szout, "\npci type:%x,device:%x\n", devbuf[i], devbuf[i + 1]);
			__drawGraphChars((unsigned char*)szout, 0);

			i += 2;
		}
	}
}

int getNetcard(DWORD * regs,DWORD * dev,int * irq){
	return getBasePort(regs,0x0200,dev,irq);
}

int getSvga(DWORD * regs,DWORD * dev,int * irq){
	return getBasePort(regs,0x0300,dev,irq);
}

int getSoundcard(DWORD * regs,DWORD * dev,int * irq){
	return getBasePort(regs,0x0401,dev,irq);
}

int getSmbus(DWORD * regs,DWORD * dev,int * irq){
	return getBasePort(regs,0x0c05,dev,irq);
}

int getUsb(DWORD * regs,DWORD * dev,int * irq){
	return getBasePort(regs,0x0c03,dev,irq);
}


int showPciDevs() {

	showAllPciDevs();

	__drawGraphChars((unsigned char*)"\n\nget all computer devices:\n", 0);

	int ret = 0;
	char szout[1024];

	DWORD svgaregs[32];
	DWORD svgadev = 0;
	int svgairq = 0;
	ret = getSvga(svgaregs, &svgadev, &svgairq);
	if (ret)
	{
		__printf(szout, "vesa int10h videoBase:%x,svga regs:%x,%x,%x,%x,%x,%x,dev:%x,irq:%x\n", gGraphBase,
			svgaregs[0], svgaregs[1], svgaregs[2], svgaregs[3], svgaregs[4], svgaregs[5],
			svgadev, svgairq);
		__drawGraphChars((unsigned char*)szout, 0);
	}


	DWORD usbregs[32];
	DWORD usbdev = 0;
	int usbirq = 0;
	ret = getUsb(usbregs, &usbdev, &usbirq);
	if (ret)
	{
		__printf(szout, "usb regs:%x,%x,%x,%x,%x,%x,dev:%x,irq:%x\n", usbregs[0], usbregs[1], usbregs[2], usbregs[3], usbregs[4], usbregs[5],
			usbdev, usbirq);
		__drawGraphChars((unsigned char*)szout, 0);
	}


	DWORD netregs[32];
	DWORD netdev = 0;
	int netirq = 0;
	ret = getNetcard(netregs, &netdev, &netirq);
	if (ret)
	{
		__printf(szout, "netcard regs:%x,%x,%x,%x,%x,%x,dev:%x,irq:%x\n", netregs[0], netregs[1], netregs[2], netregs[3], netregs[4], netregs[5],
			netdev, netirq);
		__drawGraphChars((unsigned char*)szout, 0);
	}


	DWORD smbusregs[32];
	DWORD smbusdev = 0;
	int smbusirq = 0;
	ret = getSmbus(smbusregs, &smbusdev, &smbusirq);
	if (ret)
	{
		__printf(szout, "smbus regs:%x,%x,%x,%x,%x,%x,dev:%x,irq:%x\n", smbusregs[0], smbusregs[1], smbusregs[2], smbusregs[3], smbusregs[4], smbusregs[5],
			smbusdev, smbusirq);
		__drawGraphChars((unsigned char*)szout, 0);
	}


	DWORD soundregs[32];
	DWORD sounddev = 0;
	int soundirq = 0;
	ret = getSoundcard(soundregs, &sounddev, &soundirq);
	if (ret)
	{
		__printf(szout, "sound card regs:%x,%x,%x,%x,%x,%x,dev:%x,irq:%x\n", soundregs[0], soundregs[1], soundregs[2], soundregs[3], soundregs[4], soundregs[5],
			sounddev, soundirq);
		__drawGraphChars((unsigned char*)szout, 0);
	}
	return 0;
}