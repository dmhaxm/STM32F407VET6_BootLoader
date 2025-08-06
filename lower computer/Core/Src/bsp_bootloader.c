/**-#####################################
**--File---Name--:--bootloader
**--Description--:--bootloader,BOOTLOADER_FlashWrite,FlashRead,etc.
--------------------------------------------

--Dev.-PlatForm:-vscode-&-Keil5.42

Created-By---:--DongMinghang
Created-Date-:--2025-07-28-
Version-----:--V1.0.0

Modified-Info-:--none
**-######################################*/

/*-------------------------Includes-(Header-Files)-----------------------------*/
#include "bsp_bootloader.h"
#include "usart.h"
#include <string.h>
/*-------------------------Private-typedef|define|macro(Private-in-.c-file,Public-in-.h-file)--------*/


/*-------------------------Private-variables-|-function-prototypes-(static)--------------------------*/
static BootLoaderState_t s_enBootLoaderState = BOOTLOADER_IDLE;
static uint32_t s_u32FileSize = 0;
static uint32_t s_u32ReceivedSize = 0;
static uint32_t s_u32CurrentAddress = APPLICATION_ADDRESS;
uint8_t g_u8RxBuffer[MAX_PACKET_SIZE + PACKET_HEADER_SIZE];
uint16_t g_u16RxIndex = 0;                
uint8_t g_u8PacketReady = 0;  //数据包接收完成标志
/* CRC16 查找表 */
static const uint16_t sc_u16Crc16Table[256] = {
    0x0000, 0xC0C1, 0xC181, 0x0140, 0xC301, 0x03C0, 0x0280, 0xC241,
    0xC601, 0x06C0, 0x0780, 0xC741, 0x0500, 0xC5C1, 0xC481, 0x0440,
    0xCC01, 0x0CC0, 0x0D80, 0xCD41, 0x0F00, 0xCFC1, 0xCE81, 0x0E40,
    0x0A00, 0xCAC1, 0xCB81, 0x0B40, 0xC901, 0x09C0, 0x0880, 0xC841,
    0xD801, 0x18C0, 0x1980, 0xD941, 0x1B00, 0xDBC1, 0xDA81, 0x1A40,
    0x1E00, 0xDEC1, 0xDF81, 0x1F40, 0xDD01, 0x1DC0, 0x1C80, 0xDC41,
    0x1400, 0xD4C1, 0xD581, 0x1540, 0xD701, 0x17C0, 0x1680, 0xD641,
    0xD201, 0x12C0, 0x1380, 0xD341, 0x1100, 0xD1C1, 0xD081, 0x1040,
    0xF001, 0x30C0, 0x3180, 0xF141, 0x3300, 0xF3C1, 0xF281, 0x3240,
    0x3600, 0xF6C1, 0xF781, 0x3740, 0xF501, 0x35C0, 0x3480, 0xF441,
    0x3C00, 0xFCC1, 0xFD81, 0x3D40, 0xFF01, 0x3FC0, 0x3E80, 0xFE41,
    0xFA01, 0x3AC0, 0x3B80, 0xFB41, 0x3900, 0xF9C1, 0xF881, 0x3840,
    0x2800, 0xE8C1, 0xE981, 0x2940, 0xEB01, 0x2BC0, 0x2A80, 0xEA41,
    0xEE01, 0x2EC0, 0x2F80, 0xEF41, 0x2D00, 0xEDC1, 0xEC81, 0x2C40,
    0xE401, 0x24C0, 0x2580, 0xE541, 0x2700, 0xE7C1, 0xE681, 0x2640,
    0x2200, 0xE2C1, 0xE381, 0x2340, 0xE101, 0x21C0, 0x2080, 0xE041,
    0xA001, 0x60C0, 0x6180, 0xA141, 0x6300, 0xA3C1, 0xA281, 0x6240,
    0x6600, 0xA6C1, 0xA781, 0x6740, 0xA501, 0x65C0, 0x6480, 0xA441,
    0x6C00, 0xACC1, 0xAD81, 0x6D40, 0xAF01, 0x6FC0, 0x6E80, 0xAE41,
    0xAA01, 0x6AC0, 0x6B80, 0xAB41, 0x6900, 0xA9C1, 0xA881, 0x6840,
    0x7800, 0xB8C1, 0xB981, 0x7940, 0xBB01, 0x7BC0, 0x7A80, 0xBA41,
    0xBE01, 0x7EC0, 0x7F80, 0xBF41, 0x7D00, 0xBDC1, 0xBC81, 0x7C40,
    0xB401, 0x74C0, 0x7580, 0xB541, 0x7700, 0xB7C1, 0xB681, 0x7640,
    0x7200, 0xB2C1, 0xB381, 0x7340, 0xB101, 0x71C0, 0x7080, 0xB041,
    0x5000, 0x90C1, 0x9181, 0x5140, 0x9301, 0x53C0, 0x5280, 0x9241,
    0x9601, 0x56C0, 0x5780, 0x9741, 0x5500, 0x95C1, 0x9481, 0x5440,
    0x9C01, 0x5CC0, 0x5D80, 0x9D41, 0x5F00, 0x9FC1, 0x9E81, 0x5E40,
    0x5A00, 0x9AC1, 0x9B81, 0x5B40, 0x9901, 0x59C0, 0x5880, 0x9841,
    0x8801, 0x48C0, 0x4980, 0x8941, 0x4B00, 0x8BC1, 0x8A81, 0x4A40,
    0x4E00, 0x8EC1, 0x8F81, 0x4F40, 0x8D01, 0x4DC0, 0x4C80, 0x8C41,
    0x4400, 0x84C1, 0x8581, 0x4540, 0x8701, 0x47C0, 0x4680, 0x8641,
    0x8201, 0x42C0, 0x4380, 0x8341, 0x4100, 0x81C1, 0x8081, 0x4040
};
/*-------------------------Private-function(static)-------------------------*/
static uint16_t BOOTLOADER_Crc16Calculate(uint8_t *l_pu8Data, uint16_t l_u16Length);
static void BOOTLOADER_SendResponse(uint8_t l_u8Cmd1, uint8_t l_u8Cmd2, uint8_t *l_pu8Data, uint16_t l_u16Length);
static void BOOTLOADER_SendErrorResponse(uint8_t l_u8ErrorCode);
static uint8_t BOOTLOADER_ProcessPacket(void);
static void BOOTLOADER_HandleHandshake(void);
static void BOOTLOADER_HandleFileInfo(uint8_t *l_pu8Data, uint16_t l_u16Length);
static void BOOTLOADER_HandleDataPacket(uint8_t *l_pu8Data, uint16_t l_u16Length);
static uint8_t BOOTLOADER_FlashErase(uint32_t l_u32StartAddress, uint32_t l_u32Size);
static uint8_t BOOTLOADER_FlashWrite(uint32_t l_u32Address, uint8_t *l_pu8Data, uint32_t l_u32Size);

/*
**=============================================================================
**-Function-name:--BOOTLOADER_ProcessPacket
**-Descriptions-:--Process the received UART packet.
**
**-Input--Para--:--None
**-Output-Para--:--None
**-Return-Value-:--1 if processed successfully, 0 if failed
**
**=============================================================================
*/

static uint8_t BOOTLOADER_ProcessPacket(void)
{
    if (g_u16RxIndex < 6) return 0;  /* 至少需要包头 */
    
    /* 查找起始标志 */
    uint16_t l_u16StartPos = 0;
    for (uint16_t i = 0; i < g_u16RxIndex - 1; i++) {
        if (g_u8RxBuffer[i] == START_BYTE1 && g_u8RxBuffer[i + 1] == START_BYTE2) {
            l_u16StartPos = i;
            break;
        }
    }
    
    /* 如果起始位置不在开头，移动数据 */
    if (l_u16StartPos > 0) {
        uint16_t l_u16Remaining = g_u16RxIndex - l_u16StartPos;
        memmove(g_u8RxBuffer, &g_u8RxBuffer[l_u16StartPos], l_u16Remaining);
        g_u16RxIndex = l_u16Remaining;
    }
    
    /* 再次检查是否有足够的数据 */
    if (g_u16RxIndex < 6) return 0;
    
    /* 提取数据长度 */
    uint16_t l_u16PacketLength = (g_u8RxBuffer[4] << 8) | g_u8RxBuffer[5];
    uint16_t l_u16TotalLength = 6 + l_u16PacketLength + 4; // 头部6字节 + 数据 + CRC2字节 + 结束2字节
    
    /* 检查是否收到完整的数据包 */
    if (g_u16RxIndex < l_u16TotalLength) return 0;
    
    /* 验证结束标志 */
    if (g_u8RxBuffer[l_u16TotalLength - 2] != END_BYTE1 || g_u8RxBuffer[l_u16TotalLength - 1] != END_BYTE2) {
        return 0;
    }
    
    /* 验证CRC */
    uint16_t l_u16ReceivedCrc = (g_u8RxBuffer[6 + l_u16PacketLength] << 8) | g_u8RxBuffer[6 + l_u16PacketLength + 1];
    uint16_t l_u16CalculatedCrc = BOOTLOADER_Crc16Calculate(g_u8RxBuffer, 6 + l_u16PacketLength);

    if (l_u16ReceivedCrc != l_u16CalculatedCrc) {
        BOOTLOADER_SendErrorResponse(ERR_CRC);
        return 1;
    }
    
    /* 处理命令 */
    uint8_t l_u8Cmd1 = g_u8RxBuffer[2];
    uint8_t l_u8Cmd2 = g_u8RxBuffer[3];

    if (l_u8Cmd1 == CMD_HANDSHAKE) {
        switch (l_u8Cmd2) {
            case CMD_HANDSHAKE: // 0x01 0x01
                BOOTLOADER_HandleHandshake();
                break;
            case CMD_FILE_INFO: // 0x01 0x02
                BOOTLOADER_HandleFileInfo(&g_u8RxBuffer[6], l_u16PacketLength);
                break;
            case CMD_DATA: // 0x01 0x03
                BOOTLOADER_HandleDataPacket(&g_u8RxBuffer[6], l_u16PacketLength);
                break;
            default:
                BOOTLOADER_SendErrorResponse(ERR_INVALID_CMD);
                break;
        }
    } else {
        BOOTLOADER_SendErrorResponse(ERR_INVALID_CMD);
    }
    return 1;
}
/*
**=============================================================================
**-Function-name:--BOOTLOADER_HandleHandshake
**-Descriptions-:--Handle the handshake command.
**
**-Input--Para--:--None
**-Output-Para--:--None
**-Return-Value-:--None
**
**=============================================================================
*/

static void BOOTLOADER_HandleHandshake(void)
{
    s_enBootLoaderState = BOOTLOADER_HANDSHAKE;
    BOOTLOADER_SendResponse(0x01, RESP_OK, NULL, 0);  // 发送0x01, 0x01
}
/*
**=============================================================================
**-Function-name:--BOOTLOADER_HandleFileInfo
**-Descriptions-:--Handle the file information command.
**
**-Input--Para--:--uint8_t *l_puData, uint16_t l_u16Length
**-Output-Para--:--None
**-Return-Value-:--None
**
**=============================================================================
*/

static void BOOTLOADER_HandleFileInfo(uint8_t *l_pu8Data, uint16_t l_u16Length)
{
    if (l_u16Length != 4) {
        BOOTLOADER_SendErrorResponse(ERR_INVALID_SIZE);
        return;
    }
    
    /* 提取文件大小 */
    s_u32FileSize = (l_pu8Data[0] << 24) | (l_pu8Data[1] << 16) | (l_pu8Data[2] << 8) | l_pu8Data[3];
    
    /* 检查文件大小是否合理 */
    if (s_u32FileSize == 0 || s_u32FileSize > (384 * 1024)) { // 最大384KB应用程序区域
        BOOTLOADER_SendErrorResponse(ERR_INVALID_SIZE);
        return;
    }
    
    /* 擦除Flash */
    if (BOOTLOADER_FlashErase(APPLICATION_ADDRESS, s_u32FileSize) != HAL_OK) {
        BOOTLOADER_SendErrorResponse(ERR_FLASH_ERASE);
        return;
    }
    
    /* 初始化接收参数 */
    s_u32ReceivedSize = 0;
    s_u32CurrentAddress = APPLICATION_ADDRESS;
    s_enBootLoaderState = BOOTLOADER_FILE_INFO;
    
    BOOTLOADER_SendResponse(0x02, RESP_OK, NULL, 0);  // 发送0x02, 0x01
}
/*
**=============================================================================
**-Function-name:--BOOTLOADER_HandleDataPacket
**-Descriptions-:--Handle the data packet command.
**
**-Input--Para--:--uint8_t *l_puData, uint16_t l_u16Length
**-Output-Para--:--None
**-Return-Value-:--None
**
**=============================================================================
*/
static void BOOTLOADER_HandleDataPacket(uint8_t *l_pu8Data, uint16_t l_u16Length)
{
    if (s_enBootLoaderState != BOOTLOADER_FILE_INFO && s_enBootLoaderState != BOOTLOADER_DATA_RECEIVE) {
        BOOTLOADER_SendErrorResponse(ERR_STATE);
        return;
    }
    
    /* 创建数据副本，避免DMA缓冲区覆盖问题 */
    static uint8_t s_pu8LocalDataBuffer[MAX_PACKET_SIZE];
    if (l_u16Length > MAX_PACKET_SIZE) {
        BOOTLOADER_SendErrorResponse(ERR_INVALID_SIZE);
        return;
    }
    
    /* 将数据复制到本地缓冲区 */
    memcpy(s_pu8LocalDataBuffer, l_pu8Data, l_u16Length);
    
    /* 写入Flash - 使用本地缓冲区的数据 */
    uint8_t l_u8FlashResult = BOOTLOADER_FlashWrite(s_u32CurrentAddress, s_pu8LocalDataBuffer, l_u16Length);
    if (l_u8FlashResult != HAL_OK) {
        BOOTLOADER_SendErrorResponse(l_u8FlashResult);
        return;
    }
    
    /* 更新状态 */
    s_u32CurrentAddress += l_u16Length;
    s_u32ReceivedSize += l_u16Length;
    s_enBootLoaderState = BOOTLOADER_DATA_RECEIVE;
    
    /* 检查是否接收完成 */
    if (s_u32ReceivedSize >= s_u32FileSize) {
        s_enBootLoaderState = BOOTLOADER_COMPLETE;
        BOOTLOADER_SendResponse(0x04, RESP_OK, NULL, 0);  // 发送0x04, 0x01
        
        /* 延时确保响应发送完成，然后自动重启 */
        HAL_Delay(500);
        
        /* 系统软件复位，让系统重新启动并自动跳转到新应用程序 */
        __NVIC_SystemReset();
    } else {
        BOOTLOADER_SendResponse(0x03, RESP_OK, NULL, 0);  // 发送0x03, 0x01
    }
}
/*
**=============================================================================
**-Function-name:--BOOTLOADER_FlashErase
**-Descriptions-:--Erase the specified Flash memory area.
**
**-Input--Para--:--uint32_t l_u32StartAddress, uint32_t l_u32Size
**-Output-Para--:--None
**-Return-Value-:--None
**
**=============================================================================
*/

static uint8_t BOOTLOADER_FlashErase(uint32_t l_u32StartAddress, uint32_t l_u32Size)
{
    FLASH_EraseInitTypeDef EraseInitStruct;
    uint32_t l_u32SectorError = 0;
    
    /* 解锁Flash */
    HAL_FLASH_Unlock();
    
    /* 确保擦除应用程序区域(从扇区5开始) */
    if (l_u32StartAddress < APPLICATION_ADDRESS) {
        HAL_FLASH_Lock();
        return HAL_ERROR;
    }
    
    /* STM32F407VET6 只有3个应用扇区: 扇区5、6、7 (384KB) */
    /* 根据文件大小动态计算需要擦除的扇区数 */
    uint32_t l_u32EndAddress = l_u32StartAddress + l_u32Size - 1;
    uint32_t l_u32StartSector = 5; // 应用程序从扇区5开始
    uint32_t l_u32NbSectors;
    
    if (l_u32EndAddress <= 0x0803FFFF) {        // 文件 ≤ 128KB
        l_u32NbSectors = 1;  // 只擦除扇区5
    } else if (l_u32EndAddress <= 0x0805FFFF) { // 文件 ≤ 256KB
        l_u32NbSectors = 2;  // 擦除扇区5-6
    } else {                                     // 文件 ≤ 384KB
        l_u32NbSectors = 3;  // 擦除扇区5-7 (最大)
    }

    /* 配置擦除参数 */
    EraseInitStruct.TypeErase = FLASH_TYPEERASE_SECTORS;
    EraseInitStruct.VoltageRange = FLASH_VOLTAGE_RANGE_3;
    EraseInitStruct.Sector = l_u32StartSector;
    EraseInitStruct.NbSectors = l_u32NbSectors;
    
    /* 在擦除操作前保持中断开启，让DMA能继续工作 */
    __enable_irq();
    
    /* 执行擦除 - DMA可以在此期间继续接收数据 */
    HAL_StatusTypeDef l_enStatus = HAL_FLASHEx_Erase(&EraseInitStruct, &l_u32SectorError);
    
    /* 检查擦除错误 */
    if (l_enStatus != HAL_OK || l_u32SectorError != 0xFFFFFFFF) {
        HAL_FLASH_Lock();
        return HAL_ERROR;
    }
    
    /* 锁定Flash */
    HAL_FLASH_Lock();

    return l_enStatus;
}
/*
**=============================================================================
**-Function-name:--BOOTLOADER_FlashWrite
**-Descriptions-:--Write data to the specified Flash memory area.
**
**-Input--Para--:--uint32_t l_u32Address, uint8_t *l_puData, uint32_t l_u32Size
**-Output-Para--:--None
**-Return-Value-:--None
**
**=============================================================================
*/

static uint8_t BOOTLOADER_FlashWrite(uint32_t l_u32Address, uint8_t *l_pu8Data, uint32_t l_u32Size)
{
    HAL_StatusTypeDef l_enStatus = HAL_OK;

    /* 检查地址范围 */
    if (l_u32Address < APPLICATION_ADDRESS || (l_u32Address + l_u32Size) > (APPLICATION_ADDRESS + APPLICATION_SIZE)) {
        return ERR_INVALID_ADDR;
    }

    /* 解锁Flash */
    HAL_FLASH_Unlock();
    
    /* 按字写入，确保4字节对齐 */
    for (uint32_t i = 0; i < l_u32Size; i += 4) {
        uint32_t l_u32WordData = 0;
        
        /* 正确的小端序字节组装 */
        for (uint8_t j = 0; j < 4 && (i + j) < l_u32Size; j++) {
            l_u32WordData |= ((uint32_t)l_pu8Data[i + j]) << (j * 8);
        }
        
        /* 填充不足4字节的数据为0xFF (Flash擦除后的状态) */
        if ((i + 4) > l_u32Size) {
            uint32_t l_u32Remaining = l_u32Size - i;
            for (uint8_t j = l_u32Remaining; j < 4; j++) {
                l_u32WordData |= 0xFF << (j * 8);
            }
        }
        
        /* 写入Flash */
        l_enStatus = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, l_u32Address + i, l_u32WordData);
        if (l_enStatus != HAL_OK) {
            break;
        }
        
        /* 验证写入的数据 */
        uint32_t l_u32Readback = *(volatile uint32_t*)(l_u32Address + i);
        if (l_u32Readback != l_u32WordData) {
            l_enStatus = HAL_ERROR;
            break;
        }
        
        /* 减少延时，提高写入速度 */
        if ((i % 512) == 0 && i > 0) {
            HAL_Delay(1);  // 每512字节延时1ms
        }
    }
    
    /* 锁定Flash */
    HAL_FLASH_Lock();
    
    /* 将HAL状态转换为错误代码 */
    if (l_enStatus == HAL_OK) {
        return HAL_OK;
    } else {
        return ERR_FLASH_WRITE;
    }
}
/*
**=============================================================================
**-Function-name:--BOOTLOADER_SendResponse
**-Descriptions-:--Send a response packet.
**
**-Input--Para--:--uint8_t cmd1, uint8_t cmd2, uint8_t *data, uint16_t length
**-Output-Para--:--None
**-Return-Value-:--None
**
**=============================================================================
*/

static void BOOTLOADER_SendResponse(uint8_t l_u8Cmd1, uint8_t l_u8Cmd2, uint8_t *l_pu8Data, uint16_t l_u16Length)
{
    uint8_t l_u8Packet[MAX_PACKET_SIZE + PACKET_HEADER_SIZE];
    uint16_t l_u16Index = 0;
    
    /* 构建数据包 */
    l_u8Packet[l_u16Index++] = START_BYTE1;
    l_u8Packet[l_u16Index++] = START_BYTE2;
    l_u8Packet[l_u16Index++] = l_u8Cmd1;
    l_u8Packet[l_u16Index++] = l_u8Cmd2;
    l_u8Packet[l_u16Index++] = (l_u16Length >> 8) & 0xFF;
    l_u8Packet[l_u16Index++] = l_u16Length & 0xFF;

    /* 添加数据 */
    if (l_pu8Data && l_u16Length > 0) {
        memcpy(&l_u8Packet[l_u16Index], l_pu8Data, l_u16Length);
        l_u16Index += l_u16Length;
    }
    
    /* 计算并添加CRC */
    uint16_t l_u16Crc = BOOTLOADER_Crc16Calculate(l_u8Packet, l_u16Index);
    l_u8Packet[l_u16Index++] = (l_u16Crc >> 8) & 0xFF;
    l_u8Packet[l_u16Index++] = l_u16Crc & 0xFF;

    /* 添加结束标志 */
    l_u8Packet[l_u16Index++] = END_BYTE1;
    l_u8Packet[l_u16Index++] = END_BYTE2;

    /* 发送数据包 */
    HAL_UART_Transmit(&huart1, l_u8Packet, l_u16Index, 1000);
}
/*
**=============================================================================
**-Function-name:--BOOTLOADER_SendErrorResponse
**-Descriptions-:--Send a response with an error code.
**
**-Input--Para--:--uint8_t l_u8ErrorCode
**-Output-Para--:--None
**-Return-Value-:--None
**
**=============================================================================
*/

static void BOOTLOADER_SendErrorResponse(uint8_t l_u8ErrorCode)
{
    BOOTLOADER_SendResponse(CMD_ERROR, RESP_ERROR, &l_u8ErrorCode, 1);
}
/*
**=============================================================================
**-Function-name:--BOOTLOADER_Crc16Calculate
**-Descriptions-:--Calculate the CRC16 checksum.
**
**-Input--Para--:--uint8_t l_u8ErrorCode
**-Output-Para--:--None
**-Return-Value-:--None
**
**=============================================================================
*/
static uint16_t BOOTLOADER_Crc16Calculate(uint8_t *l_pu8Data, uint16_t l_u16Length)
{
    uint16_t l_u16Crc = 0xFFFF;

    for (uint16_t i = 0; i < l_u16Length; i++) {
        uint8_t l_u8DataIndex = (uint8_t)(l_u16Crc ^ l_pu8Data[i]);
        l_u16Crc = (l_u16Crc >> 8) ^ sc_u16Crc16Table[l_u8DataIndex];
    }
    
    return l_u16Crc;
}

/*-------------------------Public--function-(defined-in-header-file)--------------------------*/

/*
**=============================================================================
**-Function-name:--BOOTLOADER_Init
**-Descriptions-:--Initialize the bootloader state and variables.
**
**-Input--Para--:--None
**-Output-Para--:--None
**-Return-Value-:--None
**
**=============================================================================
*/

void BOOTLOADER_Init(void)
{
    s_enBootLoaderState = BOOTLOADER_IDLE;
    s_u32FileSize = 0;
    s_u32ReceivedSize = 0;
    s_u32CurrentAddress = APPLICATION_ADDRESS;
    g_u16RxIndex = 0;
    g_u8PacketReady = 0;
    
    /* 启动DMA接收，等待第一个数据包 */
    HAL_UART_Receive_DMA(&huart1, g_u8RxBuffer, sizeof(g_u8RxBuffer));
}
/*
**=============================================================================
**-Function-name:--BOOTLOADER_Process
**-Descriptions-:--Process the received data packet and update the bootloader state.
**
**-Input--Para--:--None
**-Output-Para--:--None
**-Return-Value-:--None
**
**=============================================================================
*/

void BOOTLOADER_Process(void)
{
    /* 检查是否有完整数据包需要处理 */
    if (g_u8PacketReady) {
        g_u8PacketReady = 0;  /* 清除标志 */
        
        /* 处理接收到的数据包 */
        BOOTLOADER_ProcessPacket();
        
        /* 重新启动DMA接收下一个数据包 */
        g_u16RxIndex = 0;
        HAL_UART_Receive_DMA(&huart1, g_u8RxBuffer, sizeof(g_u8RxBuffer));
    }
}
/*
**=============================================================================
**-Function-name:--BOOTLOADER_JumpToApplication
**-Descriptions-:--Jump to the application code.
**
**-Input--Para--:--None
**-Output-Para--:--None
**-Return-Value-:--None
**
**=============================================================================
*/

void BOOTLOADER_JumpToApplication(void)
{
    typedef void (*pFunction)(void);
    pFunction JumpToApplication;
    uint32_t l_u32JumpAddress;

    /* 检查应用程序栈顶地址是否有效 */
    if (((*(__IO uint32_t*)APPLICATION_ADDRESS) & 0x2FFE0000) == 0x20000000) {
        /* 获取应用程序入口地址 */
        l_u32JumpAddress = *(__IO uint32_t*) (APPLICATION_ADDRESS + 4);
        JumpToApplication = (pFunction) l_u32JumpAddress;

        /* 系统复位准备 */
        __disable_irq();
        HAL_DeInit();
        SysTick->CTRL = 0;
        SysTick->LOAD = 0;
        SysTick->VAL = 0;
        
        /* 清除NVIC中断 */
        for (uint8_t i = 0; i < 8; i++) {
            NVIC->ICER[i] = 0xFFFFFFFF;
            NVIC->ICPR[i] = 0xFFFFFFFF;
        }
        
        /* 重置外设时钟 */
        RCC->AHB1RSTR = 0xFFFFFFFF; RCC->AHB1RSTR = 0x00000000;
        RCC->AHB2RSTR = 0xFFFFFFFF; RCC->AHB2RSTR = 0x00000000;
        RCC->AHB3RSTR = 0xFFFFFFFF; RCC->AHB3RSTR = 0x00000000;
        RCC->APB1RSTR = 0xFFFFFFFF; RCC->APB1RSTR = 0x00000000;
        RCC->APB2RSTR = 0xFFFFFFFF; RCC->APB2RSTR = 0x00000000;
        
        /* 重定位中断向量表到应用程序地址 */
        SCB->VTOR = APPLICATION_ADDRESS;
        
        /* 设置应用程序栈顶 */
        __set_MSP(*(__IO uint32_t*) APPLICATION_ADDRESS);
        
        /* 确保所有内存操作完成 */
        __DSB();
        __ISB();
        
        /* 跳转到应用程序 */
        JumpToApplication();
    }
}
/*
**=============================================================================
**-Function-name:--BOOTLOADER_CheckAppValid
**-Descriptions-:--Check if the application is valid.
**
**-Input--Para--:--None
**-Output-Para--:--None
**-Return-Value-:--1 if valid, 0 if invalid
**
**=============================================================================
*/

uint8_t BOOTLOADER_CheckAppValid(void)
{
    uint32_t l_u32AppsStackPtr = *(volatile uint32_t*)APP_START_ADDR;
    uint32_t l_u32AppResetHandler = *(volatile uint32_t*)(APP_START_ADDR + 4);
    
    // Check if stack pointer is in valid RAM range
    if ((l_u32AppsStackPtr & 0x2FFE0000) != 0x20000000) {
        return 0;
    }
    
    // Check if reset handler is in valid flash range
    if ((l_u32AppResetHandler < APP_START_ADDR) || (l_u32AppResetHandler > FLASH_END_ADDR)) {
        return 0;
    }
    
    // Check if reset handler has thumb bit set (LSB should be 1 for ARM Cortex-M)
    if ((l_u32AppResetHandler & 0x01) == 0) {
        return 0;
    }
    
    return 1;
}
/*
**=============================================================================
**-Function-name:--BOOTLOADER_IsUpgradeMode
**-Descriptions-:--Check if bootloader is in upgrade mode.
**
**-Input--Para--:--None
**-Output-Para--:--None
**-Return-Value-:--1 if in upgrade mode, 0 if not
**
**=============================================================================
*/

uint8_t BOOTLOADER_IsUpgradeMode(void)
{
    /* 如果不是初始状态和完成状态，说明正在进行升级操作 */
    return (s_enBootLoaderState != BOOTLOADER_IDLE && s_enBootLoaderState != BOOTLOADER_COMPLETE) ? 1 : 0;
}