//////////////////////////WIFI//CREDENCIALES///////////////////////////////////////////////////
#include <ESP8266WiFi.h>
#define STASSID "$_WiFi-Plus_$"
#define STAPSK  "MiRedWifi9876543210AZ"
/////////////////////////USAMOS//LOS//PINES//SERIALES//DEL//PM2.5/////////////////////////////
// El pin TX del sensor debe conectarse al RX del NODE MCU
#include "PMS.h"
PMS pms(Serial);
PMS::DATA data;
////////////////////////DEFINIMOS//EL//PIN//DEL//SENSOR//DHT22/////////////////////////////////
#include "DHT.h" //cargamos la librería DHT
#define DHTPIN D1 //Seleccionamos el pin en el que se conectará el sensor
#define DHTTYPE DHT22 //Se selecciona el DHT22(hay otros DHT)
DHT dht(DHTPIN, DHTTYPE); //Se inicia una variable que será usada por Arduino para comunicarse con el sensor
/////////////////////////PIN//PARA//EL//SENSOR//DE//CO////////////////////////////////////////
#define pinMQ A0
//////////////////////////CONFIGURACION//DEL//SERVIDOR////////////////////////////////////////
const char* ssid     = STASSID;
const char* password = STAPSK;
const String url =  "/aire-lima-2.5-v1.6/neodato/registrarMedidas";
const char* host = "192.168.0.112"; //localhost or www.impulso-peru.com
const uint16_t port = 80;  // HTTP PORT
//////////////////////DEFINIMOS//EL//CODIGO//DEL//SENSOR//RESPECTIVAMENTE//////////////////////
String codigoCO = "ACALLMQ001";
String codigoPM = "BLIPMSA001";
String codigoDH = "CSJLDHT001";
////////////////////CREAMOS//VARIABLES//PARA//GUARDAR//LOS//DATOS/////////////////////////////
float medidaCO = 0;
float medidaPM = 0;
float medidaTEM = 0;
float medidaHUM = 0;
/////////////////////////////////////////////////////////////////////////////////////////////
unsigned long previousMillis = 0;
String enviardatos(String datos) {
  String linea = "error";

  // Use WiFiClient para crear conexiones TCP
  Serial.print("conectando al servidor:  ");
  Serial.print(host);
  Serial.print(':');
  Serial.println(port);

  WiFiClient client;
  if (!client.connect(host, port)) {
    Serial.println("Conexion Fallida - Intente Nuevamente");
    delay(3000);
    return linea;
  }

  client.print(String("POST ") + url + " HTTP/1.1" + "\r\n" +
               "Host: " + host + "\r\n" +
               "Accept: */*" + "*\r\n" +
               "Content-Length: " + datos.length() + "\r\n" +
               "Content-Type: application/x-www-form-urlencoded" + "\r\n" +
               "\r\n" + datos);
  delay(10);
  Serial.print("enviando por la URL: ");
  Serial.println(url + "->" + datos);
  Serial.println("Enviando datos a SQL...");

  unsigned long timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > 5000) {
      Serial.println("Cliente fuera de tiempo!");
      client.stop();
      return linea;
    }
  }
  // Lee todas las lineas que recibe del servidro y las imprime por la terminal serial
  while (client.available()) {
    linea = client.readStringUntil('\r');
  }


  Serial.println(linea);
  return linea;

}
void setup() {
  /////////////////////////////////////////////////////////////////////////////////////////////
  Serial.begin(9600);
  Serial.print("chipId: ");
  String chip_id = String(ESP.getChipId());
  Serial.println(chip_id);
  /////////////////////////////////////////////////////////////////////////////////////////////
  dht.begin(); //Se inicia el sensor de DHT22
  /////////////////////////////////////////////////////////////////////////////////////////////
  Serial.println();
  Serial.println();
  /////////////////////////////////////////////////////////////////////////////////////////////
  Serial.print("Conectando Espere : ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi Conectado");
  Serial.println("My IP address is: ");
  Serial.println(WiFi.localIP());
  /////////////////////////////////////////////////////////////////////////////////////////////
}

void loop() {

 ///////////////TODOS//LOS//PINES//DEBEN//ESTAR//CONECTADOS//AL//NODE//MCU////////////////////
 /*
   Tener en cuenta que debe desconectar el PIN - RX del Node MCU
   para subir el codigo
 */
  if (pms.read(data)) {

    medidaCO = analogRead(pinMQ); //Lemos la salida analógica del MQ
    medidaPM = data.PM_AE_UG_2_5;
    medidaTEM = dht.readTemperature(); //Se lee la temperatura
    medidaHUM = dht.readHumidity(); //Se lee la humedad

    Serial.println("-----------------------------------------");
    Serial.print("Medida CO:");
    Serial.println(medidaCO);
    Serial.println("-----------------------------------------");
    Serial.print("PM 2.5 (ug/m3): ");
    Serial.println(medidaPM);
    Serial.println("-----------------------------------------");
    Serial.print("Humedad: ");
    Serial.println(medidaHUM);
    Serial.print("Temperatura: ");
    Serial.println(medidaTEM);
    Serial.println("-----------------------------------------");
    unsigned long currentMillis = millis();
    /////////////////////////////////////////////////////////////////////////////////////////////
    if (currentMillis - previousMillis >= 2000) { //Definimos el tiempo de envio
      previousMillis = currentMillis;
   //aire-lima-2.5-v1.6/neodato/registrarMedidas.php?codigox=ACALLMQ001&medidax=355.00&codigoy=BLIPMSA001&mediday=103.00&codigoz=CSJLDHT001&temperaturaz=18.30&humedadz=90.10
      enviardatos("codigox=" + codigoCO  +
                  "&medidax=" + medidaCO +
                  "&codigoy=" + codigoPM +
                  "&mediday=" + medidaPM +
                  "&codigoz=" + codigoDH +
                  "&temperaturaz=" + medidaTEM +
                  "&humedadz=" + medidaHUM);
    }
    /////////////////////////////////////////////////////////////////////////////////////////////

  }

}
