#pragma once

#include "UnknownImpl.h"
#include "ComPtr.h"

template<typename T>
class ClassFactoryImpl : public UnknownImpl<IClassFactory>
{
public:
    // IUnknown Interface

    STDMETHOD(QueryInterface)(REFIID riid, void **ppvObject)
    {
        if (!ppvObject)
            return E_INVALIDARG;

        HRESULT hr;

        if (riid == IID_IUnknown)
        {
            *ppvObject = static_cast<IUnknown*>(this);
            AddRef();

            hr = S_OK;
        }
        else if (riid == IID_IClassFactory)
        {
            *ppvObject = static_cast<IClassFactory*>(this);
            AddRef();

            hr = S_OK;
        }
        else
        {
            *ppvObject = NULL;
            hr = E_NOINTERFACE;
        }

        return hr;
    }

    // IClassFactory Interface

    STDMETHOD(CreateInstance)(IUnknown *pUnkOuter, REFIID riid, void **ppvObject)
    {
        if (pUnkOuter)
            return CLASS_E_NOAGGREGATION;
        if (!ppvObject)
            return E_INVALIDARG;

        ComPtr<T> obj = ComPtr<T>::Make();

        if (!obj)
            return E_OUTOFMEMORY;

        HRESULT hr = obj->QueryInterface(riid, ppvObject);

        return hr;
    }

    STDMETHOD(LockServer)(BOOL fLock)
    {
        return CoLockObjectExternal(this, fLock, FALSE);
    }
};

