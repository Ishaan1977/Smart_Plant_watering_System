# Smart_Plant_watering_System 🌿🤖

An edge-calculated, closed-loop **SmartPlant Monitoring and Automation System** powered by an ESP32 microcontroller running an asynchronous, non-blocking state machine. This project eliminates third-party cloud dependencies (like Blynk) by deploying a **localized HTML/CSS web server** accessible over a smartphone hotspot, paired with an interactive, presence-reactive human-machine interface (HMI) on a physical 1.34" SSD1306 OLED screen.

---

## 🚀 Key Features

* **Zero-Cloud Local Web Server:** Hosts a responsive, styled HTML/CSS dashboard directly from the ESP32 chip. Users can view real-time environmental metrics on any mobile browser over a local hotspot connection with a built-in 3-second auto-refresh cycle.
* **Presence-Reactive HMI Expression Engine:** Uses an onboard IR proximity sensor and capacitive touch pads to dynamically switch the 1.34" OLED display from a diagnostic data grid to animated, lifelike "OLED Eyes" that track human movement.
* **Closed-Loop Automated Irrigation:** Samples soil dielectric permittivity via a capacitive moisture probe (immune to galvanic corrosion) and operates a 5V centrifugal water pump via an optically isolated electromagnetic relay.
* **Precision Volumetric Tank Tracking:** Uses an HC-SR04 ultrasonic sensor tuned specifically for small-scale fluid columns ($1\text{--}5\text{ cm}$ safe operating range) to map tank capacity ($0\text{--}100\%$).
* **Hardware Fail-Safe Interlocking:** Features an integrated active piezo buzzer alarm that instantly trips and halts pump execution if the water reservoir falls below $15\%$ volume capacity to prevent pump dry-running or cavitation.

---

To protect the ESP32 boot configuration straps, the peripheral architecture is mapped directly to hardware-safe pins:

| Peripheral Device | Component Pin | ESP32 Destination | IO Type | Electrical Logic |
| :--- | :--- | :--- | :--- | :--- |
| **I2C Bus Shared** | OLED SDA / SCL | **GPIO 21 / GPIO 22** | Digital Out | 3.3V Native |
| **Irrigation Core** | Relay Signal Input | **GPIO 2** | Digital Out | 3.3V $\rightarrow$ 5V Shifted |
| **Ultrasonic Waves** | Trig / Echo | **GPIO 5 / GPIO 18** | Digital In/Out| Echo via $1\text{k}\Omega/2\text{k}\Omega$ Divider |
| **Acoustic Alarm** | Active Piezo (+) | **GPIO 19** | Digital Out | 3.3V Native |
| **Thermal Profiler** | DHT11 Data | **GPIO 23** | Digital I/O | 3.3V Single-Wire |
| **Human Proximity**| IR Module Out | **GPIO 25** | Digital Input | 3.3V Native |
| **Tactile Switch** | TTP223 Out Pin | **GPIO 26** | Digital Input | 3.3V Native |
| **Substrate Sensor**| Capacitive Probe | **GPIO 34** | Analog Input | 0.0V to 3.3V Max (ADC1) |

---

## 💻 Firmware Installation & Deployment

### 1. Prerequisites
Ensure you have the following libraries installed in your Arduino IDE:
* `DHT sensor library` by Adafruit
* `U8g2` by oliver

### 2. Flash Configuration
1. Open the project sketch `.ino` file.
2. Update the network credential constants to match your mobile phone's hotspot configuration:
   ```cpp
   const char* ssid = "YOUR_MOBILE_HOTSPOT_SSID";
   const char* pass = "YOUR_HOTSPOT_WPA2_PASSWORD";
3. Connect your ESP32 Development Board, select ESP32 Dev Module under your boards manager, and hit Upload.

### 3. Launching the Dashboard
* Keep your mobile hotspot active.

* Open the Serial Monitor (115200 Baud). Upon successful network alignment, the ESP32 will print its allocated local IP address (e.g., 192.168.43.XX).

* Launch any web browser on your connected mobile device and navigate to that IP address to load the responsive telemetry dashboard.
