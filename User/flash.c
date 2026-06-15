/*
 * flash.c
 *
 *  Created on: 2024Äê11ÔÂ1ÈÕ
 *      Author: Reed
 */

#include "debug.h"
#include "ch32v10x_flash.h"
#include "flash.h"

/* Global define */
typedef enum
{
    FAILED = 0,
    PASSED = !FAILED
} TestStatus;
#define PAGE_WRITE_START_ADDR          ((uint32_t)0x0800F000) /* Start from 60K */
#define PAGE_WRITE_END_ADDR            ((uint32_t)0x08010000) /* End at 63K */
#define FLASH_PAGE_SIZE                1024
#define FLASH_PAGES_TO_BE_PROTECTED    FLASH_WRProt_Pages60to63

/* Global Variable */
uint32_t              EraseCounter = 0x0, Address = 0x0;
uint16_t              Data = 0xAAAA;
uint32_t              WRPR_Value = 0xFFFFFFFF, ProtectedPages = 0x0;
uint32_t              NbrOfPage;
volatile FLASH_Status FLASHStatus = FLASH_COMPLETE;
volatile TestStatus MemoryProgramStatus = PASSED;
volatile TestStatus MemoryEraseStatus = PASSED;

#define Fadr    (0x0800E000)
#define Fsize   ((((128*4))>>2))
u32 buf[Fsize];

/*********************************************************************
 * @fn      main
 *
 * @brief   Main program.
 *
 * @return  none
 */
//#define FLASH_BLOCK_SIZE (128)
#define FADDR (0x0800F000)
#define FLASH_ADDR_MAX (0x8010000)
#define FLASH_BLOCK_SIZE (0x080)
#define FLASH_BLOCK_MASK (0x7f)
uint8_t flash_buffer[FLASH_BLOCK_SIZE];
uint8_t write_flash(uint32_t addr,uint8_t* data,uint32_t size)
{
    addr+=FADDR;
    if(addr+size>=FLASH_ADDR_MAX)return 1;
    FLASH_Status s;
    //size = i32_min(size, FLASH_BLOCK_SIZE);
    uint32_t addr_s = addr&~(FLASH_BLOCK_MASK);
    uint32_t addr_e = (addr+size)&~(FLASH_BLOCK_MASK);
    uint32_t es = (size/FLASH_BLOCK_SIZE)+((size%FLASH_BLOCK_SIZE)?1:0);
    s = FLASH_ROM_ERASE(addr,es * FLASH_BLOCK_SIZE);
    if(s!=FLASH_COMPLETE)
        return 2;
    s=FLASH_ROM_WRITE(addr,data,es*FLASH_BLOCK_SIZE);
    if(s!=FLASH_COMPLETE)
        return 3;
    return 0;
}
uint8_t read_flash(uint32_t addr,uint8_t* data,uint32_t size)
{
    addr+=FADDR;
    if(addr+size>=FLASH_ADDR_MAX)return 1;
    if(!data)return 2;
    //memset(buf,0,FLASH_BLOCK_SIZE);
    memcpy(data,addr,size);
    return 0;
}
