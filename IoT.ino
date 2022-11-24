//Librerias utilizadas
#include "WiFi.h"
#include "ESPAsyncWebServer.h"
#include "Adafruit_Sensor.h"
#include "DHT.h"

//ngrok http http://192.168.68.112:80
//Credenciales de la Red WiFi local
char* ssid = "Aether Deco";
char* password = "Casa1409";

//Pin digital
#define DHTPIN 15

//Sensor a utilizar
#define DHTTYPE    DHT22

DHT dht(DHTPIN, DHTTYPE);

//Crear server asíncrono en el puerto 80
AsyncWebServer server(80);

String readDHTTemperature() {
  //Leer temperatura en grados Celsius
  float t = dht.readTemperature();
  //Detectar errores en el sensor.
  if (isnan(t)) {    
    Serial.println("Error: no hay lecturas del sensor DHT.");
    return "--";
  }
  else {
    Serial.println(t);
    return String(t);
  }
}

String readDHTHumidity() {
  //Leer porcentaje de humedad
  float h = dht.readHumidity();
  if (isnan(h)) {
    Serial.println("Error: no hay lecturas del sensor DHT.");
    return "--";
  }
  else {
    Serial.println(h);
    return String(h);
  }
}

//HTML básico para alojar los datos en el web server asíncrono, coloca 2 place holders en 2 párrafos, correspondientes a los datos de temperatura y humedad.
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<body>
  <p>
    ${T:<span id="temperature">%TEMPERATURE%</span>,H:<span id="humidity">%HUMIDITY%</span>}$
  </p>
</body>
<script>
setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("temperature").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/temperature", true);
  xhttp.send();
}, 10000 ) ;

setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("humidity").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/humidity", true);
  xhttp.send();
}, 10000 ) ;
</script>
</html>)rawliteral";

//Reemplazar placeholder con el valor real de temperatura.
String processor(const String& var){
  if(var == "TEMPERATURE"){
    return readDHTTemperature();
  }
  else if(var == "HUMIDITY"){
    return readDHTHumidity();
  }
  return String();
}

void setup(){
  //Puerto serial para revisar información del ESP32
  Serial.begin(115200);

  dht.begin();
  
  //Conectar al WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Conectando a red WiFi...");
  }

  //Imprimir IP local del ESP32
  Serial.println(WiFi.localIP());

  //Ruta para la creación de la página HTML en el Server del ESP32
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });
  server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", readDHTTemperature().c_str());
  });
  server.on("/humidity", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", readDHTHumidity().c_str());
  });

  //Iniciar Server
  server.begin();
}
 
void loop(){
}
