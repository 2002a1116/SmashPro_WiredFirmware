/*
 * flash.c
 *
 *  Created on: 2024Äê11ÔÂ1ÈÕ
 *      Author: Reed
 */

#include "debug.h"

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
#define FLASH_BLOCK_SIZE (0x100)
/*
static uint8_t flash_buf[FLASH_BLOCK_SIZE];
uint8_t _flash_write(uint8_t id){
    if(id>3)return 1;
    FLASH_Status s;
    s = FLASH_ROM_ERASE(FADDR+id*FLASH_BLOCK_SIZE, FLASH_BLOCK_SIZE);
    if(s != FLASH_COMPLETE)
    {
        ////printf("check FLASH_ADR_RANGE_ERROR FLASH_ALIGN_ERROR or FLASH_OP_RANGE_ERROR\r\n");
        return 2;
    }
    s=FLASH_ROM_WRITE(FADDR+id*FLASH_BLOCK_SIZE,flash_buf , FLASH_BLOCK_SIZE);
    if(s!=FLASH_COMPLETE){
        return 3;
    }
    return 0;
}
uint8_t flash_write(uint8_t id,uint8_t* data,uint8_t len)//length==128
{
    if(!data)return -1;
    if(len>FLASH_BLOCK_SIZE)return -2;
    memset(flash_buf,0,FLASH_BLOCK_SIZE);
    memcpy(flash_buf,data,len);
    return _flash_write(id);
}
uint8_t* get_raw_flash_buf()
{
    return flash_buf;
}
uint8_t raw_flash_write(uint8_t id){
    return _flash_write(id);
}
uint8_t flash_read(uint8_t id,uint8_t* data,uint8_t len)
{
    if(id>3)return 1;
    if(!data)return -1;
    if(len>FLASH_BLOCK_SIZE)return -2;
    //memset(buf,0,FLASH_BLOCK_SIZE);
    memcpy(data,FADDR+(id*FLASH_BLOCK_SIZE),len);
    return 0;
}
*/
uint8_t flash_buffer[FLASH_BLOCK_SIZE];
uint8_t write_flash(uint32_t addr,uint8_t* data,uint32_t size)
{
    addr+=FADDR;
    if(addr+size>=FLASH_ADDR_MAX)return 1;
    FLASH_Status s;
    s = FLASH_ROM_ERASE(addr,FLASH_BLOCK_SIZE);
    if(s!=FLASH_COMPLETE)
        return 2;
    memset(flash_buffer,-1,FLASH_BLOCK_SIZE);
    memcpy(flash_buffer,data,size);
    s=FLASH_ROM_WRITE(addr,flash_buffer,FLASH_BLOCK_SIZE);
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
