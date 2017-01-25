#pragma once

extern volatile ULONG DllObjectCount;

template <typename T>
class UnknownImpl : public T
{
private:
    volatile ULONG numReferences;

public:
    UnknownImpl()
        : numReferences(1)
    {
        InterlockedIncrement(&DllObjectCount);
	}

    virtual ~UnknownImpl()
    {
        InterlockedDecrement(&DllObjectCount);
    }

    STDMETHOD_(ULONG, AddRef)()
    {
        return InterlockedIncrement(&numReferences);
    }

    STDMETHOD_(ULONG, Release)()
    {
        ULONG refs = InterlockedDecrement(&numReferences);

        if (refs == 0)
        {
            delete this;
        }

        return refs;
    }
};
