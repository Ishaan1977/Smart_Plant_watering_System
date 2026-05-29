

#include <WiFi.h>
#include <WebServer.h>
#include <DHT.h>
#include <U8g2lib.h>
#include <Wire.h>

// --- WiFi Credentials ---
const char* ssid = "Ishu bhagat ";
const char* pass = "is12ha34an";

// --- Physical Pin Allocation Matrix ---
#define DHTPIN 23
#define DHTTYPE DHT11
#define SOIL_PIN 34
#define TRIG_PIN 5
#define ECHO_PIN 18
#define PUMP_PIN 2       
#define BUZZER_PIN 19
#define IR_PIN 25
#define TOUCH_PIN 26

// --- Core State Machine Definitions ---
enum SystemState { DATA_MODE, EYE_MODE };
SystemState currentState = DATA_MODE;

unsigned long lastStateChange = 0;
unsigned long lastSensorRead = 0;
const long sensorInterval = 2000; 

// --- Volatile Global Sensor Memory Registers ---
float humidity = 0.0;
float temperature = 0.0;
int soilMoisture = 0;
float waterDistance = 0.0;
int tankLevelPercent = 0;

// --- Object Memory Allocation Instantiations ---
DHT dht(DHTPIN, DHTTYPE);
WebServer server(80); 

// U8g2 full frame buffer engine for 1.34-inch SSD1306 using hardware I2C
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE, 22, 21);

// --- HTML Dashboard Generation Function ---
void handleRoot() {
  String currentPumpStatus = (digitalRead(PUMP_PIN) == LOW) ? "ACTIVE (ON)" : "INACTIVE (OFF)";
  String currentTankStatus = (tankLevelPercent < 15) ? "LOW! Please Refill" : "OK";

  // Modern HTML/CSS string with 3-second auto-refresh
  String html = "<!DOCTYPE html><html><head>";
  html += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">";
  html += "<meta http-refresh content=\"3\">"; // Auto-refresh page every 3 seconds
  html += "<title>SmartMoneyPlant Dashboard</title>";
  html += "<style>";
  html += "body { font-family: 'Segoe UI', Arial, sans-serif; background: #f4f7f6; color: #333; text-align: center; margin: 0; padding: 20px; }";
  html += ".container { max-width: 500px; background: white; margin: 0 auto; padding: 20px; border-radius: 15px; box-shadow: 0 4px 15px rgba(0,0,0,0.1); }";
  html += "h1 { color: #2e7d32; margin-bottom: 5px; }";
  html += ".subtitle { color: #666; font-size: 14px; margin-bottom: 25px; }";
  html += ".card { background: #f9f9f9; padding: 15px; margin: 10px 0; border-radius: 8px; border-left: 5px solid #2e7d32; text-align: left; display: flex; justify-content: space-between; align-items: center; }";
  html += ".card.alert { border-left-color: #d32f2f; background: #ffebee; }";
  html += ".label { font-weight: bold; color: #555; }";
  html += ".value { font-size: 18px; font-weight: bold; color: #111; }";
  html += "</style>";
  html += "<script>setInterval(function(){ location.reload(); }, 3000);</script>"; // JavaScript fallback for auto-refresh
  html += "</head><body>";
  html += "<div class=\"container\">";
  html += "<h1>Money Plant Eco-System</h1>";
  html += "<div class=\"subtitle\">Live Remote Telemetry Portal</div>";
  
  html += "<div class=\"card\"><span class=\"label\">Ambient Temp:</span><span class=\"value\">" + String(temperature, 1) + " &deg;C</span></div>";
  html += "<div class=\"card\"><span class=\"label\">Air Humidity:</span><span class=\"value\">" + String(humidity, 1) + " %</span></div>";
  html += "<div class=\"card\"><span class=\"label\">Soil Permittivity (ADC):</span><span class=\"value\">" + String(soilMoisture) + "</span></div>";
  
  if (tankLevelPercent < 15) {
    html += "<div class=\"card alert\"><span class=\"label\">Tank Level:</span><span class=\"value\">" + String(tankLevelPercent) + " % (" + currentTankStatus + ")</span></div>";
  } else {
    html += "<div class=\"card\"><span class=\"label\">Tank Level:</span><span class=\"value\">" + String(tankLevelPercent) + " %</span></div>";
  }
  
  html += "<div class=\"card\"><span class=\"label\">Water Pump State:</span><span class=\"value\">" + currentPumpStatus + "</span></div>";
  
  html += "</div></body></html>";
  
  server.send(200, "text/html", html);
}

void setup() {
  // Initialize Serial Telemetry Debug Bus
  Serial.begin(115200);
  Serial.println("Initializing SmartPlant System Subroutines...");

  // Register GPIO Input/Output Constraints
  pinMode(PUMP_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(IR_PIN, INPUT);
  pinMode(TOUCH_PIN, INPUT);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  digitalWrite(PUMP_PIN, HIGH); 
  digitalWrite(BUZZER_PIN, LOW);

  // Wake Up Display and DHT
  dht.begin();
  u8g2.begin();
  u8g2.clearDisplay();
  
  // --- Establish Local Wi-Fi Connection ---
  Serial.print("Connecting to Hotspot: ");
  Serial.println(ssid);
  WiFi.begin(ssid, pass);
  
  u8g2.setFont(u8g2_font_6x10_tf);
  u8g2.drawStr(0, 20, "Connecting WiFi...");
  u8g2.sendBuffer();

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("\nWiFi Connected successfully!");
  Serial.print("Local IP Address: ");
  Serial.println(WiFi.localIP()); 

  // Display IP Address temporarily on startup
  u8g2.clearBuffer();
  u8g2.drawStr(0, 15, "WiFi Connected!");
  u8g2.drawStr(0, 35, "IP Address:");
  u8g2.setCursor(0, 55);
  u8g2.print(WiFi.localIP());
  u8g2.sendBuffer();
  delay(4000); 
  // Define Server Route Handling
  server.on("/", handleRoot);
  server.begin();
  Serial.println("HTTP Local Web Server Started.");
}

void loop() {
 
  server.handleClient();
  
  unsigned long currentMillis = millis();

  // --- Subsystem 1: Proximity and Context Parser ---
  if (digitalRead(IR_PIN) == HIGH) { 
    if (currentState != EYE_MODE) {
      currentState = EYE_MODE;
      u8g2.clearDisplay();
    }
    lastStateChange = currentMillis; 
  } else if (currentMillis - lastStateChange > 7000) { 
    // If presence timeout exceeds 7 seconds, return to standard diagnostic logging
    if (currentState != DATA_MODE) {
      currentState = DATA_MODE;
      u8g2.clearDisplay();
    }
  }

  // --- Subsystem 2: Task Execution Router ---
  if (currentState == EYE_MODE) {
    runAnimatedEyes();
  } else {
    if (currentMillis - lastSensorRead >= sensorInterval) {
      readSensors();
      executeAutomationLogic();
      updateDataDashboard();
      lastSensorRead = currentMillis;
    }
  }
}

// --- Sensor Processing Routines ---
void readSensors() {
  humidity = dht.readHumidity();
  temperature = dht.readTemperature();
  soilMoisture = analogRead(SOIL_PIN); 

  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  
  long duration = pulseIn(ECHO_PIN, HIGH, 30000); 
  waterDistance = (duration * 0.0343) / 2;

  // Structural Tank Level Parsing
  if (waterDistance <= 1 || waterDistance > 5) {
    waterDistance = 5; 
  }
  tankLevelPercent = map(waterDistance, 5, 1, 0, 100);
  tankLevelPercent = constrain(tankLevelPercent, 0, 100);
  
  Serial.printf("LOG: T=%.1fC, H=%.1f%%, Moisture=%d, Tank%%=%d%%\n", 
                temperature, humidity, soilMoisture, tankLevelPercent);
}

// --- Closed-Loop Control Logic ---
void executeAutomationLogic() {
  if (soilMoisture > 2800) { 
    digitalWrite(PUMP_PIN, LOW);  
  } else {
    digitalWrite(PUMP_PIN, HIGH); 
  }

  if (tankLevelPercent < 15) { 
    digitalWrite(BUZZER_PIN, HIGH); 
  } else {
    digitalWrite(BUZZER_PIN, LOW);
  }
}

// --- Visual Layout Generation Modules ---
void updateDataDashboard() {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x12_tf); 
  
  u8g2.drawStr(6, 10, "MONEY PLANT TELEMETRY");
  u8g2.drawHLine(0, 14, 128);
  
  u8g2.setCursor(0, 28);
  u8g2.print("Ambient Temp: "); u8g2.print(temperature, 1); u8g2.print(" *C");
  
  u8g2.setCursor(0, 40);
  u8g2.print("Air Humidity: "); u8g2.print(humidity, 1); u8g2.print(" %");
  
  u8g2.setCursor(0, 52);
  u8g2.print("Soil Perm:    "); u8g2.print(soilMoisture);
  
  u8g2.setCursor(0, 64);
  u8g2.print("Tank Reserve: "); u8g2.print(tankLevelPercent); u8g2.print(" %");
  
  u8g2.sendBuffer();
}

void runAnimatedEyes() {
  u8g2.clearBuffer();
  
  const int leftEyeX = 38;
  const int rightEyeX = 90;
  const int eyeY = 32;
  const int outerRadius = 15;
  const int pupilRadius = 5;
  
  int pupilOffsetX = 0;
  int pupilOffsetY = 0;
  
  if (digitalRead(TOUCH_PIN) == HIGH) {
    pupilOffsetX = 5;  
    pupilOffsetY = -2; 
  } else {
    pupilOffsetX = -2;
    pupilOffsetY = 0;
  }

  u8g2.drawDisc(leftEyeX, eyeY, outerRadius, U8G2_DRAW_ALL);
  u8g2.drawDisc(rightEyeX, eyeY, outerRadius, U8G2_DRAW_ALL);
  
  u8g2.setDrawColor(0); 
  u8g2.drawDisc(leftEyeX + pupilOffsetX, eyeY + pupilOffsetY, pupilRadius, U8G2_DRAW_ALL);
  u8g2.drawDisc(rightEyeX + pupilOffsetX, eyeY + pupilOffsetY, pupilRadius, U8G2_DRAW_ALL);
  
  u8g2.setDrawColor(1); 
  u8g2.drawPixel(leftEyeX + pupilOffsetX + 1, eyeY + pupilOffsetY - 1);
  u8g2.drawPixel(rightEyeX + pupilOffsetX + 1, eyeY + pupilOffsetY - 1);
  
  u8g2.sendBuffer();
}