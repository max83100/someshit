#include <string.h>

#include "main.h"
#include "flash.h"
#include "error_handler.h"

SPI_HandleTypeDef hspi1;

#define CF_SIZE 524288
#define PAGE_SIZE 256
#define BLOCK_SIZE 4096
#define BLOCK_MACK (UINT32_MAX - BLOCK_SIZE + 1)
#define SPI_TIMEOUT 1000

static uint8_t dataBuffer[BLOCK_SIZE];

static void setCs(bool state);
static bool resetProtection(void);
static bool writeToBlock(const uint8_t* buffer, uint32_t offset, uint32_t size);
static bool eraseBlock(uint32_t address);
static bool writePage(const uint8_t* buffer, uint32_t address);
static void shortDelay(void);

void Flash_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    setCs(false);

    GPIO_InitStruct.Pin = CS_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(CS_GPIO_Port, &GPIO_InitStruct);
    
    hspi1.Instance = SPI1;
    hspi1.Init.Mode = SPI_MODE_MASTER;
    hspi1.Init.Direction = SPI_DIRECTION_2LINES;
    hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
    hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
    hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
    hspi1.Init.NSS = SPI_NSS_SOFT;
    hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_64;
    hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
    hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
    hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
    hspi1.Init.CRCPolynomial = 10;
    if (HAL_SPI_Init(&hspi1) != HAL_OK)
        Error_Handler();
    
    resetProtection();
    
    setCs(true);
    HAL_SPI_Transmit(&hspi1, (uint8_t[]){0x01, 0x00}, 2, SPI_TIMEOUT);  
    setCs(false);
}

bool Flash_Read(void* buffer, uint32_t offset, uint32_t size)
{
    if (!buffer || offset>=CF_SIZE || size>CF_SIZE || offset+size>CF_SIZE)
        return false;

    bool state = true;
    setCs(true);
    do
    {
        if (HAL_SPI_Transmit(&hspi1, (uint8_t[]){0x03, offset>>16, offset>>8, offset}, 4, SPI_TIMEOUT) != HAL_OK)
        {
            state = false;
            break;
        }
        state = HAL_SPI_Receive(&hspi1, buffer, size, SPI_TIMEOUT) == HAL_OK;
    }while(false);
    setCs(false);
    
    return state;
}

bool Flash_Write(const void* buffer, uint32_t offset, uint32_t size)
{
    if (!buffer || offset>=CF_SIZE || size>CF_SIZE || offset+size>CF_SIZE)
        return false;

    while(size)
    {
        uint32_t len = size;
        if (len > BLOCK_SIZE)
            len = BLOCK_SIZE;
        if (offset%BLOCK_SIZE)
            len = BLOCK_SIZE-(offset%BLOCK_SIZE);
        if (len > size)
            len = size;
        if (!writeToBlock(buffer, offset, len))
            return false;
        buffer = (uint8_t*)buffer + len;
        offset+=len;
        size-=len;
    }
    return true;
}

static inline void setCs(bool state)
{
    shortDelay();
    HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, state ? GPIO_PIN_RESET : GPIO_PIN_SET);
    shortDelay();
}

static inline void shortDelay(void)
{
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();
}

static bool resetProtection(void)
{
    setCs(true);
    bool result = HAL_SPI_Transmit(&hspi1, (uint8_t[]){0x06}, 1, SPI_TIMEOUT) == HAL_OK;  
    setCs(false);
    return result;
}

static bool writeToBlock(const uint8_t* buffer, uint32_t offset, uint32_t size)
{
    uint32_t blockStart = offset & BLOCK_MACK;
    if (size != BLOCK_SIZE)
    {
        if (!Flash_Read(dataBuffer, blockStart , BLOCK_SIZE))
            return false;
    }
    memcpy(&dataBuffer[offset & ~BLOCK_MACK], buffer, size);
    
    if (!eraseBlock(blockStart))
        return false;
    
    uint8_t count = BLOCK_SIZE/PAGE_SIZE;
    for(uint8_t z=0;z<count;z++)
    {
        if (!writePage(&dataBuffer[z*PAGE_SIZE], blockStart+z*PAGE_SIZE))
            return false;
    }
    return true;
}

static bool eraseBlock(uint32_t address)
{
    if (!resetProtection())
        return false;
    
    setCs(true);
    bool result = HAL_SPI_Transmit(&hspi1, (uint8_t[]){0x20, address>>16, address>>8, address}, 4, SPI_TIMEOUT) == HAL_OK;
    setCs(false);
    HAL_Delay(50);
    return result;
}

static bool writePage(const uint8_t* buffer, uint32_t address)
{
    bool state = true;
    if (!resetProtection())
        return false;
    setCs(true);
    do
    {
        if (HAL_SPI_Transmit(&hspi1, (uint8_t[]){0x02, address>>16, address>>8, address}, 4, SPI_TIMEOUT) != HAL_OK)
        {
            state = false;
            break;
        }
        state = HAL_SPI_Transmit(&hspi1, buffer, PAGE_SIZE, SPI_TIMEOUT) == HAL_OK;
    }while(false);
    setCs(false);
    HAL_Delay(5);
    return state;
}
