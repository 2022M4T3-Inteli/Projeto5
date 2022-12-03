// ---------------------------- BIBLIOTECAS ----------------------------

#include <String.h>
#include <string.h>
#include <bits/stdc++.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

// ---------------------------- CONST ----------------------------

// ----------- Wi-Fi -----------

#define routerSSID "PLACE_ROUTER_SSID_HERE" // Definir essa constante como o SSID da rede do roteador local
#define routerPSW "PLACE_ROUTER_PASSWORD_HERE" // Definir essa constante como a senha da rede do roteador local

// ----------- Firebase -----------

#define API_KEY "FIREBASE_API_KEY" // Definir essa constante como a chave da API do RTDB do Firebase
#define DATABASE_URL "FIREBASE_DATABASE_URL" // Definir essa constante como a URL do RTDB do Firebase

// ----------- FTM -----------

// Configurações
const uint8_t FTM_FRAME_COUNT = 16;
const uint16_t FTM_BURST_PERIOD = 10;

// Semaphore
xSemaphoreHandle ftmSemaphore;
bool ftmSuccess = true;

// ---------------------------- GLOBAL VAR ----------------------------

// ----------- FTM -----------

float distances[3] = {}; // Lista que armazena as distâncias medidas em relação a cada beacon

int ftmCount = 0; // Variável que faz a contagem do número de distâncias que foram medidas


// ----------- Firebase -----------

// Configurações
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;
bool signupOK = false;

// ----------- Wi-Fi -----------

int beaconCount = 0; // Variável auxiliar para indicar em qual beacon a tag está se conectando
char* beaconsSSID[3] = {}; // Lista que armazena o SSID de cada beacon
char* beaconsPSW[3] = {}; // Lista que armazena a senha de cada beacon

// ---------------------------- FUNCTIONS ----------------------------

// ----------- Wi-Fi -----------

// Função que estabelece conexão Wi-Fi
void initWiFi(char* _ssid, char* _password) {
  WiFi.begin(_ssid, _password); // inicia a conexão usando o SSID e a senha da rede (argumentos)
  Serial.print("Connecting to WiFi ..");
  // Enquanto a tag não estiver conectada, ela ficará tentando se conectar
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

// Função que procur os beacons mais próximos para se conectar e, assim, definir em qual sala a tag está

// Lógica: o beacon carrega o nome da sala no seu SSID separado por "_" (ex: beacon1_sala1)
String searchNearBeacons() {
  int networks = WiFi.scanNetworks(); // Número de redes escaneadas pela tag

  String nearRooms[3] = {"", "", ""}; // Lista das salas mais próximas
  int beaconsInRoom[3] = {0, 0, 0}; // Número de beacons em cada sala

  String myRoom = ""; // Sala na qual a tag está
  int beaconsInMyRoom = 0; // Número de beacons dentro da sala

  // Loop que faz o scan das redes mais próximas -> obtém o nome da sala por meio do SSID dos beacons
  for (int i = 0; i < networks; i++) {
    String network = WiFi.SSID(i); // SSID da rede
''
    bool roomRegistered = false; // true: sala já foi registrada na variável "nearRooms" / false: sala ainda não registrada

    // obtém o nome da sala, que sempre virá depois do "_"

    int index = network.indexOf('_'); // posição do "_" no SSID

    if (index == -1) {
      continue; // caso o SSID não carregue o nome da sala, o i será continuado e essa rede será pulada
    }

    String networkRoom = network.substring(index + 1); // definição do nome da sala

    // loop que checa se a sala já foi registrada
    for (int j = 0; j < 3; j++) {
      // caso algum dos elementos em "nearRooms" seja o nome da sala, ela já foi registrada
      if (nearRooms[j] == networkRoom) {
        roomRegistered = true;
      }
    }

    // loop que conta o número de beacons scaneados em cada sala => a sala com mais beacons é a sala na qual a tag está
    for (int j = 0; j < 3; j++) {
      // se a sala já foi registrada, mais um beacon é adicionado na contagem dessa sala
      if (roomRegistered) {
        if (nearRooms[j] == networkRoom) {
          beaconsInRoom[j] += 1;
        }
      }
      // se a sala ainda não foi registrada, ela será e um beacon é adicionado na contagem dessa sala
      else {
        if (nearRooms[j] == "") {
          nearRooms[j] = networkRoom;
          beaconsInRoom[j] += 1;
        }
      }
    }

  }

  // a sala que possuir o maior número de beacons scaneados é a sala na qual a tag está 
  for (int i = 0; i < 3; i++) {
    if (beaconsInRoom[i] > beaconsInMyRoom) {
      beaconsInMyRoom = beaconsInRoom[i];
      myRoom = nearRooms[i];
    }
  }

  Serial.print("My Room: ");
  Serial.println(myRoom);

  return myRoom; // retorna a string com o ID da sala que a tag está
}

// ---------------------- FTM ----------------------

// função que faz o cálculo da distância a partir do "FtmReport"
void onFtmReport(arduino_event_t *event) {
  const char * status_str[5] = {"SUCCESS", "UNSUPPORTED", "CONF_REJECTED", "NO_RESPONSE", "FAIL"};
  wifi_event_ftm_report_t * report = &event->event_info.wifi_ftm_report;
  ftmSuccess = report->status == FTM_STATUS_SUCCESS;
  if (ftmSuccess) { // caso seja possível calcular o FTM
    Serial.printf("FTM Estimate: Distance RAW: %d, Distance: %.4f m, Return Time: %u ns\n", (int)report->dist_est, (float)report->dist_est / 100 - 39.7, report->rtt_est);
    // a distância estimada através do FTM é armazenada na lista "distances", de acordo com a posição indicada pelo "beaconCount"
    distances[beaconCount] += ((report->dist_est / 100) - 39.7); // conversão da distância para metros
    ftmCount ++; // mais uma distância obtida com sucesso
    Serial.println(distances[beaconCount]);
  } else { // se não foi possível calcular o FTM, o erro será printado
    Serial.print("FTM Error: ");
    Serial.println(status_str[report->status]);
  }
  xSemaphoreGive(ftmSemaphore);
}

// função que inicia uma sessão FTM e aguard por um Report -> true: conexão FTM estabelecida com sucesso / false: falha na conexão FTM
bool getFtmReport(){
  if(!WiFi.initiateFTM(FTM_FRAME_COUNT, FTM_BURST_PERIOD)){
    Serial.println("FTM Error: Initiate Session Failed");
    return false;
  }
  xSemaphoreTake(ftmSemaphore, portMAX_DELAY) == pdPASS && ftmSuccess;
  return true;
}

// função auxiliar que executa "getFtmReport()" e "onFtmReport()"
void getFtmDistance() {
  Serial.print("Initiating FTM session with Frame Count ");
  Serial.print(FTM_FRAME_COUNT); // frame count da sessão FTM (const)
  Serial.print(" and Burst Period ");
  Serial.print(FTM_BURST_PERIOD * 100); // período em ms de burst da sessão FTM (const)
  Serial.println(" ms");

  // a distância de uma tag para um beacon é calculada três vezes, para diminuir a margem de erro
  bool ftmSuccess = false; // true: conexão FTM estabelecida com sucesso / false: falha na conexão FTM 
  int i = 0; // número de conexões FTM bem sucedidas
  
  while (i < 3) {
    ftmSuccess = getFtmReport(); // tentativa de estabelecer uma conexão FTM (executa a função "getFtmReport()")
    if (ftmSuccess == true) {
      i++; // caso a conexão seja bem sucedida, a contagem aumenta
    }
  }
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

// função que faz um get no RTDB (na pasta "rooms") para saber o endereço MAC de um beacon que está na mesma sala da tag
String getBeaconAddress(int _count, String _roomName) {

  if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 15000 || sendDataPrevMillis == 0)) {
    
    sendDataPrevMillis = millis();

    String queryURL = String("/rooms/") + _roomName + String("/beacon") + String(_count + 1); // endereço onde o endereço MAC está armazenado

    if (Firebase.RTDB.getString(&fbdo, queryURL)) {
      if (fbdo.dataType() == "string") {
        String beaconMacAddress = fbdo.stringData();
        Serial.println(beaconMacAddress);
        return beaconMacAddress; // o endereço MAC do beacon é retornado no tipo string
      }
    }
    else {
      Serial.println(fbdo.errorReason());
      return String("F");
    }
  }
}

// função que faz um get no RTDB (na pasta "beacons") para saber o SSID e a senha de um beacon
void getBeaconData(int _count, String _beaconAddress) {
  sendDataPrevMillis = 0;

  if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 15000 || sendDataPrevMillis == 0)) {
    
    sendDataPrevMillis = millis();

    // get do SSID
    if (Firebase.RTDB.getString(&fbdo, (String("/beacons/") + _beaconAddress + String("/ssid")))) {
      if (fbdo.dataType() == "string") {
        String strBeaconSSID = fbdo.stringData(); // string que está armazenda no endereço "/ssid"

        Serial.println(strBeaconSSID);

        // transformação da string em char
      	char *beaconSSID = new char[strBeaconSSID.length() + 1];

        Serial.println(beaconSSID);

	      strcpy(beaconSSID, strBeaconSSID.c_str()); 
        beaconsSSID[_count] = beaconSSID; // o SSID é armazendo na lista "beaconsSSID", na posição indicada pelo argumento "_count"
      }
    }
    else {
      Serial.println(fbdo.errorReason());
    }

    // get da senha
    if (Firebase.RTDB.getString(&fbdo, (String("/beacons/") + _beaconAddress + String("/password")))) {
      if (fbdo.dataType() == "string") {
        String strBeaconPSW = fbdo.stringData(); // string que está armazenda no endereço "/password"

        // transformação da string em char
      	char *beaconPSW = new char[strBeaconPSW.length()]; 

        strcpy(beaconPSW, strBeaconPSW.c_str());
        beaconsPSW[_count] = beaconPSW; // a senha é armazenda na lista "beaconsPSW", na posição indicada pelo argumento "_count"

        Serial.println(strBeaconPSW);
        Serial.println(beaconPSW);
      }
    }
    else {
      Serial.println(fbdo.errorReason());
    }
  }
}

// função que faz um udate no RTDB (na pasta "tags") das distâncias calculadas e da sala na qual a tag está
void firebaseDataUpdate(String _macAddress, String _roomID) {
  sendDataPrevMillis = 0;

  if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 15000 || sendDataPrevMillis == 0)){
    sendDataPrevMillis = millis();

    String dburl = String("tags/") + _macAddress; // endereço URL com identificador de cada tag (endereço MAC)

    // loop que faz update das distâncias (salvas nos espaços "/distance1", "/distance2" e "/distance3")
    for (int updates=0; updates< 3; updates++){
      if (Firebase.RTDB.setFloat(&fbdo, dburl + String("/distance") + String(updates + 1), (distances[updates]))){
        Serial.println("PASSED");
        Serial.println("PATH: " + fbdo.dataPath());
        Serial.println("TYPE: " + fbdo.dataType());
      }
      else {
        Serial.println("FAILED");
        Serial.println("REASON: " + fbdo.errorReason());
      }
    }

    // update do ID da sala na qual a tag está (salvo no espaço "/room")
    if (Firebase.RTDB.setString(&fbdo, dburl + String("/room"), _roomID)){
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

// ---------------------------- SETUP ----------------------------

void setup() {
  Serial.begin(115200);

  ftmSemaphore = xSemaphoreCreateBinary(); // setup do semaphore do FTM
  
  WiFi.onEvent(onFtmReport, ARDUINO_EVENT_WIFI_FTM_REPORT); // tag configurada como "Initiator FTM" (emissor de sinal)

  WiFi.mode(WIFI_STA); // tag configurada como uma "Wi-Fi Station" (procura redes para se conectar)

  initWiFi(routerSSID, routerPSW); // conexão com o roteador local

  Serial.print("MAC Address: ");
  Serial.println(WiFi.macAddress());

  initFirebase(); // inicia a conexão com o firebase

  WiFi.disconnect(); // desconecta da rede atual
}

// ---------------------------- LOOP ----------------------------

void loop() {
  String roomID = searchNearBeacons(); // executa a função "searchNearBeacons()" para definir a sala na qual a tag está

  Serial.print("My Room: ");
  Serial.println(roomID);

  beaconCount = 0; // zera a comtagem de beacons

  // loop que faz a conexão nos três beacons para medir a distância de cada um em relação à tag
  for (int count=0; count<3; count++) {
    ftmCount = 0; // zera o número de conexôes FTM bem sucedidas

    initWiFi(routerSSID, routerPSW); // se conecta com o roteador local

    String beaconAddress = getBeaconAddress(count, roomID); // endereço MAC do beacon 1 da sala na qual a tag está

    getBeaconData(count, beaconAddress); // get do SSID e senha do beacon

    WiFi.disconnect(); // desconecta do roteador local

    Serial.print("Beacon SSID: ");
    Serial.println(beaconsSSID[count]);
    Serial.print("Beacon PSW: ");
    Serial.println(beaconsPSW[count]);

    initWiFi(beaconsSSID[count], beaconsPSW[count]); // se conecta no beacon

    getFtmDistance(); // mede a distância FTM

    beaconCount++; // adiciona um à contagem de beacons

    distances[count] = distances[count] / ftmCount; // faz uma média das distâncias calculadas pelo número de conexões FTM bem sucedidas

    WiFi.disconnect(); // desconecta do beacon
  }

  initWiFi(routerSSID, routerPSW); // se conecta no roteador local

  String myMacAddress = WiFi.macAddress(); // define o endereço MAC da tag

  firebaseDataUpdate(myMacAddress, roomID); // faz um update no RTDB com base no ID da tag (endereço MAC)

  WiFi.disconnect(); // se desconecta do roteador local

  delay(5000); // espera 5 segundos antes de realizar essa operação novamente
}
