

//��Ҫ����ͷ�ļ�������ʹ��������������û�а���Ҳ���ᵼ��
#include "main.h"
#include "utils.h"
#include "mouse.h"
#include "gdi/jpeg.h"

#pragma comment(lib,"kernel.lib")



//https://wiki.osdev.org/
//https://wiki.osdev.org/
//https://wiki.osdev.org/



#ifdef _USRDLL
int __stdcall DllMain(unsigned int hInstance, unsigned int fdwReason, unsigned int lpvReserved) {

	return 1;
}
#else
int __stdcall WinMain(unsigned int hInstance, unsigned int hPrevInstance, char * lpCmdLine, int nShowCmd)
{
	testjpeg();
	return 1;
}
#endif
