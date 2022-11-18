#include <Arduino.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <String.h>

//Forneçe as informações do processo de geração de token.
#include "addons/TokenHelper.h"
//Funções auxiliares ("helpers") da RTDB (RealTime DataBase).
#include "addons/RTDBHelper.h"

// ROTEADOR
#define rtrssid "ROUTER_SSID" // place your router ssid here
#define rtrpassword "ROUTER_PASSWORD" // place your router password here

// ---------------------- FIREBASE ----------------------

// FIREBASE API KEY
#define API_KEY "API_KEY" // place your firebase api key here

// FIREBASE RTDB URL
#define DATABASE_URL "DATABASE_URL" // place your firebase rtdb url here

// ---------------------- WI-FI ----------------------

//ESP BEACONS WI-FI ACCESS POINT
char* beacons_ssids[3] = {"SSID_BEACON_1", "SSID_BEACON_2", "SSID_BEACON_3"}; // place your beacons ssids here
char* beacons_passwords[3] = {"PASSWORD_BEACON_1", "PASSWORD_BEACON_2", "PASSWORD_BEACON_3"}; // place your beacons passwords here

// ---------------------- FTM ----------------------

// CONFIG FTM
const uint8_t FTM_FRAME_COUNT = 16;
const uint16_t FTM_BURST_PERIOD = 10;

// SEMAPHORE FTM
xSemaphoreHandle ftmSemaphore;
bool ftmSuccess = true;

// FIREBASE DATA OBJECT
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;
int count = 0;
bool signupOK = false;

// ---------------------- AUX ----------------------

int beaconCount = 0;
float distances[3] = {};
String myMacAddress;

// ---------------------- WI-FI ----------------------

// INICIA CONEXÃO WI-FI
void initWiFi(char* ssid, char* password) {
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.print("Connected to Wi-Fi IP: ");
  Serial.println(WiFi.localIP());
}

// ---------------------- FTM ----------------------

// CALCULO DO TEMPOS E DAS DISTANCIAS COM OS DADOS DO FTM REPORT
void onFtmReport(arduino_event_t *event) {
  const char * status_str[5] = {"SUCCESS", "UNSUPPORTED", "CONF_REJECTED", "NO_RESPONSE", "FAIL"};
  wifi_event_ftm_report_t * report = &event->event_info.wifi_ftm_report;
  // Set the global report status
  ftmSuccess = report->status == FTM_STATUS_SUCCESS;
  if (ftmSuccess) {
    // The estimated distance in meters may vary depending on some factors (see README file)
    Serial.printf("FTM Estimate: Distance RAW: %d, Distance: %.4f m, Return Time: %u ns\n", (int)report->dist_est, (float)report->dist_est / 100 - 39.7, report->rtt_est);
    distances[beaconCount] += ((report->dist_est / 100) - 39.7);
    Serial.println(distances[beaconCount]);
    // Pointer to FTM Report with multiple entries, should be freed after use
    free(report->ftm_report_data);
  } else {
    Serial.print("FTM Error: ");
    Serial.println(status_str[report->status]);
  }
  // Signal that report is received
  xSemaphoreGive(ftmSemaphore);
}

// INICIA UMA SESSAO FTM E AGUARDA PELO REPORT
bool getFtmReport(){
  if(!WiFi.initiateFTM(FTM_FRAME_COUNT, FTM_BURST_PERIOD)){
    Serial.println("FTM Error: Initiate Session Failed");
    return false;
  }
  // Wait for signal that report is received and return true if status was success
  xSemaphoreTake(ftmSemaphore, portMAX_DELAY) == pdPASS && ftmSuccess;
  return true;
}

void printFtmReport() {
  Serial.print("Initiating FTM session with Frame Count ");
  Serial.print(FTM_FRAME_COUNT);
  Serial.print(" and Burst Period ");
  Serial.print(FTM_BURST_PERIOD * 100);
  Serial.println(" ms");

  // Request FTM reports until one fails
  for (int i=0; i<3; i++) {
    getFtmReport();
  }
}

// ---------------------- FIREBASE ----------------------

void firebaseDataUpdate(String macAddress) {
  if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 15000 || sendDataPrevMillis == 0)){
    sendDataPrevMillis = millis();

    String dburl = "tags/" + macAddress + "/distance";

    // Escreva um Float no caminho do banco de dados 'test/float'
    for (int updates=0; updates< 3; updates++){
      if (Firebase.RTDB.setFloat(&fbdo, dburl + String(updates + 1), (distances[updates]/3))){
        Serial.println("PASSED");
        Serial.println("PATH: " + fbdo.dataPath());
        Serial.println("TYPE: " + fbdo.dataType());
      }
      else {
        Serial.println("FAILED");
        Serial.println("REASON: " + fbdo.errorReason());
      }
    }

  }
}

void setup() {
  Serial.begin(115200);

  // ---------------------- FTM ----------------------

  // Create binary semaphore (initialized taken and can be taken/given from any thread/ISR)
  ftmSemaphore = xSemaphoreCreateBinary();
  
  // Listen for FTM Report events
  WiFi.onEvent(onFtmReport, ARDUINO_EVENT_WIFI_FTM_REPORT);

  // ---------------------- WI-FI ----------------------

  WiFi.mode(WIFI_STA);

  myMacAddress = WiFi.macAddress();

  // ROTEADOR
  initWiFi(rtrssid, rtrpassword);

  // ---------------------- FIREBASE ----------------------

  // ASSIGN API KEY
  config.api_key = API_KEY;

  // ASSIGN DATABASE URL
  config.database_url = DATABASE_URL;

  /* Sign up no Firebase */
  if (Firebase.signUp(&config, &auth, "", "")){
    Serial.println("ok");
    signupOK = true;
  }
  else{
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  /* Dar assign na função callback para a task de gerar um token, que consume muito tempo */
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h
  
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  WiFi.disconnect();
  Serial.println("Disconnected...");

  // BEACONS

}

void loop() {



  for (int i = 0; i<3; i++) {
    // get beacon distance
    initWiFi(beacons_ssids[i], beacons_passwords[i]);
    Serial.print("Connected to Beacon ");
    Serial.println(i + 1);
    printFtmReport();
    WiFi.disconnect();
    Serial.println("Disconnected...");

    beaconCount++;

  }

  // update firebase data
  initWiFi(rtrssid, rtrpassword);
  Serial.print("Connected to Router");
  firebaseDataUpdate(myMacAddress);
  WiFi.disconnect();

  delay(3000);

}
