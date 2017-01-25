#pragma once

#include "UnknownImpl.h"
#include "ComPtr.h"
#include "TgaHeader.h"
#include "TgaRleReader.h"

class TgaFrameDecode : public UnknownImpl<IWICBitmapFrameDecode>
{
public:
    TgaFrameDecode(IStream *pStream, const TgaHeader &pHeader);

    // IUnknown interface
    STDMETHOD(QueryInterface)(REFIID riid, void **ppvObject);

    // IWICBitmapFrameDecode interface
    STDMETHOD(GetMetadataQueryReader)(IWICMetadataQueryReader **ppIMetadataQueryReader);
    STDMETHOD(GetColorContexts)(UINT cCount, IWICColorContext **ppIColorContexts, UINT *pcActualCount);
    STDMETHOD(GetThumbnail)(IWICBitmapSource **ppIThumbnail);    

    // IWICBitmapSource interface
    STDMETHOD(GetSize)(UINT *puiWidth, UINT *puiHeight);
    STDMETHOD(GetPixelFormat)(WICPixelFormatGUID *pPixelFormat);
    STDMETHOD(GetResolution)(double *pDpiX, double *pDpiY);
    STDMETHOD(CopyPalette)(IWICPalette *pIPalette);
    STDMETHOD(CopyPixels)(const WICRect *prc, UINT cbStride, UINT cbPixelsSize, BYTE *pbPixels);

private:
	ComPtr<IStream> stream;
	TgaHeader header;
    REFWICPixelFormatGUID pixelFormat;
    ULARGE_INTEGER lastStreamPosition;
    UINT lastPixelIndex;
    TgaRleReader rleReader;
    std::vector<TgaRleContext> scanlineRleContexts;
};