
/*
 *  NSVR_Telnet : Transparent telnet link for ESP8266
    -------------------------------------------------
    This Arduino IDE sketch for the ESP8266 platform
    (or the Arduino Platform + WiFI Shield Configuration) 
    is used to provide a transparent telnet link for the 
    NSVR MarkIII(A) Suit to a host computer. 
    It may be used in one of two ways :
    
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
    In this example, all of the keys and values are in unencrypted plain-text, 
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
    used across multiple projects to configure your program settings for
    the specific hardware and network configuration you are using. 
    
   ======================================================================================
*/

#if !defined(ARDUINO_ARCH_ESP8266)
#error "This Sketch is for the ESP8266/ESP32 Platform only..., untested on others..."
#endif
#include <ESP8266WiFi.h>

/* Use Embedis non-volatile database for settings */
#include "Embedis.h"
Embedis embedis(Serial1); /* the LOG/console Serial monitor on GPIO_2 */

static uint8_t led = 0;      /* a blinky LED (GPIO_0 output)     */

void setup() 
{
    /* start the LOG/console channel first, to log everything else... */   
    Serial1.begin(9600);
    delay(50);
    LOG( String() + F(" ") );
    LOG( String() + F("[ ==================================================== ]") );
    LOG( String() + F("[ Hardlight VR : Mark III Suit Wi-Fi Adapter           ]") );
    LOG( String() + F("[ ==================================================== ]") );


  
    /* start the Suit interface UART on Serial0 */
    Serial.begin(115200);
 
    // Keep this here... 
    // The configuration settings are loaded here ...
    setup_EEPROM();

    /* setup the embedis commands */
    setup_vcc();
    setup_commands();

    /* setup the UART0 hardware handshaking */
    static uint8_t cts_b = 14; // GPIO14
    pinMode(cts_b, OUTPUT);
    digitalWrite(cts_b, LOW);
    static uint8_t rts_b = 12; // GPIO12
    pinMode(rts_b, INPUT);
    /* Start the telnet server/client */
    setup_telnet();
    
    // setup the LED pin based on the Embedis key "led_pin"
    String led_pin_number = setting_led_pin();
    led = (uint8_t) led_pin_number.toInt();
    pinMode(led, OUTPUT);

}

void loop() 
{
    
    embedis.process();     // process any Embedis commands
    yield();              // let the RTOS run if needed
    blink (loop_wifi());          // service the Wi-Fi connection
    yield();              // let the RTOS run if needed
    String mode = setting_wifi_mode();  
    if (mode == "ap") {
      loop_telnet_server();        // process the telnet server
    } else {
      loop_telnet_client();        // process the telnet server
    }
    yield();              // let the RTOS run if needed
}

// Blink out a number. 
// More than 2 may be hard to count.
// Using 0 blinks fast and steady.
void blink(int num) {
    static unsigned long heartbeat = 0;
    static int beatcount = 0;

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
        //Serial.println(message);
    }
    Embedis::publish("log", message);
}
