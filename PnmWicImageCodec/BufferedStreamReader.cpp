#include "precomp.h"
#include "BufferedStreamReader.h"

BufferedStreamReader::BufferedStreamReader()
    : stream(NULL), buffer(NULL), bufferSize(0), position(0)
{	
}

BufferedStreamReader::~BufferedStreamReader()
{
    if (buffer)
    {
        delete[] buffer;
    }
}

HRESULT BufferedStreamReader::Initialize(IStream *stream)
{
    this->stream = stream;

    if (!buffer)
    {
        buffer = new (std::nothrow) BYTE[MAX_BUFFER_SIZE];

        if (!buffer)
            return E_OUTOFMEMORY;
    }

    ULONG read;

    HRESULT hr = stream->Read(buffer, MAX_BUFFER_SIZE, &read);

    if (FAILED(hr))
        return WINCODEC_ERR_STREAMREAD;

    bufferSize = read;
    position = 0;

    return S_OK;
}

HRESULT BufferedStreamReader::RefillBuffer()
{
    memcpy(buffer, buffer + position, bufferSize - position);

    position = bufferSize - position;

    ULONG read;

    HRESULT hr = stream->Read(buffer + position, bufferSize - position, &read);

    if (FAILED(hr))
        return WINCODEC_ERR_STREAMREAD;

    bufferSize = position + read;
    position = 0;

    return hr;
}

HRESULT BufferedStreamReader::ReadChar(char *c)
{
    if (position + sizeof(char) > bufferSize)	
        if (bufferSize == MAX_BUFFER_SIZE)
        {
            HRESULT hr = RefillBuffer();

            if (FAILED(hr))
                return hr;

            if (bufferSize < sizeof(char))
                return S_FALSE;
        }
        else
            return S_FALSE;	

    *c = buffer[position];

    position += sizeof(char);

    return S_OK;
}

HRESULT BufferedStreamReader::SkipLine()
{
    while (true)
    {
        char c;

        HRESULT hr = ReadChar(&c);

        if (hr != S_OK)
            return hr;

        if (c == 0x0A)
            return S_OK;
    }
}

HRESULT BufferedStreamReader::ReadString(char *str, ULONG maxCount)
{
    if (maxCount == 0)
        return E_INVALIDARG;

    ULONG charsRead = 0;

    while (true)
    {
        char c;

        HRESULT hr = ReadChar(&c);

        if (FAILED(hr))
            return hr;

        if (hr == S_FALSE)        
            if (charsRead == 0)            
                return S_FALSE;            
            else
            {
                *str = 0x00;
                return S_OK;
            }        

        if (isspace(c))
            if (charsRead == 0)
                continue;
            else
            {
                *str = 0x00;
                return S_OK;
            }
        
        if (c == '#')		
            if (charsRead == 0)
            {
                hr = SkipLine();
            
                if (FAILED(hr))
                    return hr;				

                if (hr == S_FALSE)
                {
                    *str = 0x00;
                    return hr;
                }

                continue;
            }
            else
            {
                *str = 0x00;
                return S_OK;
            }		

        *str = c;			
        str++;
        charsRead++;
        
        if (charsRead == maxCount)
            return WINCODEC_ERR_BADSTREAMDATA;
    }
}

HRESULT BufferedStreamReader::ReadInt(int *i)
{
    UINT origPos = position;

    if (position == bufferSize)
    {
        return ReadIntSlow(i);
    }

    char c;

    while (isspace(c = buffer[position]))
    {
        position++;

        if (position == bufferSize)
        {
            position = origPos;
            return ReadIntSlow(i);
        }
    }

    int r = 0;

    while ((c = buffer[position]) >= '0' && c <= '9')
    {
        r = r * 10 + (c - '0');
        position++;

        if (position == bufferSize)
        {
            position = origPos;
            return ReadIntSlow(i);
        }
    }

    *i = r;

    return S_OK;
}

HRESULT BufferedStreamReader::ReadIntSlow(int *i)
{
    char str[12];

    HRESULT hr = ReadString(str, sizeof(str));

    if (hr != S_OK)
        return hr;

    *i = atoi(str);

    return S_OK;
}

HRESULT BufferedStreamReader::ReadBytes(BYTE *b, ULONG count, ULONG *bytesRead)
{
    ULONG remaining = count;

    while (true)
	{
		if (bufferSize - position >= remaining)
		{
			memcpy(b, buffer + position, remaining);    
            position += remaining;
            *bytesRead = count;
            return S_OK;
		}
		
		memcpy(b, buffer + position, bufferSize - position);  
		b += bufferSize - position;
		remaining -= bufferSize - position;
		position = bufferSize;
        
		HRESULT hr = RefillBuffer();

		if (FAILED(hr))
			return hr;		

        if (bufferSize == 0)
        {
            *bytesRead = count - remaining;
            return S_OK;
        }
	}
}

HRESULT BufferedStreamReader::GetPosition(ULARGE_INTEGER *streamPosition) const
{
    HRESULT hr = stream->Seek({ 0 }, STREAM_SEEK_CUR, streamPosition);

    if (FAILED(hr))
        return WINCODEC_ERR_STREAMNOTAVAILABLE;

    (*streamPosition).QuadPart -= bufferSize - position;

    return S_OK;
}

HRESULT BufferedStreamReader::Seek(LARGE_INTEGER move, DWORD origin)
{
    if (origin == STREAM_SEEK_CUR)
        move.QuadPart -= bufferSize - position;

	HRESULT hr = stream->Seek(move, origin, NULL);

	if (FAILED(hr))
		return WINCODEC_ERR_STREAMNOTAVAILABLE;

	ULONG read;

    hr = stream->Read(buffer, MAX_BUFFER_SIZE, &read);

    if (FAILED(hr))
        return WINCODEC_ERR_STREAMREAD;

    bufferSize = read;
    position = 0;

	return S_OK;
}