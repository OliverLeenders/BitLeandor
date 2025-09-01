#pragma once
#define NOMINMAX 1
#ifdef WIN64
#include <winsock.h>
#include <windows.h>
#else
#include <sys/select.h>
#include <sys/time.h>
#include <unistd.h>
#include <cstdio>
#endif

extern int input_waiting();
