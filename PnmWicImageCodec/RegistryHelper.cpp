#include "precomp.h"
#include "RegistryHelper.h"

HRESULT RegistryHelper::CreateKey(HKEY key, LPCSTR subKey, LPCSTR newKey)
{
    HKEY k;

    LONG result = RegOpenKeyEx(key, subKey, 0, KEY_WRITE, &k);

    if (result != ERROR_SUCCESS)
        return HRESULT_FROM_WIN32(result);

    HKEY kNew;

    result = RegCreateKeyEx(k, newKey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &kNew, NULL);

    if (result == ERROR_SUCCESS)
        RegCloseKey(kNew);

    RegCloseKey(k);

    return HRESULT_FROM_WIN32(result);
}

HRESULT RegistryHelper::SetBinaryValue(HKEY key, LPCSTR subKey, LPCSTR value, const BYTE *data, DWORD length)
{
    HKEY k;

    LONG result = RegOpenKeyEx(key, subKey, 0, KEY_WRITE, &k);

    if (result != ERROR_SUCCESS)
        return HRESULT_FROM_WIN32(result);

    result = RegSetValueEx(k, value, 0, REG_BINARY, data, length);

    RegCloseKey(k);

    return HRESULT_FROM_WIN32(result);
}

HRESULT RegistryHelper::SetDWORDValue(HKEY key, LPCSTR subKey, LPCSTR value, DWORD data)
{
    HKEY k;

    LONG result = RegOpenKeyEx(key, subKey, 0, KEY_WRITE, &k);

    if (result != ERROR_SUCCESS)
        return HRESULT_FROM_WIN32(result);

    result = RegSetValueEx(k, value, 0, REG_DWORD, (const BYTE *)&data, sizeof(data));

    RegCloseKey(k);

    return HRESULT_FROM_WIN32(result);
}

HRESULT RegistryHelper::SetSZValue(HKEY key, LPCSTR subKey, LPCSTR value, LPCSTR data)
{
    HKEY k;

    LONG result = RegOpenKeyEx(key, subKey, 0, KEY_WRITE, &k);

    if (result != ERROR_SUCCESS)
        return HRESULT_FROM_WIN32(result);
    
    result = RegSetValueEx(k, value, 0, REG_SZ, (const BYTE *)data, lstrlen(data) + 1);

    RegCloseKey(k);

    return HRESULT_FROM_WIN32(result);
}

HRESULT RegistryHelper::DeleteKey(HKEY key, LPCSTR subKey, BOOL recursive)
{
    if (recursive)
        return HRESULT_FROM_WIN32(RegDeleteTree(key, subKey));
    else
        return HRESULT_FROM_WIN32(RegDeleteKey(key, subKey));
}

HRESULT RegistryHelper::DeleteValue(HKEY key, LPCSTR subKey, LPCSTR value)
{
    HKEY k;

    LONG result = RegOpenKeyEx(key, subKey, 0, KEY_WRITE, &k);

    if (result != ERROR_SUCCESS)
        return HRESULT_FROM_WIN32(result);

    result = RegDeleteValue(k, value);

    RegCloseKey(k);

    return HRESULT_FROM_WIN32(result);
}


STDAPI DllRegisterServer()
{
    HRESULT hr = S_OK;

    char moduleFileName[MAX_PATH];

    GetModuleFileName(g_DllInstance, moduleFileName, ARRAYSIZE(moduleFileName));

    if (SUCCEEDED(hr))
        hr = RegistryHelper::CreateKey(HKEY_CLASSES_ROOT, "CLSID", "{B3FE5BCB-5C20-4A66-8994-6D6A7A654EAE}");
    if (SUCCEEDED(hr))
        hr = RegistryHelper::SetSZValue(HKEY_CLASSES_ROOT, "CLSID\\{B3FE5BCB-5C20-4A66-8994-6D6A7A654EAE}", "Version", "1.0.0.0");
    if (SUCCEEDED(hr))
        hr = RegistryHelper::SetSZValue(HKEY_CLASSES_ROOT, "CLSID\\{B3FE5BCB-5C20-4A66-8994-6D6A7A654EAE}", "SpecVersion", "1.0.0.0");
    if (SUCCEEDED(hr))
        hr = RegistryHelper::SetSZValue(HKEY_CLASSES_ROOT, "CLSID\\{B3FE5BCB-5C20-4A66-8994-6D6A7A654EAE}", "ColorManagementVersion", "1.0.0.0");
    if (SUCCEEDED(hr))
        hr = RegistryHelper::SetSZValue(HKEY_CLASSES_ROOT, "CLSID\\{B3FE5BCB-5C20-4A66-8994-6D6A7A654EAE}", "MimeTypes", "image/x-portable-bitmap,image/x-portable-graymap,image/x-portable-pixmap,image/x-portable-anymap");
    if (SUCCEEDED(hr))
        hr = RegistryHelper::SetSZValue(HKEY_CLASSES_ROOT, "CLSID\\{B3FE5BCB-5C20-4A66-8994-6D6A7A654EAE}", "FileExtensions", ".pbm,.pgm,.ppm,.pnm");
    if (SUCCEEDED(hr))
        hr = RegistryHelper::SetDWORDValue(HKEY_CLASSES_ROOT, "CLSID\\{B3FE5BCB-5C20-4A66-8994-6D6A7A654EAE}", "SupportAnimation", 0);
    if (SUCCEEDED(hr))
        hr = RegistryHelper::SetDWORDValue(HKEY_CLASSES_ROOT, "CLSID\\{B3FE5BCB-5C20-4A66-8994-6D6A7A654EAE}", "SupportChromakey", 0);
    if (SUCCEEDED(hr))
        hr = RegistryHelper::SetDWORDValue(HKEY_CLASSES_ROOT, "CLSID\\{B3FE5BCB-5C20-4A66-8994-6D6A7A654EAE}", "SupportLossless", 1);
    if (SUCCEEDED(hr))
        hr = RegistryHelper::SetDWORDValue(HKEY_CLASSES_ROOT, "CLSID\\{B3FE5BCB-5C20-4A66-8994-6D6A7A654EAE}", "SupportMultiframe", 1);
    if (SUCCEEDED(hr))
        hr = RegistryHelper::SetSZValue(HKEY_CLASSES_ROOT, "CLSID\\{B3FE5BCB-5C20-4A66-8994-6D6A7A654EAE}", "ContainerFormat", "{04E4B063-AB64-4B87-9822-048E1EFADE26}");
    if (SUCCEEDED(hr))
        hr = RegistryHelper::SetSZValue(HKEY_CLASSES_ROOT, "CLSID\\{B3FE5BCB-5C20-4A66-8994-6D6A7A654EAE}", "Author", "Christoph Hausner");
    if (SUCCEEDED(hr))
        hr = RegistryHelper::SetSZValue(HKEY_CLASSES_ROOT, "CLSID\\{B3FE5BCB-5C20-4A66-8994-6D6A7A654EAE}", "Vendor", "{AFED144E-1208-4CBE-B085-18F10F14676D}");
    if (SUCCEEDED(hr))
        hr = RegistryHelper::SetSZValue(HKEY_CLASSES_ROOT, "CLSID\\{B3FE5BCB-5C20-4A66-8994-6D6A7A654EAE}", "Description", "PNM Image Codec for WIC");
    if (SUCCEEDED(hr))
        hr = RegistryHelper::SetSZValue(HKEY_CLASSES_ROOT, "CLSID\\{B3FE5BCB-5C20-4A66-8994-6D6A7A654EAE}", "FriendlyName", "PNM WIC Image Codec");
    if (SUCCEEDED(hr))
        hr = RegistryHelper::CreateKey(HKEY_CLASSES_ROOT, "CLSID\\{B3FE5BCB-5C20-4A66-8994-6D6A7A654EAE}", "Formats");
    if (SUCCEEDED(hr))
        hr = RegistryHelper::CreateKey(HKEY_CLASSES_ROOT, "CLSID\\{B3FE5BCB-5C20-4A66-8994-6D6A7A654EAE}\\Formats", "{6fddc324-4e03-4bfe-b185-3d77768dc905}");
    if (SUCCEEDED(hr))
        hr = RegistryHelper::CreateKey(HKEY_CLASSES_ROOT, "CLSID\\{B3FE5BCB-5C20-4A66-8994-6D6A7A654EAE}\\Formats", "{6fddc324-4e03-4bfe-b185-3d77768dc908}");
    if (SUCCEEDED(hr))
        hr = RegistryHelper::CreateKey(HKEY_CLASSES_ROOT, "CLSID\\{B3FE5BCB-5C20-4A66-8994-6D6A7A654EAE}\\Formats", "{6fddc324-4e03-4bfe-b185-3d77768dc90b}");
    if (SUCCEEDED(hr))
        hr = RegistryHelper::CreateKey(HKEY_CLASSES_ROOT, "CLSID\\{B3FE5BCB-5C20-4A66-8994-6D6A7A654EAE}\\Formats", "{6fddc324-4e03-4bfe-b185-3d77768dc90d}");
    if (SUCCEEDED(hr))
        hr = RegistryHelper::CreateKey(HKEY_CLASSES_ROOT, "CLSID\\{B3FE5BCB-5C20-4A66-8994-6D6A7A654EAE}\\Formats", "{6fddc324-4e03-4bfe-b185-3d77768dc915}");
    if (SUCCEEDED(hr))
        hr = RegistryHelper::CreateKey(HKEY_CLASSES_ROOT, "CLSID\\{B3FE5BCB-5C20-4A66-8994-6D6A7A654EAE}", "InprocServer32");
    if (SUCCEEDED(hr))
        hr = RegistryHelper::SetSZValue(HKEY_CLASSES_ROOT, "CLSID\\{B3FE5BCB-5C20-4A66-8994-6D6A7A654EAE}\\InprocServer32", "", moduleFileName);
    if (SUCCEEDED(hr))
        hr = RegistryHelper::SetSZValue(HKEY_CLASSES_ROOT, "CLSID\\{B3FE5BCB-5C20-4A66-8994-6D6A7A654EAE}\\InprocServer32", "ThreadingModel", "Both");
    if (SUCCEEDED(hr))
        hr = RegistryHelper::CreateKey(HKEY_CLASSES_ROOT, "CLSID\\{B3FE5BCB-5C20-4A66-8994-6D6A7A654EAE}", "Patterns\\0");
    if (SUCCEEDED(hr))
        hr = RegistryHelper::SetDWORDValue(HKEY_CLASSES_ROOT, "CLSID\\{B3FE5BCB-5C20-4A66-8994-6D6A7A654EAE}\\Patterns\\0", "Position", 0);
    if (SUCCEEDED(hr))
        hr = RegistryHelper::SetDWORDValue(HKEY_CLASSES_ROOT, "CLSID\\{B3FE5BCB-5C20-4A66-8994-6D6A7A654EAE}\\Patterns\\0", "Length", 3);

    BYTE pattern[3] = { 0x50, 0x31, 0x00 }; // allowing P[1..6][\x0A|\x0D|\x20]

    if (SUCCEEDED(hr))
        hr = RegistryHelper::SetBinaryValue(HKEY_CLASSES_ROOT, "CLSID\\{B3FE5BCB-5C20-4A66-8994-6D6A7A654EAE}\\Patterns\\0", "Pattern", pattern, ARRAYSIZE(pattern));

    BYTE mask[3] = { 0xFF, 0xF8, 0xD0 };

    if (SUCCEEDED(hr))
        hr = RegistryHelper::SetBinaryValue(HKEY_CLASSES_ROOT, "CLSID\\{B3FE5BCB-5C20-4A66-8994-6D6A7A654EAE}\\Patterns\\0", "Mask", mask, ARRAYSIZE(mask));

    if (SUCCEEDED(hr))
        hr = RegistryHelper::CreateKey(HKEY_CLASSES_ROOT, "CLSID", "{7ED96837-96F0-4812-B211-F13C24117ED3}\\Instance\\{B3FE5BCB-5C20-4A66-8994-6D6A7A654EAE}");
    if (SUCCEEDED(hr))
        hr = RegistryHelper::SetSZValue(HKEY_CLASSES_ROOT, "CLSID\\{7ED96837-96F0-4812-B211-F13C24117ED3}\\Instance\\{B3FE5BCB-5C20-4A66-8994-6D6A7A654EAE}", "CLSID", "{B3FE5BCB-5C20-4A66-8994-6D6A7A654EAE}");
    if (SUCCEEDED(hr))
        hr = RegistryHelper::SetSZValue(HKEY_CLASSES_ROOT, "CLSID\\{7ED96837-96F0-4812-B211-F13C24117ED3}\\Instance\\{B3FE5BCB-5C20-4A66-8994-6D6A7A654EAE}", "FriendlyName", "PNM WIC Image Codec");

    if (SUCCEEDED(hr))
        SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);

    return hr;
}

STDAPI DllUnregisterServer()
{
    HRESULT hr = S_OK;

    if (SUCCEEDED(hr))
        hr = RegistryHelper::DeleteKey(HKEY_CLASSES_ROOT, "CLSID\\{B3FE5BCB-5C20-4A66-8994-6D6A7A654EAE}", TRUE);

    if (SUCCEEDED(hr))
        hr = RegistryHelper::DeleteKey(HKEY_CLASSES_ROOT, "CLSID\\{7ED96837-96F0-4812-B211-F13C24117ED3}\\Instance\\{B3FE5BCB-5C20-4A66-8994-6D6A7A654EAE}", TRUE);

    if (SUCCEEDED(hr))
        SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);

    return hr;
}