#include "stdafx.h"

UINT __stdcall SHChangeNotifyCustomAction(MSIHANDLE hInstall)
{
    HRESULT hr = WcaInitialize(hInstall, "SHChangeNotifyCustomAction");

	ExitOnFailure(hr, "Failed to initialize");    

    SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);

LExit:
    UINT er = SUCCEEDED(hr) ? ERROR_SUCCESS : ERROR_INSTALL_FAILURE;

	return WcaFinalize(er);
}

extern "C" BOOL WINAPI DllMain(HINSTANCE hInst, ULONG ulReason, LPVOID)
{
    switch (ulReason)
    {
    case DLL_PROCESS_ATTACH:
        WcaGlobalInitialize(hInst);
        break;
    case DLL_PROCESS_DETACH:
        WcaGlobalFinalize();
        break;
    }

	return TRUE;
}
