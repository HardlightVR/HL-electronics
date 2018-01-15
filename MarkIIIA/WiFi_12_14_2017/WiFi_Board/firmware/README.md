# NSVR_Telnet

A primitive Telnet Server/Client connection for the ESP8266,
written for the Arduino IDE.

The NSVR_Telnet application employs the Embedis database to hold 
key/value pairs that are used to control program operation without
the need to make changes to the operating code itself.

In order to configure a NSVR Wi-Fi board for the MarkII Suit :

1) Program the NSVR Wi-Fi board with "NSVR_Telnet", note the MAC:ID

2) Install the NSVR Wi-Fi board in the MarkII Suit.

3) Program the NSVR Wi-Fi board with "Embedis_ESP8266", note the MAC:ID

4) Use the command line to configure the matching pair Wi-Fi Board

5) "set wifi_mode sta"

6) "set wifi_ssid NSVR:mac:id"

7) "set wifi_passwd (base64::encode(mac, 6))"

8) Reprogram the NSVR Wi-Fi board with "NSVR_Telnet"

9) Now the two units are a "matched pair" and will connect automatically

Note:
-----
```C
// The default password is a lowercased base64 of the access point MAC.
String setting_default_passphrase() {
    uint8_t mac[6];
    wifi_get_macaddr(SOFTAP_IF, mac);
    String passphrase = base64::encode(mac, 6);
    passphrase.toLowerCase();
    return passphrase;
}
```