#include "CRC8_SAE_J1850.h"
#include "stdio.h"


// CRC LUT
uint8_t crcTable[256];


uint8_t CalcCRC(uint8_t * buf, uint8_t len) 
{
    const uint8_t * ptr = buf;
    uint8_t _crc = 0xFF;

    while(len--) _crc = crcTable[_crc ^ *ptr++];

    return ~_crc;
}


void CRC_Init(void) 
{
    uint8_t _crc;
    for (int i = 0; i < 0x100; i++) 
    {
        _crc = i;

        for (uint8_t bit = 0; bit < 8; bit++) _crc = (_crc & 0x80) ? ((_crc << 1) ^ 0x1D) : (_crc << 1);

        crcTable[i] = _crc;
    }
}





