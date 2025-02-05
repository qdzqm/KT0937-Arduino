
/*
 * clone from,https://www.youtube.com/watch?time_continue=1&v=XKgglNlFYhY&feature=emb_logo
 * source code, https://yahoo.jp/box/gNtSmg
 * schematic, https://goo.gl/rjEPcN 
 * 
 * 2019-NOV-16, compile Arduino 1.8.9
 * xiaolaba
 * disable / change F_CPU to 16MHZ, as default, Arduino Nano (ATmega168) testing
 * 
 **/
 

// FM/AM Radio KT0913 I2C ATmega328P clock:8MHz 20171126 1400
#include <avr/io.h>
//#define F_CPU 8000000UL // disable, Arduino Nano, default 16MHZ XTAL, not uses RC clock
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

unsigned int i,j,p,n,tunf=250,tuna=1143,ctm,cts,cta,ctb,ad,ad1;
unsigned char vol,reh,rel,ch,cl,fa=0,bb=0,ss,ss1,sk1,sk2,pw=1,mut,bsi[500];
unsigned char ifd=0x04;
unsigned int ads,cmd,bt,cti,ib;

#define on TCCR1B|=0b00001100;TCCR0B=0b00000101;EIMSK=1;
#define off TCCR1B&=0b00000000;TCCR0B=0b00000000;EIMSK=0;

//---------- Font Frequency ---------------------------------
unsigned char f[10][22]={
  { //0
  0xfc,0xfe,0xfe,0x07,0x03,0x03,0x03,0x07,0xfe,0xfe,0xfc,
  0x3f,0x7f,0x7f,0xe0,0xc0,0xc0,0xc0,0xe0,0x7f,0x7f,0x3f},

  { //1
  0x00,0x00,0x08,0x08,0x0c,0xfe,0xff,0xff,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0xff,0xff,0xff,0x00,0x00,0x00},

  { //2
  0x1c,0x1e,0x1e,0x07,0x03,0x03,0x83,0xc7,0xfe,0xfe,0x7c,
  0xe0,0xf0,0xf8,0xfc,0xde,0xcf,0xc7,0xc3,0xc1,0xc0,0xc0},

  { //3
  0x0c,0x0e,0x06,0x03,0x83,0x83,0x83,0xc7,0xfe,0x7e,0x3c,
  0x30,0x70,0x60,0xc0,0xc1,0xc1,0xc1,0xe3,0x7f,0x7f,0x3e},

  { //4
  0x00,0x00,0x00,0xc0,0xf0,0x78,0x1e,0xff,0xff,0x00,0x00,
  0x18,0x1e,0x1f,0x1b,0x18,0x18,0x18,0xff,0xff,0x18,0x18},

  { //5
  0xff,0xff,0xff,0xc3,0xc3,0xc3,0xc3,0xc3,0xc3,0x83,0x03,
  0x31,0x71,0x61,0xc0,0xc0,0xc0,0xc0,0xe1,0x7f,0x7f,0x3f},

  { //6
  0xfc,0xfe,0xfe,0x87,0xc3,0xc3,0xc3,0xc3,0xc6,0x86,0x00,
  0x3f,0x7f,0x7f,0xe1,0xc0,0xc0,0xc0,0xe1,0x7f,0x7f,0x3f},

  { //7
  0x03,0x03,0x03,0x03,0x03,0x03,0x83,0xe3,0xfb,0x3f,0x0f,
  0x00,0x00,0x00,0xe0,0xf8,0x3e,0x0f,0x03,0x00,0x00,0x00},

  { //8
  0x3c,0x7e,0xfe,0xc7,0x83,0x83,0x83,0xc7,0xfe,0x7e,0x3c,
  0x3e,0x7f,0x7f,0xe3,0xc1,0xc1,0xc1,0xe3,0x7f,0x7f,0x3e},

  { //9
  0x78,0xfe,0xfe,0x87,0x03,0x03,0x03,0x87,0xfe,0xfe,0xf8,
  0x00,0x61,0x61,0xc3,0xc3,0xc3,0xc3,0xe1,0x7f,0x7f,0x3f}
};

//---------- Font FM/AM -------------------------------------
unsigned char fm_am[2][19]={
  { //FM
  0xff,0xff,0x0c,0x0c,0x0c,0x0c,0x00,0x00,0x00,0xff,
  0xff,0x0f,0x3c,0x78,0x3c,0x0f,0xff,0xff,0x00},

  { //AM
  0xe0,0xfc,0x3f,0x33,0x33,0x3f,0xfc,0xe0,0x00,0x00,
  0xff,0xff,0x0f,0x3c,0x78,0x3c,0x0f,0xff,0xff}
};

//---------- Font MHz/kHz -----------------------------------
unsigned char mhz_khz[2][14]={
  {0xfc,0x18,0x60,0x18,0xfc,0x00,0xfc,0x20,0x20,0xfc,0x00,0xc8,0xa8,0x98}, //MHz
  {0xfc,0x20,0x50,0x88,0x00,0xfc,0x20,0x20,0xfc,0x00,0xc8,0xa8,0x98,0x00}  //kHz
};

//---------- Font Indicator ---------------------------------
unsigned char ind[5][10]={
  {0xb0,0xa8,0xa8,0x68,0x00,0x08,0x08,0xf8,0x08,0x08}, //ST
  {0x78,0xcc,0x84,0x84,0x84,0x84,0x84,0x84,0x84,0xfc}, //Battery
  {0xb0,0xa8,0xa8,0x68,0x00,0xb0,0xa8,0xa8,0x68,0x00}, //SS
  {0x18,0x60,0x80,0x60,0x18,0xe0,0xa0,0xe0,0x00,0xf8}, //Vol
  {0x00,0xf8,0x30,0xc0,0x30,0xf8,0x00,0xe0,0x80,0xe0}  //Mute
};

//---------- Broadcasting Station Image ---------------------
unsigned char wri[500]={

};

//---------- KT0913 Write -----------------------------------
void wt(unsigned char reg,unsigned char dah,unsigned char dal){ 

  TWCR=0b10100100;while(!(TWCR & 0x80)); //Start bit
  if((TWSR & 0xF8)==0x08){ //Start
    TWDR=0b01101010; //SLA+W
    TWCR=0b10000100;while(!(TWCR & 0x80));

    TWDR=reg; //Register Address
    TWCR=0b10000100;while(!(TWCR & 0x80));
    TWDR=dah; //Data_H [15:8]
    TWCR=0b10000100;while(!(TWCR & 0x80));
    TWDR=dal; //Data_L [7:0]
    TWCR=0b10000100;while(!(TWCR & 0x80));
  }
  TWCR=0b10010100; //Stop bit
}

//---------- KT0913 Read ------------------------------------
void rd(unsigned char reg){ 

  TWCR=0b10100100;while(!(TWCR & 0x80)); //Start bit
  if((TWSR & 0xF8)==0x08){ //Start
    TWDR=0b01101010; //SLA+W
    TWCR=0b10000100;while(!(TWCR & 0x80));
    TWDR=reg; //Register Address
    TWCR=0b10000100;while(!(TWCR & 0x80));
  }
  TWCR=0b10100100;while(!(TWCR & 0x80)); //Start bit
  if((TWSR & 0xF8)==0x10){ //Start
    TWDR=0b01101011; //SLA+R
    TWCR=0b10000100;while(!(TWCR & 0x80));
    TWCR=0b11000100;while(!(TWCR & 0x80)); //Receive
    reh=TWDR; //Data_H [15:8]
    TWCR=0b11000100;while(!(TWCR & 0x80)); //Receive
    rel=TWDR; //Data_L [7:0]
    TWCR=0b10000100;while(!(TWCR & 0x80));
  }
  TWCR=0b10010100; //Stop bit
  _delay_ms(0.1);
}

//---------- OLED Command -----------------------------------
void cmd2(unsigned char val){ 

  TWCR=0b10100100;while(!(TWCR & 0x80)); //Start bit
  if((TWSR & 0xF8)==0x08){ //Start
    TWDR=0b01111000; //SLA+W
    TWCR=0b10000100;while(!(TWCR & 0x80));
    TWDR=0b10000000; //Control byte
    TWCR=0b10000100;while(!(TWCR & 0x80));
    TWDR=val; //Data
    TWCR=0b10000100;while(!(TWCR & 0x80));
  }
  TWCR=0b10010100; //Stop bit
}

//---------- Page Address -----------------------------------
void pag(unsigned char pa){cmd2(0xB0+pa);}

//---------- Column Address ---------------------------------
void clm(unsigned char ca){
  ch=(ca+2)/16+16;
  cl=(ca+2)%16;
  cmd2(ch);cmd2(cl);
}

void tw1(){
  TWDR=0b01111000; //SLA+W
  TWCR=0b10000100;while(!(TWCR & 0x80));
  TWDR=0b01000000; //Control byte
  TWCR=0b10000100;while(!(TWCR & 0x80));
}

void dat2(unsigned char a,unsigned char b){ 

  TWCR=0b10100100;while(!(TWCR & 0x80)); //Start bit
  if((TWSR & 0xF8)==0x08){ //Start
    tw1();
    for(j=0;j<a;j++){
      TWDR=b; //Data
      TWCR=0b10000100;while(!(TWCR & 0x80));
    }
  }
  TWCR=0b10010100; //Stop bit
  _delay_ms(0.1);
}

void indicator(unsigned char a){ 

  TWCR=0b10100100;while(!(TWCR & 0x80)); //Start bit
  if((TWSR & 0xF8)==0x08){ //Start
    tw1();
    for(j=0;j<10;j++){
      TWDR=ind[a][j]; //Font Data
      TWCR=0b10000100;while(!(TWCR & 0x80));
    }
  }
  TWCR=0b10010100; //Stop bit
  _delay_ms(1);
}

void fm_am_logo(){ 

  if(fa){ //AM
    pag(6);
    clm(21);dat2(18,0x00);
    clm(22);dat2(2,0xC0);
    clm(29);dat2(2,0xC0);
    clm(36);dat2(2,0xC0);
    pag(7);
    clm(94);dat2(3,0x00);
    clm(38);dat2(1,0x00);
    OCR1A=50000;
  }

  else{ //FM
    pag(6);
    clm(29);dat2(8,0x00);
    clm(21);dat2(7,0xC0);
    clm(30);dat2(2,0xC0);
    clm(37);dat2(2,0xC0);
    pag(7);
    clm(19);dat2(2,0x00);
    clm(108);dat2(2,0x00);
    OCR1A=10000;
  }
  
  pag(7);
  //-------- FM/AM -----------
  if(fa){clm(19);}else{clm(21);}  
  TWCR=0b10100100;while(!(TWCR & 0x80)); //Start bit
  if((TWSR & 0xF8)==0x08){ //Start
    tw1();
    for(j=0;j<19;j++){
      TWDR=fm_am[fa][j]; // FM/AM
      TWCR=0b10000100;while(!(TWCR & 0x80));
    }
  }
  TWCR=0b10010100; //Stop bit
  _delay_ms(0.1);
  //-------- MHz/kHz ---------
  if(fa){clm(97);}else{clm(94);}
  TWCR=0b10100100;while(!(TWCR & 0x80)); //Start bit
  if((TWSR & 0xF8)==0x08){ //Start
    tw1();
    for(j=0;j<14;j++){
      TWDR=mhz_khz[fa][j]; // FM/AM
      TWCR=0b10000100;while(!(TWCR & 0x80));
    }
  }
  TWCR=0b10010100; //Stop bit
  _delay_ms(0.1);
}

//---------- OLED Frequency ---------------------------
void fre(unsigned char a,unsigned int b){ 

  for(i=0;i<2;i++){
    pag(6+i);
    clm(a);
    TWCR=0b10100100;while(!(TWCR & 0x80)); //Start bit
    if((TWSR & 0xF8)==0x08){ //Start
      tw1();
      for(j=(11*i);j<(11*(i+1));j++){
        TWDR=f[b][j]; //Data
        TWCR=0b10000100;while(!(TWCR & 0x80));
      }
    }
    TWCR=0b10010100; //Stop bit
  }
}

//---------- EEPROM Write -----------------------------
void eep_write(unsigned int a){ 

  for(i=0;i<16;i++){
    TWCR=0b10100100;while(!(TWCR & 0x80)); //Start bit
    if((TWSR & 0xF8)==0x08){ //Start
      TWDR=0b10100000; //SLA+W
      TWCR=0b10000100;while(!(TWCR & 0x80));

      TWDR=0b00000000+i/8; //High Byte

      TWCR=0b10000100;while(!(TWCR & 0x80));
      TWDR=0b00000000+i*32; //Low Byte
      TWCR=0b10000100;while(!(TWCR & 0x80));
      for(j=0;j<32;j++){
        TWDR=wri[i*32+j]; //Data
        TWCR=0b10000100;while(!(TWCR & 0x80));
      }
    }
    TWCR=0b10010100; //Stop bit
    _delay_ms(10);
  }
}

//---------- EEPROM Read -----------------------------
void eep_read(unsigned int a){

  TWCR=0b10100100;while(!(TWCR & 0x80)); //Start bit
  if((TWSR & 0xF8)==0x08){ //Start
    TWDR=0b10100000; //SLA+W
    TWCR=0b10000100;while(!(TWCR & 0x80));
    TWDR=0b00000000+a*2; //High Byte
    TWCR=0b10000100;while(!(TWCR & 0x80));
    TWDR=0b00000000; //Low Byte
    TWCR=0b10000100;while(!(TWCR & 0x80));
  }
  TWCR=0b10100100;while(!(TWCR & 0x80)); //Start bit
  if((TWSR & 0xF8)==0x10){ //Start
    TWDR=0b10100001; //SLA+R
    TWCR=0b10000100;while(!(TWCR & 0x80));

    for(j=0;j<500;j++){
      TWCR=0b11000100;while(!(TWCR & 0x80)); //Receive
      bsi[j]=TWDR; //Data
    }
    TWCR=0b10000100;while(!(TWCR & 0x80));
  }
  TWCR=0b10010100; //Stop bit
}

//---------- Broadcasting Station Image ---------------
void image(unsigned int b){ 

  eep_read(b);
  for(i=0;i<5;i++){
    pag(i);
    clm(15);
    TWCR=0b10100100;while(!(TWCR & 0x80)); //Start bit
    if((TWSR & 0xF8)==0x08){ //Start
      tw1();
      for(j=(100*i);j<(100*(i+1));j++){
        TWDR=bsi[j]; //Data
        TWCR=0b10000100;while(!(TWCR & 0x80));
      }
    }
    TWCR=0b10010100; //Stop bit
  }
}

void tune(){

  if(fa){ //AM
    wt(0x16,0b10000000,0b00000010); // FM/AM[15]
    if(tuna<768){wt(0x17,0b10000010,tuna-512);} //531-767
    if(tuna>767 && tuna<1024){wt(0x17,0b10000011,tuna-768);} //768-1023
    if(tuna>1023 && tuna<1280){wt(0x17,0b10000100,tuna-1024);} //1024-1279
    if(tuna>1279 && tuna<1536){wt(0x17,0b10000101,tuna-1280);} //1280-1535
    if(tuna>1535){wt(0x17,0b10000110,tuna-1536);} //1536-1602

    if(tuna>999){fre(41,tuna/1000);}
      else{pag(6);clm(43);dat2(6,0x00);pag(7);clm(46);dat2(3,0x00);}
    fre(54,tuna%1000/100);
    fre(68,tuna%100/10);
    fre(82,tuna%10);
  }
  else{ //FM
    wt(0x16,0b00000000,0b00000010); // FM/AM[15]
    if(tunf<255){wt(0x03,0b10000101,tunf);} //76.0-76.7
    if(tunf>255 && tunf<511){wt(0x03,0b10000110,tunf-256);} //76.8-89.5
    if(tunf>511){wt(0x03,0b10000111,tunf-512);} //89.6-96.0

    fre(45,(tunf/2+640)/100);
    fre(59,(tunf/2+640)%100/10);
    fre(79,(tunf/2+640)%10);
  }
  TCNT1=4000;

  if(fa){
    switch(tuna){
      case 558:image(9);break;
      case 666:image(10);break;
      case 828:image(11);break;
      case 1143:image(12);break;
      default:for(i=0;i<5;i++){pag(i);clm(15);dat2(100,0);};break;
    }
  }
  else{
    switch(tunf){
      case 250:image(0);break;
      case 324:image(1);break;
      case 422:image(2);break;
      case 482:image(3);break;
      case 508:image(4);break;
      case 518:image(5);break;
      case 532:image(6);break;
      case 558:image(7);break;
      case 586:image(8);break;
      default:for(i=0;i<5;i++){pag(i);clm(15);dat2(100,0);};break;
    }
  }
}

//---------- KT0913 Initialize ------------------------
void kt0913_ini(){
  wt(0x04,0b11000000+bb,0b10000000); // bb:Bass Boost[9:8]
  wt(0x0F,0b10001000,0b00000000); // Volume[4:0]
  wt(0x16,0b00000000,0b10000010); // FM/AM[15], AU_GAIN[7:6](-3dB)
  wt(0x22,0b10101111,0b11000000); // AM Band Width[7:6](6kHz)
  wt(0x33,0b00010100,0b00000001); // AM Channel Space[15:14](9kHz)
}

//---------- OLED Initialize(SH1106) ------------------
void oled_ini(){
  PORTB&=~(1<<4);_delay_ms(1);PORTB|=(1<<4);_delay_ms(1); //Reset
  cmd2(0x40); //Start line
  cmd2(0xA1); //Segment re-map : Left rotates
  cmd2(0xC8); //Scan direction
  cmd2(0x81);cmd2(0x80); //Contrast
  tune();
  for(i=0;i<8;i++){pag(i);clm(0);dat2(128,0);} //All clear
  pag(0);clm(118);indicator(1); //Battery
  pag(7);clm(1);indicator(2); //SS
  pag(7);clm(118);indicator(3); //Vol
  fm_am_logo(); //FM/AM
  pag(7);clm(73);dat2(3,0xe0); //Decimal point
  cmd2(0xAF); //Display on
}

//---------- Operation --------------------------------
void ope(){

  if(cmd==0xA25D && cts==0){ //Power
    pw^=1;sk1=0;sk2=0;
  }

  if(pw){
    switch(cmd){

    case 0xB04F: //FM/AM (CLOCK)
      if(cts==0){
        fa^=1;
        pag(6);clm(43);dat2(50,0x00);
        pag(7);clm(43);dat2(50,0x00);
        for(i=0;i<7;i++){pag(i);clm(1);dat2(10,0);}
        if(fa==0){pag(7);clm(73);dat2(3,0xe0);} //decimal point
        fm_am_logo();
        tune();
      }
      break;

    case 0xE01F: //Bass Boost (EQ)
      bb++;
      if(bb>3){bb=0;}
      wt(0x04,0b11100000+bb,0b10000000); // Bass[9:8]
      _delay_ms(500);
      break;

    case 0x02FD: //Tune down (<<)
      if(cts%2==0){
        if((tunf>240)&&(fa==0)){tunf-=2;tune();}
        if((tuna>531)&& fa){tuna-=9;tune();}
      }
      if(cts>3){sk1=1;}else{sk1=0;sk2=0;}
      break;

    case 0xC23D: //Tune up (>>)
      if(cts%2==0){
        if((tunf<640)&&(fa==0)){tunf+=2;tune();}
        if((tuna<1602)&& fa){tuna+=9;tune();}
      }
      if(cts>3){sk2=1;}else{sk1=0;sk2=0;}
      break;

    case 0xA857: //Volume down
      if(mut){
        pag(7);clm(118);indicator(3);
        for(i=0;i<vol;i++){
          PORTB&=~(1<<2);_delay_ms(35);PORTB|=(1<<2);_delay_ms(5);
        }
        mut=0;
      }
      else{
        PORTB&=~(1<<1);
        if(vol>0){
          vol--;
          pag(6-(vol-1)/4);clm(121);
          if(vol==0){pag(6),clm(121);dat2(4,0x00);}
          else{
            if(vol%4==0){dat2(4,255);pag(5-(vol-1)/4);clm(121);dat2(4,0x00);}
            else{dat2(4,255<<(4-vol%4)*2);}
          }
        }
        _delay_ms(35);
        PORTB|=(1<<1);
      }
      break;

    case 0x906f: //Volume up
      if(vol<23){
        if(mut){
          pag(7);clm(118);indicator(3);
          for(i=0;i<vol;i++){
            PORTB&=~(1<<2);_delay_ms(35);PORTB|=(1<<2);_delay_ms(5);
          }
          mut=0;
        }
        else{
          vol++;
          PORTB&=~(1<<2);
          pag(6-(vol-1)/4);clm(121);
          if(vol%4==0){dat2(4,255);}
            else{dat2(4,255<<(4-vol%4)*2);}
          _delay_ms(35);
          PORTB|=(1<<2);
        }
      }
      break;

    case 0xE21D: //Mute
      if(cts==0){
        mut^=1;
        if(mut){
          for(i=0;i<vol;i++){
            PORTB&=~(1<<1);_delay_ms(35);PORTB|=(1<<1);_delay_ms(5);
          }
        }
        else{
          pag(7);clm(118);indicator(3);
          for(i=0;i<vol;i++){
            PORTB&=~(1<<2);_delay_ms(35);PORTB|=(1<<2);_delay_ms(5);
          }
        }
      }
      break;

/*    
    case 0x629D: //EEPROM Write
      if(cts==0){eep_write(0);}
      break;
*/    

    case 0x30CF:
      if(cts==0){
      sk1=0;sk2=0;
        if(fa){tuna=558;}else{tunf=250;} //558 CRK, 76.5 FM COCOLO
        tune();
      }
      break;

    case 0x18E7:
      if(cts==0){
      sk1=0;sk2=0;
        if(fa){tuna=666;}else{tunf=324;} //666 NHK-1, 80.2 FM 802
        tune();
      }
      break;

    case 0x7A85:
      if(cts==0){
      sk1=0;sk2=0;
        if(fa){tuna=828;}else{tunf=422;} //828 NHK-2, 85.1 FM OSAKA
        tune();
      }
      break;

    case 0x10EF:
      if(cts==0){
      sk1=0;sk2=0;
        if(fa){tuna=1143;}else{tunf=482;} //1143 KBS, 88.1 NHK-FM
        tune();
      }
      break;

    case 0x38C7:
      if(cts==0){sk1=0;sk2=0;tunf=508;tune();} //89.4 α?-Station
      break;

    case 0x5AA5:
      if(cts==0){sk1=0;sk2=0;tunf=518;tune();} //89.9 Kiss-FM
      break;

    case 0x42BD:
      if(cts==0){sk1=0;sk2=0;tunf=532;tune();} //90.6 MBS
      break;

    case 0x4AB5:
      if(cts==0){sk1=0;sk2=0;tunf=558;tune();} //91.9 OBC
      break;

    case 0x52AD:
      if(cts==0){sk1=0;sk2=0;tunf=586;tune();} //93.3 ABC
      break;

    }//switch
  }//if(pw)
}//void

ISR(INT0_vect){ //IR Remote Control

  //------ 1st --------------------------------
  cti=0;while(~PIND & ifd){cti++;_delay_ms(0.1);} //leader(16T) check
  if(cti>80 && cti<100){    //cti:90(16T)

    cti=0;while((PIND & ifd)&&(cti<55)){cti++;_delay_ms(0.1);} //blank(8T) check

    if(cti>40){   //cti:45(8T)
      cts=0;
      for(ib=0;ib<32;ib++){
        while(~PIND & ifd){}
        cti=0;while((PIND & ifd)&&(cti<20)){cti++;_delay_ms(0.1);}
        if(cti>12){bt=1;}else{bt=0;} //cti:16(3T)
        cmd=cmd<<1;   //cmd:command
        cmd|=bt;
        if(ib==15){ads=cmd;cmd=0;}  //ads:address
      }
      if(ads==0x00FF){
        ope();
        cti=0;while((PIND & ifd)&&(cti<10)){cti++;_delay_ms(0.1);}  
        while(~PIND & ifd){} //stop bit
      }
    }//if(cti>30
  }//if(cti>40

  //------ Repeat -----------------------------
  if(pw){
  while(1){
    cti=0;while((PIND & ifd)&&(cti<1100)){cti++;_delay_ms(0.1);}

    cti=0;while(~PIND & ifd){cti++;_delay_ms(0.1);} //leader(16T) check
    if(cti>50 && cti<100){    //cti:90(16T)
      cti=0;while((PIND & ifd)&&(cti<31)){cti++;_delay_ms(0.1);} //blank(4T) check
      if(cti>15 && cti<26){cts++;ope();}//cti:22(4T)
    }
    else{break;}
    while(~PIND & ifd){}
  }//while(1)
  }//if(pw)
  ads=0;cmd=0;
}//ISR(INT0

ISR(TIMER1_COMPA_vect){ //ST,SS

  if(fa){rd(0x24);} //AM Read
    else{
      rd(0x12); //FM Read
      if(((reh & 3)==3)&&(fa==0)){pag(0);clm(1);indicator(0);} //ST(Stereo)
        else{pag(0);clm(1);dat2(10,0x00);}
    }

  //------ SS (Signal Strength) ---------
  if(fa){ss=reh & 0b00011111;}else{ss=rel>>3;}
  if(ss>24){ss=24;}

  if(ss1!=ss){
    for(i=1;i<(6-(ss-1)/4);i++){pag(i);clm(4);dat2(4,0x00);}
    for(i=(7-(ss-1)/4);i<7;i++){pag(i);clm(4);dat2(4,0xFF);}

    pag(6-(ss-1)/4);clm(4);
    if(ss==0){pag(6);clm(4);dat2(4,0x00);}
    else{
      if(ss%4==0){dat2(4,0xFF);pag(5-(ss-1)/4);clm(4);dat2(4,0x00);}
      else{dat2(4,0xFF<<(4-ss%4)*2);}
    }
  }
  ss1=ss;
}

ISR(TIMER0_COMPA_vect){
                  
  cta++;

  if(cta%10==0){ //Battery Meter

    ADCSRA=ADCSRA | 0b01000000;
    while(ADCSRA & 0b01000000);
    ad+=ADC;

    if(cta==100){               
      ad=ad/10;
      if(ad1!=ad){
        pag(0);
        if(ad>929){clm(119);dat2(1,0xfc);ad=929;}else{clm(119);dat2(1,0xcc);}
        if(ad<840){pw=0;}
        if(ad<849){ad=848;}
        clm(120);dat2(7,0x84);
        clm(127-(ad-838)/11);for(i=0;i<(ad-838)/11;i++){dat2(1,0xfc);}
      }
      cta=0;
      ad1=ad;
      ad=0;
    }
  }

  if(cta%30<15 && mut){pag(7);clm(118);indicator(4);} //Mute
  if(cta%30>14 && mut){pag(7);clm(118);dat2(10,0);}
}

//--------- Main -------------------------------------------------------
int main(void){

  DDRB =0b01011110; // PB6:VCC, PB4:RES, PB3:SD, PB2:Vol_up, PB1:Vol_down
  PORTB=0b01010110;
  DDRD =0b00000000; // PD2:IR
  PORTD=0b00000000;

  TWBR =0b00000111; // TWBR:7
  TWSR =0b00000000; // SCL:8MHz/(16+2*7*1)=266kHz

  TCCR1A=0b00000000; // ST, SS
  TCCR1B=0b00001100;
  TIMSK1=0b00000010;
  OCR1A =10000;

  TCCR0A=0b00000010; // Battery, Mute
  TCCR0B=0b00000101;
  TIMSK0=0b00000010;
  OCR0A =254;

  ADMUX =0b00000000; // AREF:3.3V, ADC0 enable
  ADCSRA=0b10000111; // CK/128

  SMCR  =0b00000101; // Power down mode
  EICRA =0b00000001; // INT0: Any logical change
  EIMSK =0b00000001; // INT0: Enable
  
  _delay_ms(500);
  kt0913_ini();
  PORTB|=(1<<3);
  _delay_ms(100);
  for(i=0;i<32;i++){cmd=0xA857;ope();} // Volume down
  wt(0x0F,0b10001000,0b00011111); // Volume[4:0]
  oled_ini();
  tune();
  for(i=0;i<5;i++){cmd=0x906F;ope();} // Volume up
  sei();

//---------------------------------------------------------
while(1){

if(sk1){ //Seek-
  for(;;){
    if(fa){ //AM
      tuna-=9;
      if(tuna<531){tuna=531;sk1=0;}
      off;tune();on;
      _delay_ms(180);
      if(sk1==0 || tuna==531){break;}   
    }
    else{ //FM 
      tunf-=2;
      if(tunf<240){tunf=240;sk1=0;}
      off;tune();on;TCNT1=9990;
      _delay_ms(150);
      if(sk1==0 || tunf==240){break;}
    }
  }
}

if(sk2){ //Seek+
  for(;;){
    if(fa){ //AM
      tuna+=9;
      if(tuna>1602){tuna=1602;sk2=0;}
      off;tune();on;
      _delay_ms(180);
      if(sk2==0 || tuna==1602){break;}    
    }
    else{ //FM 
      tunf+=2;
      if(tunf>640){tunf=640;sk2=0;}
      off;tune();on;TCNT1=9990;
      _delay_ms(150);
      if(sk2==0 || tunf==640){break;}
    }
  }
}

if(pw && PRR){ //Power on
  off;
  PORTB|=0b01010110;
  _delay_ms(100);
  oled_ini();
  if(vol>0){
    pag(6-(vol-1)/4);clm(121);
    if(vol%4==0){dat2(4,255);}else{dat2(4,255<<(4-vol%4)*2);}
    for(i=0;i<(vol-1)/4;i++){pag(6-i);clm(121);dat2(4,255);}
  }
  _delay_ms(500); 
  kt0913_ini();
  wt(0x0F,0b10001000,0b00011111); // Volume[4:0]
  tune();
  PORTB|=(1<<3);
  on;PRR=0;ADCSRA=0b10000111;
}

if(pw==0){ //Power off
  TCCR1B=0;
  TCCR0B=0;
  PORTB=0;
  ad1=0;
  ss1=0;
  ADCSRA=0;
  PRR=1;
  _delay_ms(1000);
  pw=0;
  sleep_mode();
}

}//while(1)
}//int main