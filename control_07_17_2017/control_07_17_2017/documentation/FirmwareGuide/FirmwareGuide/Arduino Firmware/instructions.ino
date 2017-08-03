
// ================================================================
// STATUS_PING
// ================================================================
void status_ping() {
  Serial.write(pingPacket, 16);
}


// ================================================================
// GET_VERSION
// ================================================================
void get_version() {
  versionPacket[3] = SUIT_VERSION_A;//MKII
  versionPacket[4] = SUIT_VERSION_B; //revision 4, 25release
  Serial.write(versionPacket, 16);
}

// ================================================================
// DRV_PLAY_EFFECT
// ================================================================
void drv_play_effect(char inData[80]) {
  //constrain  values to allowed 
  int drv_addr = constrain(inData[4], 0, 64);
  int drv_effect = constrain(inData[5], 0, 123);
  //calculate TCA and driver addresses
  int splitter_addr = drv_addr/8;
  int driver_addr = drv_addr % 8;
  
  //write to the motor
  Wire.beginTransmission(TCA_Address[splitter_addr]);
  Wire.write(0x01<<driver_addr); 
  Wire.endTransmission();
  
  drv.setWaveform(0, drv_effect);  // play effect 
  drv.setWaveform(1, 0);           // end waveform
  // play the effect!
  drv.go();
  
  //close channel in case the next instruction is to new TCA
  Wire.beginTransmission(TCA_Address[splitter_addr]);
  Wire.write(0x00); 
  Wire.endTransmission();
}

// ================================================================
// DRV_READ_DATA
// ================================================================
void drv_read_data(char inData[80]) {

  uint8_t dataPacket[16] = { '$', 0x02, 0x15,0,0,0,0,0,0,0,0,0, 0x00, 0x00, '\r', '\n' };
  
  int drv_addr = constrain(inData[4], 0, 64);
  int splitter_addr = drv_addr/8;
  int driver_addr = drv_addr % 8; 
  
  //write to the motor
  Wire.beginTransmission(TCA_Address[splitter_addr]);
  Wire.write(0x01<<driver_addr); 
  Wire.endTransmission();
  
  dataPacket[3] = drv.readRegister8(inData[5]);
  dataPacket[4] = inData[4]; //need to return drv number!
  dataPacket[5] = inData[5];
  
  //close channel in case the next instruction is to new TCA
  Wire.beginTransmission(TCA_Address[splitter_addr]);
  Wire.write(0x00); 
  Wire.endTransmission();
  
  Serial.write(dataPacket, 16);
}

// ================================================================
// DRV_LOAD_CONTINUOUS
// ================================================================
void drv_load_continuous(char inData[80]) {
  //constrain  values to allowed 
  int drv_addr = constrain(inData[4], 0, 64);
  int drv_effect = constrain(inData[5], 0, 123);
  //calculate TCA and driver addresses
  int splitter_addr = drv_addr/8;
  int driver_addr = drv_addr % 8;
  
  //write to the motor
  Wire.beginTransmission(TCA_Address[splitter_addr]);
  Wire.write(0x01<<driver_addr); 
  Wire.endTransmission();
  
  drv.setWaveform(0, drv_effect);  // play effect 
  drv.setWaveform(1, 0);           // end waveform
  
  //close channel in case the next instruction is to new TCA
  Wire.beginTransmission(TCA_Address[splitter_addr]);
  Wire.write(0x00); 
  Wire.endTransmission();
}

// ================================================================
// DRV_HALT_SINGLE
// ================================================================
void drv_halt_single(char inData[80])
{
  //constrain values to allowed 
  int drv_addr = constrain(inData[4], 0, 64);
  int drv_effect = constrain(inData[5], 0, 123);
  int splitter_addr = drv_addr/8;
  int driver_addr = drv_addr % 8; 
  
  //write to the motor
  Wire.beginTransmission(TCA_Address[splitter_addr]);
  Wire.write(0x01<<driver_addr); 
  Wire.endTransmission();
  
  drv.setWaveform(0, drv_effect);  // play effect 
  drv.setWaveform(1, 0);           // end waveform
  // halt the effect
  drv.halt();
  
  //close channel in case the next instruction is to new TCA
  Wire.beginTransmission(TCA_Address[splitter_addr]);
  Wire.write(0x00); 
  Wire.endTransmission();
  
  //set the desired bit in the motor_Status array to 0
  motorStatus[splitter_addr] &= ~(1 << driver_addr);
}

// ================================================================
// DRV_PLAY_CONTINUOUS
// ================================================================
void drv_play_continuous(char inData[80])
{
  //constrain values to allowed 
  int drv_addr = constrain(inData[4], 0, 64);
  int drv_effect = constrain(inData[5], 0, 123);
  int splitter_addr = drv_addr/8;
  int driver_addr = drv_addr % 8; 
  
  //write to the motor
  Wire.beginTransmission(TCA_Address[splitter_addr]);
  Wire.write(0x01<<driver_addr); 
  Wire.endTransmission();
  
  drv.setWaveform(0, drv_effect);  // play effect 
  drv.setWaveform(1, 0);           // end waveform
  // play the effect!
  drv.go();
  
  //close channel in case the next instruction is to new TCA
  Wire.beginTransmission(TCA_Address[splitter_addr]);
  Wire.write(0x00); 
  Wire.endTransmission();
  
  //set the desired bit in the motor_Status array to 1
  motorStatus[splitter_addr] |= (1 << driver_addr);
  //x &= ~(1 << n); forces nth bit to be zero
}

// ================================================================
// DRV_STREAM_CONTINUOUS
// ================================================================
void drv_stream_continuous(char inData[80])
{
  //sets the contents of the continuous play status array
  //automatically activated up by the continuous play routine
  for(int i=0;i<7;i++)
  {
    motorStatus[i] = inData[i+4];
  }
}

// ================================================================
// DRV_AUDIOMODE_ENABLE
// ================================================================
void drv_audiomode_enable(char inData[80])
{
  //calculate drv address
  int drv_addr = constrain(inData[4], 0, 64);
  int splitter_addr = drv_addr/8;
  int driver_addr = drv_addr % 8; 
  
  //create the mask for audiovibe control
  char peaktime = constrain(inData[7], 0, 4);
  char filter = constrain(inData[8], 0, 4);
  char audiomask = (filter || peaktime<<2);
  
  
  //write to the motor
  Wire.beginTransmission(TCA_Address[splitter_addr]);
  Wire.write(0x01<<driver_addr); 
  Wire.endTransmission();
  
  drv.setMode(DRV2605_MODE_AUDIOVIBE);
  
  //Set sensitivities
  drv.writeRegister8(DRV2605_REG_AUDIOMAX,inData[5]); //default 0x22
  drv.writeRegister8(DRV2605_REG_AUDIOLVL,inData[6]); //default 0x04
  //set control
  drv.writeRegister8(DRV2605_REG_AUDIOCTRL,drv.readRegister8(DRV2605_REG_AUDIOCTRL)&&0xF0); 
  drv.writeRegister8(DRV2605_REG_AUDIOCTRL,drv.readRegister8(DRV2605_REG_AUDIOCTRL)||audiomask);
  
  // ac coupled input, puts in 0.9V bias
  drv.writeRegister8(DRV2605_REG_CONTROL1, 0x20);  
  // analog input
  drv.writeRegister8(DRV2605_REG_CONTROL3, 0xA3);  
  
  //close channel in case the next instruction is to new TCA
  Wire.beginTransmission(TCA_Address[splitter_addr]);
  Wire.write(0x00); 
  Wire.endTransmission();
}

// ================================================================
// DRV_INTRIGMODE_ENABLE
// ================================================================
void drv_intrigmode_enable(char inData[80])
{
  //constrain values to allowed 
  int drv_addr = constrain(inData[4], 0, 64);
  int splitter_addr = drv_addr/8;
  int driver_addr = drv_addr % 8; 
  
  //write to the motor
  Wire.beginTransmission(TCA_Address[splitter_addr]);
  Wire.write(0x01<<driver_addr); 
  Wire.endTransmission();
  
  drv.selectLibrary(1);
  drv.setMode(DRV2605_MODE_INTTRIG);
  
  //close channel in case the next instruction is to new TCA
  Wire.beginTransmission(TCA_Address[splitter_addr]);
  Wire.write(0x00); 
  Wire.endTransmission();
}

// ================================================================
// DRV_RTPMODE_ENABLE
// ================================================================
void drv_rtpmode_enable(char inData[80])
{
  //constrain values to allowed 
  int drv_addr = constrain(inData[4], 0, 64);
  int splitter_addr = drv_addr/8;
  int driver_addr = drv_addr % 8; 
  
  //write to the motor
  Wire.beginTransmission(TCA_Address[splitter_addr]);
  Wire.write(0x01<<driver_addr); 
  Wire.endTransmission();
  
  drv.setMode(DRV2605_MODE_REALTIME);  
  
  //close channel in case the next instruction is to new TCA
  Wire.beginTransmission(TCA_Address[splitter_addr]);
  Wire.write(0x00); 
  Wire.endTransmission();
}

// ================================================================
// DRV_PLAY_RTP
// ================================================================
void drv_play_rtp(char inData[80])
{
  //constrain values to allowed 
  int drv_addr = constrain(inData[4], 0, 64);
  int splitter_addr = drv_addr/8;
  int driver_addr = drv_addr % 8; 
  
  //write to the motor
  Wire.beginTransmission(TCA_Address[splitter_addr]);
  Wire.write(0x01<<driver_addr); 
  Wire.endTransmission();
  
  drv.writeRegister8(DRV2605_REG_RTPIN, inData[5]);  
  
  //close channel in case the next instruction is to new TCA
  Wire.beginTransmission(TCA_Address[splitter_addr]);
  Wire.write(0x00); 
  Wire.endTransmission();
}

