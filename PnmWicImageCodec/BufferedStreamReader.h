#pragma once

#include "ComPtr.h"

class BufferedStreamReader
{
public:
	BufferedStreamReader();
	~BufferedStreamReader();

	HRESULT Initialize(IStream *stream);

	HRESULT ReadChar(char *c);
	HRESULT SkipLine();
	HRESULT ReadString(char *str, ULONG maxCount);
	HRESULT ReadInt(int *i);
    HRESULT ReadIntSlow(int *i);
	HRESULT ReadBytes(BYTE *b, ULONG count, ULONG *bytesRead);

	HRESULT GetPosition(ULARGE_INTEGER *streamPosition) const;
	HRESULT Seek(LARGE_INTEGER move, DWORD origin);

private:
    HRESULT RefillBuffer();

	ComPtr<IStream> stream;	
	BYTE *buffer;
	UINT bufferSize;
	UINT position;

	static const UINT MAX_BUFFER_SIZE = 65536;
};

