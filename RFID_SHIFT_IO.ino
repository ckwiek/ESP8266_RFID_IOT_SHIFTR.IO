/*
 * ***************************************************************************************
   Este programa foi testado por mim e funciona, sua fibalidade é didatica,
   pode ser alterado, parcial ou completamente sem previo aviso.

   Ebora funcionou adquedamente comigo, não existe garantia que funcionara com voce,
   mas se seguir as instruções do tutorial deveria funcionar.
   *****************************************************************************************
   Sobre mim:

   Engenheiro eletronico amante de eletronica e programação.

   ******************************************************************************************

   Link do video de apoio: https://youtu.be/a9HOjz0DkN8

   Link do projeto completo no Github: https://github.com/ckwiek/ESP8266_RFID_IOT_SHIFTR.IO

   *******************************************************************************************

   Contato e-mail: contato@carloskwiek.com.br 
   
    <http://www.gnu.org/licenses/>.

    ******************************************************************************************
*/
 
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPUpdateServer.h>
 
#include <MQTTClient.h> 
#include <SPI.h>
#include <MFRC522.h>
 
#define SS_PIN  D8 //Pin on Nodemcu ou Wemos
#define RST_PIN D3 //Pin on WeMos Nodemcu ou D1 Wemos
#define LED     D1 //LED GPIO
 
const char* host = "rfid_reader"; // isso sera usado no shiftr.io
const char* ssid = "Nome sua rede";
const char* password = "Senha sua rede";
 
const char* brocker = "broker.shiftr.io"; // endereço do broker
const char* mqttUser = "4f4c2f27";        // nome de usuario que o site lhe dara depois d ese inscrever
const char* mqttPass = "93f87408e2d580e8"; // Senha que o site lhe dara depois de se inscrever 
 
ESP8266WebServer httpServer(80);
ESP8266HTTPUpdateServer httpUpdater;
 
WiFiClient wifi;
MQTTClient mqtt;
 
MFRC522 rfid(SS_PIN, RST_PIN); // Instanciado a classe
MFRC522::MIFARE_Key key;
 
unsigned int batt;
double battV;
unsigned long oldMillis;
 
void connect();
 
void setup(void){
  pinMode (LED, OUTPUT);
  digitalWrite (LED,HIGH);
  SPI.begin();
  rfid.PCD_Init();
 
  Serial.begin(115200);
  Serial.println();
  Serial.println("Carregando o sketch..."); // Voce pode alterar esta mensagem a seu gosto e necessidade
  WiFi.mode(WIFI_AP_STA);
  WiFi.begin(ssid, password);
  
 
  mqtt.begin(brocker, wifi);
  mqtt.publish("/rfid_reader/resetReason", ESP.getResetReason());
 
  connect();
 
  MDNS.begin(host);
 
  //Anexar para diferentes páginas.
  httpUpdater.setup(&httpServer);
 
  httpServer.on("/", handleRoot);
 
  httpServer.begin();
 
  MDNS.addService("http", "tcp", 80);
  Serial.println("Sistema carregado!"); // Voce pode alterar esta mensagem a seu gosto e necessidade
  Serial.println("Aproxime seu cartao administrador"); // Voce pode alterar esta mensagem a seu gosto e necessidade
}
 
void loop(void){
  if(!mqtt.connected()) {
    connect();
  }
 
  httpServer.handleClient();
  mqtt.loop();
  delay(10);
 
  handleRFID();
 
  //coisas que só devem ser transmitidas a cada 60 segundos
  if (millis()-oldMillis > 60000) {
    batt = analogRead(A0);
    battV = mapDouble(batt, 0, 1023, 0.0, 6.6);
    mqtt.publish("/rfid_reader/batt", String(battV));
    mqtt.publish("/rfid_reader/battRaw", String(batt));
 
    oldMillis = millis();
  }
}
 
void connect() {
  while(WiFi.waitForConnectResult() != WL_CONNECTED){
    WiFi.begin(ssid, password);
    Serial.println("WiFi failed, retrying.");
  }
 
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
 
  while (!mqtt.connect(host, mqttUser, mqttPass)) {
    Serial.print(".");
  }
  Serial.println("\nconnected!");
}
 
void handleRoot() {
  httpServer.send(200, "text/plain", "It works!!!");
}
 
void messageReceived(String topic, String payload, char * bytes, unsigned int length) {
  Serial.print("incoming: ");
  Serial.print(topic);
  Serial.print(" - ");
  Serial.print(payload);
  Serial.println();
}
 
void handleRFID() {
  if (!rfid.PICC_IsNewCardPresent()) return;
  if (!rfid.PICC_ReadCardSerial()) return;
  mqtt.publish("/rfid_reader/uid", printHex(rfid.uid.uidByte, rfid.uid.size));
  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
}
 
double mapDouble(double x, double in_min, double in_max, double out_min, double out_max)
{
  double temp = (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
  temp = (int) (4*temp + .5);
  return (double) temp/4;
}
 
  String printHex(byte *buffer, byte bufferSize) {
  String id = "";
  for (byte i = 0; i < bufferSize; i++) {
    id += buffer[i] < 0x10 ? "0" : "";
    id += String(buffer[i], HEX);

    if((id) == ("Aqui a ID de sua TAG")){  //ID ENTRADA
       digitalWrite (LED,HIGH);
       delay(200);
       digitalWrite (LED,LOW);
       Serial.println ("Administrador logado"); // Voce pode alterar esta mensagem a seu gosto e necessidade
    }
    
    if((id) == ("Aqui a ID de sua outra TAG")){  //ID ENTRADA
       digitalWrite (LED,HIGH);
       delay(200);
       digitalWrite (LED,LOW);
       Serial.println ("Fulano chegou em casa"); // Voce pode alterar esta mensagem a seu gosto e necessidade
      
    }
     if((id) == ("Aqui a ID de sua outra TAG")){   //ID SAIDA
       digitalWrite (LED,HIGH);
       delay(200);
       digitalWrite (LED,LOW);
       Serial.println ("Ciclano saiu de casa"); // Voce pode alterar esta mensagem a seu gosto e necessidade
      
     }
  }
  return id;
}
  
  


