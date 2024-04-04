#ifndef BUFFER_H
#define BUFFER_H

#include <stdint.h>
#include <stdbool.h>

typedef struct
{
    uint8_t* data;
    uint16_t len;
    uint16_t count;
    uint16_t pos;
    uint8_t byte;
} Buffer_t;

void Buffer_Init(Buffer_t* buffer, uint8_t *data, uint16_t len);
void Buffer_Append(Buffer_t* buffer);
void Buffer_AppendArray(Buffer_t* buffer, const uint8_t* array, uint16_t len);
void Buffer_AppendInteger(Buffer_t* buffer, uint32_t value, uint8_t decimals, bool leadingZeros);
int16_t Buffer_Find(const Buffer_t* buffer, const uint8_t* what, uint16_t len);
void Buffer_Shift(Buffer_t* buffer, uint16_t len);
void Buffer_Clear(Buffer_t* buffer);

#endif //BUFFER_H
