#include <windows.h>
#include <tchar.h>
#include <wrl.h>
#include <fbxsdk.h>
#include "Application.h"

int WINAPI WinMain(__in HINSTANCE hInstance, __in_opt HINSTANCE hPrevInstance, __in LPSTR lpCmdLine, __in int nShowCmd)
{
	Prototype::Application applicationInstance(hInstance);

	if (!applicationInstance.Init())
	{
		return 1;
	}

	fbxsdk::FbxManager* manager = fbxsdk::FbxManager::Create();
	manager->Destroy();

	return applicationInstance.Run();
}

