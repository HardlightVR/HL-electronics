// ================================================================
// FLICKER LEDs
// ================================================================
void flicker_led(int led_num)
{
  digitalWrite(led_num, HIGH);
  delay(100);
  digitalWrite(led_num, LOW);
  delay(100);
  digitalWrite(led_num, HIGH);
  delay(100);  
  digitalWrite(led_num, LOW);
  delay(100);  
  digitalWrite(led_num, HIGH);  
}

//write initialization packet
void write_init(int message, int device, int success, int param1, int param2)
{
  iniPacket[3] = inicount; //what step of the init
  iniPacket[4] = message; 
  iniPacket[5] = device;
  iniPacket[6] = success;
  iniPacket[7] = param1;
  iniPacket[8] = param2;
  inicount++;
  
  Serial.write(iniPacket, 16);
}

//write init no params
void write_init(int message, int device, int success)
{
  iniPacket[3] = inicount; //what step of the init
  iniPacket[4] = message; 
  iniPacket[5] = device;
  iniPacket[6] = success;
  iniPacket[7] = 0;
  iniPacket[8] = 0;
  inicount++;
  
  Serial.write(iniPacket, 16);
}


//write init no device
void write_init(int message, int success)
{
  iniPacket[3] = inicount; //what step of the init
  iniPacket[4] = message; 
  iniPacket[5] = 0;
  iniPacket[6] = success;
  iniPacket[7] = 0;
  iniPacket[8] = 0;
  inicount++;
  
  Serial.write(iniPacket, 16);
}

// ================================================================
// INIT DRVs
// ================================================================
void init_motor_drivers()
{
  //NOTE: DRVs don't like to be bulk initialized!!!
  //they must be iterated individually.
  //Serial.println("starting init");
  //init motor drivers
  //splitter loop
  for (int i=0;i<7;i++) {
    //drv loop
    for (int j=0; j<8; j++) {
      //open TCA channel
      Wire.beginTransmission(TCA_Address[i]);
      Wire.write(0x01<<j); 
      Wire.endTransmission();
      
      drv.begin();
      drv.selectLibrary(1);
      // I2C trigger by sending 'go' command 
      // default, internal trigger when sending GO command
      drv.setMode(DRV2605_MODE_INTTRIG);
      
      //Test routine - activates every motor on body
      drv.setWaveform(0, 1);  // play effect 
      drv.setWaveform(1, 0);  // end waveform
      drv.go();
      delay(100);
      
      Wire.beginTransmission(TCA_Address[i]);
      Wire.write(0x00); 
      Wire.endTransmission();
    }
  }
}


// ================================================================
// INIT TRACKING
// ================================================================
void init_tracking()
{

  for (int i=0;i<4;i++)
  {
    Wire.beginTransmission(TCA_Address[7]);
    Wire.write(0x01<<i); 
    Wire.endTransmission();
    
    if(bno.begin())
    { //if it starts, we're good to go.
      write_init(MPU_START_OK,i,1);
      delay(100);
      //delete this in final version
      bno.setExtCrystalUse(true);      
    }
    
    Wire.beginTransmission(TCA_Address[7]);
    Wire.write(0x00); 
    Wire.endTransmission();
  }
}

// ================================================================
// CONTINUOUS_PLAY_UPDATE
// ================================================================
void continuous_play_update()
{
  //CONTINUOUS PLAY
  //(only checks one driver per cycle to save execution time)
  //drv loop
  for (int j=0; j<8; j++) {
    //if motor status bit is high, force go bit high again.
    if ( (motorStatus[cont_count] & (1 << j)) == (1 << j) )
    {
      Wire.beginTransmission(TCA_Address[cont_count]);
      Wire.write(0x01<<j);
      Wire.endTransmission();
      drv.go();
      Wire.beginTransmission(TCA_Address[cont_count]);
      Wire.write(0x00);
      Wire.endTransmission();
    }
  }
  cont_count++;
  if(cont_count>6) cont_count=0;
}

// ================================================================
// TRACKING_UPDATE
// ================================================================
void tracking_update()
{
  uint8_t n = cont_count;
  //uint8_t n = 0;
  //open channel
  Wire.beginTransmission(TCA_Address[7]);
  Wire.write(0x01<<n); 
  Wire.endTransmission();
  
  //create buffer array
  uint8_t readbuffer[8];
  memset (readbuffer, 0, 8);
  
    /* Read quat data (8 bytes) */
  bno.readLen(BNO055_QUATERNION_DATA_W_LSB_ADDR, readbuffer, 8);
  
  //store data to packet 0w 1x y2 z3
  teapotPacket[3] = readbuffer[1];
  teapotPacket[4] = readbuffer[0];
  teapotPacket[5] = readbuffer[3];
  teapotPacket[6] = readbuffer[2];
  teapotPacket[7] = readbuffer[5];
  teapotPacket[8] = readbuffer[4];
  teapotPacket[9] = readbuffer[7];
  teapotPacket[10] = readbuffer[6];
  //Write the IMU number too!
  teapotPacket[11] = n;
  teapotPacket[12]++; // packetCount, loops at 0xFF on purpose
  //write packet back to computer
  Serial.write(teapotPacket, 16);

  Wire.beginTransmission(TCA_Address[7]);
  Wire.write(0x00); 
  Wire.endTransmission();
}
  


