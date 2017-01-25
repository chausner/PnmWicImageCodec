#pragma once

#include "PnmHeader.h"
#include "TgaHeader.h"

UINT GetBitsPerPixelFromHeader(const PnmHeader &pnmHeader);
REFWICPixelFormatGUID GetWicPixelFormatFromHeader(const PnmHeader &pnmHeader);
REFWICPixelFormatGUID GetWicPixelFormatFromTgaHeader(const TgaHeader &tgaHeader);