#include "precomp.h"
#include "TgaRleReader.h"

TgaRleReader::TgaRleReader(IStream *stream, int bitsPerPixel) : stream(stream)
{
    bytesPerPixel = (bitsPerPixel + 7) / 8;

    lastRunLengthPacketPixel = new BYTE[bytesPerPixel];
    remainingPixelsFromLastPacket = 0;
}

TgaRleReader::~TgaRleReader()
{
    delete[] lastRunLengthPacketPixel;
}

HRESULT TgaRleReader::Read(BYTE *pixels, UINT pixelCount)
{
    BYTE *p = pixels;

    UINT pixelsRead = 0;

    if (remainingPixelsFromLastPacket > 0)
    {
        if (lastPacketWasRunLength)
        {
            for (UINT i = 0; i < min(remainingPixelsFromLastPacket, pixelCount); i++)
                memcpy(p + i * bytesPerPixel, lastRunLengthPacketPixel, bytesPerPixel);
        }
        else
        {
            ULONG bytesRead;

            HRESULT hr = stream->Read(p, min(remainingPixelsFromLastPacket, pixelCount) * bytesPerPixel, &bytesRead);

            if (FAILED(hr))
                return WINCODEC_ERR_STREAMREAD;
            if (bytesRead != min(remainingPixelsFromLastPacket, pixelCount) * bytesPerPixel)
                return WINCODEC_ERR_BADIMAGE;
        }

        p += min(remainingPixelsFromLastPacket, pixelCount) * bytesPerPixel;
        pixelsRead += min(remainingPixelsFromLastPacket, pixelCount);
        remainingPixelsFromLastPacket -= min(remainingPixelsFromLastPacket, pixelCount);
    }

    while (pixelsRead < pixelCount)
    {
        BYTE b;

        ULONG bytesRead;

        HRESULT hr = stream->Read(&b, 1, &bytesRead);

        if (FAILED(hr))
            return WINCODEC_ERR_STREAMREAD;
        if (bytesRead != 1)
            return WINCODEC_ERR_BADIMAGE;

        UINT packetLength = (b & 0x7F) + 1;

        if (b < 0x80)
        {
            hr = stream->Read(p, min(packetLength, pixelCount - pixelsRead) * bytesPerPixel, &bytesRead);

            if (FAILED(hr))
                return WINCODEC_ERR_STREAMREAD;
            if (bytesRead != min(packetLength, pixelCount - pixelsRead) * bytesPerPixel)
                return WINCODEC_ERR_BADIMAGE;

            if (packetLength > pixelCount - pixelsRead)
            {
                lastPacketWasRunLength = false;
                remainingPixelsFromLastPacket = packetLength - (pixelCount - pixelsRead);
            }
        }
        else
        {
            hr = stream->Read(p, bytesPerPixel, &bytesRead);

            if (FAILED(hr))
                return WINCODEC_ERR_STREAMREAD;
            if (bytesRead != bytesPerPixel)
                return WINCODEC_ERR_BADIMAGE;

            for (UINT i = 1; i < min(packetLength, pixelCount - pixelsRead); i++)
                memcpy(p + i * bytesPerPixel, p, bytesPerPixel);

            if (packetLength > pixelCount - pixelsRead)
            {
                memcpy(lastRunLengthPacketPixel, p, bytesPerPixel);
                lastPacketWasRunLength = true;
                remainingPixelsFromLastPacket = packetLength - (pixelCount - pixelsRead);
            }
        }

        p += min(packetLength, pixelCount - pixelsRead) * bytesPerPixel;
        pixelsRead += min(packetLength, pixelCount - pixelsRead);
    }

    return S_OK;
}

HRESULT TgaRleReader::Skip(UINT pixelCount)
{
    // dummy implementation
    /*std::vector<BYTE> pixels(pixelCount * bytesPerPixel);

    HRESULT hr = Read(pixels.data(), pixelCount);

    return hr;*/

    UINT pixelsRead = 0;

    if (remainingPixelsFromLastPacket > 0)
    {
        if (!lastPacketWasRunLength)
        {
            LARGE_INTEGER position;

            position.QuadPart = min(remainingPixelsFromLastPacket, pixelCount) * bytesPerPixel;

            HRESULT hr = stream->Seek(position, STREAM_SEEK_CUR, NULL);

            if (FAILED(hr))
                return WINCODEC_ERR_STREAMNOTAVAILABLE;
        }

        pixelsRead += min(remainingPixelsFromLastPacket, pixelCount);
        remainingPixelsFromLastPacket -= min(remainingPixelsFromLastPacket, pixelCount);
    }

    while (pixelsRead < pixelCount)
    {
        BYTE b;

        ULONG bytesRead;

        HRESULT hr = stream->Read(&b, 1, &bytesRead);

        if (FAILED(hr))
            return WINCODEC_ERR_STREAMREAD;
        if (bytesRead != 1)
            return WINCODEC_ERR_BADIMAGE;

        UINT packetLength = (b & 0x7F) + 1;

        if (b < 0x80)
        {
            LARGE_INTEGER position;

            position.QuadPart = min(packetLength, pixelCount - pixelsRead) * bytesPerPixel;

            hr = stream->Seek(position, STREAM_SEEK_CUR, NULL);

            if (FAILED(hr))
                return WINCODEC_ERR_STREAMNOTAVAILABLE;

            if (packetLength > pixelCount - pixelsRead)
            {
                lastPacketWasRunLength = false;
                remainingPixelsFromLastPacket = packetLength - (pixelCount - pixelsRead);
            }
        }
        else
        {
            if (packetLength > pixelCount - pixelsRead)
            {
                hr = stream->Read(lastRunLengthPacketPixel, bytesPerPixel, &bytesRead);

                if (FAILED(hr))
                    return WINCODEC_ERR_STREAMREAD;
                if (bytesRead != bytesPerPixel)
                    return WINCODEC_ERR_BADIMAGE;

                lastPacketWasRunLength = true;
                remainingPixelsFromLastPacket = packetLength - (pixelCount - pixelsRead);
            }
            else
            {
                LARGE_INTEGER position;

                position.QuadPart = bytesPerPixel;

                hr = stream->Seek(position, STREAM_SEEK_CUR, NULL);

                if (FAILED(hr))
                    return WINCODEC_ERR_STREAMNOTAVAILABLE;
            }
        }

        pixelsRead += min(packetLength, pixelCount - pixelsRead);
    }

    return S_OK;
}

HRESULT TgaRleReader::GetContext(TgaRleContext *context) const
{
    HRESULT hr = stream->Seek({ 0 }, STREAM_SEEK_CUR, &context->StreamPosition);

    if (FAILED(hr))
        return WINCODEC_ERR_STREAMNOTAVAILABLE;

    context->RemainingPixelsFromLastPacket = remainingPixelsFromLastPacket;
    context->LastPacketWasRunLength = lastPacketWasRunLength;
    memcpy(context->LastRunLengthPacketPixel, lastRunLengthPacketPixel, bytesPerPixel);

    return S_OK;
}

HRESULT TgaRleReader::SetContext(const TgaRleContext &context)
{
    HRESULT hr = stream->Seek(*(LARGE_INTEGER*)&context.StreamPosition, STREAM_SEEK_SET, NULL);

    if (FAILED(hr))
        return WINCODEC_ERR_STREAMNOTAVAILABLE;

    remainingPixelsFromLastPacket = context.RemainingPixelsFromLastPacket;
    lastPacketWasRunLength = context.LastPacketWasRunLength;
    memcpy(lastRunLengthPacketPixel, context.LastRunLengthPacketPixel, bytesPerPixel);

    return S_OK;
}

void TgaRleReader::Reset()
{
    remainingPixelsFromLastPacket = 0;
}
