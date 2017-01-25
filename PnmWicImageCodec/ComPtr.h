#pragma once

template<typename T>
class ComPtr
{
public:
    ComPtr();
    ComPtr(const ComPtr<T>& c);
    ComPtr(T *p);

    template<typename Type = T, typename... ArgTypes>
    static ComPtr<T> Make(ArgTypes&&... args);

    ComPtr(ComPtr<T>&& c);
    ~ComPtr();

    ComPtr<T>& operator=(const ComPtr<T>& c);
    ComPtr<T>& operator=(ComPtr<T>&& c);
    ComPtr<T>& operator=(T * p);

    T *operator->() const;
    bool operator!() const;

    operator T*() const;

private:
    T *p;
};

template<typename T>
ComPtr<T>::ComPtr() : p(nullptr)
{
}

template<typename T>
ComPtr<T>::ComPtr(const ComPtr<T>& c)
{
    p = c.p;

    if (p)
    {
        p->AddRef();
    }
}

template<typename T>
ComPtr<T>::ComPtr(T *p) : p(p)
{
    if (p)
    {
        p->AddRef();
    }
}

template<typename T>
template<typename Type, typename... ArgTypes>
ComPtr<T> ComPtr<T>::Make(ArgTypes&& ...args)
{
    ComPtr<T> c;

    c.p = new (std::nothrow) Type(std::forward<ArgTypes>(args)...);

    return c;
}

template<typename T>
ComPtr<T>::ComPtr(ComPtr<T>&& c)
{
    p = c.p;
    c.p = nullptr;
}

template<typename T>
ComPtr<T>& ComPtr<T>::operator=(const ComPtr<T>& c)
{
    if (this != &c)
    {
        if (p)
        {
            p->Release();
        }

        p = c.p;

        if (p)
        {
            p->AddRef();
        }
    }

    return *this;
}

template<typename T>
ComPtr<T>& ComPtr<T>::operator=(ComPtr<T>&& c)
{
    if (this != &c)
    {
        if (p)
        {
            p->Release();
        }

        p = c.p;
        c.p = nullptr;
    }

    return *this;
}

template<typename T>
ComPtr<T>& ComPtr<T>::operator=(T *p)
{
    if (this->p != p)
    {
        if (this->p)
        {
            this->p->Release();
        }

        this->p = p;

        if (this->p)
        {
            this->p->AddRef();
        }
    }

    return *this;
}

template<typename T>
ComPtr<T>::~ComPtr()
{
    if (p)
    {
        p->Release();
    }
}

template<typename T>
T *ComPtr<T>::operator->() const
{
    return p;
}

template<typename T>
bool ComPtr<T>::operator!() const
{
    return !p;
}

template<typename T>
ComPtr<T>::operator T*() const
{
    return p;
}