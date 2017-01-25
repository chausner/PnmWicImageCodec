#include "precomp.h"
#include "Utilities.h"
#include "TgaFrameDecode.h"

TgaFrameDecode::TgaFrameDecode(IStream *stream, const TgaHeader &header)
    : stream(stream), header(header), pixelFormat(GetWicPixelFormatFromTgaHeader(header)),
    lastPixelIndex(0), lastStreamPosition(), rleReader(stream, header.BitsPerPixel)
{
}

// IUnknown Interface   

STDMETHODIMP TgaFrameDecode::QueryInterface(REFIID riid, void **ppvObject)
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

STDMETHODIMP TgaFrameDecode::GetMetadataQueryReader(IWICMetadataQueryReader **ppIMetadataQueryReader)
{
    return WINCODEC_ERR_UNSUPPORTEDOPERATION;
}

STDMETHODIMP TgaFrameDecode::GetColorContexts(UINT cCount, IWICColorContext **ppIColorContexts, UINT *pcActualCount)
{
    if (!pcActualCount)
        return E_INVALIDARG;

    *pcActualCount = 0;

    return S_OK;
} 

STDMETHODIMP TgaFrameDecode::GetThumbnail(IWICBitmapSource **ppIThumbnail)
{
    return WINCODEC_ERR_CODECNOTHUMBNAIL;
}

STDMETHODIMP TgaFrameDecode::GetSize(UINT *puiWidth, UINT *puiHeight)
{
    if (!puiWidth || !puiHeight)
        return E_INVALIDARG;

	*puiWidth = header.ImageWidth;
	*puiHeight = header.ImageHeight;

	return S_OK;
}

STDMETHODIMP TgaFrameDecode::GetPixelFormat(WICPixelFormatGUID *pPixelFormat)
{
    if (!pPixelFormat)
        return E_INVALIDARG;

	*pPixelFormat = pixelFormat;

	return S_OK;
}

STDMETHODIMP TgaFrameDecode::GetResolution(double *pDpiX, double *pDpiY)
{
    if (!pDpiX || !pDpiY)
        return E_INVALIDARG;

	*pDpiX = 96;
	*pDpiY = 96;

	return S_OK;
}

STDMETHODIMP TgaFrameDecode::CopyPalette(IWICPalette *pIPalette)
{
    if (header.ImageType != TgaImageType::Indexed)
        return WINCODEC_ERR_PALETTEUNAVAILABLE;

    if (!pIPalette)
        return E_INVALIDARG;

    int entries = min(header.PaletteEntries + header.PaletteFirstEntryIndex, 256);

    WICColor *colors = new (std::nothrow) WICColor[entries];

    if (!colors)
        return E_OUTOFMEMORY;

    memset(colors, 0, sizeof(WICColor) * entries);

    switch (header.PaletteBpp)
    {
    case 8:
        for (int i = 0; i < entries - header.PaletteFirstEntryIndex; i++)
        {
            colors[header.PaletteFirstEntryIndex + i] =
                0xFF000000 |
                (header.Palette[i] << 16) |
                (header.Palette[i] << 8) |
                header.Palette[i];
        }
        break;
    case 15:
    case 16:
        for (int i = 0; i < entries - header.PaletteFirstEntryIndex; i++)
        {
            USHORT v = (header.Palette[i * 2 + 1] << 8) | header.Palette[i * 2];
            
            BYTE a = header.AlphaBits == 1 ? ((v & 0x8000) == 0 ? 0x00 : 0xFF) : 0xFF;
            BYTE r = (v & 0x7C00) >> 10;
            r = (r << 3) | (r >> 2);
            BYTE g = (v & 0x03E0) >> 5;
            g = (g << 3) | (g >> 2);
            BYTE b = v & 0x001F;
            b = (b << 3) | (b >> 2);

            colors[header.PaletteFirstEntryIndex + i] =
                (a << 24) |
                (r << 16) |
                (g << 8) |
                b;
        }
        break;
    case 24:
        for (int i = 0; i < entries - header.PaletteFirstEntryIndex; i++)
        {
            colors[header.PaletteFirstEntryIndex + i] =
                0xFF000000 | 
                (header.Palette[i * 3 + 2] << 16) |
                (header.Palette[i * 3 + 1] << 8) |
                header.Palette[i * 3];
        }
        break;
    case 32:
        for (int i = 0; i < entries - header.PaletteFirstEntryIndex; i++)
        {
            colors[header.PaletteFirstEntryIndex + i] =
                (header.Palette[i * 4 + 3] << 24) |
                (header.Palette[i * 4 + 2] << 16) |
                (header.Palette[i * 4 + 1] << 8) |
                header.Palette[i * 4];
        }
        break;
    }

    HRESULT hr = pIPalette->InitializeCustom(colors, entries);

    delete[] colors;

    return hr;
}

static void FlipScanlineHorz(BYTE *pixels, UINT stride, int bytesPerPixel)
{
    if (bytesPerPixel == 1)
    {
        BYTE *end = pixels + stride / 2;

        BYTE *p = pixels;

        while (p < end)
        {
            BYTE tmp = *p;
            *p = *(pixels + stride - 1 - (p - pixels));
            *(pixels + stride - 1 - (p - pixels)) = tmp;
            p++;
        }
    }
    else if (bytesPerPixel == 2)
    {
        BYTE *end = pixels + stride / 2 / 2 * 2;

        BYTE *p = pixels;

        while (p < end)
        {
            USHORT tmp = *(USHORT*) p;
            *(USHORT*) p = *(USHORT*) (pixels + stride - 2 - (p - pixels));
            *(USHORT*) (pixels + stride - 2 - (p - pixels)) = tmp;
            p += 2;
        }
    }
    else if (bytesPerPixel == 3)
    {
        BYTE *end = pixels + stride / 3 / 2 * 3;

        BYTE *p = pixels;

        while (p < end)
        {
            BYTE tmp1 = *p;
            BYTE tmp2 = *(p + 1);
            BYTE tmp3 = *(p + 2);
            *p = *(pixels + stride - 3 - (p - pixels));
            *(p + 1) = *(pixels + stride - 2 - (p - pixels));
            *(p + 2) = *(pixels + stride - 1 - (p - pixels));
            *(pixels + stride - 3 - (p - pixels)) = tmp1;
            *(pixels + stride - 2 - (p - pixels)) = tmp2;
            *(pixels + stride - 1 - (p - pixels)) = tmp3;
            p += 3;
        }
    }
    else if (bytesPerPixel == 4)
    {
        BYTE *end = pixels + stride / 4 / 2 * 4;

        BYTE *p = pixels;

        while (p < end)
        {
            UINT tmp = *(UINT*)p;
            *(UINT*)p = *(UINT*)(pixels + stride - 4 - (p - pixels));
            *(UINT*)(pixels + stride - 4 - (p - pixels)) = tmp;
            p += 4;
        }
    }
}

STDMETHODIMP TgaFrameDecode::CopyPixels(const WICRect *prc, UINT cbStride, UINT cbPixelsSize, BYTE *pbPixels)
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

    bool flipVert = header.ImageOrigin == TgaImageOrigin::BottomLeft || header.ImageOrigin == TgaImageOrigin::BottomRight;
    bool flipHorz = header.ImageOrigin == TgaImageOrigin::TopRight || header.ImageOrigin == TgaImageOrigin::BottomRight;

    int bytesPerPixel = (header.BitsPerPixel + 7) / 8;

    HRESULT hr;

    // seek to first pixel of the rect where we can begin decoding
    if (header.Compression == TgaCompression::RunLengthEncoded && flipVert)
    {
        if (scanlineRleContexts.size() < rect.Y + 1)
        {
            scanlineRleContexts.resize(header.ImageHeight);

            ULARGE_INTEGER position;

            position.QuadPart = header.ImageDataOffset;

            hr = stream->Seek(*(LARGE_INTEGER*)&position, STREAM_SEEK_SET, NULL);

            if (FAILED(hr))
                return WINCODEC_ERR_STREAMNOTAVAILABLE;

            rleReader.Reset();

            for (int y = header.ImageHeight - 1; y >= 0; y--)
            {
                TgaRleContext context;

                hr = rleReader.GetContext(&context);

                if (FAILED(hr))
                    return hr;

                scanlineRleContexts[y] = context;

                hr = rleReader.Skip(header.ImageWidth);

                if (FAILED(hr))
                    return hr;               
            }
        }

        hr = rleReader.SetContext(scanlineRleContexts[rect.Y + rect.Height - 1]);

        if (FAILED(hr))
            return hr;      

        if (!flipHorz)
            hr = rleReader.Skip(rect.X);
        else
            hr = rleReader.Skip(header.ImageWidth - (rect.X + rect.Width));

        if (FAILED(hr))
            return hr;
    }
    else
    {
        UINT startX = !flipHorz ? rect.X : (header.ImageWidth - (rect.X + rect.Width));
        UINT startY = !flipVert ? rect.Y : (header.ImageHeight - (rect.Y + rect.Height));

        if (lastPixelIndex == startY * header.ImageWidth + startX && lastPixelIndex != 0)
        {
            // stream should be at the correct position already
        }
        else
        {
            if (header.Compression == TgaCompression::Uncompressed)
            {
                ULARGE_INTEGER position;

                position.QuadPart = header.ImageDataOffset + (startY * header.ImageWidth + startX) * bytesPerPixel;

                hr = stream->Seek(*(LARGE_INTEGER*)&position, STREAM_SEEK_SET, NULL);

                if (FAILED(hr))
                    return WINCODEC_ERR_STREAMNOTAVAILABLE;
            }
            else
            {
                ULARGE_INTEGER position;

                position.QuadPart = header.ImageDataOffset;

                hr = stream->Seek(*(LARGE_INTEGER*)&position, STREAM_SEEK_SET, NULL);

                if (FAILED(hr))
                    return WINCODEC_ERR_STREAMNOTAVAILABLE;

                rleReader.Reset();

                hr = rleReader.Skip(startY * header.ImageWidth + startX);

                if (FAILED(hr))
                    return hr;
            }
        }
    }

    BYTE *currentDestination;
    
    if (!flipVert)
        currentDestination = pbPixels;
    else
        currentDestination = pbPixels + (rect.Height - 1) * cbStride;

    UINT scanlineLength = rect.Width * bytesPerPixel;

    for (int y = 0; y < rect.Height; y++)
    {
        if (header.Compression == TgaCompression::Uncompressed)
        {
            ULONG bytesRead;

            hr = stream->Read(currentDestination, scanlineLength, &bytesRead);

            if (FAILED(hr))
                return WINCODEC_ERR_STREAMREAD;
            if (bytesRead != scanlineLength)
                return WINCODEC_ERR_BADIMAGE;
        }
        else
        {
            hr = rleReader.Read(currentDestination, rect.Width);

            if (FAILED(hr))
                return hr;
        }

        if (flipHorz)
        {
            FlipScanlineHorz(currentDestination, scanlineLength, bytesPerPixel);
        }

        if (!flipVert)
            currentDestination += cbStride;
        else
            currentDestination -= cbStride;

        if (rect.Width != header.ImageWidth && y != rect.Height - 1)
        {
            if (header.Compression == TgaCompression::Uncompressed)
            {
                LARGE_INTEGER position;

                position.QuadPart = (header.ImageWidth - rect.Width) * bytesPerPixel;

                hr = stream->Seek(position, STREAM_SEEK_CUR, NULL);

                if (FAILED(hr))
                    return WINCODEC_ERR_STREAMNOTAVAILABLE;
            }
            else
            {
                hr = rleReader.Skip(header.ImageWidth - rect.Width);

                if (FAILED(hr))
                    return hr;
            }
        }
    }

    if (!flipVert)
    {
        lastPixelIndex = (rect.Y + rect.Height - 1) * header.ImageWidth + rect.X + rect.Width;

        hr = stream->Seek({ 0 }, STREAM_SEEK_CUR, &lastStreamPosition);

        if (FAILED(hr))
            return WINCODEC_ERR_STREAMNOTAVAILABLE;
    }

    return S_OK;
}
