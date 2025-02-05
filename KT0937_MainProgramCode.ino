#include <Wire.h>
const unsigned int data[]=
{
  0x0f,0x1f, //addressDisMUTE
  //0x0f,0x00, //addressMUTE
  0x10,0x00, // StandbyLDO Dis
  0x3a,0x1f, //highSWagc
  0x52,0x0e, //lowSWagc
  0x16,0x9a, //SW switch(0x8a is MW)
  0x1a,0xc8, //addressDisSoftMute
  0x2f,0x01, //SoftmuteFilter Dis
  //0x51,0x02, //CHpinDialmode
  0x51,0x01, //CHpinKeymode
  0x62,0x41, //GAIN and BandWidth set
  0x67,0x1b, //0x1b(27) defaultAGC
  0x68,0x05, //0x16(22) defaultAGC
  0x69,0x80, //MWvolume
  0x71,0x80, //CH ADC off
  0x98,0x02, //Low AMchan 14:8 0x02
  0x99,0x0a, //Low AMchan 7:0 0x0a
  0x9a,0x04, //Chan num 11:8 0x04
  0x9b,0x4a, //Chan num 7:0 0x4a
  0x8c,0x06, //High AMchan 14:8 0x06
  0x8d,0x54, //High AMchan 7:0 0x54
  0x18,0x59, //FM frequency step MW_SPACE
  //0x19,0x31, // SW_SPACE
  0xa0,0x17, //CH_GUARD
  //0x74,0x01, //register CH_ADC_WIN
  //0x75,0x14, //register CH_ADC_WIN
  //0x88,0x46, //CangeToAM (keep currient band)
  0x88,0xc6, //CangeToAM (cange band)
  0x71,0x04, //CH ADC on
  0xf4,0xb0, //SW muteDis
  //0x4f,0x81, //INTpin High
  //0x4f,0x82, //INTpin Low
  0xff,0x00, //finish
};
int dat03=0;
int pwrfs=0;
int CHhighbyte=0;
int CHlowbyte=0;
int CHhbShift=0;

void setup() {
  Wire.begin();
  Wire.beginTransmission(0x35);
  Wire.write(0x02);
  Wire.endTransmission(false);
  Wire.requestFrom(0x35,1);
  dat03=Wire.read();
  delay(1);

  Wire.beginTransmission(0x35);
  Wire.write(0x1b);
  Wire.endTransmission(false);
  Wire.requestFrom(0x35,1);
  pwrfs=Wire.read();
  delay(100);

  Wire.beginTransmission(0x35);
  Wire.write(0x0e); //address
  Wire.write(0x00); //dataPoweron
  Wire.endTransmission();
  delay(1);

  Wire.beginTransmission(0x35);
  Wire.write(0x76); //address
  Wire.write(0xa6); //dataPoweron
  Wire.endTransmission();

  Wire.beginTransmission(0x35);
  Wire.write(0x04); //address
  Wire.write(0x80); //ClockInitializeCompSignal
  Wire.endTransmission();

  int i=0;
  while (data[i] !=0xff) {
    Wire.beginTransmission(0x35);
    Wire.write(data[i]); //address
    Wire.write(data[i+1]); //data
    Wire.endTransmission();
    i+=2;
    delay(100);
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  //Serial.println(dat03);
  //Serial.println(pwrfs);

  Wire.beginTransmission(0x35);
  Wire.write(0xe4);
  Wire.endTransmission(false);
  Wire.requestFrom(0x35,1);
  CHhighbyte=Wire.read();
  Wire.beginTransmission(0x35);
  Wire.write(0xe5);
  Wire.endTransmission(false);
  Wire.requestFrom(0x35,1);
  CHlowbyte=Wire.read();
  CHhbShift = CHhighbyte*0B100000000 + CHlowbyte;
  Serial.println(CHhbShift);
  delay(1000);
}