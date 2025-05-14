/*
 * uart.c
 *
 *  Created on: 2025Äê4ÔÂ24ÈÕ
 *      Author: Reed
 */

#include "debug.h"
#include "string.h"
#include "ch32v10x_usbfs_device.h"
#include "ring_buffer.h"
#include "ns_com.h"
#include "ns_com_mux.h"
#include "uart.h"
#include "global_api.h"
#include "spi.h"
#include "conf.h"

//void USART1_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
__attribute__ ((aligned(4))) uint8_t UART1_RxBuffer[DEF_UART1_BUF_SIZE];  // UART2 Rx Buffer

uint8_t uart_rx_rb_buf[UART_PKG_SIZE*UART_RINGBUFFER_PKG_CAP];
uint8_t uart_tx_rb_buf[UART_PKG_SIZE*UART_RINGBUFFER_PKG_CAP];
ring_buffer uart_rx_rb;
ring_buffer uart_tx_rb;
volatile uint16_t UART1_TimeOut;                                           // UART2 RX timeout flag
volatile uint8_t  UART1_Tx_Flag = 0;                                       // UART2 TX flag

volatile uint16_t UART1_RX_CurCnt = 0;                                     // UART2 DMA current remain count
volatile uint16_t UART1_RX_LastCnt = 0;                                    // UART2 DMA last remain count
volatile uint16_t UART1_Rx_RemainLen = 0;                                  // UART2 RX data remain len
volatile uint16_t UART1_Rx_Deal_Ptr = 0;                                   // UART2 RX data deal pointer

void UART1_DMA_Init( void )
{
    DMA_InitTypeDef DMA_InitStructure = {0};

   // RCC_AHBPeriphClockCmd( RCC_AHBPeriph_DMA1, ENABLE );

    /* UART2 Tx DMA initialization */
    DMA_Cmd( DMA1_Channel4, DISABLE );
    DMA_DeInit( DMA1_Channel4 );
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)(&USART1->DATAR);
    //DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)Data_Buffer;
    DMA_InitStructure.DMA_MemoryBaseAddr = NULL;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
    DMA_InitStructure.DMA_BufferSize = 0;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
    DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init( DMA1_Channel4, &DMA_InitStructure );

    /* UART2 Rx DMA initialization */
    DMA_Cmd( DMA1_Channel5, DISABLE );
    DMA_DeInit( DMA1_Channel5 );
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)UART1_RxBuffer;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
    DMA_InitStructure.DMA_BufferSize = DEF_UART1_BUF_SIZE;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;
    DMA_Init( DMA1_Channel5, &DMA_InitStructure );
    DMA_Cmd( DMA1_Channel5, ENABLE );
    USART_DMACmd(USART1, USART_DMAReq_Rx, ENABLE);
}


/*********************************************************************
 * @fn      UART2_Init
 *
 * @brief   UART2 DMA initialization
 *
 * @return  none
 */
void UART1_Init( void )
{
    GPIO_InitTypeDef  GPIO_InitStructure = {0};
    USART_InitTypeDef USART_InitStructure = {0};

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
    //GPIO_PinRemapConfig(AFIO_PCFR1_USART1_REMAP, ENABLE);
    //AFIO_PCFR1_USART1_REMAP=0;
    //RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

    /* UART2 GPIO Init */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    //GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* UART2 Init */
    USART_InitStructure.USART_BaudRate = UART_BAUD_RATE;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    USART_Init(USART1, &USART_InitStructure);

    USART_ClearFlag( USART1, USART_FLAG_TC );
    //USART_ITConfig(USART1, USART_IT_TC, ENABLE);
    USART_Cmd(USART1, ENABLE);

    ring_buffer_init(&uart_rx_rb, uart_rx_rb_buf, UART_RINGBUFFER_PKG_CAP, UART_PKG_SIZE);
    ring_buffer_init(&uart_tx_rb, uart_tx_rb_buf, UART_RINGBUFFER_PKG_CAP, UART_PKG_SIZE);

}

void UART1_DMA_Tx(uint8_t *pbuf,uint16_t len)
{
    USART_ClearFlag(USART1, USART_FLAG_TC);
    DMA_Cmd( DMA1_Channel4, DISABLE );
    DMA1_Channel4->MADDR = (uint32_t)pbuf;
    DMA1_Channel4->CNTR = (uint32_t)len;
    DMA_Cmd( DMA1_Channel4, ENABLE );
    USART_DMACmd(USART1, USART_DMAReq_Tx, ENABLE);
}

/*********************************************************************
 * @fn      UART2_Tx_Service
 *
 * @brief   UART2 tx service routine that sends the data received by
 *          USB-HID via uart2.
 *
 * @return  none
 */
void UART1_Tx_Service( void )
{
    uint16_t pkg_len = 0;
    uint8_t *pbuf;
    static uint8_t cnt=0,f=0;
    ////printf("tx service\r\n");
    if (UART1_Tx_Flag)
    {
        ////printf("tx flag status %d\r\n",USART_GetFlagStatus(USART1, USART_FLAG_TC));
        if (USART_GetFlagStatus(USART1, USART_FLAG_TC))                                  // Check whether uart2 has finished sending.
        {
            USART_ClearFlag(USART1, USART_FLAG_TC);
            USART_DMACmd(USART1, USART_DMAReq_Tx, DISABLE);
            UART1_Tx_Flag = 0;
        }
        /*if(DMA_GetFlagStatus(DMA1_FLAG_TC4)){
            USART_DMACmd(USART1, USART_DMAReq_Tx, DISABLE);
            UART1_Tx_Flag = 0;
        }*/
    }
    else
    {
        ////printf("txss %d\r\n",uart_tx_rb.capcity);
        if(uart_tx_rb.size){
            pbuf=&uart_tx_rb_buf[uart_tx_rb.top*UART_PKG_SIZE];
            if(uart_tx_rb.top>uart_tx_rb.end){
                pkg_len=uart_tx_rb.capcity-uart_tx_rb.top;
                uart_tx_rb.size=uart_tx_rb.end+1;
                uart_tx_rb.top=0;
            }else {
                pkg_len=uart_tx_rb.size;
                uart_tx_rb.top=uart_tx_rb.end+1;
                if(uart_tx_rb.top>=uart_tx_rb.capcity)
                    uart_tx_rb.top=0;
                uart_tx_rb.size=0;
            }
            pkg_len=pkg_len*UART_PKG_SIZE;
            uart_tx_rb.full=0;
            UART1_DMA_Tx( pbuf, pkg_len );
            UART1_Tx_Flag = 1;
            ////printf("send len %d\r\n",pkg_len);
        }
        ////printf("txsd %d\r\n",uart_tx_rb.capcity);
    }
}
int8_t rx_ptr=0;
uint8_t rx_filter_buf[UART_PKG_SIZE];
void UART1_Rx_Service( void )
{
    uint16_t u16_temp;
    UART1_RX_CurCnt = DMA_GetCurrDataCounter(DMA1_Channel5);
    ////printf("rx\r\n");// Get DMA remaining count
    if (UART1_RX_LastCnt != UART1_RX_CurCnt)
    {
        if (UART1_RX_LastCnt > UART1_RX_CurCnt)
        {
            u16_temp = UART1_RX_LastCnt - UART1_RX_CurCnt;
        }
        else
        {
            u16_temp = UART1_RX_LastCnt + ( DEF_UART1_BUF_SIZE - UART1_RX_CurCnt );
        }

        UART1_RX_LastCnt = UART1_RX_CurCnt;
        ////printf("u16temp :%d\r\n",u16_temp);
        if ((UART1_Rx_RemainLen + u16_temp) > DEF_UART1_BUF_SIZE )
        {
            UART1_Rx_RemainLen=DEF_UART1_BUF_SIZE;
            ////printf("remain len: %d ,temp : %d\r\n",UART1_Rx_RemainLen,u16_temp);
            ////printf("Uart2 RX_buffer overflow\n");                                           // overflow: New data overwrites old data
        }
        else
        {
            UART1_Rx_RemainLen += u16_temp;
        }
        UART1_TimeOut = 0;
    }
    ////printf("r2 %d\r\n",UART1_Rx_RemainLen);
    //if(UART1_Rx_RemainLen)trigger_hardfault();
    uint8_t max_uart_byte_recv_once=128;
    while(UART1_Rx_RemainLen&&max_uart_byte_recv_once--)
    {
        if(UART1_RxBuffer[UART1_Rx_Deal_Ptr]&UART_PKG_HEADER_MASK)//HEADER
        {
            rx_ptr=1;
            rx_filter_buf[0]=UART1_RxBuffer[UART1_Rx_Deal_Ptr];
        }
        else if(rx_ptr>0){//join a packet
                rx_filter_buf[rx_ptr++]=UART1_RxBuffer[UART1_Rx_Deal_Ptr];
                if(rx_ptr>=UART_PKG_SIZE){//recive one
                    rx_ptr=0;
                    if(!check_uart_pkt((uart_packet*)rx_filter_buf))//if check returns true,check fail,drop it
                    {
                        decode_uart_pkt((uart_packet*)rx_filter_buf);
                        ////printf("uart pkt decode typ %d\r\n",((uart_packet*)rx_filter_buf)->typ);
                        u16_temp=ring_buffer_push(&uart_rx_rb, rx_filter_buf, UART_PKG_SIZE, 0);
                        //if(u16_temp)
                            //printf("uart push error:%d\r\n",u16_temp);
                    }
                }
        }
        else {
            rx_ptr=0;
        }
        --UART1_Rx_RemainLen;
        ++UART1_Rx_Deal_Ptr;
        if (UART1_Rx_Deal_Ptr >= DEF_UART1_BUF_SIZE){
            UART1_Rx_Deal_Ptr = 0x00;
        }
    }
    ////printf("rlen:%d\r\n",UART1_Rx_RemainLen);
    ////printf("rx end\r\n");
}

void encode_uart_pkt(uart_packet* pkt){
    if(!pkt)return;
    /*pkt->high_bit=(pkt->arr[0]&UART_PKG_HEADER_MASK)>>7;
    pkt->high_bit|=(pkt->arr[1]&UART_PKG_HEADER_MASK)>>6;
    pkt->high_bit|=(pkt->arr[2]&UART_PKG_HEADER_MASK)>>5;
    ////printf("arr0 %d %d %d  high %d\r\n",pkt->arr[0],pkt->arr[1],pkt->arr[2],pkt->arr[2]&~UART_PKG_HEADER_MASK);
    pkt->load=pkt->load&UART_PKG_LOAD_MASK;
    ////printf("arr1 %d %d %d  high %d\r\n",pkt->arr[0],pkt->arr[1],pkt->arr[2],pkt->high_bit);
    pkt->cksum=pkt->header^pkt->data[0]^pkt->data[1]^pkt->data[2]^pkt->data[3];*/
    pkt->hb1=0;
    pkt->hb2=0;
    for(int i=0;i<7;++i){
        pkt->hb1|=(pkt->load[i]&UART_PKG_HEADER_MASK)>>(7^i);//equal high_bit>>7<<i
    }
    pkt->hb2|=(pkt->load[7]&UART_PKG_HEADER_MASK)>>(7);
    pkt->hb2|=(pkt->load[8]&UART_PKG_HEADER_MASK)>>(6);
    for(int i=0;i<9;++i)
        pkt->load[i]&=~UART_PKG_HEADER_MASK;//remove starter
    pkt->cksum=pkt->id;
    for(int i=0;i<11;++i)
        pkt->cksum^=pkt->data[i];
    pkt->_empty=0;
    pkt->_empty2=0;
    pkt->_empty3=0;
}
void decode_uart_pkt(uart_packet* pkt){
    if(!pkt)return;
    /*pkt->arr[0]|=UART_PKG_HEADER_MASK&(pkt->high_bit<<7);
    pkt->arr[1]|=UART_PKG_HEADER_MASK&(pkt->high_bit<<6);
    pkt->arr[2]|=UART_PKG_HEADER_MASK&(pkt->high_bit<<5);
    pkt->header&=~UART_PKG_HEADER_MASK;*/
    for(int i=0;i<7;++i){
        pkt->load[i]|=((pkt->hb1>>i)&1)<<7;
    }
    pkt->load[7]|=(pkt->hb2&1)<<7;
    pkt->load[8]|=(pkt->hb2&2)<<6;
}
uint8_t check_uart_pkt(uart_packet* pkt){//if ok return false aka 0
    if(!pkt)return -1;
    //return pkt->cksum!=(pkt->header^pkt->data[0]^pkt->data[1]^pkt->data[2]^pkt->data[3]^UART_PKG_HEADER_MASK);
    uint8_t res=pkt->id;
    for(int i=0;i<11;++i)
        res^=pkt->data[i];
    return res!=pkt->cksum;
}
static uart_packet send_buf;
uint8_t send_uart_pkt(uart_packet* pkt)
{
    if(!pkt)return -2;
    memcpy(&send_buf,pkt,UART_PKG_SIZE);
    encode_uart_pkt(&send_buf);
    send_buf.starter=1;
    return ring_buffer_push(&uart_tx_rb, (uint8_t*)&send_buf, UART_PKG_SIZE, 0);
}
uint8_t send_uart_large(uint8_t* buf,uint8_t len,uint8_t typ)
{
    uart_packet pkg;
    memset(&pkg,0,sizeof(uart_packet));
    uint8_t id=0,res=0;
    pkg.typ=typ;
    while(len>=UART_PKG_LOAD_LENGTH)
    {
        memcpy(pkg.load,buf,UART_PKG_LOAD_LENGTH);
        buf+=UART_PKG_LOAD_LENGTH;
        pkg.id=id++;
        //encode_uart_pkt(&pkg);
        res = send_uart_pkt(&pkg);
        if(res)
            return res;
    }
    if(len)
    {
        memset(&pkg,0,sizeof(uart_packet));
        memcpy(pkg.load,buf,len);
        pkg.id=id++;
        pkg.typ=typ;
        //encode_uart_pkt(&pkg);
        send_uart_pkt(&pkg);
    }
    return res;
}
