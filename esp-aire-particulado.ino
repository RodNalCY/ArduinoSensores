https://blog.bricogeek.com/noticias/electronica/comparativa-y-analisis-completo-de-los-modulos-wifi-esp8266-y-esp32/
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <HTTPClient.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <DHT.h>
#define DHTTYPE DHT11     // DHT 11 
#define DHTPIN 26  
#define DHTLED 25
#define RETRY_LIMIT  20

 DHT dht(DHTPIN, DHTTYPE);
 
const char* ssid = "TP-LINK_40000D";
const char* password = "80821885";
String key = "$2y$10$.Qgbl9e1XVUlztQUZp1OI./E6T04BFemiGxy62R0KzZZ9gj5m9eFG";
String estacion= "CSLDTH001";

void setup()
{
    pinMode(DHTLED,OUTPUT);
    dht.begin();
    Serial.begin(115200);
    WiFi.begin(ssid,password);
    while (WiFi.status()!= WL_CONNECTED){
      delay(500);
      Serial.print(".");
    }
    
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP Address");
    Serial.println(WiFi.localIP());
 
}

void loop()
{
    int rtl = RETRY_LIMIT;
    float h =  dht.readHumidity();
    float t= dht.readTemperature();    
    delay(500);
    
    while (isnan(t) || isnan(h)){
      Serial.println("Verificar sensor otra vez - " + rtl);
       h =  dht.readHumidity();
       t= dht.readTemperature();
      delay(500);
      if (--rtl <1){
        ESP.restart(); // A veces, el DHT11 seguía devolviendo NAN. Un reinicio solucionó esto.
      }
    }
    if ((WiFi.status() == WL_CONNECTED)) {   
    
      digitalWrite(DHTLED,HIGH);
    //Abrir una nueva conexion 
      HTTPClient http;
      http.begin("https://impulso-peru.com/aire-particulado/api/th/crear.php");
      http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    //formato del POST request.
      int httpResponseCode = http.POST("estacion="+estacion+"&temperatura=" + t +"&humedad=" + h  +"&key="+key);

      if (httpResponseCode >0){
          //Verificando el estatus y el link completo
        String response = http.getString();
        Serial.println(httpResponseCode);
        Serial.println(response);
      }
      else if(httpResponseCode !=200){
        ESP.restart();
        Serial.print("Hay un error en la conexion de internet.. al enviar el post");
        
      }
      else{
        Serial.print("Error al enviar post");
        Serial.println(httpResponseCode);
      }
    //cerrar HTTP request.Para poder solicitar una nueva data
      http.end();
  
    //Valores del sensor
      Serial.println("Temperatura = " + String(t));
      Serial.println("humedad = " + String(h));
    }
    else{      
      digitalWrite(DHTLED,LOW);
      ESP.restart(); // A veces, se pierde la conexión. Un reinicio solucionó esto.
      }
    
   //cada 6 segundos se enviara los datos
    delay(6000);
    
    
}
