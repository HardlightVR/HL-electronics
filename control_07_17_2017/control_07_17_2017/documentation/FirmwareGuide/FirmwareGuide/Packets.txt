# Introduction to Packets

Packets are how the user's computer communicates with the NullSpace hardware. They consist of a series of bytes, that dictate what the hardware should be doing and request information in return. Packets are sorted by their Instruction Byte, which determines what the packet actually does, and have a variable number of additional bytes for alighment, data integrity, and command parameters. To better represent how bytes are sent most values in this readme are in hexadecimal with a format of 0xXX: if this confuses you, you can brush up on how hexadecimal works and why it is important here: https://en.wikipedia.org/wiki/Hexadecimal

Email lucian@nullspacevr.com with questions. 


# Sent Packets:

Sent packets can have variable length. Every packet will look something like this (bytes are separated by the | character):

| 0x24 | 0x02 | INST | LENGTH | PARAM1 | …. PARAMX | 0xFF | 0xFF | {0x0A} |

The structure is as follows:

**Header**

| 0x24 | 0x02 |

These two bytes indicate the packet is starting. Their values are arbitrary, and are based off the packet structure used by Invensense's teapot demo. 

**Signifiers**

| INST | LENGTH |

The instruction byte determines what the packet is supposed to do, whether that's to write a command to a motor, request a status byte from the arduino, or request a stream of tracking data. You can find a complete summary of the different possible instructions in this file. Not every instruction has the same number of bytes, so the length byte exists to tell the arduino how many byte's it's supposted to read. Both of these signifier bytes will exist in every packet. 

**Parameters**

| PARAM1 | PARAM2 | …. PARAMX |

Parameter bytes contain the information of an instruction. This could be pad numbers, effect numbers, register values, etc. The number of parameter bytes varies by instruction. See each individual instruction to find out what the parameter values are. 

**Footer**

| 0xFF | 0xFF | 0x0A |

The footer concludes the packet. These three bytes conclude every possible packet and should always be included in this exact format.

# Return Packets:
Unlike sent packets, return packets are always 16 bytes in length. Typically most of those bytes are 0. Every packet will look something like this (bytes are separated by the | character):

| 0x24 | 0x02 | RETURN_TYPE | PARAM1 | ...PARAM_11 | 0x00 | 0x00 | 0x0d (\r) | 0x0a (\n) |

*or*

```{ '$', 0x02, 0x33, 0,0, 0,0, 0,0, 0,0,  0,  0x00, 0x00, '\r', '\n' }```

# Return packet list
<table>
<tr>
<td>Return Type</td>
<td>Name</td>
<td>Purpose:</td>
<td>Parameters:</td>
</tr>
<tr>
<td>0x01</td>
<td>Version</td>
<td>Return the suit version, directly programmed</td>
<td>[3]mark#, [4]revision#</td>
</tr>
<tr>
<td>0x02</td>
<td>Ping</td>
<td>Return an empty byte to test connection</td>
<td>void</td>
</tr>
<tr>
<td>0x03</td>
<td>InitMessage</td>
<td>A message packet that returns status data</td>
<td>```iniPacket[3] = inicount;
  iniPacket[4] = message; 
  iniPacket[5] = device;
  iniPacket[6] = success;
  iniPacket[7] = param1;
  iniPacket[8] = param2;</td>```
</tr>
<tr>
<td>0x15</td>
<td>DRV Data</td>
<td>Return a byte from the DRV (usuallys status)</td>
<td>[3]byte,[4]drv#,[5]reg#</td>
</tr>
<tr>
<td>0x33</td>
<td>teapotPacket</td>
<td>Return 8 bytes of tracking data</td>
<td>[3,4]w [5,6]x, [7,8]y, [9,10]z, [11]imu#, [12]count#</td>
</tr>
<tr>
<td>0x34</td>
<td>Overflow</td>
<td>Return an empty byte to signify overflow</td>
<td>void</td>
</tr>
</table>


# Other Notes

#### Example: Tracking return packet

| 0x24 | 0x02 | 0x33 | IMU_NUMBER | Q1_1 | Q1_2 | Q2_1 | Q2_2 | Q3_1 | Q3_2 | Q4_1 | Q4_2 | Q2_2 | 0x00 | 0x00 |

Quaternion are two byte undivided integers for efficiency. To get their real values, perform the following operation:
QuaternionN = ((QN_1 << 8) | QN_2) / 16384.0f

*Note*
Quaternion data is ordered WXYZ

#### DRV Read Data

DRV Read data will attemp to read a single byte of data from anywhere in the DRV's register map, as specified in the instruction. Here are some common locations to read from:

* **0x00 Status Register**: used for detecting issues. We only really care about bits 0 and 1, which are flags for temperature and current overloads (which shut down the chip and require a reset), as shown below.
<table>
<tr>
<td>Bit 7-5</td>
<td>Bit 4</td>
<td>Bit 3</td>
<td>Bit 2</td>
<td>Bit 1</td>
<td>Bit 0</td>
</tr>
<tr>
<td>Device ID</td>
<td>Reserved</td>
<td>Diagnostic result</td>
<td>Reserved</td>
<td>OVER TEMPERATURE</td>
<td>OVER CURRENT</td>
</tr>
</table>

* **0x01 Control Register**: used for checking standby and reading the MODE. Chips start in Standby, so if you see that bit asserted, the chip hasn't been initialized. Read MODE to see the different modes. 
<table>
<tr>
<td>Bit 7</td>
<td>Bit 6</td>
<td>Bit 5-3</td>
<td>Bit 2-0</td>
</tr>
<tr>
<td>Dev Reset</td>
<td>STANDBY</td>
<td>Reserved</td>
<td>MODE</td>
</tr>
</table>
**Modes:** don't worry about the ones that aren't listed, we can't use them.
	* [0] INTRIG: Internal Trigger Mode. The default state, where effects are loaded into memory, and the Go bit is used to turn them on. 
	* [4] AUDIOVIBE: Audio Mode. Sound is applied to the external trigger pin and used to play haptics based on audio. 
	* [5] RTP: Real Time Playback Mode. Set a DRV to this mode and it'll play a constant buzz at an intensity corresponding to the contents of the 0x02 RTP_INPUT register (0 is off, 255 is full strength). 

* **0x02 RTP INPUT**: check this to see what volume the RTP mode is set to play at.
* **0x04 WAV_FRM_SEQ1**: bits 0-6 of this register should tell you what effect you're currently set to play when using INTRIG mode. Ignore the 7th bit. 

# Instruction List:
Note: instructions in Italics are reserved but *not implemented* (yet)

### Instructions 0x00-0x0F:
Reserved for use by the Arduino. Speculative stuff:

<table>
<tr>
<td>Value:</td>
<td>Name</td>
<td>Purpose:</td>
<td>Parameters:</td>
</tr>
<tr>
<td>0x00</td>
<td></td>
<td></td>
<td></td>
</tr>
<tr>
<td>0x01</td>
<td>GET_VERSION</td>
<td>Ask for the version of the suit</td>
<td>void</td>
</tr>
<tr>
<td>0x02</td>
<td>STATUS_PING</td>
<td>Ping the arduino for a status packet</td>
<td>void</td>
</tr>
</table>


### Instructions 0x10 - 0x2F:

Reserved for use by the haptic driver network 

<table>
<tr>
<td>Value:</td>
<td>Name</td>
<td>Purpose:</td>
<td>Parameters</td>
</tr>
<tr>
<td>0x10</td>
<td></td>
<td></td>
<td></td>
</tr>
<tr>
<td>0x11</td>
<td> *DRV_RESET_DRIVERS* </td>
<td>Resets every driver on the body</td>
<td>void</td>
</tr>
<tr>
<td>0x12</td>
<td> DRV_INIT_MOTOR_DRIVERS </td>
<td>Initialize every driver on the body</td>
<td>void</td>
</tr>
<tr>
<td>0x13</td>
<td>DRV_PLAY_EFFECT</td>
<td>Write an effect to a single driver and go</td>
<td>pad, effect</td>
</tr>
<tr>
<td>0x14</td>
<td> *DRV_WRITE_DATA* </td>
<td>Write to a register on a single driver</td>
<td>pad, register, data</td>
</tr>
<tr>
<td>0x15</td>
<td> DRV_READ_DATA </td>
<td>Read from a register on a single driver</td>
<td>pad, register</td>
</tr>
<tr>
<td>0x16</td>
<td>DRV_LOAD_CONTINUOUS </td>
<td>Load an effect without playing it</td>
<td>pad, effect</td>
</tr>
<tr>
<td>0x17</td>
<td>DRV_HALT_SINGLE</td>
<td>Stop a single motor by deselecting Go register</td>
<td>pad</td>
</tr>
<tr>
<td>0x18</td>
<td> DRV_STREAM_CONTINUOUS </td>
<td>Streams continuous play data. Does not change go registers by itself, depends on the contplay routine to activate.</td>
<td>[4->10]contplay state</td>
</tr>
<tr>
<td>0x19</td>
<td>DRV_PLAY_CONTINUOUS</td>
<td>Play a motor continuously</td>
<td>pad, effect</td>
</tr>
<tr>
<td>0x1A</td>
<td>DRV_AUDIOMODE_ENABLE</td>
<td>Turns on audio mode for one motor</td>
<td>[3]pad, [4]audiomax d0x22, [5]audiomin d0x04, [6]peaktime d0x01, [7]filter d0x01</td>
</tr>
<tr>
<td>0x1B</td>
<td>DRV_INTRIGMODE_ENABLE</td>
<td>Switches to intrig mode (default)</td>
<td>pad</td>
</tr>
<tr>
<td>0x1C</td>
<td>DRV_RTPMODE_ENABLE </td>
<td>Sets mode to Realtime </td>
<td>pad</td>
</tr>
<tr>
<td>0x1D</td>
<td>DRV_PLAY_RTP </td>
<td>Load a volume from 0 to 255 using RTP</td>
<td>pad, volume/td>
</tr>
<tr>
<td>0x1E</td>
<td> *DRV_SET_WAVEFORM* </td>
<td>Add a waveform to the current queue</td>
<td>pad, waveform</td>
</tr>
<tr>
<td>0x1F</td>
<td> *DRV_GO* </td>
<td>Play the current queue</td>
<td>pad</td>
</tr>
<tr>
<td>0x20</td>
<td> *DRV_CHECK_INIT* </td>
<td>Check the initialization status of a single DRV</td>
<td>pad</td>
</tr>
<tr>
<td>0x21</td>
<td> *DRV_CHECK_INIT_ALL* </td>
<td>Check the initialization status of every DRV</td>
<td>void</td>
</tr>

</table>


### Instructions 0x30 - ?:

Reserved for use by the inertial tracking network

<table>
<tr>
<td>0x32</td>
<td>INIT_TRACKING</td>
<td>Re-initialize the tracking network</td>
<td>void</td>
</tr>
<tr>
<td>0x33</td>
<td>MPU_GET_DATA</td>
<td>gets data from one MPU</td>
<td>sensor_id</td>
</tr>
<tr>
<td>0x34</td>
<td>ENABLE_TRACKING</td>
<td>Turn on tracking</td>
<td>void</td>
</tr>
<tr>
<td>0x35</td>
<td>DISABLE_TRACKING</td>
<td>turn off tracking</td>
<td>void</td>
</tr>
</table>

That's all, folks. 
