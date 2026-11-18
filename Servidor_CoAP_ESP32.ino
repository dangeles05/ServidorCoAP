// ============================================================
// Servidor CoAP en ESP32 - Práctica de laboratorio
// Recursos ofrecidos: /sensor (lectura) y /led (acción)
// Todo SIMULADO: no se conecta ningún componente externo.
// Transporte: UDP, puerto 5683 (puerto oficial de CoAP)
// Librería: CoAPSimple, de hirotakaster
// Placa: ESP32 (NodeMCU / DevKit) en Arduino IDE 2.x
//
// Único cambio respecto al código del repositorio: las
// respuestas devuelven el TOKEN de la petición (ver responder()),
// para que clientes como aiocoap (cliente_coap.py) las acepten.
// ============================================================

#include <WiFi.h>
#include <WiFiUdp.h>
#include <coap-simple.h>

// --- Credenciales de la red ---------------------------------
// OJO: la ESP32 solo trabaja en redes Wi-Fi de 2.4 GHz.
const char* ssid     = "TU_SSID_WIFI";
const char* password = "TU_PASSWORD_WIFI";

// --- Objetos globales ---------------------------------------
WiFiUDP udp;
Coap coap(udp);

// ------------------------------------------------------------
// Responde al cliente con código 2.05 Content.
// Igual que coap.sendResponse(ip, port, messageid, texto) del
// original, pero agregando packet.token y packet.tokenlen.
// RFC 7252 exige que la respuesta lleve el mismo token que la
// petición; sin él, aiocoap descarta la respuesta.
// ------------------------------------------------------------
void responder(CoapPacket &packet, IPAddress ip, int port, const char* texto) {
  coap.sendResponse(ip, port, packet.messageid,
                    texto, strlen(texto),
                    COAP_CONTENT, COAP_TEXT_PLAIN,
                    packet.token, packet.tokenlen);
}

// ============================================================
// CALLBACK DEL RECURSO /led (simulado: no hay LED conectado)
// ============================================================
void callback_led(CoapPacket &packet, IPAddress ip, int port) {
  Serial.println("Petición CoAP recibida en /led");

  // packet.payload no trae '\0' al final: copiamos y lo agregamos.
  char p[packet.payloadlen + 1];
  memcpy(p, packet.payload, packet.payloadlen);
  p[packet.payloadlen] = '\0';

  String payloadStr(p);
  Serial.print("Payload recibido: ");
  Serial.println(payloadStr);

  responder(packet, ip, port, "Estado: OK desde ESP32 CoAP Server");
}

// ============================================================
// CALLBACK DEL RECURSO /sensor (temperatura simulada)
// ============================================================
void callback_sensor(CoapPacket &packet, IPAddress ip, int port) {
  Serial.println("Petición CoAP recibida en /sensor");

  // Valor base 25.4 °C + variación aleatoria de 0.0 a 4.9 °C.
  float temperaturaSimulada = 25.4 + (random(0, 50) / 10.0);

  String respuesta = "Temp: " + String(temperaturaSimulada) + " C";
  Serial.println(respuesta);

  responder(packet, ip, port, respuesta.c_str());
}

// ============================================================
// setup(): se ejecuta UNA sola vez al encender o resetear
// ============================================================
void setup() {
  Serial.begin(115200);
  delay(1000);

  WiFi.begin(ssid, password);
  Serial.print("Conectando a Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nConectado a la red");
  Serial.print("Dirección IP del Servidor CoAP: ");
  Serial.println(WiFi.localIP());

  // Tabla de rutas: sin este registro, la ruta da 4.04 Not Found.
  coap.server(callback_led, "led");
  coap.server(callback_sensor, "sensor");

  coap.start();   // abre el puerto UDP 5683
}

// ============================================================
// loop(): solo escuchar. NADA de delays largos aquí.
// ============================================================
void loop() {
  coap.loop();
}
