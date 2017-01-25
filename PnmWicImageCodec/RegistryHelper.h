#pragma once

class RegistryHelper
{
public:
    static HRESULT CreateKey(HKEY key, LPCSTR subKey, LPCSTR newKey);
    static HRESULT SetBinaryValue(HKEY key, LPCSTR subKey, LPCSTR value, const BYTE *data, DWORD length);
    static HRESULT SetDWORDValue(HKEY key, LPCSTR subKey, LPCSTR value, DWORD data);
    static HRESULT SetSZValue(HKEY key, LPCSTR subKey, LPCSTR value, LPCSTR data);
    static HRESULT DeleteKey(HKEY key, LPCSTR subKey, BOOL recursive);
    static HRESULT DeleteValue(HKEY key, LPCSTR subKey, LPCSTR value);
};

