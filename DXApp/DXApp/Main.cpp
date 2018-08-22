#include <windows.h>
#include <tchar.h>
#include <wrl.h>
#include "Application.h"

using namespace Prototype;

int WINAPI WinMain(__in HINSTANCE hInstance, __in_opt HINSTANCE hPrevInstance, __in LPSTR lpCmdLine, __in int nShowCmd)
{
	Application applicationInstance(hInstance);

	if (!applicationInstance.Initialize())
	{
		return 1;
	}

	return applicationInstance.Run();
}

