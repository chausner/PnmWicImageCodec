#include "precomp.h"
#include "Utilities.h"
#include "BufferedStreamReader.h"
#include "PnmHeader.h"
#include "PnmDecoder.h"

// {04E4B063-AB64-4B87-9822-048E1EFADE26}
static const GUID CLSID_PnmContainer = { 0x4e4b063, 0xab64, 0x4b87, { 0x98, 0x22, 0x4, 0x8e, 0x1e, 0xfa, 0xde, 0x26 } };

// {B3FE5BCB-5C20-4A66-8994-6D6A7A654EAE}
static const GUID CLSID_PnmDecoder = { 0xb3fe5bcb, 0x5c20, 0x4a66, { 0x89, 0x94, 0x6d, 0x6a, 0x7a, 0x65, 0x4e, 0xae } };

// IUnknown Interface    

STDMETHODIMP PnmDecoder::QueryInterface(REFIID riid, void **ppvObject)
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

STDMETHODIMP PnmDecoder::QueryCapability(IStream *pIStream, DWORD *pCapability)
{
    if (!pIStream || !pCapability)
        return E_INVALIDARG;

    *pCapability = 0;

    ULARGE_INTEGER position;

    HRESULT hr = pIStream->Seek({ 0 }, STREAM_SEEK_CUR, &position);

    if (FAILED(hr))
        return WINCODEC_ERR_STREAMNOTAVAILABLE;

    BufferedStreamReader streamReader;

    hr = streamReader.Initialize(pIStream);

    if (FAILED(hr))
        return hr;

    PnmHeader header;

    hr = header.Parse(streamReader);

    if (SUCCEEDED(hr))
        *pCapability = WICBitmapDecoderCapabilityCanDecodeAllImages;

    HRESULT hr2 = pIStream->Seek(*(LARGE_INTEGER*)&position, STREAM_SEEK_SET, NULL);

    if (FAILED(hr2))
        return WINCODEC_ERR_STREAMNOTAVAILABLE;

    return hr;
}

STDMETHODIMP PnmDecoder::Initialize(IStream *pIStream, WICDecodeOptions cacheOptions)
{
    if (!pIStream)
        return E_INVALIDARG;

    frames.clear();

    BufferedStreamReader streamReader;

    HRESULT hr = streamReader.Initialize(pIStream);

    if (FAILED(hr))
        return hr;

    while (true)
    {
        PnmHeader header;

        hr = header.Parse(streamReader);

        if (FAILED(hr))
        {
            if (hr == WINCODEC_ERR_BADHEADER && !frames.empty()) 
            {
                hr = S_OK; // probably reached end of stream
                break; 
            }
            else
                return hr;
        }

        ULARGE_INTEGER position;

        hr = streamReader.GetPosition(&position);

        if (FAILED(hr))
            return hr;

        ComPtr<PnmFrameDecode> frame = ComPtr<PnmFrameDecode>::Make(pIStream, header, position);

        frames.push_back(frame);

        if (header.AsciiFormat) // ASCII images may contain only 1 frame
            break;

        UINT pnmBpp = GetBitsPerPixelFromHeader(header);

        UINT pnmStride = (header.ImageWidth * pnmBpp + 7) / 8;

        UINT pnmDataLength = pnmStride * header.ImageHeight;

        LARGE_INTEGER move;

        move.QuadPart = pnmDataLength;

        hr = streamReader.Seek(move, STREAM_SEEK_CUR);

        if (FAILED(hr))
            return hr;
    }

    return S_OK;
}

STDMETHODIMP PnmDecoder::GetContainerFormat(GUID *pguidContainerFormat)
{
    if (!pguidContainerFormat)
        return E_INVALIDARG;

    *pguidContainerFormat = CLSID_PnmContainer;

    return S_OK;
}

STDMETHODIMP PnmDecoder::GetDecoderInfo(IWICBitmapDecoderInfo **ppIDecoderInfo)
{
    ComPtr<IWICImagingFactory> factory;
    ComPtr<IWICComponentInfo> componentInfo;

    HRESULT hr = CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, IID_IWICImagingFactory, (LPVOID*) &factory);

    if (SUCCEEDED(hr))    
        hr = factory->CreateComponentInfo(CLSID_PnmDecoder, reinterpret_cast<IWICComponentInfo **>(&componentInfo));    

    if (SUCCEEDED(hr))    
        hr = componentInfo->QueryInterface(IID_IWICBitmapDecoderInfo, (void**)ppIDecoderInfo);

    return hr;
}

STDMETHODIMP PnmDecoder::CopyPalette(IWICPalette *pIPalette)
{
    return WINCODEC_ERR_PALETTEUNAVAILABLE;
}

STDMETHODIMP PnmDecoder::GetMetadataQueryReader(IWICMetadataQueryReader **ppIMetadataQueryReader)
{
    return WINCODEC_ERR_UNSUPPORTEDOPERATION;
}

STDMETHODIMP PnmDecoder::GetPreview(IWICBitmapSource **ppIPreview)
{
    return WINCODEC_ERR_UNSUPPORTEDOPERATION;
}

STDMETHODIMP PnmDecoder::GetColorContexts(UINT cCount, IWICColorContext **ppIColorContexts, UINT *pcActualCount)
{
    if (!pcActualCount)
        return E_INVALIDARG;

    *pcActualCount = 0;

    return S_OK;
}

STDMETHODIMP PnmDecoder::GetThumbnail(IWICBitmapSource **ppIThumbnail)
{	
    return WINCODEC_ERR_CODECNOTHUMBNAIL;
}

STDMETHODIMP PnmDecoder::GetFrameCount(UINT *pCount)
{
    if (!pCount)
        return E_INVALIDARG;

    *pCount = static_cast<UINT>(frames.size());

    return S_OK;
}

STDMETHODIMP PnmDecoder::GetFrame(UINT index, IWICBitmapFrameDecode **ppIBitmapFrame)
{
    if (index >= static_cast<UINT>(frames.size()) || !ppIBitmapFrame)
        return E_INVALIDARG;

    return frames[index]->QueryInterface(IID_IWICBitmapFrameDecode, (void**)ppIBitmapFrame);
}