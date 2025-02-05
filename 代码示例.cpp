  //FM freq setting
  // low edge frequency of FM band with 50KHz per LSB:set to 
  Wire.beginTransmission(DEVICE_ADDRESS);
  Wire.write(0x98);
  Wire.write(0x05);      // 05F0(HEX) = 1520(DEC) x 50KHz = 76MHz 
  Wire.endTransmission();
  delay(100);
  //
  Wire.beginTransmission(DEVICE_ADDRESS);
  Wire.write(0x99);
  Wire.write(0xF0);     // 05F0(HEX) = 1520(DEC) x 50KHz = 76MHz
  Wire.endTransmission();
  delay(100);
  
  // Channel number of band:set to 
  Wire.beginTransmission(DEVICE_ADDRESS);
  Wire.write(0x9A);
  Wire.write(0x01); //   154h=680/2   680(DEC) =  2200(DEC) - 1520(DEC) : 110MHz - 76MHz
  Wire.endTransmission();
  delay(100);
  //
  Wire.beginTransmission(DEVICE_ADDRESS);
  Wire.write(0x9B);
  Wire.write(0x54);
  Wire.endTransmission();
  delay(100);
  //FM freq setting end
  
  
  
    //smute gain: set to -36dB
  Wire.beginTransmission(DEVICE_ADDRESS);
  Wire.write(0xE3);      // Volume control
  Wire.write(0x02);      // 0x00 Mute 0x01-0x80 20log(SMUTE_GAIN/128)
  Wire.endTransmission();
  delay(100);
  
  //预存电台示例
  const unsigned int FM[]=
{
  0xE4,0x06, // HF, 78.9MHz
  0xE5,0x2A, // LF, 78.9MHz
  0xE4,0x06, // HF, 80.0MHz
  0xE5,0x40, // LF, 80.0MHz
  0xE4,0x06, // HF, 80.7MHz
  0xE5,0x4e, // LF, 80.7MHz
  0xE4,0x06, // HF, 81.3MHz
  0xE5,0x5A, // LF, 81.3MHz
  0xE4,0x06, // HF, 82.5MHz
  0xE5,0x72, // LF, 82.5MHz
};