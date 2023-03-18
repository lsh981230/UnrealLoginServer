#pragma once

#include <Windows.h>

struct MY_OVERLAPPED
{
	OVERLAPPED overlap;


	enum { MODE_RECV, MODE_SEND, MODE_REQSEND };
	int mode;
};