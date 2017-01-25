#include "precomp.h"
#include "Utilities.h"

UINT GetBitsPerPixelFromHeader(const PnmHeader &pnmHeader)
{
    switch (pnmHeader.PnmType)
    {
    case PnmType::Bitmap:
        return 1;
    case PnmType::Graymap:
        return pnmHeader.MaxColorValue <= 255 ? 8 : 16;
    case PnmType::Pixmap:
        return pnmHeader.MaxColorValue <= 255 ? 24 : 48;
    default:
        return 0;
    }
}

REFWICPixelFormatGUID GetWicPixelFormatFromHeader(const PnmHeader &pnmHeader)
{
    switch (pnmHeader.PnmType)
    {
    case PnmType::Bitmap:
        return GUID_WICPixelFormatBlackWhite;
    case PnmType::Graymap:
        return pnmHeader.MaxColorValue <= 255 ? GUID_WICPixelFormat8bppGray : GUID_WICPixelFormat16bppGray;        
    case PnmType::Pixmap:
        return pnmHeader.MaxColorValue <= 255 ? GUID_WICPixelFormat24bppRGB : GUID_WICPixelFormat48bppRGB;
    default:
        return GUID_WICPixelFormatUndefined;
    }
}

REFWICPixelFormatGUID GetWicPixelFormatFromTgaHeader(const TgaHeader &tgaHeader)
{
    switch (tgaHeader.ImageType)
    {
    case TgaImageType::Indexed:
        switch (tgaHeader.BitsPerPixel)
        {
        case 1:
            return GUID_WICPixelFormat1bppIndexed;
        case 2:
            return GUID_WICPixelFormat2bppIndexed;
        case 4:
            return GUID_WICPixelFormat4bppIndexed;
        case 8:
            return GUID_WICPixelFormat8bppIndexed;
        }
        break;
    case TgaImageType::RGB:
        switch (tgaHeader.BitsPerPixel)
        {
        case 16:
            return GUID_WICPixelFormat16bppBGR555;
        case 24:
            return GUID_WICPixelFormat24bppBGR;
        case 32:
            return tgaHeader.PreMultipliedAlpha ? GUID_WICPixelFormat32bppPBGRA : GUID_WICPixelFormat32bppBGRA;
        }
        break;
    case TgaImageType::Grayscale:
        switch (tgaHeader.BitsPerPixel)
        {
        case 1:
            return GUID_WICPixelFormatBlackWhite;
        case 2:
            return GUID_WICPixelFormat2bppGray;
        case 4:
            return GUID_WICPixelFormat4bppGray;
        case 8:
            return GUID_WICPixelFormat8bppGray;
        case 16:
            return GUID_WICPixelFormat16bppGray;
        }
        break;
    }

    return GUID_WICPixelFormatUndefined;
}