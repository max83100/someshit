#include <string.h>

#include "buffer.h"

static const uint32_t powArray[8] = {1, 10, 100, 1000, 10000, 100000, 1000000, 10000000};

void Buffer_Init(Buffer_t* buffer, uint8_t *data, uint16_t len)
{
    buffer->data = data;
    buffer->len = len;
    buffer->count = 0;
    buffer->pos = 0;
}

void Buffer_Append(Buffer_t* buffer)
{
    if (buffer->count == buffer->len)
        return;
    buffer->data[buffer->count++] = (buffer->byte >= 0x61 && buffer->byte <= 0x7A) ? (buffer->byte - 0x20) : buffer->byte;
}

int16_t Buffer_Find(const Buffer_t* buffer, const uint8_t* what, uint16_t len)
{
    int16_t result = -1;
    if (len > buffer->count)
        return result;
    for(uint16_t z=0;z<=buffer->count-len;z++)
    {
        if (!memcmp(&buffer->data[z], what, len))
        {
            result = z;
            break;
        }
    }
    return result;
}

void Buffer_Shift(Buffer_t* buffer, uint16_t len)
{
    if (len >= buffer->count)
        Buffer_Clear(buffer);
    else
    {
        memmove(buffer->data, &buffer->data[len], buffer->count-len);
        buffer->count -= len;
        if (len >= buffer->pos)
            buffer->pos = 0;
        else
            buffer->pos -= len;
    }
}

void Buffer_AppendArray(Buffer_t* buffer, const uint8_t* array, uint16_t len)
{
    if (buffer->pos)
        Buffer_Shift(buffer, buffer->pos);
    if (buffer->count+len > buffer->len)
        len = buffer->len - buffer->count;
    memcpy(&buffer->data[buffer->count], array, len);
    buffer->count += len;
}

void Buffer_AppendInteger(Buffer_t* buffer, uint32_t value, uint8_t decimals, bool leadingZeros)
{
    if (decimals > 8)
        decimals = 8;
    bool start = false;
    for(int8_t z=decimals-1;z>=0;z--)
    {
        uint8_t sym = (value/powArray[z])%10;
        if (sym || start || leadingZeros)
        {
            buffer->byte = sym+'0';
            Buffer_Append(buffer);
            start = true;
        }
    }
}

inline void Buffer_Clear(Buffer_t* buffer)
{
    buffer->count = buffer->pos = 0;
}
