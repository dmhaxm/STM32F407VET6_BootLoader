#ifndef _BOOTLOADER_H_
#define _BOOTLOADER_H_
#ifdef __cplusplus
extern "C" {
#endif

/*-Includes-(Header-Files)--------------------------------------------------*/
#include "main.h"
/*---------------------------------Config-----------------------------------*/

/*---------------------------------Public-extern|define---------------------*/
/* Protocol defines */
#define START_BYTE1               0x55
#define START_BYTE2               0xAA
#define END_BYTE1                 0xF0
#define END_BYTE2                 0xA5

#define MAX_PACKET_SIZE           2048
#define PACKET_HEADER_SIZE        10    /* 起始(2) + 命令(2) + 长度(2) + CRC(2) + 结束(2) */
#define RX_TIMEOUT_MS             10000  /* 接收超时10秒，匹配上位机 */

/* Flash memory defines */
#define APP_START_ADDR            APPLICATION_ADDRESS
#define FLASH_END_ADDR            0x080FFFFF  /* STM32F407VET6 Flash结束地址 */

/* Commands */
#define CMD_HANDSHAKE             0x01
#define CMD_FILE_INFO             0x02
#define CMD_DATA                  0x03
#define CMD_COMPLETE              0x04
#define CMD_ERROR                 0x05

/* Response status */
#define RESP_OK                   0x01
#define RESP_ERROR                0x00

/* Error codes */
#define ERR_NONE                  0x00
#define ERR_CRC                   0x01
#define ERR_FLASH_ERASE           0x02
#define ERR_FLASH_WRITE           0x03
#define ERR_INVALID_ADDR          0x04
#define ERR_INVALID_SIZE          0x05
#define ERR_INVALID_CMD           0x06
#define ERR_STATE                 0x07
/*---------------------------------TypeDef----------------------------------*/
/* BOOTLOADER states */
typedef enum {
    BOOTLOADER_IDLE,
    BOOTLOADER_HANDSHAKE,
    BOOTLOADER_FILE_INFO,
    BOOTLOADER_DATA_RECEIVE,
    BOOTLOADER_COMPLETE,
    BOOTLOADER_ERROR
} BootLoaderState_t;

/*---------------------------------Public-macro/inline----------------------*/


/*---------------------------------Public-function--------------------------*/

void BOOTLOADER_Init(void);
void BOOTLOADER_Process(void);
void BOOTLOADER_JumpToApplication(void);
uint8_t BOOTLOADER_CheckAppValid(void);
uint8_t BOOTLOADER_IsUpgradeMode(void);

/*---------------------------------External-variables-----------------------*/
extern uint8_t g_u8RxBuffer[];  /* 全局接收缓冲区 */
extern uint16_t g_u16RxIndex;    /* 全局接收索引 */
extern uint8_t g_u8PacketReady;  /* 全局数据包就绪标志 */

#ifdef __cplusplus
}
#endif

#endif /* _BOOTLOADER_H_ */