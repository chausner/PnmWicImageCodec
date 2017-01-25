#include "precomp.h"
#include "Utilities.h"
#include "TgaHeader.h"
#include "TgaDecoder.h"

// {1C0050CC-CA4C-4DCB-A6B6-39075902E958}
static const GUID CLSID_TgaContainer = { 0x1C0050CC, 0xCA4C, 0x4DCB, { 0xA6, 0xB6, 0x39, 0x07, 0x59, 0x02, 0xe9, 0x58 } };

// {36CA8E28-C18A-4B27-AB39-4A2A7D57EB69}
static const GUID CLSID_TgaDecoder = { 0x36CA8E28, 0xC18A, 0x4B27, { 0xab, 0x39, 0x4a, 0x2a, 0x7d, 0x57, 0xeb, 0x69 } };

// IUnknown Interface    

STDMETHODIMP TgaDecoder::QueryInterface(REFIID riid, void **ppvObject)
{
    if (!ppvObject)
        return E_INVALIDARG;

    if (riid == IID_IUnknown)
    {
        *ppvObject = static_cast<IUnknown*>(this);
        AddRef();
    }
    else if (riid == IID_IWICBitmapDecoder)
    {
        *ppvObject = static_cast<IWICBitmapDecoder*>(this);
        AddRef();
    }
    else
    {
        *ppvObject = NULL;
        return E_NOINTERFACE;
    }

    return S_OK;
}

// IWICBitmapDecoder Interface

STDMETHODIMP TgaDecoder::QueryCapability(IStream *pIStream, DWORD *pCapability)
{
    if (!pIStream || !pCapability)
        return E_INVALIDARG;

    *pCapability = 0;

    ULARGE_INTEGER position;

    HRESULT hr = pIStream->Seek({ 0 }, STREAM_SEEK_CUR, &position);

    if (FAILED(hr))
        return hr;

    TgaHeader header;

    hr = header.Parse(pIStream);

    if (SUCCEEDED(hr))
        *pCapability = WICBitmapDecoderCapabilityCanDecodeAllImages;

    HRESULT hr2 = pIStream->Seek(*(LARGE_INTEGER*)&position, STREAM_SEEK_SET, NULL);

    if (FAILED(hr2))
        return hr2;

    return hr;
}

STDMETHODIMP TgaDecoder::Initialize(IStream *pIStream, WICDecodeOptions cacheOptions)
{
    if (!pIStream)
        return E_INVALIDARG;

    frames.clear();

    TgaHeader header;

    HRESULT hr = header.Parse(pIStream);

    if (FAILED(hr))
        return hr;

    if (header.ImageType != TgaImageType::NoImage)
    {
        ComPtr<TgaFrameDecode> frame = ComPtr<TgaFrameDecode>::Make(pIStream, header);

        frames.push_back(frame);
    }

    return S_OK;
}

STDMETHODIMP TgaDecoder::GetContainerFormat(GUID *pguidContainerFormat)
{
    if (!pguidContainerFormat)
        return E_INVALIDARG;

    *pguidContainerFormat = CLSID_TgaContainer;

    return S_OK;
}

STDMETHODIMP TgaDecoder::GetDecoderInfo(IWICBitmapDecoderInfo **ppIDecoderInfo)
{
    ComPtr<IWICImagingFactory> factory;
    ComPtr<IWICComponentInfo> componentInfo;

    HRESULT hr = CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, IID_IWICImagingFactory, (LPVOID*)&factory);

    if (SUCCEEDED(hr))    
        hr = factory->CreateComponentInfo(CLSID_TgaDecoder, reinterpret_cast<IWICComponentInfo **>(&componentInfo));    

    if (SUCCEEDED(hr))    
        hr = componentInfo->QueryInterface(IID_IWICBitmapDecoderInfo, (void**)ppIDecoderInfo);

    return hr;
}

STDMETHODIMP TgaDecoder::CopyPalette(IWICPalette *pIPalette)
{
    if (frames.empty())
        return WINCODEC_ERR_PALETTEUNAVAILABLE;
    else
        return frames[0]->CopyPalette(pIPalette);
}

STDMETHODIMP TgaDecoder::GetMetadataQueryReader(IWICMetadataQueryReader **ppIMetadataQueryReader)
{
    return WINCODEC_ERR_UNSUPPORTEDOPERATION;
}

STDMETHODIMP TgaDecoder::GetPreview(IWICBitmapSource **ppIPreview)
{
    return WINCODEC_ERR_UNSUPPORTEDOPERATION;
}

STDMETHODIMP TgaDecoder::GetColorContexts(UINT cCount, IWICColorContext **ppIColorContexts, UINT *pcActualCount)
{
    if (!pcActualCount)
        return E_INVALIDARG;

    *pcActualCount = 0;

    return S_OK;
}

STDMETHODIMP TgaDecoder::GetThumbnail(IWICBitmapSource **ppIThumbnail)
{	
    return WINCODEC_ERR_CODECNOTHUMBNAIL;
}

STDMETHODIMP TgaDecoder::GetFrameCount(UINT *pCount)
{
    if (!pCount)
        return E_INVALIDARG;

    *pCount = static_cast<UINT>(frames.size());

    return S_OK;
}

STDMETHODIMP TgaDecoder::GetFrame(UINT index, IWICBitmapFrameDecode **ppIBitmapFrame)
{
    if (index >= static_cast<UINT>(frames.size()) || !ppIBitmapFrame)
        return E_INVALIDARG;

    return frames[index]->QueryInterface(IID_IWICBitmapFrameDecode, (void**)ppIBitmapFrame);
}