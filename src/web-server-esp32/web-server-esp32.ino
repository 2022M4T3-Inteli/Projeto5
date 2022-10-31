// importação das bibliotecas usadas
#include <WiFi.h>
#include <WebServer.h>

// id e senha da rede Wi-Fi
const char* ssid = "ESP32";  // nome
const char* password = "12345678";  // senha

// configurações de endereço IP, gateway e subnet
IPAddress local_ip(192,168,1,1);
IPAddress gateway(192,168,1,1);
IPAddress subnet(255,255,255,0);

// definição da porta do servidor
WebServer server(80);

// definição da porta e do status do LED
uint8_t LEDpin = 11;
bool LEDstatus = LOW;

// definição da porta e do status do BUZZER
uint8_t BUZZERpin = 3;
bool BUZZERstatus = LOW;

void setup() {
  Serial.begin(115200);

  // define as portas do ESP32 usadas pelo LED e pelo BUZZER como outputs
  pinMode(LEDpin, OUTPUT);
  pinMode(BUZZERpin, OUTPUT);

  // seta o ESP32 para ser um Wi-Fi Acess Point
  WiFi.softAP(ssid, password); // define o nome e a senha da rede
  WiFi.softAPConfig(local_ip, gateway, subnet); // configurações de IP
  delay(100);
  
  // definição dos URLs do servidor e quais funções serão executadas quando eles são acessados
  server.on("/", handle_OnConnect);
  server.on("/ledon", handle_ledon);
  server.on("/ledoff", handle_ledoff);
  server.on("/buzzeron", handle_buzzeron);
  server.on("/buzzeroff", handle_buzzeroff);
  server.onNotFound(handle_NotFound);
  
  // inicia o web server
  server.begin();
  Serial.println("HTTP server started");
}
void loop() {
  server.handleClient();
  // caso o LEDstatus seja HIGH (true), o LED fica ligado
  if(LEDstatus)
  {digitalWrite(LEDpin, HIGH);}
  // caso contrário, o LED fica desligado
  else
  {digitalWrite(LEDpin, LOW);}
  // caso o BUZZERstatus seja HIGH (true), o BUZZER fica tocando
  if(BUZZERstatus)
  {tone(BUZZERpin, 440, 300);
  delay(700);}
  // caso contrário, o BUZZER fica desligado
  else
  {digitalWrite(BUZZERpin, LOW);}
}

// função executada quando o cliente conecta no servidor pela primeira vez
void handle_OnConnect() {
  // define os status do LED e do BUZZER como desligados
  LEDstatus = LOW;
  BUZZERstatus = LOW;
  Serial.println("LED Status: OFF");
  Serial.println("BUZZER Status: OFF");
  server.send(200, "text/html", SendHTML(LEDstatus, BUZZERstatus)); // envia o HTML de acordo com os status do LED e do BUZZER (ambos desligados nesse caso)
}

// função executada quando o cliente aperta o botão de ON no status do LED
void handle_ledon() {
  LEDstatus = HIGH; // define o status do LED como ligado
  Serial.println("LED Status: ON");
  server.send(200, "text/html", SendHTML(true, BUZZERstatus)); // envia o HTML de acordo com os status do LED (ligado) e do BUZZER
}

// função executada quando o cliente aperta o botão de OFF no status do LED
void handle_ledoff() {
  LEDstatus = LOW; // define o status do LED como desligado
  Serial.println("LED Status: OFF");
  server.send(200, "text/html", SendHTML(false, BUZZERstatus)); // envia o HTML de acordo com os status do LED (desligado) e do BUZZER
}

// função executada quando o cliente aperta o botão de ON no status do BUZZER
void handle_buzzeron() {
  BUZZERstatus = HIGH; // define o status do BUZZER como ligado
  Serial.println("BUZZER Status: ON");
  server.send(200, "text/html", SendHTML(LEDstatus, true)); // envia o HTML de acordo com os status do LED e do BUZZER (ligado)
}

// função executada quando o cliente aperta o botão de OFF no status do buzzer
void handle_buzzeroff() {
  BUZZERstatus = LOW; // define o status do BUZZER como desligado
  Serial.println("BUZZER Status: OFF");
  server.send(200, "text/html", SendHTML(LEDstatus, false)); // envia o HTML de acordo com os status do LED e do BUZZER (desligado)
}

// função executada caso o usuário acesse um URL não definido
void handle_NotFound(){
  server.send(404, "text/plain", "Not found");
}

// função que define o HTML da página de acordo com os status do LED e do BUZZER
String SendHTML(uint8_t ledstat, uint8_t buzzerstat){
  String ptr = "<!DOCTYPE html> <html>\n";
  ptr +="<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  ptr +="<title>LED Control</title>\n";
  // css 
  ptr +="<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n";
  ptr +="body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;} h3 {color: #444444;margin-bottom: 50px;}\n";
  ptr +=".button {display: block;width: 80px;background-color: #3498db;border: none;color: white;padding: 13px 30px;text-decoration: none;font-size: 25px;margin: 0px auto 35px;cursor: pointer;border-radius: 4px;}\n";
  ptr +=".button-on {background-color: #3498db;}\n";
  ptr +=".button-on:active {background-color: #2980b9;}\n";
  ptr +=".button-off {background-color: #34495e;}\n";
  ptr +=".button-off:active {background-color: #2c3e50;}\n";
  ptr +="p {font-size: 14px;color: #888;margin-bottom: 10px;}\n";
  ptr +="</style>\n";
  // fim css
  ptr +="</head>\n";
  ptr +="<body>\n";
  ptr +="<h1>ESP32 Web Server</h1>\n"; //h1
  ptr +="<h3>Rastrear Ativos</h3>\n"; //h3
  
  // variações dos botões
  
  if(ledstat) // se o status do LED estiver como ligado, a página mostra o status do LED como "ON" e o botão com a opção de desligá-lo, que leva para a URL "/ledoff"
  {ptr +="<p>LED Status: ON</p><a class=\"button button-off\" href=\"/ledoff\">OFF</a>\n";}
  else // caso contrário, o status do LED fica como "OFF" e o botão, com a opção de ligá-lo, leva para a URL "/ledon"
  {ptr +="<p>LED Status: OFF</p><a class=\"button button-on\" href=\"/ledon\">ON</a>\n";}

  if(buzzerstat) // se o status do BUZZER estiver como ligado, a página mostra o status do BUZZER como "ON" e o botão com a opção de desligá-lo, que leva para a URL "/buzzeroff"
  {ptr +="<p>BUZZER Status: ON</p><a class=\"button button-off\" href=\"/buzzeroff\">OFF</a>\n";}
  else // caso contrário, o status do BUZZER fica como "OFF" e o botão, com a opção de ligá-lo, leva para a URL "/buzzeron"
  {ptr +="<p>BUZZER Status: OFF</p><a class=\"button button-on\" href=\"/buzzeron\">ON</a>\n";}

  ptr +="</body>\n";
  ptr +="</html>\n";
  return ptr;
}