#pragma once
//type 0-alaw 1-ulaw
int Pcm2G711(void * pcmbuff,int pcmbufflen,void * g711buff,int g711bufflen,int type);
int G7112Pcm(void * g711buff,int g711bufflen,void * pcmbuff,int pcmbufflen,int type);

unsigned char linear2alaw(int  pcm_val);
int alaw2linear(unsigned char a_val);
unsigned char linear2ulaw( int  pcm_val);
int ulaw2linear(unsigned char u_val);
unsigned char alaw2ulaw(unsigned char aval);
unsigned char ulaw2alaw(unsigned char uval);
