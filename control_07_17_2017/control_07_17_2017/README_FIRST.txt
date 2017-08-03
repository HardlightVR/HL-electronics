Control PCBA 07_17_2017 Release Notes :
----------------------------------------

1) Separate STM32 power from VR Suit Power using existing LMS1585AIS 3.3V Voltage Regulator
2) Add second 3.3V Voltage Regulator (LDO) for STM32 ( MIC5365-3.3YC5-TR3 = $0.12)
3) Add PCB stuffing option for  STM32 USB support using Wireless Connector.
4) Change FTDI USB-UART from UART1 to UART2 (RX/TX/RTS/CTS)
   Change FTDI VCCIO from 5V to 3.3 volt (CPU)
5) Change Wireless Connector from UART2 to UART1
6) Add 8mhz Crystal, series resistor, and tuning capacitors to STM32
7) Add Ferrite bead for STM32 VDDA Power.
8) Remove BMD-300 footprint (add to Wireless Board)
9) Regenerate Gerber files with improved CAM & DRC files
     for 7 mil trace/space rule using tented vias to reduce solder shorts in manufacturing

IMPORTANT CHANGES!
------------------
1) The Pinout of the Wireless Expansion Connector was changed to enable USB transmission line routing.
2) SC5/SD5 moved to the Wireless Expansion Connector to reduce capacitive loading on I2C2 (motor) bus
3) FTDI_ON (!PWREN) was moved to GPIO PC8
4) LED1 was moved from 3.3V power to GPIO PC13 (RED)
5) LED2 was added to GPIO PC14 (GREEN)
6) LED3 was added to GPIO PC15 (BLUE)  (LED's can now be used to indicate board status...)
7) UART6 (External FTDI Adapter connector was added for backchannel debugging)
8) BOOT1 Control was added for USB/UART bootloading
9) Reset RC time constant circuit added
10) FT232RL and PCA9548 added to Reset circuit for stability

Note: if extra functions (LED2/3) not need, just remove those line items from the BOM.

-----------------------------------------------------------------------------------------
