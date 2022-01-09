#pragma once
#include "def.h"

char * dw2str(int dw);

unsigned short bcd2asc(char bcd);

unsigned char bcd2binary(char bcd);

extern "C" __declspec(dllexport) void __kCmosTimer();
