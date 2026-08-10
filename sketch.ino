//                               IoT CHALLENGE 1                  
//                        Team Leader Person Code: 11047334                  


#include <WiFi.h>
#include <esp_now.h>

// ---------------------------- Pin Definitions --------------------------------
#define PIN_TRIG 12
#define PIN_ECHO 14

// ---------------- Constants & Configuration ----------------
#define DISTANCE_THRESHOLD 50  // Threshold distance (cm) for occupancy detection
#define SLEEP_DURATION 39      // X = (34 mod 50) + 5 = 39 seconds
#define INTO_CM 58             // Proportionality factor to get distance in cm
#define TX_WAIT_US 600         // Wait time to ensure transmission
#define DEBUG true             // If set TRUE will print timings and received message.

// --------------------------- ESP-NOW Configuration ---------------------------
uint8_t sinkAddress[] = {0x8C, 0xAA, 0xB5, 0x84, 0xFB, 0x90}; 
esp_now_peer_info_t sink;


// Timing variables
unsigned long t_wakeup, t_sensor_start, t_sensor_end, t_transmission_start, t_transmission_end, t_idle_start, t_idle_end, t_deep_sleep;

// ----------------------------- Callback Functions ----------------------------
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
    Serial.print("-----> Send status: ");
    Serial.println(status == ESP_NOW_SEND_SUCCESS ? "SUCCESS" : "FAILURE");
}

void OnDataRecv(const esp_now_recv_info *mac_address, const uint8_t *data, int length) {
  if(DEBUG) {
    char message[length];
    memcpy(message, data, length);
    Serial.print("-----> Message received: " + String(message) + "\n");
  }
}

// ----------------- Occupancy Measurement -----------------
bool measureOccupancy() {
    t_sensor_start = micros();  // Start time of sensor reading

    digitalWrite(PIN_TRIG, LOW);
    delayMicroseconds(2);
    digitalWrite(PIN_TRIG, HIGH);
    delayMicroseconds(10);
    digitalWrite(PIN_TRIG, LOW);

    long duration = pulseIn(PIN_ECHO, HIGH); 

    float distance = duration / INTO_CM;

    Serial.print("----> Measured Distance: ");
    Serial.print(distance);
    Serial.println(" cm");

    t_sensor_end = micros();  // End time of sensor reading

    return (distance >= DISTANCE_THRESHOLD); // Returns true if the spot is occupied
}

// ----------------- Setup Function -----------------
void setup() {
    t_wakeup = micros(); // Wakeup time

    Serial.begin(115200);
    Serial.println("\n############################");
    Serial.println("***** Initialization ******");

    // Sensor pins initialization
    pinMode(PIN_TRIG, OUTPUT);
    pinMode(PIN_ECHO, INPUT);
    
    t_idle_start = micros(); // Start of idle period

    // Measure parking occupancy
    bool isOccupied = measureOccupancy();
    String statusMessage = isOccupied ? "FREE" : "OCCUPIED";
    Serial.print("----> Parking Status: ");
    Serial.println(statusMessage);

    t_idle_end = micros(); // End of idle period before Wi-Fi initialization

    // Wi-Fi and ESP-NOW setup
    WiFi.mode(WIFI_STA);
    WiFi.setTxPower(WIFI_POWER_2dBm);
    
    if (esp_now_init() != ESP_OK) {
        Serial.println(" ESP-NOW Initialization Failed");
        return;
    }

      esp_now_register_send_cb(OnDataSent);
      esp_now_register_recv_cb(OnDataRecv);

    memcpy(sink.peer_addr, sinkAddress, 6);
    sink.channel = 0;
    sink.encrypt = false;

    if (esp_now_add_peer(&sink) != ESP_OK) {
        Serial.println(" Failed to add ESP-NOW peer");
        return;
    }

    t_transmission_start = micros(); // Transmission start time
    
    esp_now_send(sinkAddress, (uint8_t*)statusMessage.c_str(), statusMessage.length() + 1);
    delayMicroseconds(TX_WAIT_US);

    t_transmission_end =t_transmission_end+TX_WAIT_US;

    t_transmission_end = micros(); // Transmission end time

    // Disable Wi-Fi to save power
    Serial.println("----> Disabling Wi-Fi...");
    WiFi.mode(WIFI_OFF);

    t_deep_sleep = micros(); // Time before entering deep sleep

    // Convert time from microseconds to seconds
    float sensor_time = (t_sensor_end - t_sensor_start) / 1000000.0;
    float transmission_time = (t_transmission_end - t_transmission_start) / 1000000.0;
    float idle_time = (t_idle_end - t_idle_start) / 1000000.0;
    float total_active_time = (t_deep_sleep - t_wakeup) / 1000000.0;

    // Print execution times
    Serial.println("\n[Execution Times in seconds]");
    Serial.println(" -> Sensor Reading Time: " + String(sensor_time, 5) + " s");
    Serial.println(" -> Transmission Time: " + String(transmission_time, 5) + " s");
    Serial.println(" -> Idle Time: " + String(idle_time, 5) + " s");
    Serial.println(" -> Total Active Time Before Sleep: " + String(total_active_time, 5) + " s");
    Serial.println(" -> Sleep Duration: " + String((float)SLEEP_DURATION, 5) + " seconds");

    // Enter deep sleep
    Serial.println("#Entering Deep Sleep#...");
    Serial.println("\n############################\n");
    Serial.flush();
    esp_sleep_enable_timer_wakeup(SLEEP_DURATION * 1000000); 
    esp_deep_sleep_start();
}

void loop()
{
  // Not executed due to deep sleep
}
