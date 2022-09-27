#pragma once
#define NOMINMAX 1
#include <winsock.h>
#ifdef WIN64
#include <windows.h>
#endif

extern int input_waiting();
