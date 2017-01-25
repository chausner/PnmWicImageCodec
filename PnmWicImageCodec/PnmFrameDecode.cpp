#include "precomp.h"
#include "Utilities.h"
#include "BufferedStreamReader.h"
#include "PnmFrameDecode.h"

PnmFrameDecode::PnmFrameDecode(IStream *stream, const PnmHeader &header, ULARGE_INTEGER frameStreamPosition)
    : stream(stream), header(header), frameStreamPosition(frameStreamPosition), pixelFormat(GetWicPixelFormatFromHeader(header)),
    lastPixelIndex(0), lastStreamPosition()
{
}

// IUnknown Interface   

STDMETHODIMP PnmFrameDecode::QueryInterface(REFIID riid, void **ppvObject)
{
    if (!ppvObject)
        return E_INVALIDARG;

    if (riid == IID_IUnknown)
    {
        *ppvObject = static_cast<IUnknown*>(this);
        AddRef();
    }
    else if (riid == IID_IWICBitmapFrameDecode)
    {
        *ppvObject = static_cast<IWICBitmapFrameDecode*>(this);
        AddRef();
    }
    else if (riid == IID_IWICBitmapSource)
    {
        *ppvObject = static_cast<IWICBitmapSource*>(this);
        AddRef();
    }
    else
    {
        *ppvObject = NULL;
        return E_NOINTERFACE;
    }

    return S_OK;
}

// IWICBitmapFrameDecode Interface

STDMETHODIMP PnmFrameDecode::GetMetadataQueryReader(IWICMetadataQueryReader **ppIMetadataQueryReader)
{
    return WINCODEC_ERR_UNSUPPORTEDOPERATION;
}

STDMETHODIMP PnmFrameDecode::GetColorContexts(UINT cCount, IWICColorContext **ppIColorContexts, UINT *pcActualCount)
{
    if (!pcActualCount)
        return E_INVALIDARG;

    *pcActualCount = 0;

    return S_OK;
} 

STDMETHODIMP PnmFrameDecode::GetThumbnail(IWICBitmapSource **ppIThumbnail)
{
    return WINCODEC_ERR_CODECNOTHUMBNAIL;
}

STDMETHODIMP PnmFrameDecode::GetSize(UINT *puiWidth, UINT *puiHeight)
{
    if (!puiWidth || !puiHeight)
        return E_INVALIDARG;

	*puiWidth = header.ImageWidth;
	*puiHeight = header.ImageHeight;

	return S_OK;
}

STDMETHODIMP PnmFrameDecode::GetPixelFormat(WICPixelFormatGUID *pPixelFormat)
{
    if (!pPixelFormat)
        return E_INVALIDARG;

    *pPixelFormat = pixelFormat;

	return S_OK;
}

STDMETHODIMP PnmFrameDecode::GetResolution(double *pDpiX, double *pDpiY)
{
    if (!pDpiX || !pDpiY)
        return E_INVALIDARG;

	*pDpiX = 96;
	*pDpiY = 96;

	return S_OK;
}

STDMETHODIMP PnmFrameDecode::CopyPalette(IWICPalette *pIPalette)
{
    return WINCODEC_ERR_PALETTEUNAVAILABLE;
}

STDMETHODIMP PnmFrameDecode::CopyPixels(const WICRect *prc, UINT cbStride, UINT cbPixelsSize, BYTE *pbPixels)
{
    if (!pbPixels)
        return E_INVALIDARG;

    WICRect rect;

    if (prc)
        rect = *prc;
    else
    {
        rect.X = 0;
        rect.Y = 0;
        rect.Width = header.ImageWidth;
        rect.Height = header.ImageHeight;
    }

    if (rect.X < 0 || rect.Y < 0 || rect.Width < 0 || rect.Height < 0 ||
        rect.X >= header.ImageWidth || rect.Y >= header.ImageHeight ||
        rect.X + rect.Width > header.ImageWidth || rect.Y + rect.Height > header.ImageHeight)
        return E_INVALIDARG;

    if (rect.Width == 0 || rect.Height == 0)
        return S_OK;

    if (header.AsciiFormat)
    {
        return CopyPixelsAscii(rect, cbStride, cbPixelsSize, pbPixels);
    }
    else
    {
        return CopyPixelsBinary(rect, cbStride, cbPixelsSize, pbPixels);
    }
}

STDMETHODIMP PnmFrameDecode::CopyPixelsAscii(const WICRect &rect, UINT cbStride, UINT cbPixelsSize, BYTE *pbPixels)
{
    UINT bpp = GetBitsPerPixelFromHeader(header);

    UINT rectWicScanlineLength = (rect.Width * bpp + 7) / 8;

    if (cbStride < rectWicScanlineLength || cbPixelsSize < rect.Height * cbStride)
        return E_INVALIDARG;

    BufferedStreamReader bufferedStreamReader;

    HRESULT hr = bufferedStreamReader.Initialize(stream);

    if (FAILED(hr))
        return hr;

    int components = header.PnmType == PnmType::Pixmap ? 3 : 1;

    // seek to pixel with number (rect.Y * imageWidth + rect.X)
    if (lastPixelIndex == rect.Y * header.ImageWidth + rect.X && lastPixelIndex != 0)
    {
        hr = bufferedStreamReader.Seek(*(LARGE_INTEGER*)&lastStreamPosition, STREAM_SEEK_SET);

        if (FAILED(hr))
            return hr;
    }
    else
    {
        hr = bufferedStreamReader.Seek(*(LARGE_INTEGER*)&frameStreamPosition, STREAM_SEEK_SET);

        if (FAILED(hr))
            return hr;

        for (UINT i = 0; i < components * (rect.Y * header.ImageWidth + rect.X); i++)
        {
            int v;

            hr = bufferedStreamReader.ReadInt(&v);

            if (FAILED(hr))
                return hr;
            if (hr == S_FALSE)
                return WINCODEC_ERR_BADIMAGE;
        }
    }

    BYTE *currentDestination = pbPixels;

    for (int y = 0; y < rect.Height; y++)
    {
        // read rect.Width pixels
        if (header.PnmType != PnmType::Bitmap)
        {
            if (header.MaxColorValue == 255)
                for (int x = 0; x < components * rect.Width; x++)
                {
                    int v;

                    hr = bufferedStreamReader.ReadInt(&v);

                    if (FAILED(hr))
                        return hr;
                    if (hr == S_FALSE)
                        return WINCODEC_ERR_BADIMAGE;

                    currentDestination[x] = (BYTE) min(v, 255);
                }
            else if (header.MaxColorValue == 65535)
                for (int x = 0; x < components * rect.Width; x++)
                {
                    int v;

                    hr = bufferedStreamReader.ReadInt(&v);

                    if (FAILED(hr))
                        return hr;
                    if (hr == S_FALSE)
                        return WINCODEC_ERR_BADIMAGE;

                    *(USHORT*) &currentDestination[x * 2] = (USHORT) min(v, 65535);
                }
            else if (header.MaxColorValue < 255)
            {
                double f = 255.0 / header.MaxColorValue;

                for (int x = 0; x < components * rect.Width; x++)
                {
                    int v;

                    hr = bufferedStreamReader.ReadInt(&v);

                    if (FAILED(hr))
                        return hr;
                    if (hr == S_FALSE)
                        return WINCODEC_ERR_BADIMAGE;

                    currentDestination[x] = (BYTE) min((int) ((BYTE) v * f + 0.5), 255);
                }
            }
            else
            {
                double f = 65535.0 / header.MaxColorValue;

                for (int x = 0; x < components * rect.Width; x++)
                {
                    int v;

                    hr = bufferedStreamReader.ReadInt(&v);

                    if (FAILED(hr))
                        return hr;
                    if (hr == S_FALSE)
                        return WINCODEC_ERR_BADIMAGE;

                    *(USHORT*) &currentDestination[x * 2] = (USHORT) min((int) ((USHORT) v * f + 0.5), 65535);
                }
            }
        }
        else
        {
            BYTE b = 0;            

            for (int x = 0; x < rect.Width; x++)
            {
                int v;

                hr = bufferedStreamReader.ReadInt(&v);

                if (FAILED(hr))
                    return hr;
                if (hr == S_FALSE)
                    return WINCODEC_ERR_BADIMAGE;

                b = (b << 1) | min(v, 1);

                if (x % 8 == 7)
                {
                    currentDestination[x / 8] = ~b;
                    b = 0;
                }
            }

            if (rect.Width % 8 != 0)
            {
                b <<= 8 - (rect.Width % 8);
                currentDestination[rect.Width / 8] = ~b;
            }
        }

        currentDestination += cbStride;

        // skip imageWidth - rect.Width pixels
        if (y != rect.Height - 1)
        {
            for (UINT i = 0; i < components * (header.ImageWidth - rect.Width); i++)
            {
                int v;

                hr = bufferedStreamReader.ReadInt(&v);

                if (FAILED(hr))
                    return hr;
                if (hr == S_FALSE)
                    return WINCODEC_ERR_BADIMAGE;
            }
        }
    }

    lastPixelIndex = (rect.Y + rect.Height - 1) * header.ImageWidth + rect.X + rect.Width;
    hr = bufferedStreamReader.GetPosition(&lastStreamPosition);

    return hr;
}

STDMETHODIMP PnmFrameDecode::CopyPixelsBinary(const WICRect &rect, UINT cbStride, UINT cbPixelsSize, BYTE *pbPixels)
{
    UINT bpp = GetBitsPerPixelFromHeader(header);

    UINT rectPnmScanlineLength = (((rect.X + rect.Width) * bpp + 7) / 8) - (rect.X * bpp) / 8;

    UINT rectWicScanlineLength = (rect.Width * bpp + 7) / 8;

    if (cbStride < rectWicScanlineLength || cbPixelsSize < rect.Height * cbStride)
        return E_INVALIDARG;

    UINT pnmStride = (header.ImageWidth * bpp + 7) / 8;

    ULARGE_INTEGER currentStreamPosition;

    currentStreamPosition.QuadPart = frameStreamPosition.QuadPart + rect.Y * pnmStride + (rect.X * bpp) / 8;

    HRESULT hr = stream->Seek(*(LARGE_INTEGER*)&currentStreamPosition, STREAM_SEEK_SET, NULL);

    if (FAILED(hr))
        return WINCODEC_ERR_STREAMNOTAVAILABLE;

    BYTE *currentDestination = pbPixels;

    LARGE_INTEGER pnmStrideFill;

    pnmStrideFill.QuadPart = pnmStride - rectPnmScanlineLength;

    for (int y = 0; y < rect.Height; y++)
    {
        ULONG bytesRead;

        hr = stream->Read(currentDestination, rectWicScanlineLength, &bytesRead);

        if (FAILED(hr))
            return WINCODEC_ERR_STREAMREAD;
        if (bytesRead != rectWicScanlineLength)
            return WINCODEC_ERR_BADIMAGE;

        if (bpp == 16 || bpp == 48)
        {
            for (UINT i = 0; i < rectWicScanlineLength; i += 2) // convert to little-endian
                *(USHORT*) &currentDestination[i] = _byteswap_ushort(*(USHORT*) &currentDestination[i]);
        }
        else if (bpp == 1)
        {
            for (UINT i = 0; i < rectWicScanlineLength; i++) // invert bitmap	
                currentDestination[i] = ~currentDestination[i];

            if (rectWicScanlineLength < rectPnmScanlineLength) // rect.X % 8 != 0
            {
                BYTE lastByte;

                hr = stream->Read(&lastByte, 1, &bytesRead);

                if (FAILED(hr))
                    return WINCODEC_ERR_STREAMREAD;
                if (bytesRead != 1)
                    return WINCODEC_ERR_BADIMAGE;

                int shift = rect.X % 8;

                for (UINT i = 0; i < rectWicScanlineLength - 1; i++)
                {
                    currentDestination[i] =
                        (currentDestination[i] << shift) | (currentDestination[i + 1] >> (8 - shift));
                }

                currentDestination[rectWicScanlineLength - 1] =
                    (currentDestination[rectWicScanlineLength - 1] << shift) | (~lastByte >> (8 - shift));
            }
        }

        if (header.PnmType != PnmType::Bitmap &&
            header.MaxColorValue != 255 && header.MaxColorValue != 65535)
        {
            if (header.MaxColorValue < 255)
            {
                double f = 255.0 / header.MaxColorValue;

                for (UINT i = 0; i < rectWicScanlineLength; i++)
                    currentDestination[i] = (BYTE) min((int) (currentDestination[i] * f + 0.5), 255);
            }
            else
            {
                double f = 65535.0 / header.MaxColorValue;

                for (UINT i = 0; i < rectWicScanlineLength; i += 2)
                    *(USHORT*) &currentDestination[i] = (USHORT) min((int) (*(USHORT*) &currentDestination[i] * f + 0.5), 65535);
            }
        }

        currentDestination += cbStride;

        if (pnmStrideFill.QuadPart != 0 && y != rect.Height - 1)
        {
            hr = stream->Seek(pnmStrideFill, STREAM_SEEK_CUR, NULL);

            if (FAILED(hr))
                return WINCODEC_ERR_STREAMNOTAVAILABLE;
        }
    }

    return S_OK;
}
