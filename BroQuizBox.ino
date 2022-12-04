
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>

/****************** DEFINES ***********/
// Access point configuration
#define AP_SSID       "test_esp"
#define AP_PSK        "test"

#define WIFI_CONNECT_MAX_ATTEMPT 15

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
  E_WIFI_AP,
  E_WIFI_STATION_CONNECTED
}EGeneralState;

/************* GLOBAL VARIABLES ************/
static ESP8266WebServer gServer(80);
static EGeneralState  gEState = E_WIFI_STATION_SCAN;

/********************** ISR ****************/
// Catch button ISR
void ICACHE_RAM_ATTR IsrBigButtonPushed() {
  digitalWrite(GPIO3_LED, LED_ON);
}

/********************** APP ***************/
void handleRoot() {
  gServer.send(200, "text/html", "<h1>You are connected</h1>");
}

void setup() {
  // Setup GPIOS
  pinMode(GPIO3_LED, OUTPUT);
  pinMode(GPIO0_BUTTON, INPUT_PULLUP);
  pinMode(GPIO2_LED, OUTPUT);

  // Start debug serial
  //Serial.begin(115200);
  Serial.println();
  Serial.println("*******START OF PROGRAM*****");
  Serial.print("max wifi attempt : ");
  Serial.println(WIFI_CONNECT_MAX_ATTEMPT);

}

void loop() {
  char WifiStationSSID[32] = "ArthurWifi";
  char WifiStationPWD[64] = "tuturwifi";
  uint8_t WifiConnectAttempt = 0U;


  switch (gEState)
  {
    case E_WIFI_STATION_SCAN:
      Serial.println("New sate : E_WIFI_STATION_SCAN");
      WifiConnectAttempt = 0U;
      // Try to connect to wifi station
      WiFi.mode(WIFI_STA);
      WiFi.begin(WifiStationSSID, WifiStationPWD);

      Serial.print("scan start : ssid=");
      Serial.print(WifiStationSSID);
      Serial.print(", pwd=");
      Serial.println(WifiStationPWD);

      // Wait for connection
      while ((WiFi.status() != WL_CONNECTED) && (WifiConnectAttempt < WIFI_CONNECT_MAX_ATTEMPT))
      {
        WifiConnectAttempt++;
        digitalWrite(GPIO2_LED, LED_ON);
        delay(500);
        digitalWrite(GPIO2_LED, LED_OFF);
        delay(500);
        Serial.print(".");
      }

      if (WiFi.status() == WL_CONNECTED)
      {
        WiFi.setAutoReconnect(true);
        Serial.println();
        Serial.print("Connected, IP address: ");
        Serial.println(WiFi.localIP());
        Serial.print("RSSI : ");
        Serial.println(WiFi.RSSI());
        gEState = E_WIFI_STATION_CONNECTED;
      }
      else
      {
        Serial.println();
        Serial.println("Cannot connect to WIFI");
        //gEState = E_WIFI_AP;
      }
      break;

    case E_WIFI_AP:
      Serial.println("New sate : E_WIFI_AP");
      // Start AP wifi
      WiFi.softAP(AP_SSID, AP_PSK);

      gServer.on("/", handleRoot);
      break;

    case E_WIFI_STATION_CONNECTED:
      // Attach ISR to button
      attachInterrupt(GPIO0_BUTTON, IsrBigButtonPushed, FALLING);
      digitalWrite(GPIO2_LED, LED_ON);
      Serial.println("New sate : E_WIFI_STATION_CONNECTED");
      while(1)
      {
        delay(5000);
      }

      detachInterrupt(GPIO0_BUTTON);
      break;

    default:
      Serial.println("New sate : UNKNOWN");
      gEState = E_WIFI_STATION_SCAN;
      break;
  }
  gServer.handleClient();
  // digitalWrite(LED_BUILTIN, LED_OFF);
  // delay(1000);
  // digitalWrite(LED_BUILTIN, LED_ON);
  // delay(2000);
}
