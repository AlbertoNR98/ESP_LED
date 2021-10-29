// Firmware LED Strip SMD5050 y ESP8266EX
// Librerias WiFi y EEPROM
#include <ESP8266WiFi.h>
#include <EEPROM.h>

// Credenciales WiFi
const char* ssid = "";
const char* password = "";

// Configuracion con IP estatica y puerto 80 para el servidor HTTP
const IPAddress local_IP(192,168,1,111);
const IPAddress gateway(192,168,1,1);
const IPAddress subnet(255,255,255,0);
const IPAddress primaryDNS(80,58,61,250);
WiFiServer server(local_IP,80);

// Variables para decodificar el HTTP GET
String redString = "0"; 
String greenString = "0";
String blueString = "0";
int pos1 = 0;
int pos2 = 0;
int pos3 = 0;
int pos4 = 0;

//Valores iniciales de los pines -> Color ocre
int redValue = 255;
int greenValue = 212;
int blueValue = 0;

//Direcciones de la EEPROM
int address = 0;

// Variable para almacenar el HTTP request
String header;

// Pines de los LEDS -> PWM
const int redPin = 13;     // GPIO13
const int greenPin = 12;   // GPIO12
const int bluePin = 14;    // GPIO14

// Resolucion de PWM -> 8 bits
const int range = 256;

// Tiempo actual
unsigned long currentTime = millis();
// Tiempo anterior
unsigned long previousTime = 0; 
// Timeout (milisegundos)
const long timeoutTime = 2000;

// Cabecera de las funciones
void initialFadeIn();
void connectToWiFi();
void sendResponse(WiFiClient);
void manageRequest();

void setup() {
  Serial.begin(9600);
  // Lee de la EEPROM el último estado
  EEPROM.begin(3*sizeof(int));  //Mapea la EEPROM en un byte-array de RAM 
  EEPROM.get(address, redValue);
  address += sizeof(redValue);
  EEPROM.get(address, greenValue);
  address += sizeof(greenValue);
  EEPROM.get(address, blueValue);

  // Configura la resolucion de PWM
  analogWriteRange(range);
  // Animacion inicial con los colores del ultimo estado
  initialFadeIn();
  // Conexion a WiFi
  connectToWiFi();
  // Arranca el servidor
  server.begin();
}

void loop(){
  WiFiClient client = server.available();   // Escucha a posibies clientes

  if (client) {                             // Si un cliente se conecta
    currentTime = millis();
    previousTime = currentTime;
    Serial.println("Nuevo cliente.");       
    String currentLine = "";                // Guarda en un String la informacion que viene del cliente
    while (client.connected() && currentTime - previousTime <= timeoutTime) {            // Mientras el cliente este conectado
      currentTime = millis();
      if (client.available()) {             // Si hay bytes del cliente por leer
        char c = client.read();             // Lee el ultimo byte
        Serial.write(c);                    // Lo muestra en el puerto serie
        header += c;
        if (c == '\n') {                    // Si el byte es un salto de linea
          // Si la linea actual esta en blanco, hay dos saltos de linea juntos -> Final de la request (por formato) -> Responder
          if (currentLine.length() == 0) {
            sendResponse(client); // Envia la respuesta -> web
            manageRequest();  // Gestiona la peticion
            break;  // Rompe el while
          } else { // Si llega una nueva linea, limpia la linea actual para recibirla
            currentLine = "";
          }
        } else if (c != '\r') {  // Si el byte recibido no es un retorno de carro, concatenar a la informacion recibida
          currentLine += c;      
        }
      }
    }
    
    // Limpia la variable que contiene el header
    header = "";

    // Escribe los valores en la EEPROM -> Debe estar mapeada en RAM (con el metodo BEGIN)
    address = 0;
    EEPROM.put(address, redValue);
    address += sizeof(redValue);
    EEPROM.put(address, greenValue);
    address += sizeof(greenValue);
    EEPROM.put(address, blueValue);
    address += sizeof(blueValue);
    EEPROM.commit();  // Escribe en EEPROM la informacion del byte-array de RAM que la emula durante la ejecucion
    
    // Cierra la conexion
    client.stop();
    Serial.println("Cliente desconectado.");
    Serial.println("");
  }
}

// Animacion de LED 1 -> Fade in
void initialFadeIn() {
  int fadeInRed = 0;
  int fadeInGreen = 0;
  int fadeInBlue = 0;
  while (fadeInRed <= redValue || fadeInGreen <= greenValue || fadeInBlue <= blueValue){
    if (fadeInRed <= redValue) {
      analogWrite(redPin, fadeInRed);
      fadeInRed++;
    }
    if (fadeInGreen <= greenValue) {
      analogWrite(greenPin, fadeInGreen);
      fadeInGreen++;
    }
    if (fadeInBlue <= blueValue) {
      analogWrite(bluePin, fadeInBlue);
      fadeInBlue++;
    }
    delay(15);
  }
}

// Conexion a WiFi
void connectToWiFi() {
  Serial.print("Conectando a ");
  Serial.println(ssid);
  if(!WiFi.config(local_IP,gateway,subnet,primaryDNS)){
    Serial.println("Error de configuracion");
  }
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  // Muestra la IP asignada y arranca el servidor
  Serial.println("");
  Serial.println("WiFi conectada.");
  Serial.println("IP asignada: ");
  Serial.println(WiFi.localIP());
  Serial.println("Puerta de enlace: ");
  Serial.println(WiFi.gatewayIP());
  Serial.println("Mascara de subred: ");
  Serial.println(WiFi.subnetMask());
  Serial.println("DNS primario: ");
  Serial.println(WiFi.dnsIP());
  WiFi.setAutoReconnect(true);
  WiFi.persistent(true);
}

// Envia la respuesta HTTP
void sendResponse(WiFiClient client) {
  // Header -> comienzan con un codigo de respuesta (HTTP/1.1 200 OK)
  // Sigue un content-type, para que el cliente que es lo que recibe -> Luego, una linea en blanco
  client.println("HTTP/1.1 200 OK");
  client.println("Content-type:text/html");
  client.println("Connection: close");
  client.println();
                   
  // Content -> Envia el codigo de la pagina web
  client.println("<!DOCTYPE html><html lang=\"es\">");
  client.println("<head><meta charset=\"UTF-8\">");
  client.println("<meta name=\"viewport\" content=\"width=device-width,initial-scale=1.0\">");
  client.println("<title>LED Color Picker</title>");
  client.println("<style>@import url(\"https://fonts.googleapis.com/css?family=Roboto:100\");");
  client.println("body{font-family:roboto;font-size:100%;background:linear-gradient(-45deg, #ee7752, #e73c7e, #23a6d5, #23d5ab);background-size:400% 400%;animation: gradient 15s ease infinite;height:80vh;}");
  client.println("@keyframes gradient{0%{background-position:0% 50%;}50%{background-position:100% 50%;}100%{background-position:0% 50%;}}");
  client.println(".row{text-align:center;color:white;margin-bottom:3%;}");
  client.println(".btn-wrapper{text-align:center;margin-bottom:3%;}");
  client.println(".btn{background:none;border:2px solid #FFFFFF;border-radius:8px;color:white;padding: 16px 32px;text-align:center;font-family:roboto;font-weight:bold;font-size:16px;margin:4px 2px;transition:0.3s;display:inline-block;text-decoration:none;cursor:pointer;}");
  client.println(".btn:hover{background-color:white;color:black;}.rgb-picker-wrapper{display:flex;justify-content:center;align-items:center;}");
  client.println("#rgb-picker{width:80px;height:80px;border:2px solid #FFFFFF;border-radius: 8px;}</style></head>");
  client.println("<body><div class=\"row\"><h1 style=\"font-size:80px;\">LED Color Picker</h1>");
  client.println("<h2 style=\"font-size:24px;\">Selecciona un color para la tira LED</h2></div>");
  client.println("<div class=\"btn-wrapper\"><a class=\"btn\" id=\"change-btn\" href=\"#\" role=\"button\" onclick=\"sendColor()\">Cambiar color</a>");
  client.println("<a class=\"btn\" id=\"poweroff-btn\" href=\"#\" role=\"button\" onclick=\"powerOff()\">Apagar</a></div>");
  client.println("<div class=\"rgb-picker-wrapper\"><input type=\"color\" id=\"rgb-picker\"></div><script>");
  client.println("let colorInput=document.getElementById('rgb-picker');let colorValue=colorInput.value;");
  client.println("colorInput.addEventListener('input',()=>{colorValue=colorInput.value;});");
  client.println("function sendColor(){document.getElementById('change-btn').href=\"?r\"+parseInt(colorValue.substring(1,3),\"16\")+\"g\"+parseInt(colorValue.substring(3,5),\"16\")+\"b\"+parseInt(colorValue.substring(5,7),\"16\")+\"&\";}");
  client.println("function powerOff(){document.getElementById('poweroff-btn').href=\"?r0g0b0&\";}</script></body></html>");
  
  // Linea en blanco para finalizar la respuesta HTTP
  client.println();
}

// Gestiona la peticion
void manageRequest() {
  // Ejemplo de peticion de color fijo: /?r40g210b20&
  // Red = 40 | Green = 210 | Blue = 20 -> Acaba con &
  if(header.indexOf("GET /?r") >= 0) {  // Si la info recibida contiene esta subcadena, tratar como peticion
    pos1 = header.indexOf('r'); // Indices en los que se encuentran estos caracteres
    pos2 = header.indexOf('g');
    pos3 = header.indexOf('b');
    pos4 = header.indexOf('&');
    redString = header.substring(pos1+1, pos2); // Valores de cada canal en el GET -> String
    greenString = header.substring(pos2+1, pos3);
    blueString = header.substring(pos3+1, pos4);
    redValue = redString.toInt(); // Pasar de String a enteros los valores de cada canal para escribirlos en los pines
    greenValue = greenString.toInt();
    blueValue = blueString.toInt();
    /*
    Serial.println(redValue);
    Serial.println(greenValue);
    Serial.println(blueValue);
    */
    analogWrite(redPin, redValue);
    analogWrite(greenPin, greenValue);
    analogWrite(bluePin, blueValue);
  }
  //Nota: Para meter animaciones, idear un nuevo formato de peticion y poner aqui con else if
}