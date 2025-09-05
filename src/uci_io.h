#pragma once

// Ensure Windows headers don't define min/max macros
#ifndef NOMINMAX
#define NOMINMAX 1
#endif

// Correct platform detection: MSVC / MinGW define _WIN32, not WIN32
#ifdef _WIN32
#include <windows.h>
#else
#include <sys/select.h>
#include <sys/time.h>
#include <unistd.h>
#include <cstdio>
#endif

// Returns number of pending input events (0 if none)
int input_waiting();
