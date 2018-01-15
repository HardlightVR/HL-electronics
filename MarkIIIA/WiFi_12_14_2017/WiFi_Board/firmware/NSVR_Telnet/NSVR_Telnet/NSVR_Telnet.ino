
/*
 *  NSVR_Telnet : Transparent telnet link for ESP8266
    -------------------------------------------------
    This Arduino IDE sketch for the ESP8266 platform
    (or the Arduino Platform + WiFI Shield Configuration) 
    is used to provide a transparent telnet link for the NSVR Suit
    to a host computer. It may be used in one of two ways :
    
    1) As a Wi-Fi Access Point, which the Host Computer connects with directly. 

      1A) Flash the MarkIII Suit Wi-Fi board with this program
      1B) Install the MarkIII Suit Wi-Fi board into a suit
      1C) Power on the suit, note a new Wi-Fi Access Point named NVSR:xx:xx:xx:xx:xx:xx
          where "xx" is the internal MAC-ID of the Wi-Fi Interface.
      1D) Connect your Windows 10 Wi-Fi to the Wi-Fi Access Point named NSVR:xx:xx:xx:xx:xx:xx,
          using the password of base64(xx:xx:xx:xx:xx:xx).
          It will issue a DHCP IP address (IPV4) to your Win10 host in the "192.168.4.zzz" range,
          the Wi-Fi Access Point named NVSR:xx:xx:xx:xx:xx:xx will have the address "192.168.4.1".
      1E) Install "HW VSP3 - Virtual Serial Port" (free application)
          https://www.hw-group.com/products/hw_vsp/index_en.html 
          and then run the "HW Virtual Serial Port" application.
      1F) Use the "HW Virtual Serial Port" application to create a COMx: port on the Win10 host
          that is connected to 192.168.4.1:23 (telnet) with the correct password.
      1G) Run the NSVR DiagTool application and connect to the COMx: port, wirelessly.
      
    2) As a Wi-Fi Access Point, which the "NVSR Wi-Fi Host Adapter" connects with directly
       2A) Plug the "NVSR Wi-Fi Host Adapter" into your WIN10 host computer, 
           it will enumerate as a COMy: port and automatically connect to it's matching suit.
           (it will match the suit using the MAC:ID of the MarkIII Suit Wi-Fi Board to which it is paired.)

   ====================================================================================== 
    SECURITY CAUTION: (WARNING!)
    These routines employ only simple authentication methods.
    These are not intended to be highly secure, that requires a bit more work.

    SIMPLE AUTHENTICATION:
    The authentication scheme used here is very, very simple, where the
    default device password is a lowercased base64 function of that 
    devices access point Media Access Control, or Ethernet Hardware Address.
    
    The default device password will be printed out on the Serial Debug Monitor during bootup,
    and should be unique for each device. This is important in a debug situation
    to get an idea of what the device is doing.

    Embedis itself is transparent, so a more secure implementation would encrypt
    the values (and even the keys themselves) prior to calling Embedis. 
    In this sketch, all of the keys and values are in unencrypted plain-text, 
    for ease of debugging.
    
    A more secure inplementation would use encryted keys and values, 
    however that is beyond of the scope of this first Wi-Fi SOW.

   ====================================================================================== 
    Based on Open Source Software: (WARNING!)
    
    This program is based on several existing open source tools (Arduino) 
    and libraries (Serial, Embedis, Telnet, ESP8266, etc.)
    
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
   ======================================================================================  
    Embedis Internal Database :
    
    The Embedis Server (similar in operation to the Redis.IO server)
    can be used to get/set keys in the embedded processor's EEPROM database,
    such as the the WiFi SSID and Passphrase. You no longer need to embed 
    that "data" into your "program"! The EEPROM is persistant, and can be
    used across multiple programs to configure your program setting for
    the specific hardware and network configuration you are using. 
    
    Now, you don't need to recompile your program and reflash your device
    in order to change the device settings anymore. If your ESP8266 WiFi
    doesn't connect, it reverts to an Access Point with a Web Server to allow
    you to change the configuration settings and join another WiFi network.

    We think that this is a much better mechanism for storing settings between projects.
    Once you start using Embedis for your projects, you'll see how quick and easy
    it is to move and reconfigure your devices without needing to recompile 
    and reflash your device firmware just to change a configuration setting.
    Now settings can be changed "on-the-fly" with just a web browser.
    (some settings changes however, will require a restart/reboot to take effect.)
    
   ======================================================================================
*/
/*
  
*/
#if !defined(ARDUINO_ARCH_ESP8266)
#error "This Sketch is for the ESP8266/ESP32 Platform only!"
#error "untested on other platforms..."
#endif

#include <ESP8266WiFi.h>
/*  Embedis Dictionary Server 
 *  Database  : Located in the flash emulated EEPROM area
 *  Interface : Serial1 (TXD=IO2, RXD=none!) Write/Log/Debug Only (no input) 
*/
#include "Embedis.h"
Embedis embedis(Serial1); /* the LOG/console Serial monitor */

static uint8_t led;      /* a blinky LED (GPIO output)     */

void setup() 
{
    /* start the LOG/console channel first, to log everything else... */   
    Serial1.begin(9600);
    delay(50);
    Serial.begin(115200);
    LOG( String() + F(" ") );
    LOG( String() + F("[ ==================================================== ]") );
    LOG( String() + F("[ Embedis : WWW/Telnet/CLI Servers Sketch for ESP8266! ]") );
    LOG( String() + F("[ ==================================================== ]") );
    
    setup_EEPROM();   // keep this second, the configuration settings are loaded here
    setup_vcc();
    setup_commands();
    setup_telnet();

    
    // setup the LED pin based on the Embedis key "led_pin"
    String led_pin_number = setting_led_pin();
    led = (uint8_t) led_pin_number.toInt();
    pinMode(led, OUTPUT);
    LOG( String() + F("[ Embedis : Platform led_pin_number: ") +  led_pin_number + F(" led_pin: ") + led + F(" ]"));
}

void loop() 
{
    embedis.process();    // process the Embedis Command Line
    yield();
    blink( loop_wifi() ); // blink the LED
    yield();
    loop_telnet();        // process the telnet server
    yield();
}


// Blink out a number. More than 2 may be hard to count.
// Using 0 blinks fast and steady.
void blink(int num) {
    static unsigned long heartbeat = 0;
    static int beatcount = 0;
    static uint8_t led = 13;

    unsigned long now = millis();
    if (now > heartbeat) {
        if (digitalRead(led)) {
            digitalWrite (led, 0);
            if (!num) heartbeat = now + 250;
            else heartbeat = now + 1;
        } else {
            digitalWrite (led, 1);
            if (!num) heartbeat = now + 250;
            else {
                if (beatcount) {
                    --beatcount;
                    heartbeat = now + 175;
                } else {
                    led++;
                    if (led > 32) led = 30;
                    beatcount = num - 1;
                    heartbeat = now + 999;
                }
            }
        }
    }
}


// This will log to an embedis channel called "log".
// Use SUBSCRIBE LOG to get these messages.
// Logs are also printed to Serial until an empty message is received.
// Success with WiFi sends an empty message.
void LOG(const String& message) {
    static bool inSetup = true;
    if (inSetup) {
        if (!message.length()) {
            inSetup = false;
            return;
        }
        //SERIAL_PORT_MONITOR.println(message);
        Serial1.println(message);
    }
    Embedis::publish("log", message);
}
