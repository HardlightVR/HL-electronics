//NS-FIRMWARE 2.13
//New instructions
//NOTES: Modes, audio, rtp, and streaming contplay
//PROBLEMS: 
//Lucian 7.29.16

//MK version
#define SUIT_VERSION_A      0x02
//iteration
#define SUIT_VERSION_B      0x05

#include <Wire.h>
#include "NS_DRV2605.h"
#include "NS_BNO055.h"

Adafruit_DRV2605 drv;
NS_BNO055 bno;

// ================================================================
// DEFINITIONS
// ================================================================

//This option will activate timing functions that can be used to measure 
//cycle or instruction execution times
//#define TIMING

#define BEGIN_INIT           0x00
#define MPU_BEGIN_INIT       0x01
#define MPU_START_OK         0x02
#define MPU_CONNECT_OK       0x03
#define MPU_DMP_INIT_OK      0x04
#define MPU_CONFIG_OK        0x05
#define MPU_CONFIG_FAIL_MSG  0x06
#define MPU_DMP_ENABLE_OK    0x07
#define MPU_DMP_READY_OK     0x08
#define MPU_END_INIT         0x09


// ================================================================
// VARIABLES
// ================================================================

//MPU6050 mpu2(0x69);
int led_num = 6;
int led_gnd = 9;

//declare all splitter addresses (7 possible, 0x77 used for MPUs)
uint8_t TCA_Address[8] = {0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77};
//status array for continuous play
uint8_t motorStatus[8] = {0,0,0,0,0,0,0,0};

char inData[80]; //Storage array for incoming data
int cont_count = 0; //continuous play counter
int indata_count = 0;
bool tracking_ready = false;
int inicount = 0; //global var for tracking initialization

//return tracking data packet
uint8_t teapotPacket[16] = { '$', 0x02, 0x33, 0,0, 0,0, 0,0, 0,0,  0,  0x00, 0x00, '\r', '\n' };
uint8_t overflowPacket[16] = { '$', 0x02, 0x34, 0, 0,0,0,0,0,0,0,0,0,0, '\r', '\n' };

//other packets
uint8_t versionPacket[16] = { '$', 0x02, 0x01, 0,0,0,0,0,0,0,0,0, 0x00, 0x00, '\r', '\n' };
uint8_t pingPacket[16] = { '$', 0x02, 0x02, 0,0,0,0,0,0,0,0,0, 0x00, 0x00, '\r', '\n' };
uint8_t iniPacket[16] = { '$', 0x02, 0x03, 0,0,0,0,0,0,0,0,0, 0x00, 0x00, '\r', '\n' };
//inipacket: count, message, device, success, param1, param2

// ================================================================
// SETUP
// ================================================================

void setup() {
  Serial.begin(115200);  
  pinMode(led_num, OUTPUT); //LED pin
  pinMode(led_gnd, OUTPUT);
  digitalWrite(led_gnd, LOW); 
  //enable VBUS (used for unplug detection)
  USBCON|=(1<<OTGPADE);
  
  flicker_led(led_num);
  
  //Initialize electronics
  init_motor_drivers();
  init_tracking();
  get_version();
  
}

// ================================================================
// MAIN LOOP
// ================================================================

void loop() {
  //if serial isn't connected turn everything off
  if(!(USBSTA&(1<<VBUS))){  //checks state of VBUS
      if(Serial){
        Serial.end();
      }
      memset(motorStatus,0,sizeof(motorStatus));
      tracking_ready=false;
   } else {
      if(!Serial)
         Serial.begin(115200);
   }
  
  while (Serial.available() > 0)
  { 
      char recieved = Serial.read();
      inData[indata_count] = recieved; 
      
      // Process message when new line character is recieved
      if (recieved == '\n')
      {
        //gather universal data for integrity and instruction
        int instruction = inData[2];
        int p_length = inData[3];
        
        //TODO; alignment integrity check
        
        //see if the packet has the integrity bytes at the end
//        if(!(inData[p_length-1]==0xFF && inData[p_length-2]==0xFF)) {
//          indata_count++;
//          continue;
//        }
        
        //Now that we know the packet is correct, we reset the count
        indata_count = 0;
        
        switch (instruction) {
        case 0x01:
          get_version();
          break;
        case 0x02:
          status_ping();
          break;
        case 0x12:
          init_motor_drivers();
          break;
        case 0x13:
          drv_play_effect(inData); 
          break;
        case 0x15:
          drv_read_data(inData); 
          break;
        case 0x16:
          drv_load_continuous(inData);
          break;
        case 0x17:
          drv_halt_single(inData);
          break;
        case 0x18:
          drv_stream_continuous(inData);
          break;
        case 0x19:
          drv_play_continuous(inData);
          break;
        case 0x1A:
          drv_audiomode_enable(inData);
          break;
        case 0x1B:
          drv_intrigmode_enable(inData);
          break;
        case 0x1C:
          drv_rtpmode_enable(inData);
          break;
        case 0x1D:
          drv_play_rtp(inData);
          break;
        case 0x32:
          init_tracking();
          break;
        case 0x34://turn tracking back on after disconnect
          tracking_ready = true;
          break;
        case 0x35://turn tracking off
          tracking_ready = false;
          break;
        default: 
          break;
        }//end case
      } else {//end received if
        indata_count++;
      }
  }//end while
  
  //update the continuous play motors
  continuous_play_update();
  if (tracking_ready) tracking_update();

}

////TIMING
//unsigned long start = micros();
//
//unsigned long vend = micros();
//unsigned long delta = vend - start;
//Serial.println(delta);
