#include <Arduino.h>
#include <SPI.h>
#include <WiFiNINA.h>
#include <Wire.h>
#include "rgb_lcd.h"
#include "arduino_secrets.h"
// WIFI
const char* ssid = SECRET_SSID;
const char* pass = SECRET_PASS;
int keyIndex = 0;                 // your network key index number (needed only for WEP)
int status = WL_IDLE_STATUS;
WiFiServer server(80);
void printWifiStatus();

// Buzzer
const int buzzerPin = 5;
void buzOff();

// LCD screen
rgb_lcd lcd;
int colorR = 0;
int colorG = 0;
int colorB = 0;
char *text;
void displayAlarm(int type, String where, String what);

void setup() {
  //Initialize serial and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  // check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    // don't continue
    while (true);
  }

  String fv = WiFi.firmwareVersion();
  if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
    Serial.println("Please upgrade the firmware");
  }

  // attempt to connect to WiFi network:
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);

    // wait 10 seconds for connection:
    delay(10000);
  }
  server.begin();
  // you're connected now, so print out the status:
  printWifiStatus();
    // Initialize the LCD
  lcd.begin(16, 2);
  lcd.setRGB(colorR, colorG, colorB);
}


void loop() {
  // listen for incoming clients
  WiFiClient client = server.available();
  if (client) {
    Serial.println("new client");
    // an HTTP request ends with a blank line
    boolean currentLineIsBlank = true;
    String currentLine = "";
    int type;
    String where = "";
    String what = "";
    boolean endsWith = false;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();  
        if (c != '\r') {  
          Serial.write(c);
          currentLine += c;
        } else {  
          if (currentLine.startsWith("GET /?type=")) {
            type = 22;
            while (currentLine[type] != '%' && type < currentLine.length())
            {
              where += currentLine[type];
              type += 1;
            }
            type += 12;
            while (currentLine[type] != '%' && type < currentLine.length())
            {
              what += currentLine[type];
              type += 1;
            }
            type = currentLine[11] - '0';
          }
          currentLine = "";
        }
        
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the HTTP request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {
          // send a standard HTTP response header
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");  // the connection will be closed after completion of the response
          client.println("Refresh: 5");  // refresh the page automatically every 5 sec
          client.println();

          //function action here!
          client.print("[{\"type\" : ");
          client.print(type); 
          client.print(",");
          client.print("\"where\" : ");
          client.print(where);
          client.print(",");
          client.print("\"what\" : ");
          client.print(what);
          client.print("}]");
          //buzOff();
          displayAlarm(type, where, what);
          break;
        }
        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
        } else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    // give the web browser time to receive the data
    delay(1);

    // close the connection:
    client.stop();
    Serial.println("client disconnected");
  }
}


void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}

void buzOff(){
  digitalWrite(buzzerPin, HIGH);
  delay(500);
  digitalWrite(buzzerPin, LOW);

}

// assumning that we can read a int and a string 
void displayAlarm(int type, String where, String what){
  if (type == 2) {
    // Very Hot - Bright Red
    colorR = 255;
    colorG = 0;
    colorB = 0;
    text = "Danger";
  } 
  if (type == 1) {
    // Mild - Yellow
    colorR = 255;
    colorG = 255;
    colorB = 0;
    text = "Warning";
  }
  if (type == 0) {
    // Very Cold - Dark Blue
    colorR = 0;
    colorG = 0;
    colorB = 139;
    text = "Cold";
  }
  lcd.setRGB(colorR, colorG, colorB);
  lcd.clear();  // Clear the LCD screen
  lcd.setCursor(0, 0);  // Set cursor to the first row, first column
  lcd.print(text);
  lcd.print(": ");
  lcd.print(what);
  lcd.setCursor(0, 1);  // Set cursor to the second row, first column
  lcd.print(where);
  delay(10000);
  lcd.clear();  // Clear the LCD screen
}