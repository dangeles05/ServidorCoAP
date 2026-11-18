# 📡 Servidor CoAP en ESP32 con Arduino IDE

Práctica de laboratorio de la materia **Internet de las Cosas**
**Instituto Tecnológico Superior del Occidente del Estado de Hidalgo (ITSOEH)**
Ingeniería en Tecnologías de la Información y Comunicaciones
Docente: Mtro. Saúl Isaí Soto Ortiz

**Alumno(s):** 
Dilan Emir Angeles Martinez 230110252
Abraham Calva 230110637
Gustavo López Paz 230110531


**Fecha:** Noviembre 2026

---

## 📖 Descripción

En esta práctica la tarjeta **ESP32** funciona como un **servidor CoAP** (Constrained Application Protocol, RFC 7252) conectado por Wi-Fi. Desde una computadora, un cliente escrito en Python envía peticiones al servidor y muestra sus respuestas.

CoAP sigue el modelo REST de la web (recursos con URI y métodos `GET`, `POST`, `PUT`, `DELETE`), pero funciona sobre **UDP** en lugar de TCP. Por eso no necesita abrir una conexión, usa una cabecera binaria de solo 4 bytes y consume menos energía, lo que lo hace ideal para dispositivos IoT.

El servidor ofrece dos recursos, ambos **simulados** (no se conecta ningún componente externo):

| Recurso | Método | Qué hace | Respuesta |
| :--- | :--- | :--- | :--- |
| `/sensor` | `GET` | Genera una temperatura simulada entre 25.4 y 30.3 °C | `Temp: 27.10 C` |
| `/led` | `POST` | Recibe un payload (`ON` / `OFF`) y lo muestra en el Monitor Serie | `Estado: OK desde ESP32 CoAP Server` |

---

## 🎯 Objetivo

Comprender el funcionamiento del protocolo CoAP en arquitecturas IoT, configurar la ESP32 como servidor CoAP con Arduino IDE y validar la comunicación mediante peticiones desde un cliente externo.

---

## 📁 Archivos del repositorio

| Archivo | Descripción |
| :--- | :--- |
| `Servidor_CoAP_ESP32.ino` | Código del servidor CoAP para la ESP32, comentado. |
| `cliente_coap_mediciones.py` | Cliente en Python que ejecuta las pruebas P1 a P4 en una sola corrida. |
| `README.md` | Este documento. |

---

## 🛠 Materiales

### Hardware
- 1 tarjeta ESP32 (NodeMCU / DevKit).
- 1 cable USB **de datos** (los cables solo de carga no funcionan).
- 1 computadora.
- Red Wi-Fi de **2.4 GHz** (la ESP32 no se conecta a 5 GHz).

### Software
- Arduino IDE 2.x con el soporte para tarjetas **esp32** (Espressif Systems).
- Librería **CoAPSimple** de *hirotakaster*.
- Python 3 con la librería `aiocoap`.
- Wireshark (opcional, para analizar los paquetes).

---

## 🔌 Funcionamiento

```
 COMPUTADORA (cliente)          ROUTER Wi-Fi             ESP32 (servidor CoAP)
 cliente_coap_mediciones.py     Red 2.4 GHz              Puerto UDP 5683
                                                      
 GET  coap://IP/sensor   ───►   datagramas UDP   ───►   callback_sensor()
 POST coap://IP/led      ───►                    ───►   callback_led()
                         ◄───                    ◄───   respuesta (2.05 / 4.04)
```

1. La ESP32 se conecta al Wi-Fi y recibe una dirección IP.
2. `coap.server()` registra los recursos `/sensor` y `/led`, cada uno con su función *callback*.
3. `coap.start()` abre el puerto UDP **5683** y `coap.loop()` queda escuchando.
4. Cuando llega una petición, la librería ejecuta el *callback* de esa ruta.
5. El *callback* arma la respuesta y la envía al cliente con el mismo **Message ID** y **Token** de la petición.

---

## ⚙️ Instalación y uso

### 1. Preparar Arduino IDE
1. *Herramientas → Placa → Administrador de Tarjetas* → buscar **esp32** e instalar.
2. *Herramientas → Administrar Bibliotecas* → buscar **CoAPSimple** (hirotakaster) e instalar.
3. Seleccionar la placa **ESP32 Dev Module** y el puerto correcto.

### 2. Cargar el servidor
1. Abrir `Servidor_CoAP_ESP32.ino`.
2. Cambiar las credenciales de la red:
   ```cpp
   const char* ssid     = "TU_SSID_WIFI";
   const char* password = "TU_PASSWORD_WIFI";
   ```
3. Cargar el código y abrir el **Monitor Serie a 115200 baudios**.
4. Presionar **RST** y anotar la IP que aparece:
   ```
   Conectado a la red
   Dirección IP del Servidor CoAP: 192.168.1.50
   ```

### 3. Ejecutar el cliente
La computadora debe estar en la **misma red Wi-Fi** que la ESP32.

```bash
pip install aiocoap
```

Cambiar la IP en `cliente_coap_mediciones.py`:
```python
IP_ESP32 = "192.168.1.50"   # <-- la IP de TU placa
```

Ejecutar:
```bash
python cliente_coap_mediciones.py
```

---

## ✅ Pruebas y resultados

| # | Petición | Resultado esperado | Resultado obtenido |
| :--- | :--- | :--- | :--- |
| **P1** | `GET coap://<IP>/sensor` | 2.05 Content con `Temp: XX.X C` | ✅ 2.05 Content |
| **P2** | `POST coap://<IP>/led` con `ON` | `Estado: OK...` y `Payload recibido: ON` en el Monitor Serie | ✅ 2.05 Content |
| **P3** | `GET coap://<IP>/temperatura` | 4.04 Not Found | ✅ 4.04 |
| **P4** | Repetir P1 cinco veces | Temperaturas distintas entre 25.4 y 30.3 °C | ✅ Ver tabla |

### Salida del cliente

```
=== P1 / P4: lecturas del sensor simulado ===
GET /sensor       -> 2.05 Content  |  Temp: 30.30 C  (112 ms)
GET /sensor       -> 2.05 Content  |  Temp: 30.00 C  (14 ms)
GET /sensor       -> 2.05 Content  |  Temp: 25.70 C  (20 ms)
GET /sensor       -> 2.05 Content  |  Temp: 29.60 C  (123 ms)
GET /sensor       -> 2.05 Content  |  Temp: 26.00 C  (151 ms)

=== P2: POST /led con payload ON ===
POST /led         -> 2.05 Content  |  Estado: OK desde ESP32 CoAP Server  (186 ms)

=== P3: ruta inexistente ===
GET /temperatura  -> 4.04  (Message ID 0404, 10 ms)
```

### P4: lecturas de temperatura

| Lectura | Temperatura (°C) | Tiempo de respuesta |
| :---: | :---: | :---: |
| 1 | 30.30 | 112 ms |
| 2 | 30.00 | 14 ms |
| 3 | 25.70 | 20 ms |
| 4 | 29.60 | 123 ms |
| 5 | 26.00 | 151 ms |

### P3: interpretación del 4.04
La ruta `/temperatura` no fue registrada con `coap.server()` en `setup()`, por eso el servidor responde **4.04 Not Found**. No es un error del código: es el servidor indicando que ese recurso no existe.

### Evidencias

<!-- Sube tus capturas a una carpeta img/ y descomenta las líneas -->
<!-- ![Monitor Serie con la IP](img/monitor_serie_ip.png) -->
<!-- ![Monitor Serie con el payload ON](img/monitor_serie_led.png) -->
<!-- ![Salida del cliente](img/cliente_pruebas.png) -->
<!-- ![Captura de Wireshark](img/wireshark.png) -->

---

## 🔍 Problema encontrado: el Token

Al principio, el cliente `aiocoap` **no mostraba ninguna respuesta**, aunque la ESP32 sí contestaba.

**Causa:** el código original respondía con `coap.sendResponse(ip, port, messageid, texto)`, que **no incluye el Token** de la petición. El RFC 7252 exige que la respuesta lleve el mismo Token, y `aiocoap` descarta las respuestas que no lo traen.

**Solución:** se creó la función `responder()`, que envía la respuesta con el Token:

```cpp
coap.sendResponse(ip, port, packet.messageid,
                  texto, strlen(texto),
                  COAP_CONTENT, COAP_TEXT_PLAIN,
                  packet.token, packet.tokenlen);
```

En la prueba P3, el 4.04 lo genera la propia librería CoAPSimple, también sin Token. Por eso el cliente envía esa petición por UDP directo y **sin Token**, para que la respuesta coincida.

---

## ❓ Preguntas de análisis

**1. ¿Por qué CoAP puede permitirse usar UDP, si UDP no garantiza la entrega? ¿Qué mecanismo compensa esa falta y en qué casos ni siquiera se usa?**
CoAP compensa UDP con mensajes CON: exige ACK y reintenta con el mismo Message ID; los NON no usan confirmación.

**2. En tu captura de Wireshark, ¿la respuesta de `/sensor` fue un ACK independiente o viajó "a cuestas" en el mismo mensaje que el payload? ¿Cómo lo sabes?**
Viajó a cuestas: Wireshark muestra un solo mensaje ACK con código 2.05, el payload y el mismo Message ID.

**3. ¿Qué campo te permitió saber que la respuesta correspondía a tu petición y no a la de un compañero?**
El Token empareja respuesta y petición; sin él, aiocoap descartaba las respuestas. El Message ID empareja ACK y CON.

**4. Una petición HTTP/1.1 mínima ocupa varios cientos de bytes con sus cabeceras. Calcula cuántos bytes ocupa tu mensaje CoAP de respuesta en la captura y compara.**
La respuesta CoAP ocupa 21 bytes más el token; HTTP necesita cientos de bytes y varios paquetes TCP extra.

**5. Si este servidor controlara una bomba de riego con batería, ¿usarías mensajes CON o NON para reportar la humedad cada minuto? Justifica el costo en energía de tu elección.**
NON para la humedad: ahorra batería y perder una lectura no importa; CON para las órdenes a la bomba.

**6. ¿Qué riesgo de seguridad implica un servidor CoAP sin DTLS dentro de una red compartida, y quién podría explotarlo en el laboratorio?**
Sin DTLS nada se cifra ni autentica: cualquier compañero en la red puede espiar el tráfico o mandar órdenes.

---

## 🛠️ Solución de problemas

| Síntoma | Causa probable | Solución |
| :--- | :--- | :--- |
| La ESP32 no aparece en el puerto | Cable solo de carga | Usar un cable USB de datos |
| Puntos infinitos en el Monitor Serie | SSID/contraseña incorrectos o red de 5 GHz | Revisar credenciales y usar red de 2.4 GHz |
| Caracteres raros en el Monitor Serie | Velocidad incorrecta | Configurar a 115200 baudios |
| El cliente no recibe respuesta | Redes distintas, aislamiento de clientes o firewall | Misma red (o hotspot del celular) y permitir Python en el firewall |
| Respuesta 4.04 inesperada | Ruta mal escrita | Las rutas distinguen mayúsculas: `/sensor`, no `/Sensor` |

---

## 📝 Conclusiones

- CoAP permite que un dispositivo IoT como la ESP32 ofrezca recursos web consumiendo muy pocos recursos: cada intercambio son solo dos datagramas UDP, sin conexión.
- La librería CoAPSimple facilita crear rutas: registrar un recurso es asociar un nombre con una función *callback*.
- El Token y el Message ID son esenciales para emparejar peticiones y respuestas; sin el Token, el cliente descarta las respuestas aunque lleguen.
- La fiabilidad en CoAP se decide por mensaje (CON o NON), según la importancia del dato.
- Sin DTLS no hay cifrado ni autenticación, lo que representa un riesgo en redes compartidas.

---

## 📚 Referencias

- Shelby, Z., Hartke, K. y Bormann, C. (2014). *The Constrained Application Protocol (CoAP)*, RFC 7252, IETF.
- Librería CoAPSimple para Arduino, de hirotakaster.
- Librería aiocoap para Python.
- Práctica original: [Práctica de Laboratorio: Servidor CoAP en ESP32 con Arduino IDE](https://github.com/ssoto123/Practica-de-Laboratorio-Servidor-CoAP-en-ESP32-con-Arduino-IDE), Mtro. Saúl Isaí Soto Ortiz.
