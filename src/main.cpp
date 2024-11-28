#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include "LittleFS.h"
#include <ESPAsyncWebServer.h>
#include <AsyncWebSocket.h>

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

String ssid;
String pass;
String ip;
String gateway;

//File paths to save data provided from initial wifiManager page
const char* ssidPath = "/ssid.txt";
const char* passPath = "/pass.txt";
const char* ipPath = "/ip.txt";
const char* gatewayPath = "/gateway.txt";

IPAddress localIP; //Hardcoded in HTML
IPAddress localGateway; //Hardcoded in HTML
IPAddress subnet(255, 255, 0, 0);

//Webpace controls param names
const char* PARAM_INPUT_1 = "ssid";
const char* PARAM_INPUT_2 = "pass";
const char* PARAM_INPUT_3 = "ip";
const char* PARAM_INPUT_4 = "gateway";

//GPIOs
const int column_pair_1_1_pin_plus = 16;
const int column_pair_1_2_pin_minus = 17;
const int column_pair_1_3_pin_plus = 18;
const int column_pair_1_4_pin_minus = 19;

const int column_pair_2_1_pin_plus = 32;
const int column_pair_2_2_pin_minus = 33;
const int column_pair_2_3_pin_plus = 26;
const int column_pair_2_4_pin_minus = 27;

//Timer variables
unsigned long previousMillis = 0;
const long interval = 10000;  //Interval to wait for establishing Wi-Fi connection

String activeColumnPairState;

//Initialize WiFi
bool initWiFi() {
  if (ssid == "" || ip == "") {
    Serial.println("Undefined SSID or IP address.");
    return false;
  }

  WiFi.mode(WIFI_STA);
  localIP.fromString(ip.c_str());
  localGateway.fromString(gateway.c_str());

  if (!WiFi.config(localIP, localGateway, subnet)){
    Serial.println("STA Failed to configure");
    return false;
  }
  WiFi.begin(ssid.c_str(), pass.c_str());
  Serial.println("Connecting to WiFi...");

  unsigned long currentMillis = millis();
  previousMillis = currentMillis;

  while (WiFi.status() != WL_CONNECTED) {
    currentMillis = millis();
    delay(100);
    if (currentMillis - previousMillis >= interval) {
      Serial.println("Failed to connect.");
      return false;
    }
  }
  Serial.println(WiFi.localIP());
  return true;
}

//Initialize LittleFS
void initLittleFS() {
  if (!LittleFS.begin(true)) {
    Serial.println("An error has occurred while mounting LittleFS");
  }
  Serial.println("LittleFS mounted successfully");
}

//Read File from LittleFS
String readFile(fs::FS &fs, const char * path){
  Serial.printf("Reading file: %s\r\n", path);

  File file = fs.open(path); //TODO: Try-catch
  if (!file || file.isDirectory()) {
    Serial.println("- failed to open file for reading");
    return String();
  }
  
  String fileContent;
  while (file.available()) {
    fileContent = file.readStringUntil('\n');
    break;     
  }
  return fileContent;
}

// Write file to LittleFS
void writeFile(fs::FS &fs, const char * path, const char * message){
  Serial.printf("Writing file: %s\r\n", path);

  File file = fs.open(path, FILE_WRITE);
  if (!file) {
    Serial.println("- failed to open file for writing");
    return;
  }
  if (file.print(message)) {
    Serial.println("- file written");
  } else {
    Serial.println("- write failed");
  }
}

String processor(const String& var) {
  if (var == "STATE") {
    if (digitalRead(column_pair_1_1_pin_plus) && digitalRead(column_pair_1_3_pin_plus) &&
        digitalRead(column_pair_2_1_pin_plus) && digitalRead(column_pair_2_3_pin_plus)) {
          activeColumnPairState = "Żadna";
    } else if (digitalRead(column_pair_1_1_pin_plus) && digitalRead(column_pair_1_3_pin_plus)) {
      activeColumnPairState = "Para druga";
    } else {
      activeColumnPairState = "Para pierwsza";
    }
    return activeColumnPairState;
  } 
  
  return String();
}

  void setPins(String pair) {
    //pair = first, second, off
    digitalWrite(column_pair_1_1_pin_plus, LOW);
    digitalWrite(column_pair_1_2_pin_minus, LOW);
    digitalWrite(column_pair_1_3_pin_plus, LOW);
    digitalWrite(column_pair_1_4_pin_minus, LOW);
    
    digitalWrite(column_pair_2_1_pin_plus, LOW);
    digitalWrite(column_pair_2_2_pin_minus, LOW);
    digitalWrite(column_pair_2_3_pin_plus, LOW);
    digitalWrite(column_pair_2_4_pin_minus, LOW);

    if (pair == "first") {
      digitalWrite(column_pair_1_1_pin_plus, HIGH);
      digitalWrite(column_pair_1_2_pin_minus, HIGH);
      digitalWrite(column_pair_1_3_pin_plus, HIGH);
      digitalWrite(column_pair_1_4_pin_minus, HIGH);
      Serial.println(pair);
    } else if (pair == "second") {
      digitalWrite(column_pair_2_1_pin_plus, HIGH);
      digitalWrite(column_pair_2_2_pin_minus, HIGH);
      digitalWrite(column_pair_2_3_pin_plus, HIGH);
      digitalWrite(column_pair_2_4_pin_minus, HIGH);
      Serial.println(pair);
    } else if (pair == "off") {
      Serial.println("Turning relays off...");
    } else {
      Serial.println("Unknown parameter provided for setPins. Doing nothing...");
    }
  }

String generateDropdownHTML(std::vector<String> dropdownOptions) {
    String dropdownHTML;

    for (const String& option : dropdownOptions) {
        dropdownHTML += "<option value='" + option + "'>" + option + "</option>";
    }

    return dropdownHTML;
}

std::vector<String> scanVisibleNetworks() {
  std::vector<String> ssidDropdownOptions;
  
  int n = WiFi.scanNetworks();
  Serial.println("Scan done!");

  if (n == 0) {
    Serial.println("No networks found.");
  } else {
    Serial.println();
    Serial.print(n);
    Serial.println(" networks found");
    for (int i = 0; i < n; ++i) {
      // Print SSID and RSSI for each network found
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(WiFi.SSID(i));
      Serial.print(" (");
      Serial.print(WiFi.RSSI(i));
      Serial.print(")");
      Serial.println((WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? " " : "*");
      delay(10);

      ssidDropdownOptions.push_back(WiFi.SSID(i));
    }
  }
  return ssidDropdownOptions;
}

String generateSsidDropdownJsonArray() {
  std::vector<String> scannedNetworks = scanVisibleNetworks();
    
    String jsonArray = "[";
    for (size_t i = 0; i < scannedNetworks.size(); ++i) {
        jsonArray += "\"" + scannedNetworks[i] + "\"";
        if (i < scannedNetworks.size() - 1) jsonArray += ",";
    }
    jsonArray += "]";
    Serial.println("Generated JSON: " + jsonArray);
    return jsonArray;

}

void setup() {
  Serial.begin(115200);
  Serial.println("Booting...");

  initLittleFS();
  
  pinMode(column_pair_1_1_pin_plus, OUTPUT);
  pinMode(column_pair_1_2_pin_minus, OUTPUT);
  pinMode(column_pair_1_3_pin_plus, OUTPUT);
  pinMode(column_pair_1_4_pin_minus, OUTPUT);

  pinMode(column_pair_2_1_pin_plus, OUTPUT);
  pinMode(column_pair_2_2_pin_minus, OUTPUT);
  pinMode(column_pair_2_3_pin_plus, OUTPUT);
  pinMode(column_pair_2_4_pin_minus, OUTPUT);

  //Relays are controlled by HIGH state, hence to turn them off we need to set the state to LOW
  digitalWrite(column_pair_1_1_pin_plus, LOW);
  digitalWrite(column_pair_1_2_pin_minus, LOW);
  digitalWrite(column_pair_1_3_pin_plus, LOW);
  digitalWrite(column_pair_1_4_pin_minus, LOW);

  digitalWrite(column_pair_2_1_pin_plus, LOW);
  digitalWrite(column_pair_2_2_pin_minus, LOW);
  digitalWrite(column_pair_2_3_pin_plus, LOW);
  digitalWrite(column_pair_2_4_pin_minus, LOW);

  ssid = readFile(LittleFS, ssidPath);
  pass = readFile(LittleFS, passPath);
  ip = readFile(LittleFS, ipPath);
  gateway = readFile (LittleFS, gatewayPath);
  Serial.println(ssid);
  Serial.println(pass);
  Serial.println(ip);
  Serial.println(gateway);

  if (initWiFi()) {
    //Route for root / web page
    Serial.println("initwifi true");
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
      Serial.println("wystawiam index");
      request->send(LittleFS, "/index.html", "text/html", false, processor);
      Serial.println("index wystawiony");
    });
    server.serveStatic("/", LittleFS, "/");
    
    server.on("/firstpair", HTTP_GET, [](AsyncWebServerRequest *request) {
      setPins("first");
      request->send(LittleFS, "/index.html", "text/html", false, processor);
    });

    server.on("/secondpair", HTTP_GET, [](AsyncWebServerRequest *request) {
      setPins("second");
      request->send(LittleFS, "/index.html", "text/html", false, processor);
    });

    server.on("/alloff", HTTP_GET, [](AsyncWebServerRequest *request) {
      setPins("off");
      request->send(LittleFS, "/index.html", "text/html", false, processor);
    });
  } else {
    //Connect to Wi-Fi network with SSID and password
    Serial.println("Setting AP (Access Point)");
    
    //NULL sets an open Access Point
    WiFi.softAP("R4_ESP32_AccessPoint", NULL);

    IPAddress IP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(IP); 

    server.on("/getOptions", HTTP_GET, [](AsyncWebServerRequest *request) {
      Serial.println("Inside getOptions");
      String ssidDropdownJsonArray = generateSsidDropdownJsonArray();
      request->send(200, "application/json", ssidDropdownJsonArray);
  });

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
      request->send(LittleFS, "/wifimanager.html", "text/html", false, processor);
    });
    server.serveStatic("/", LittleFS, "/");
  
    server.on("/", HTTP_POST, [](AsyncWebServerRequest *request) {
      int params = request -> params();
      for (int i = 0; i < params; i++) {
        const AsyncWebParameter* p = request -> getParam(i);
        if(p -> isPost()){
          // HTTP POST ssid value
          if (p -> name() == PARAM_INPUT_1) {
            ssid = p -> value().c_str();
            Serial.print("SSID set to: ");
            Serial.println(ssid);
            // Write file to save value
            writeFile(LittleFS, ssidPath, ssid.c_str());
          }
          // HTTP POST pass value
          if (p -> name() == PARAM_INPUT_2) {
            pass = p -> value().c_str();
            Serial.print("Password set to: ");
            Serial.println(pass);
            // Write file to save value
            writeFile(LittleFS, passPath, pass.c_str());
          }
          // HTTP POST ip value
          if (p -> name() == PARAM_INPUT_3) {
            ip = p -> value().c_str();
            Serial.print("IP Address set to: ");
            Serial.println(ip);
            // Write file to save value
            writeFile(LittleFS, ipPath, ip.c_str());
          }
          // HTTP POST gateway value
          if (p -> name() == PARAM_INPUT_4) {
            gateway = p -> value().c_str();
            Serial.print("Gateway set to: ");
            Serial.println(gateway);
            // Write file to save value
            writeFile(LittleFS, gatewayPath, gateway.c_str());
          }
          //Serial.printf("POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
        }
      }

      request->send(200, "text/plain", "Done. ESP will restart, connect to your router and go to IP address: " + ip);
      delay(3000);
      ESP.restart();
    });
  }
  server.begin();
}

void loop() {}