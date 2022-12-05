
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>

/****************** DEFINES ***********/
// GPIO configuration
#define GPIO0_BUTTON  0
#define GPIO1_TX      1
#define GPIO2_LED     2
#define GPIO3_LED     3

#define LED_ON  1
#define LED_OFF 0

#define DEBUG

/**************** STRUCTS ****************/
typedef enum
{
  E_WIFI_STATION_SCAN,
  E_WIFI_STATION_CONNECTED
}EGeneralState;

/************* GLOBAL VARIABLES ************/
static  EGeneralState       gEState               = E_WIFI_STATION_SCAN;

static  char                gWifiStationSSID[32]  = "ArthurWifi";
static  char                gWifiStationPWD[64]   = "tuturwifi";

WiFiClient client;
HTTPClient http;
const char* serverName = "http://192.168.137.153:8080/answer/1";
int httpResponseCode = 0;
bool BigButtonHasBeenPressed = false;

/********************** ISR ****************/
// Catch button ISR
void ICACHE_RAM_ATTR IsrBigButtonPushed() {
  BigButtonHasBeenPressed = true;
}

/********************** APP ***************/
void setup() {
  // Setup GPIOS
  pinMode(GPIO3_LED, OUTPUT);
  pinMode(GPIO0_BUTTON, INPUT_PULLUP);
  pinMode(GPIO2_LED, OUTPUT);

  // Start debug serial
  //Serial.begin(115200);
  //Serial.println();
  //Serial.println("*******START OF PROGRAM*****");
}

void loop() {

  switch (gEState)
  {
    case E_WIFI_STATION_SCAN:
      //Serial.println("New sate : E_WIFI_STATION_SCAN");
      // Try to connect to wifi station
      WiFi.mode(WIFI_STA);
      WiFi.begin(gWifiStationSSID, gWifiStationPWD);

      //Serial.print("scan start : ssid=");
      //Serial.print(gWifiStationSSID);
      //Serial.print(", pwd=");
      //Serial.println(gWifiStationPWD);

      // Wait for connection
      while ((WiFi.status() != WL_CONNECTED))
      {
        digitalWrite(GPIO2_LED, LED_OFF);
        delay(500);
        digitalWrite(GPIO2_LED, LED_ON);
        delay(500);
        //Serial.print(".");
      }

      if (WiFi.status() == WL_CONNECTED)
      {
        WiFi.setAutoReconnect(true);
        //Serial.println();
        //Serial.print("Connected, IP address: ");
        //Serial.println(WiFi.localIP());
        //Serial.print("RSSI : ");
        //Serial.println(WiFi.RSSI());
        gEState = E_WIFI_STATION_CONNECTED;
      }
      else
      {
        //Serial.println();
        //Serial.println("Cannot connect to WIFI");
      }
      break;

    case E_WIFI_STATION_CONNECTED:
      // Attach ISR to button
      attachInterrupt(GPIO0_BUTTON, IsrBigButtonPushed, FALLING);
      digitalWrite(GPIO2_LED, LED_ON);
      //Serial.println("New sate : E_WIFI_STATION_CONNECTED");
      while(1)
      {
        if (BigButtonHasBeenPressed == true)
        {
          BigButtonHasBeenPressed = false;
          // Your Domain name with URL path or IP address with path
          http.begin(client, serverName);

          // Send HTTP POST request
          httpResponseCode = http.POST("");

          if (httpResponseCode == 200)
          {
            String payload = http.getString();
            //Serial.print("response from serv : ");
            //Serial.println(payload);
            if (strstr(payload.c_str(), "true") != NULL)
            {
              //Serial.print("J'ai la main !!!!");
              digitalWrite(GPIO3_LED, LED_ON);
              delay(5000);
              digitalWrite(GPIO3_LED, LED_OFF);
            }
            else
            {
              //Serial.print("Trop tard :(((");
            }
          }
        }
        delay(20);
      }
      detachInterrupt(GPIO0_BUTTON);
      break;

    default:
      //Serial.println("New sate : UNKNOWN");
      gEState = E_WIFI_STATION_SCAN;
      break;
  }
}
