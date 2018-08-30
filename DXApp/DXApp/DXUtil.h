#pragma once

#include <d3d11.h>
#include <DirectXColors.h>
#include "DXERR/dxerr.h"
#pragma comment(lib, "d3d11.lib")

#ifdef _DEBUG
//#ifndef HR
#define HR(x) \
{\
	HRESULT hr = x; \
	if (FAILED(hr))\
	{\
		DXTraceW(__FILEW__, __LINE__, hr, L#x, TRUE);\
	}\
}
#else
#define HR(x) x;
#endif


#define WINDOW_WIDTH  1280;
#define WINDOW_HEIGHT  720;

namespace Memory
{

	template <class T> void SafeDelete(T& t)
	{
		if (t)
		{
			delete t;
			t = nullptr;
		}
	}

	template <class T> void SafeDeleteArray(T& t)
	{
		if (t)
		{
			delete[] t;
			t = nullptr;
		}
	}

	template <class T> void SafeRelease(T& t)
	{
		if (t)
		{
			t->Release();
			t = nullptr;
		}
	}
}
