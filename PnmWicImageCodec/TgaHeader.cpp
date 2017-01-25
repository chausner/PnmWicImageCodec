#include "precomp.h"
#include "TgaHeader.h"

HRESULT TgaHeader::Parse(IStream *stream)
{
    TgaRawHeader rawHeader;

    ULONG bytesRead;

    HRESULT hr = stream->Read(&rawHeader, sizeof(rawHeader), &bytesRead);

    if (FAILED(hr))
        return WINCODEC_ERR_STREAMREAD;
    if (bytesRead != sizeof(rawHeader))
        return WINCODEC_ERR_BADHEADER;

    switch (rawHeader.ImageType)
    {
    case 0:
        ImageType = TgaImageType::NoImage;
        Compression = TgaCompression::Uncompressed;
        break;
    case 1:
        ImageType = TgaImageType::Indexed;
        Compression = TgaCompression::Uncompressed;
        break;
    case 2:
        ImageType = TgaImageType::RGB;
        Compression = TgaCompression::Uncompressed;
        break;
    case 3:
        ImageType = TgaImageType::Grayscale;
        Compression = TgaCompression::Uncompressed;
        break;
    case 9:
        ImageType = TgaImageType::Indexed;
        Compression = TgaCompression::RunLengthEncoded;
        break;
    case 10:
        ImageType = TgaImageType::RGB;
        Compression = TgaCompression::RunLengthEncoded;
        break;
    case 11:
        ImageType = TgaImageType::Grayscale;
        Compression = TgaCompression::RunLengthEncoded;
        break;
    default:
        return WINCODEC_ERR_BADHEADER;
    }

    if (ImageType == TgaImageType::Indexed && rawHeader.ColorMapType != 1)
        return WINCODEC_ERR_BADHEADER;

    if (ImageType == TgaImageType::Indexed)
    {
        PaletteEntries = rawHeader.ColorMapLength;
        PaletteFirstEntryIndex = rawHeader.ColorMapFirstEntryIndex;

        switch (rawHeader.ColorMapEntrySize)
        {
        case 8:
        case 15:
        case 16:
        case 24:
        case 32:
            PaletteBpp = rawHeader.ColorMapEntrySize;
            break;
        default:
            return WINCODEC_ERR_BADHEADER;
        }
    }
    else
    {
        PaletteEntries = 0;
        Palette = std::vector<BYTE>(0);
    }

    ImageWidth = rawHeader.ImageWidth;
    ImageHeight = rawHeader.ImageHeight;

    switch (rawHeader.PixelDepth)
    {
    case 8:
    case 16:
    case 24:
    case 32:
        BitsPerPixel = rawHeader.PixelDepth;
        break;
    default:
        return WINCODEC_ERR_BADHEADER;
    }

    AlphaBits = rawHeader.ImageDescriptor & 0x0F;

    PreMultipliedAlpha = false;

    ImageOrigin = static_cast<TgaImageOrigin>((rawHeader.ImageDescriptor >> 4) & 0x03);

    ImageDataOffset = sizeof(TgaRawHeader);

    // skip Image ID if present

    if (rawHeader.IDLength != 0)
    {
        LARGE_INTEGER position;

        position.QuadPart = rawHeader.IDLength;

        hr = stream->Seek(position, STREAM_SEEK_CUR, NULL);

        if (FAILED(hr))
            return WINCODEC_ERR_STREAMNOTAVAILABLE;

        ImageDataOffset += rawHeader.IDLength;
    }

    // read color map if present

    if (rawHeader.ColorMapType == 1)
    {
        UINT paletteSize = rawHeader.ColorMapLength * ((rawHeader.ColorMapEntrySize + 7) / 8);

        Palette.resize(paletteSize);

        hr = stream->Read(Palette.data(), paletteSize, &bytesRead);

        if (FAILED(hr))
            return WINCODEC_ERR_STREAMREAD;
        if (bytesRead != paletteSize)
            return WINCODEC_ERR_BADHEADER;

        ImageDataOffset += paletteSize;
    }

    LARGE_INTEGER position;

    position.QuadPart = -(LONGLONG)sizeof(TgaRawFooter);

    hr = stream->Seek(position, STREAM_SEEK_END, NULL);

    if (FAILED(hr))
        goto skipFooter;

    TgaRawFooter rawFooter;

    hr = stream->Read(&rawFooter, sizeof(rawFooter), &bytesRead);

    if (FAILED(hr) || bytesRead != sizeof(rawFooter))
        goto skipFooter;

    if (memcmp(rawFooter.Signature, "TRUEVISION-XFILE.", sizeof(rawFooter.Signature)) != 0)
        goto skipFooter;

    if (rawFooter.ExtensionAreaOffset == 0)
        goto skipFooter;

    position.QuadPart = rawFooter.ExtensionAreaOffset;

    hr = stream->Seek(position, STREAM_SEEK_SET, NULL);

    if (FAILED(hr))
        goto skipFooter;

    TgaExtensionArea extensionArea;

    hr = stream->Read(&extensionArea, sizeof(extensionArea), &bytesRead);

    if (FAILED(hr) || bytesRead != sizeof(extensionArea))
        goto skipFooter;

    if (extensionArea.ExtensionSize != sizeof(TgaExtensionArea))
        goto skipFooter;

    PreMultipliedAlpha = BitsPerPixel == 32 && extensionArea.AttributesType == 4;

    skipFooter:

    return S_OK;
}