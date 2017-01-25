#pragma once

#include "BufferedStreamReader.h"

enum class PnmType : BYTE
{
    Bitmap,
    Graymap,
    Pixmap
};

struct PnmHeader
{
    PnmType PnmType;
	bool AsciiFormat;
	UINT ImageWidth;
	UINT ImageHeight;
    USHORT MaxColorValue;
    
	HRESULT Parse(BufferedStreamReader &streamReader);
};

