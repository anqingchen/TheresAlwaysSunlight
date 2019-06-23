#include <ESP8266WiFi.h>        // Include the Wi-Fi library
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <ArduinoJson.h>

#define day D2
#define natural D5
#define warm D6

int dayPwm = 0;
int naturalPwm = 0;
int warmPwm = 0;
bool sunSim = true;

struct temperature {
  int dayBrightness;
  int naturalBrightness;
  int warmBrightness;
};

const temperature colors[] = {
  {0, 0, 0},
  {0, 0, 6},
  {0, 0, 12},
  {0, 0, 18},
  {0, 0, 24},
  {0, 0, 30},
  {0, 0, 36},
  {0, 0, 42},
  {0, 0, 48},
  {0, 0, 54},
  {0, 0, 60},
  {0, 0, 66},
  {0, 0, 72},
  {0, 0, 78},
  {0, 0, 84},
  {0, 0, 90},
  {0, 0, 96},
  {0, 6, 90},
  {0, 12, 84},
  {0, 18, 78},
  {0, 24, 72},
  {0, 30, 66},
  {0, 36, 60},
  {0, 42, 54},
  {0, 48, 48},
  {0, 54, 42},
  {0, 60, 36},
  {0, 66, 30},
  {10, 72, 24},
  {10, 78, 18},
  {10, 84, 12},
  {10, 90, 6},
  {10, 96, 0},
  {10, 90, 0},
  {20, 84, 0},
  {30, 78, 0},
  {40, 72, 0},
  {50, 66, 0},
  {60, 60, 0},
  {70, 54, 0},
  {80, 48, 0},
  {90, 42, 0},
  {100, 36, 0},
  {100, 30, 0},
  {100, 24, 0},
  {100, 18, 0},
  {100, 12, 0},
  {100, 12, 0},
  {100, 12, 0},
  {100, 12, 0},
  {100, 12, 0},
  {100, 12, 0},
  {90, 18, 0},
  {80, 24, 0},
  {70, 30, 0},
  {60, 36, 0},
  {50, 42, 0},
  {40, 48, 0},
  {30, 54, 0},
  {20, 60, 0},
  {10, 66, 0},
  {10, 72, 0},
  {10, 78, 0},
  {10, 84, 0},
  {10, 90, 0},
  {10, 96, 0},
  {10, 96, 0},
  {0, 90, 6},
  {0, 84, 12},
  {0, 78, 18},
  {0, 72, 24},
  {0, 66, 30},
  {0, 60, 36},
  {0, 54, 42},
  {0, 48, 48},
  {0, 42, 54},
  {0, 36, 60},
  {0, 30, 66},
  {0, 24, 72},
  {0, 18, 78},
  {0, 12, 84},
  {0, 6, 90},
  {0, 0, 96},
  {0, 0, 96},
  {0, 0, 96},
  {0, 0, 96},
  {0, 0, 90},
  {0, 0, 84},
  {0, 0, 78},
  {0, 0, 72},
  {0, 0, 66},
  {0, 0, 60},
  {0, 0, 54},
  {0, 0, 48},
  {0, 0, 42},
  {0, 0, 36},
  {0, 0, 30},
  {0, 0, 24},
  {0, 0, 18},
  {0, 0, 12},
  {0, 0, 6}
};

const char* ssid     = "ATTCKKauI2";        // The SSID (name) of the Wi-Fi network you want to connect to
const char* password = "j6uimtp+iypv";     // The password of the Wi-Fi network
const char* host = "api.openweathermap.org";
const char* url = "/data/2.5/weather";

String httpHeader;

char* zipcode = "77059";
const char* appid = "e9e7fad46ec26e3dfec25369a336c721";

String param = String(url) + "?zip=" + zipcode + ",us&appid=" + appid;

double sunrise, sunset;

WiFiUDP ntpUDP;
WiFiServer server(80);

NTPClient timeClient(ntpUDP);
os_timer_t myTimer;
bool tickOccured;


void setup() {
// Set pins  
  pinMode(day, OUTPUT);
  pinMode(natural, OUTPUT);
  pinMode(warm, OUTPUT);
  
// Start the Serial communication to send messages to the computer  
  Serial.begin(115200);         
  delay(10);
  Serial.println('\n');
  
  WiFi.mode(WIFI_AP);
  WiFi.softAP("Sunlight Simulator", "thereisnospoon", 1, false);

  wifiConnect();

  Serial.println('\n');
  Serial.println("Connection established!");  
  Serial.print("IP address:\t");
  Serial.println(WiFi.localIP());         // Send the IP address of the ESP8266 to the computer
  server.begin();

// Setup Time NTP
  timeClient.begin();

// Initialize Timer
  tickOccured = false;
  timer_init();
}

void loop() {
// WiFi Server Setup
  WiFiClient client = server.available();
  if (client) {                             // If a new client connects,
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        httpHeader += c;
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
                      
            // turns the GPIOs on and off
            if (httpHeader.indexOf("GET /day/") >= 0) {
              String temp = httpHeader.substring(httpHeader.indexOf("/day/") + 5, httpHeader.indexOf("/day/") + 8);
              dayPwm = temp.toInt();
              analogWrite(day, dayPwm);
            } else if (httpHeader.indexOf("GET /natural/") >= 0) {
              String temp = httpHeader.substring(httpHeader.indexOf("/natural/") + 9, httpHeader.indexOf("/natural/") + 12);
              naturalPwm = temp.toInt();
              analogWrite(natural, naturalPwm);
            } else if (httpHeader.indexOf("GET /warm/") >= 0) {
              String temp = httpHeader.substring(httpHeader.indexOf("/warm/") + 6, httpHeader.indexOf("/warm/") + 9);
              warmPwm = temp.toInt();
              analogWrite(warm, warmPwm);
            } else if (httpHeader.indexOf("GET /sunsim/") >= 0) {
              String temp = httpHeader.substring(httpHeader.indexOf("/sunsim/") + 8, httpHeader.indexOf("/warm/") + 9);
              if(temp.equals("1")) {
                sunSim = true;
              } else {
                sunSim = false;
              }
            }
            // Web Page Heading
            client.println("<body><h1>ESP8266 Web Server</h1>"); 
            // The HTTP response ends with another blank line
            client.println();
            // Break out of the while loop
            break;       
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    httpHeader = "";
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
  
// Mainprogram ISR
  if(tickOccured == true) {
    Serial.println("Tick Occured");
    Serial.println("==========================================");
    if(sunSim == true) {
      updateSun();
      int temp = findPercentage();
      Serial.print("Sun Progress at: "); Serial.print(temp); Serial.println(" percent");
      dayPwm = pwmConvert(colors[temp].dayBrightness);
      naturalPwm = pwmConvert(colors[temp].naturalBrightness);
      warmPwm = pwmConvert(colors[temp].warmBrightness);
      analogWrite(day, dayPwm);
      analogWrite(natural, naturalPwm);
      analogWrite(warm, warmPwm);
    } else {
      Serial.println("Sun Simulation is OFF");
    }
    tickOccured = false;  
    Serial.print("Current DayLED Brightness: "); Serial.println(dayPwm);
    Serial.print("Current NaturalLED Brightness: "); Serial.println(naturalPwm);
    Serial.print("Current WarmLED Brightness: "); Serial.println(warmPwm);
    Serial.println(" ");
  }
}

int findPercentage() {
  int dayLength, timePassed, percentage;
  timeClient.update();
  Serial.print("Current UTC Time: ");
  Serial.println(timeClient.getEpochTime());
  dayLength = sunset - sunrise;
  timePassed = timeClient.getEpochTime() - sunrise;
  percentage = (timePassed * 100) / (dayLength); 
  if (percentage > 100 || percentage < 0) {
    percentage = 0; 
  }
  return percentage;
}

void updateSun() {
  WiFiClient client;
  Serial.print("Connecting to: ");
  Serial.println(host);
  if(client.connect(host, 80)){
    client.print(String("GET ") + param + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "User-Agent: ESP8266\r\n" +
               "Connection: close\r\n\r\n");
  } else {
    Serial.println("Unable to connect to host!");
    return;
  }
  String line = "";
  line = client.readStringUntil('\n');
  Serial.println(line);
  if(client.find("\r\n\r\n")) {
    DynamicJsonDocument doc(4096);
    DeserializationError error = deserializeJson(doc, client);
    if(error) {
      Serial.println(error.c_str());
      return;
    }
    const char* city = doc["name"];
    Serial.print("City: "); Serial.println(city); 
    sunrise = doc["sys"]["sunrise"];
    sunset = doc["sys"]["sunset"];
  } 
}

void timerCallback(void *pArg) {
  tickOccured = true;
}

void timer_init() {
  os_timer_setfn(&myTimer, timerCallback, NULL);
  os_timer_arm(&myTimer, 60000, true);
}

int pwmConvert(int input) {
  return map(input, 0, 100, 0, 255);
}

void wifiConnect() {
  // Connect to WiFi Network
  WiFi.begin(ssid, password);             // Connect to the network
  Serial.print("Connecting to ");
  Serial.print(ssid); 
  Serial.println(" ...");

  int i = 0;
  while (WiFi.status() != WL_CONNECTED) { // Wait for the Wi-Fi to connect
    delay(1000);
    Serial.print(++i); 
    Serial.print(' ');
//    if(i >= 20) {
//      WiFi.beginSmartConfig();
//      while(1) {
//        delay(1000);
//        if(WiFi.smartConfigDone()) {
//          Serial.println("SmartConfig Success");
//          break;
//        }
//      }
//    }
  }
}
