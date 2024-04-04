#ifndef FLASH_H
#define FLASH_H

#include <stdbool.h>
#include <stdint.h>

void Flash_Init(void);
bool Flash_Read(void* buffer, uint32_t offset, uint32_t size);
bool Flash_Write(const void* buffer, uint32_t offset, uint32_t size);

#endif
