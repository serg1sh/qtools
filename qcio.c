#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>

struct termios sioparm;
int siofd; // fd для работы с Последовательным портом

//***********************
//* Дамп области памяти *
//***********************

void dump(char buffer[],int len,long base) {
unsigned int i,j;
char ch;

for (i=0;i<len;i+=16) {
  printf("%06lx: ",(long)(base+i));
  for (j=0;j<16;j++){
   if ((i+j) < len) printf("%02x ",buffer[i+j]&0xff);
   else printf("   ");
  }
  printf(" *");
  for (j=0;j<16;j++) {
   if ((i+j) < len) {
    ch=buffer[i+j];
    if ((ch < 0x20)|(ch > 0x80)) ch='.';
    putchar(ch);
   }
   else putchar(' ');
  }
  printf("*\n");
}
}

//***********************
//* Дамп области памяти *
//***********************

void errdump(char buffer[],int len,long base) {
unsigned int i,j;
char ch;

for (i=0;i<len;i+=16) {
  fprintf(stderr,"%06lx: ",(long)(base+i));
  for (j=0;j<16;j++){
   if ((i+j) < len) fprintf(stderr,"%02x ",buffer[i+j]&0xff);
   else fprintf(stderr,"   ");
  }
  fprintf(stderr," *");
  for (j=0;j<16;j++) {
   if ((i+j) < len) {
    ch=buffer[i+j];
    if ((ch < 0x20)|(ch > 0x80)) ch='.';
    fputc(ch,stderr);
   }
   else fputc(' ',stderr);
  }
  fprintf(stderr,"*\n");
}
}

//*************************************************
//*  Вычисление CRC-16 
//*************************************************
unsigned short crc16(char* buf, int len) {

unsigned short crctab[] = {
    0x0000, 0x1189, 0x2312, 0x329b, 0x4624, 0x57ad, 0x6536, 0x74bf,   // 0
    0x8c48, 0x9dc1, 0xaf5a, 0xbed3, 0xca6c, 0xdbe5, 0xe97e, 0xf8f7,   // 8
    0x1081, 0x0108, 0x3393, 0x221a, 0x56a5, 0x472c, 0x75b7, 0x643e,   //16
    0x9cc9, 0x8d40, 0xbfdb, 0xae52, 0xdaed, 0xcb64, 0xf9ff, 0xe876,   //24
    0x2102, 0x308b, 0x0210, 0x1399, 0x6726, 0x76af, 0x4434, 0x55bd,   //32
    0xad4a, 0xbcc3, 0x8e58, 0x9fd1, 0xeb6e, 0xfae7, 0xc87c, 0xd9f5,   //40
    0x3183, 0x200a, 0x1291, 0x0318, 0x77a7, 0x662e, 0x54b5, 0x453c,   //48
    0xbdcb, 0xac42, 0x9ed9, 0x8f50, 0xfbef, 0xea66, 0xd8fd, 0xc974,   //56
    0x4204, 0x538d, 0x6116, 0x709f, 0x0420, 0x15a9, 0x2732, 0x36bb,   //64
    0xce4c, 0xdfc5, 0xed5e, 0xfcd7, 0x8868, 0x99e1, 0xab7a, 0xbaf3,
    0x5285, 0x430c, 0x7197, 0x601e, 0x14a1, 0x0528, 0x37b3, 0x263a,
    0xdecd, 0xcf44, 0xfddf, 0xec56, 0x98e9, 0x8960, 0xbbfb, 0xaa72,
    0x6306, 0x728f, 0x4014, 0x519d, 0x2522, 0x34ab, 0x0630, 0x17b9,
    0xef4e, 0xfec7, 0xcc5c, 0xddd5, 0xa96a, 0xb8e3, 0x8a78, 0x9bf1,
    0x7387, 0x620e, 0x5095, 0x411c, 0x35a3, 0x242a, 0x16b1, 0x0738,
    0xffcf, 0xee46, 0xdcdd, 0xcd54, 0xb9eb, 0xa862, 0x9af9, 0x8b70,
    0x8408, 0x9581, 0xa71a, 0xb693, 0xc22c, 0xd3a5, 0xe13e, 0xf0b7,
    0x0840, 0x19c9, 0x2b52, 0x3adb, 0x4e64, 0x5fed, 0x6d76, 0x7cff,
    0x9489, 0x8500, 0xb79b, 0xa612, 0xd2ad, 0xc324, 0xf1bf, 0xe036,
    0x18c1, 0x0948, 0x3bd3, 0x2a5a, 0x5ee5, 0x4f6c, 0x7df7, 0x6c7e,
    0xa50a, 0xb483, 0x8618, 0x9791, 0xe32e, 0xf2a7, 0xc03c, 0xd1b5,
    0x2942, 0x38cb, 0x0a50, 0x1bd9, 0x6f66, 0x7eef, 0x4c74, 0x5dfd,
    0xb58b, 0xa402, 0x9699, 0x8710, 0xf3af, 0xe226, 0xd0bd, 0xc134,
    0x39c3, 0x284a, 0x1ad1, 0x0b58, 0x7fe7, 0x6e6e, 0x5cf5, 0x4d7c,
    0xc60c, 0xd785, 0xe51e, 0xf497, 0x8028, 0x91a1, 0xa33a, 0xb2b3,
    0x4a44, 0x5bcd, 0x6956, 0x78df, 0x0c60, 0x1de9, 0x2f72, 0x3efb,
    0xd68d, 0xc704, 0xf59f, 0xe416, 0x90a9, 0x8120, 0xb3bb, 0xa232,
    0x5ac5, 0x4b4c, 0x79d7, 0x685e, 0x1ce1, 0x0d68, 0x3ff3, 0x2e7a,
    0xe70e, 0xf687, 0xc41c, 0xd595, 0xa12a, 0xb0a3, 0x8238, 0x93b1,
    0x6b46, 0x7acf, 0x4854, 0x59dd, 0x2d62, 0x3ceb, 0x0e70, 0x1ff9,
    0xf78f, 0xe606, 0xd49d, 0xc514, 0xb1ab, 0xa022, 0x92b9, 0x8330,
    0x7bc7, 0x6a4e, 0x58d5, 0x495c, 0x3de3, 0x2c6a, 0x1ef1, 0x0f78
};
int i;
unsigned short crc=0xffff;  

for(i=0;i<len;i++)  crc=crctab[(buf[i]^crc)&0xff]^((crc>>8)&0xff);  
return (~crc)&0xffff;
}

//***************************************************
//*  Отсылка команды в порт и получение результата  *
//***************************************************
int send_cmd(unsigned char* incmdbuf, int blen, unsigned char* iobuf) {
  
int i,iolen,escflag,bcnt,incount;
unsigned char c;
unsigned char cmdbuf[4096];

bcnt=blen;
memcpy(cmdbuf,incmdbuf,blen);
*((unsigned short*)(cmdbuf+bcnt))=crc16(cmdbuf,bcnt);
bcnt+=2;
 
iolen=0;
for(i=0;i<bcnt;i++) {
   switch (cmdbuf[i]) {
     case 0x7e:
       iobuf[iolen++]=0x7d;
       iobuf[iolen++]=0x5e;
       break;
      
     case 0x7d:
       iobuf[iolen++]=0x7d;
       iobuf[iolen++]=0x5d;
       break;
      
     default:
       iobuf[iolen++]=cmdbuf[i];
   }
 }
iobuf[iolen++]=0x7e;
iobuf[iolen]=0;
 
tcflush(siofd,TCIOFLUSH);  // сбрасываем недчитанный буфер ввода
//fprintf(stderr,"\n -- send: %i --",iolen);
//errdump(iobuf,iolen,0);
if (write(siofd,iobuf,iolen) == 0) return 0;  
tcdrain(siofd);  // ждем окончания вывода блока

iolen=0;
escflag=0;
incount=0;
while (read(siofd,&c,1) == 1) {
  incount++;
  if ((c == 0x7e)&&(iolen != 0)) {
    iobuf[iolen++]=0x7e;
    break;
  }  
  if (c == 0x7d) {
    escflag=1;
    continue;
  }
  if (escflag == 1) { 
    c|=0x20;
    escflag=0;
  }  
  iobuf[iolen++]=c;
}  
//fprintf(stderr,"\n --- received: %i-----\n",incount);
//errdump(iobuf,(iolen<16)?iolen:16,0);
return iolen;
 
}


//*************************************
// Настройка Последовательного порта
//*************************************

int open_port(char* devname) {


siofd = open(devname, O_RDWR | O_NOCTTY |O_SYNC);
if (siofd == -1) return 0;

bzero(&sioparm, sizeof(sioparm)); // готовим блок атрибутов termios
sioparm.c_cflag = B115200 | CS8 | CLOCAL | CREAD | PARENB | PARODD;
sioparm.c_iflag = 0;  // INPCK;
sioparm.c_oflag = 0;
sioparm.c_lflag = 0;
sioparm.c_cc[VTIME]=10; // timeout  
sioparm.c_cc[VMIN]=0;  
tcsetattr(siofd, TCSANOW, &sioparm);
return 1;
}

//***********************************8
//* Чтение области памяти
//***********************************8

int memread(char* membuf,int adr, int len) {
char iobuf[600];
char cmdbuf[]={3,0,0,0,0,0,2};
int i,iolen;
int blklen=512;

// Чтение блоками по 512 байт  
for(i=0;i<len;i+=512)  {  
 *((unsigned int*)&cmdbuf[1])=i+adr;  //вписываем адрес
 if ((i+512) > len) {
   blklen=len-i;
   *((unsigned short*)&cmdbuf[5])=blklen;  //вписываем длину
 }  
 iolen=send_cmd(cmdbuf,7,iobuf);
 if (iolen <blklen) {
   printf("\n Ошибка в процессе обработки команды, iolen=%i\n",iolen);
   return 0;
 }  
 memcpy(membuf+i,iobuf+6,blklen);
} 
return 1;
}

//***********************************8
//* Чтение слова из памяти
//***********************************8
int mempeek(int adr) {

unsigned char iobuf[600];
unsigned char cmdbuf[]={3,0,0,0,0,4,0};
int i,iolen;

*((unsigned int*)&cmdbuf[1])=adr;  //вписываем адрес
iolen=send_cmd(cmdbuf,7,iobuf);
return *((unsigned int*)&iobuf[6]);
}  
  
//******************************************
//*  Запись слова в память
//******************************************
int mempoke(int adr, int data) {

unsigned char iobuf[300];
unsigned char cmdbuf[]={5,0,0,0,0,0,0,0,0,0};
int iolen;

*((unsigned int*)&cmdbuf[2])=adr;  //вписываем адрес
*((unsigned int*)&cmdbuf[6])=data;  //вписываем данные

iolen=send_cmd(cmdbuf,10,iobuf);

if (strncmp(iobuf+2,"OK",2) == 0) return 1;
return 0;
}

//*********************************************
//* Чтение блока флешки по указанному адресу 
//*********************************************

int flash_read(int adr0, int adr1) {
  
unsigned char iobuf[300];
unsigned char cmdbuf[]={9,0,0,0,0,0,0,0,0,0};
int iolen;

// reset flash
mempoke(0x1b400024,0x6745d); // ECC off
mempoke(0x1b400000,0xd);
mempoke(0x1b400010,0x1);

while ((mempeek(0x1b400014)&0xf) != 0); // ждем окончания выполнения команды

*((unsigned int*)&cmdbuf[2])=adr0;  //вписываем адрес
*((unsigned int*)&cmdbuf[6])=adr1;  //вписываем данные
iolen=send_cmd(cmdbuf,10,iobuf);
while ((mempeek(0x1b400014)&0xf) != 0); // ждем окончания выполнения команды
return iolen;
}
