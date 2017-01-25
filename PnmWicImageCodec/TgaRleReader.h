#pragma once

#include "ComPtr.h"

struct TgaRleContext
{
    ULARGE_INTEGER StreamPosition;
    BYTE RemainingPixelsFromLastPacket;
    bool LastPacketWasRunLength;
    BYTE LastRunLengthPacketPixel[4];
};

class TgaRleReader
{
public:
    TgaRleReader(IStream *stream, int bitsPerPixel);
    ~TgaRleReader();

    HRESULT Read(BYTE *pixels, UINT pixelCount);
    HRESULT Skip(UINT pixelCount);
    HRESULT GetContext(TgaRleContext *context) const;
    HRESULT SetContext(const TgaRleContext &context);
    void Reset();

private:
    ComPtr<IStream> stream;
    int bytesPerPixel;

    BYTE *lastRunLengthPacketPixel;
    bool lastPacketWasRunLength;
    UINT remainingPixelsFromLastPacket;
};