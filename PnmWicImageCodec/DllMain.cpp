#include "precomp.h"
#include "ComPtr.h"
#include "ClassFactoryImpl.h"
#include "PnmDecoder.h"
#include "TgaDecoder.h"

volatile ULONG DllObjectCount = 0;

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, void **ppvObject)
{
    if (!ppvObject)
        return E_INVALIDARG;

    ComPtr<IClassFactory> classFactory;

    if (rclsid == CLSID_PnmDecoder)
        classFactory = ComPtr<IClassFactory>::Make<ClassFactoryImpl<PnmDecoder> >();
    else if (rclsid == CLSID_TgaDecoder)
        classFactory = ComPtr<IClassFactory>::Make<ClassFactoryImpl<TgaDecoder> >();
    else
        return CLASS_E_CLASSNOTAVAILABLE;

    if (!classFactory)
        return E_OUTOFMEMORY;

    HRESULT hr = classFactory->QueryInterface(riid, ppvObject);

    return hr;
}

STDAPI DllCanUnloadNow()
{
    return DllObjectCount == 0 ? S_OK : S_FALSE;
}

STDAPI_(BOOL) DllMain(HINSTANCE hinstDLL, ULONG fdwReason, LPVOID lpvReserved)
{
    switch (fdwReason)
    {
    case DLL_PROCESS_ATTACH:
        _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#ifndef _DEBUG
        DisableThreadLibraryCalls(hinstDLL);
#endif
        break;
    case DLL_PROCESS_DETACH:
        _CrtDumpMemoryLeaks();
        break;
    }

    return TRUE;
}