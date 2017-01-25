#pragma once

#include "UnknownImpl.h"
#include "ComPtr.h"
#include "TgaFrameDecode.h"

extern const GUID CLSID_TgaContainer;

extern const GUID CLSID_TgaDecoder;

class TgaDecoder : public UnknownImpl<IWICBitmapDecoder>
{
public:
    // IUnknown Interface
    STDMETHOD(QueryInterface)(REFIID riid, void **ppvObject);

    // IWICBitmapDecoder Interface
    STDMETHOD(QueryCapability)(IStream *pIStream, DWORD *pCapability);
    STDMETHOD(Initialize)(IStream *pIStream, WICDecodeOptions cacheOptions);
    STDMETHOD(GetContainerFormat)(GUID *pguidContainerFormat);
    STDMETHOD(GetDecoderInfo)(IWICBitmapDecoderInfo **ppIDecoderInfo);
    STDMETHOD(CopyPalette)(IWICPalette *pIPalette);
    STDMETHOD(GetMetadataQueryReader)(IWICMetadataQueryReader **ppIMetadataQueryReader);
    STDMETHOD(GetPreview)(IWICBitmapSource **ppIPreview);
    STDMETHOD(GetColorContexts)(UINT cCount, IWICColorContext **ppIColorContexts, UINT *pcActualCount);
    STDMETHOD(GetThumbnail)(IWICBitmapSource **ppIThumbnail);
    STDMETHOD(GetFrameCount)(UINT *pCount);
    STDMETHOD(GetFrame)(UINT index, IWICBitmapFrameDecode **ppIBitmapFrame);

private:
    std::vector<ComPtr<TgaFrameDecode> > frames;   
};
