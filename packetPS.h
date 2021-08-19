#pragma once
#include <stdint.h>
int TransPSFrame(char *pFrame, int nFrameLength, uint32_t nTimeStamp, unsigned char *PSFrameBuffer);
	int GetPSHeader(uint8_t *header, uint64_t mtime, uint16_t farmLen);
	int GetSinglePESHeader(uint8_t *header, uint64_t mtime, uint16_t farmLen);
	void SetPESTimeStamp(uint8_t *buff, uint64_t ts);


