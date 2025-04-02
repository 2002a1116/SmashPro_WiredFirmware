/*
 * uart_com.c
 *
 *  Created on: 2024��10��16��
 *      Author: Reed
 */

#include "debug.h"
#include "string.h"
#include "ch32v10x_usbfs_device.h"
#include "ring_buffer.h"
#include "ns_com.h"
#include "ns_com_mux.h"
#include "uart_com.h"

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
    DMA_InitStructure.DMA_BufferSize = 256;
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
    //printf("tx service\r\n");
    if (UART1_Tx_Flag)
    {
        //printf("tx flag status %d\r\n",USART_GetFlagStatus(USART1, USART_FLAG_TC));
        if (USART_GetFlagStatus(USART1, USART_FLAG_TC))                                  // Check whether uart2 has finished sending.
        {
            USART_ClearFlag(USART1, USART_FLAG_TC);
            USART_DMACmd(USART1, USART_DMAReq_Tx, DISABLE);
            UART1_Tx_Flag = 0;
        }
    }
    else
    {
        //printf("txss %d\r\n",uart_tx_rb.capcity);
        if(uart_tx_rb.size){
            pbuf=&uart_tx_rb_buf[uart_tx_rb.top*UART_PKG_SIZE];
            if(uart_tx_rb.size+uart_tx_rb.top>uart_tx_rb.capcity){
                pkg_len=uart_tx_rb.capcity-uart_tx_rb.top;
                uart_tx_rb.size=uart_tx_rb.end/UART_PKG_SIZE+1;
                uart_tx_rb.top=0;
            }else {
                pkg_len=uart_tx_rb.size;
                uart_tx_rb.top=1;
                uart_tx_rb.end=0;
                uart_tx_rb.size=0;
            }
            pkg_len=pkg_len*UART_PKG_SIZE;
            uart_tx_rb.full=(uint8_t)(uart_tx_rb.capcity<=uart_tx_rb.size);
            UART1_DMA_Tx( pbuf, pkg_len );
            UART1_Tx_Flag = 1;
            //printf("send len %d\r\n",pkg_len);
        }
        //printf("txsd %d\r\n",uart_tx_rb.capcity);
    }
}

void ck_uart_cap(uint8_t id)
{
    return;
    if(!uart_tx_rb.capcity){
        printf("error at %d\r\n",id);
        exit(0);
        //Delay_Us(5000);
        /*while(1){
        }*/
    }
}
int8_t rx_ptr=0;
uint8_t rx_filter_buf[UART_PKG_SIZE];
void UART1_Rx_Service( void )
{
    ck_uart_cap(20);
    uint16_t u16_temp;
    UART1_RX_CurCnt = DMA_GetCurrDataCounter(DMA1_Channel5);
    ck_uart_cap(21);
    //printf("rx\r\n");// Get DMA remaining count
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
        //printf("u16temp :%d\r\n",u16_temp);
        if ((UART1_Rx_RemainLen + u16_temp) > DEF_UART1_BUF_SIZE )
        {
            //printf("remain len: %d ,temp : %d\r\n",UART1_Rx_RemainLen,u16_temp);
            //printf("Uart2 RX_buffer overflow\n");                                           // overflow: New data overwrites old data
        }
        else
        {
            UART1_Rx_RemainLen += u16_temp;
        }
        UART1_TimeOut = 0;
    }
    ck_uart_cap(22);
    //printf("r2 %d\r\n",UART1_Rx_RemainLen);
    while(UART1_Rx_RemainLen)
    {
        //printf("recv uart pkt len %d\r\n",UART1_Rx_RemainLen);
        //printf("value 0x%02x POS %d\r\n",UART1_RxBuffer[UART1_Rx_Deal_Ptr],UART1_Rx_Deal_Ptr);
        ck_uart_cap(23);
        if(UART1_RxBuffer[UART1_Rx_Deal_Ptr]&UART_PKG_HEADER_MASK)//HEADER
        {
            rx_ptr=1;
            rx_filter_buf[0]=UART1_RxBuffer[UART1_Rx_Deal_Ptr];
        }
        else if(rx_ptr>0){//join a packet
            ck_uart_cap(24);
                rx_filter_buf[rx_ptr++]=UART1_RxBuffer[UART1_Rx_Deal_Ptr];
                ck_uart_cap(25);
                while(rx_ptr>=UART_PKG_SIZE){//recive one
                    rx_ptr=0;
                    ck_uart_cap(26);
                    //printf("uart pkt recved\r\n");
                    //printf("uart pkt cksum %d %d\r\n",((uart_packet*)rx_filter_buf)->cksum,
                    //        ((uart_packet*)rx_filter_buf)->header^UART_PKG_HEADER_MASK);
                    if(check_uart_pkt((uart_packet*)rx_filter_buf))//if true,check fail,drop it
                    {
                        //rx_ptr=0;
                        break;
                    }
                    ck_uart_cap(27);
                    decode_uart_pkt((uart_packet*)rx_filter_buf);
                    ck_uart_cap(28);
                    //printf("uart pkt decode typ %d\r\n",((uart_packet*)rx_filter_buf)->typ);
                    u16_temp=ring_buffer_push(&uart_rx_rb, rx_filter_buf, UART_PKG_SIZE, 0);
                    ck_uart_cap(29);
                    if(u16_temp)
                        printf("uart push error:%d\r\n",u16_temp);
                    //printf("rb pushed\r\n");
                }
                //break;
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
    //printf("rlen:%d\r\n",UART1_Rx_RemainLen);
    ck_uart_cap(30);
    //printf("rx end\r\n");
}

void encode_uart_pkt(uart_packet* pkt){
    if(!pkt)return;
    pkt->high_bit=(pkt->arr[0]&UART_PKG_HEADER_MASK)>>7;
    pkt->high_bit|=(pkt->arr[1]&UART_PKG_HEADER_MASK)>>6;
    pkt->high_bit|=(pkt->arr[2]&UART_PKG_HEADER_MASK)>>5;
    //printf("arr0 %d %d %d  high %d\r\n",pkt->arr[0],pkt->arr[1],pkt->arr[2],pkt->arr[2]&~UART_PKG_HEADER_MASK);
    pkt->load=pkt->load&UART_PKG_LOAD_MASK;
    //printf("arr1 %d %d %d  high %d\r\n",pkt->arr[0],pkt->arr[1],pkt->arr[2],pkt->high_bit);
    pkt->cksum=pkt->header^pkt->data[0]^pkt->data[1]^pkt->data[2]^pkt->data[3];
}
void decode_uart_pkt(uart_packet* pkt){
    if(!pkt)return;
    pkt->arr[0]|=UART_PKG_HEADER_MASK&(pkt->high_bit<<7);
    pkt->arr[1]|=UART_PKG_HEADER_MASK&(pkt->high_bit<<6);
    pkt->arr[2]|=UART_PKG_HEADER_MASK&(pkt->high_bit<<5);
    pkt->header&=~UART_PKG_HEADER_MASK;
}
uint8_t check_uart_pkt(uart_packet* pkt){//if ok return false aka 0
    if(!pkt)return -1;
    return pkt->cksum!=(pkt->header^pkt->data[0]^pkt->data[1]^pkt->data[2]^pkt->data[3]^UART_PKG_HEADER_MASK);
}
static uart_packet send_buf;
uint8_t send_uart_pkt(uart_packet* pkt)
{
    if(!pkt)return -2;
    memcpy(&send_buf,pkt,UART_PKG_SIZE);
    encode_uart_pkt(&send_buf);
    send_buf.header|=UART_PKG_HEADER_MASK;
    return ring_buffer_push(&uart_tx_rb, (uint8_t*)&send_buf, UART_PKG_SIZE, 0);
}
uint8_t send_uart_large(uint8_t* buf,uint8_t len,uint8_t typ)
{
    uart_packet pkg;
    memset(&pkg,0,sizeof(uart_packet));
    uint8_t id=0,res=0;
    pkg.typ=typ;
    while(len>=3)
    {
        memcpy(pkg.arr,buf,3);
        buf+=3;
        pkg.id=id++;
        //encode_uart_pkt(&pkg);
        res = send_uart_pkt(&pkg);
        if(res)
            return res;
    }
    if(len)
    {
        memset(&pkg,0,sizeof(uart_packet));
        memcpy(pkg.arr,buf,len);
        pkg.id=id++;
        pkg.typ=typ;
        //encode_uart_pkt(&pkg);
        send_uart_pkt(&pkg);
    }
    return res;
}
