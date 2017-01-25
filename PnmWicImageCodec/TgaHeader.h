#pragma once

#pragma pack(push, 1)

struct TgaRawHeader
{
    BYTE IDLength;
    BYTE ColorMapType;
    BYTE ImageType;
    USHORT ColorMapFirstEntryIndex;
    USHORT ColorMapLength;
    BYTE ColorMapEntrySize;
    USHORT XOrigin;
    USHORT YOrigin;
    USHORT ImageWidth;
    USHORT ImageHeight;
    BYTE PixelDepth;
    BYTE ImageDescriptor;
};

struct TgaRawFooter
{
    UINT ExtensionAreaOffset;
    UINT DeveloperDirectoryOffset;
    char Signature[18];
};

struct TgaExtensionArea
{
    USHORT ExtensionSize;
    char AuthorName[41];
    char AuthorComments[324];
    USHORT DateTimeStamp[6];
    char JobName[41];
    USHORT JobTime[3];
    char SoftwareID[41];
    BYTE SoftwareVersion[3];
    UINT KeyColor;
    USHORT PixelAspectRatio[2];
    USHORT GammeValue[2];
    UINT ColorCorrectionOffset;
    UINT PostageStampOffset;
    UINT ScanLineOffset;
    BYTE AttributesType;
};

#pragma pack(pop)

enum class TgaImageType
{
    NoImage,
    Indexed,
    RGB,
    Grayscale
};

enum class TgaCompression
{
    Uncompressed,
    RunLengthEncoded
};

enum class TgaImageOrigin
{
    BottomLeft,
    BottomRight,
    TopLeft,
    TopRight
};

struct TgaHeader
{
    TgaImageType ImageType;
    TgaCompression Compression;
    USHORT ImageWidth;
    USHORT ImageHeight;
    BYTE BitsPerPixel;
    BYTE AlphaBits;
    bool PreMultipliedAlpha;
    USHORT PaletteEntries;
    USHORT PaletteFirstEntryIndex;
    BYTE PaletteBpp;
    std::vector<BYTE> Palette;
    TgaImageOrigin ImageOrigin;
    UINT ImageDataOffset;    
    
	HRESULT Parse(IStream *stream);
};

