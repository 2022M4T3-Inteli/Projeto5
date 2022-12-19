// ---------------------------- BIBLIOTECAS ----------------------------

#include <String.h>
#include <bits/stdc++.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

// ---------------------------- CONST ----------------------------

// ----------- Wi-Fi -----------

#define routerSSID "ROUTER_SSID" // Definir essa constante como o SSID da rede do roteador local
#define routerPSW "ROUTER_PASSWORD" // Definir essa constante como a senha da rede do roteador local

// ----------- Firebase -----------

#define API_KEY "FIREBASE_API_KEY" // Definir essa constante como a chave da API do RTDB do Firebase
#define DATABASE_URL "FIREBASE_DATABASE_URL" // Definir essa constante como a URL do RTDB do Firebase

// ---------------------------- GLOBAL VAR ----------------------------

// ----------- Wi-Fi -----------

char* beaconSSID; // SSID da rede do beacon

char* beaconPSW; // senha da rede do beacon

// ----------- Firebase -----------

// Configurações

FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;
bool signupOK = false;

// ---------------------------- FUNCTIONS ----------------------------

// ----------- Wi-Fi -----------

// Função que estabelece conexão Wi-Fi
void initWiFi(char* _ssid, char* _password) {
  WiFi.begin(_ssid, _password); // inicia a conexão usando o SSID e a senha da rede (argumentos)
  Serial.print("Connecting to WiFi ..");
  // Enquanto o beacon não estiver conectada, ela ficará tentando se conectar
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  // print do SSID e do IP da rede conectada
  Serial.print("Connected to Wi-Fi Network ");
  Serial.println(_ssid);
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
}

// ----------- Firebase -----------

// função que inicia a conexão do esp32 com o RTDB do Firebase
void initFirebase() {

  // configurações iniciais do Firebase
  config.api_key = API_KEY;

  config.database_url = DATABASE_URL;

  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("ok");
    signupOK = true;
  }
  else {
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  config.token_status_callback = tokenStatusCallback; 

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}

// função que faz um get no RTDB (na pasta "beacons") para saber o SSID e a senha do beacon
void getBeaconData(String _beaconAddress) {
  sendDataPrevMillis = 0;

  if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 15000 || sendDataPrevMillis == 0)) {
    
    sendDataPrevMillis = millis();

    // get do SSID
    if (Firebase.RTDB.getString(&fbdo, (String("/beacons/") + _beaconAddress + String("/ssid")))) {
      if (fbdo.dataType() == "string") {
        String strBeaconSSID = fbdo.stringData(); // string que está armazenda no endereço "/ssid"

        Serial.println(strBeaconSSID);

        // transformação da string em char
      	char *charBeaconSSID = new char[strBeaconSSID.length() + 1];

        Serial.println(charBeaconSSID);

	      strcpy(charBeaconSSID, strBeaconSSID.c_str()); 
        beaconSSID = charBeaconSSID; // o SSID é armazendo na variável "beaconSSID"
      }
    }
    else {
      Serial.println(fbdo.errorReason());
    }

    // get da senha
    if (Firebase.RTDB.getString(&fbdo, (String("/beacons/") + _beaconAddress + String("/password")))) {
      if (fbdo.dataType() == "string") {
        String strBeaconPSW = fbdo.stringData(); // string que está armazenda no endereço "/password

        // transformação da string em char
      	char *charBeaconPSW = new char[strBeaconPSW.length()]; 

        strcpy(charBeaconPSW, strBeaconPSW.c_str());
        beaconPSW = charBeaconPSW; // a senha é armazenda na variável "beaconPSW"

        Serial.println(strBeaconPSW);
        Serial.println(beaconPSW);
      }
    }
    else {
      Serial.println(fbdo.errorReason());
    }
  }

}

// ---------------------------- SETUP ----------------------------

void setup() {
  
  Serial.begin(115200);

  WiFi.mode(WIFI_STA); // beacon configurado como uma "Wi-Fi Station" (procura redes para se conectar)

  initWiFi(routerSSID, routerPSW); // conexão com o roteador local

  initFirebase(); // inicia a conexão com o firebase

  String macAddress = WiFi.macAddress(); // define o endereço MAC do beacon
  
  getBeaconData(macAddress); // get do SSID e da senha do beacon

  WiFi.disconnect(); // desconecta do roteador local

  WiFi.mode(WIFI_AP); // beacon configurado como uma "Wi-Fi Access Point" (abre uma rede para as tags se conectarem)

  WiFi.softAP(beaconSSID, beaconPSW, 1, 0, 4, true); // inicia um Access Point como um FTM Responder (opção de receber um pacote FTM ativada)

  IPAddress IP = WiFi.softAPIP(); // endereço IP do beacon
  Serial.print("AP IP address: ");
  Serial.println(IP);
  
  Serial.print("Beacon MacAddress: ");
  Serial.println(macAddress);

  Serial.print("Network SSID: ");
  Serial.println(beaconSSID);

  

}

void loop() {
  // put your main code here, to run repeatedly:

}