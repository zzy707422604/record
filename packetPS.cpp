#include "stdafx.h"
#include "packetPS.h"


const uint8_t PS_HEAD[] = {
	/*PS头*/
	0x00, 0x00, 0x01, 0xba,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/*时间戳*/
	0x01, 0x47, 0xb3,
	0xf8
};

const uint8_t SYS_MAP_HEAD[] = {
	/*PS_SYS头*/
	0x00, 0x00, 0x01, 0xbb,
	0x00, 0x0c,								/*sys头长度,不含自己，6+3*流的数目*/
	0x80, 0xa3, 0xd9,								/*速率*/
	0x04, 0xe1,								/*音频流数，视频流数加3个1标识*/
	0xff,									/**/
	0xb9, 0xe0, 0x00, 0xb8, 0xc0, 0x40,		/*流信息，b9视频，b8音频*/
	/*PS_MAP头*/
	0x00, 0x00, 0x01, 0xbc,
	0x00, 0x12,								/*psm长度*/
	0x04, 0xff,								/**/
	0x00, 0x00, 0x00, 0x08,					/*固定2路流*/
	0x1b, 0xe0, 0x00, 0x00,					/*视频，第一个字节(0x1b), 更具不同的视频编码改变即可封装不同的流，见开头宏定义*/
	0x90, 0xc0, 0x00, 0x00,					/*音频，同视频*/
	0x00, 0x00, 0x00, 0x00					/*4b CRC，暂时没设置*/
};

const uint8_t PES_HEAD[] = {
	/*PS_PES头*/
	0x00, 0x00, 0x01, 0xe0,
	0x00, 0x00,								/*pes长度*/
	0x80, 0xc0,								/*附加信息*/
	0x0a,									/*附加信息长度*/
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /*pts和pds*/
};

//unsigned char PSFrameBuffer[10 * 1024 * 1024]; //转换后的ps帧缓存区
int TransPSFrame(char *pFrame, int nFrameLength, uint32_t nTimeStamp, unsigned char * PSFrameBuffer)
{
	if (!pFrame || !nFrameLength)
		return 0;

	//每个pes最多65400数据
	int PesLenth = nFrameLength > 65400 ? 65400 : nFrameLength;
	//第一个pes需要有ps头，其它不需要，音频直接打包pes（00 00 01 c0)
	int psHeadLen = GetPSHeader(PSFrameBuffer, nTimeStamp, PesLenth);
	memcpy(PSFrameBuffer + psHeadLen, pFrame, PesLenth);
	int psSize = psHeadLen + PesLenth;
	int pod = PesLenth;

	nFrameLength -= PesLenth;
	while (nFrameLength > 0)
	{
		PesLenth = nFrameLength > 65400 ? 65400 : nFrameLength;
		psHeadLen = GetSinglePESHeader(PSFrameBuffer + psSize, nTimeStamp, PesLenth);

		memcpy(PSFrameBuffer + psSize + psHeadLen, pFrame + pod, PesLenth);
		psSize += (PesLenth + psHeadLen);
		pod += PesLenth;
		nFrameLength -= PesLenth;
	}

	//static FILE *fp = fopen("my.ps", "wb");
	//if(fp)
	//fwrite(PSFrameBuffer, 1, psSize, fp);

	return psSize;
}


int GetPSHeader(uint8_t *header, uint64_t mtime, uint16_t farmLen)
{
	GetSinglePESHeader(header, mtime, farmLen);
	*(header + 3) = 0xc0;
	return sizeof(PES_HEAD);
}

int GetSinglePESHeader(uint8_t *header, uint64_t mtime, uint16_t farmLen)
{
	farmLen += 13;
	memcpy(header, PES_HEAD, sizeof(PES_HEAD));
	*(header + 4) = (uint8_t)(farmLen >> 8);
	*(header + 5) = (uint8_t)farmLen;

	SetPESTimeStamp(header, mtime);
	return sizeof(PES_HEAD);
}

void SetPESTimeStamp(uint8_t *buff, uint64_t ts)
{
	buff += 9;
	// PTS
	buff[0] = (uint8_t)(((ts >> 30) & 0x07) << 1) | 0x30 | 0x01;
	buff[1] = (uint8_t)((ts >> 22) & 0xff);
	buff[2] = (uint8_t)(((ts >> 15) & 0xff) << 1) | 0x01;
	buff[3] = (uint8_t)((ts >> 7) & 0xff);
	buff[4] = (uint8_t)((ts & 0xff) << 1) | 0x01;
	// DTS
	buff[5] = (uint8_t)(((ts >> 30) & 0x07) << 1) | 0x10 | 0x01;
	buff[6] = (uint8_t)((ts >> 22) & 0xff);
	buff[7] = (uint8_t)(((ts >> 15) & 0xff) << 1) | 0x01;
	buff[8] = (uint8_t)((ts >> 7) & 0xff);
	buff[9] = (uint8_t)((ts & 0xff) << 1) | 0x01;
}