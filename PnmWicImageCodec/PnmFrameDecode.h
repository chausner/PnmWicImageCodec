#pragma once

#include "UnknownImpl.h"
#include "ComPtr.h"
#include "PnmHeader.h"

class PnmFrameDecode : public UnknownImpl<IWICBitmapFrameDecode>
{
public:
    PnmFrameDecode(IStream *stream, const PnmHeader &pHeader, ULARGE_INTEGER frameStreamPosition);

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
    STDMETHODIMP PnmFrameDecode::CopyPixelsAscii(const WICRect &rect, UINT cbStride, UINT cbPixelsSize, BYTE *pbPixels);
    STDMETHODIMP PnmFrameDecode::CopyPixelsBinary(const WICRect &rect, UINT cbStride, UINT cbPixelsSize, BYTE *pbPixels);

	ComPtr<IStream> stream;
	PnmHeader header;
	ULARGE_INTEGER frameStreamPosition;
    REFWICPixelFormatGUID pixelFormat;
    ULARGE_INTEGER lastStreamPosition;
    UINT lastPixelIndex;
};