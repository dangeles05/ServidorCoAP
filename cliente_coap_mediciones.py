# cliente_coap_mediciones.py - Pruebas P1 a P4 (todo simulado)
# Uso: python cliente_coap_mediciones.py
# Requiere: pip install aiocoap
import asyncio
import socket
import time
from aiocoap import Context, Message, GET, POST

IP_ESP32 = "192.168.1.50"   # <-- la IP de TU placa
ESPERA = 5                  # segundos máximos por petición
LECTURAS = 5                # P4: cuántas lecturas tomar


async def pedir(contexto, codigo, ruta, payload=b""):
    msg = Message(code=codigo, uri=f"coap://{IP_ESP32}/{ruta}", payload=payload)
    t0 = time.time()
    try:
        r = await asyncio.wait_for(contexto.request(msg).response, ESPERA)
        ms = (time.time() - t0) * 1000
        texto = r.payload.decode(errors="replace")
        print(f"{codigo} /{ruta:<12} -> {r.code}  |  {texto}  ({ms:.0f} ms)")
        return texto
    except asyncio.TimeoutError:
        print(f"{codigo} /{ruta:<12} -> sin respuesta en {ESPERA} s")
    except Exception as e:
        print(f"{codigo} /{ruta:<12} -> error {type(e).__name__}: {e}")
    return None


def get_sin_token(ruta):
    """GET armado a mano SIN token (TKL=0).
    CoAPSimple responde las rutas no registradas sin token; aiocoap
    descarta esas respuestas, así que para P3 usamos UDP directo."""
    ruta_b = ruta.encode()
    # Cabecera: Ver=1, CON, TKL=0 -> 0x40 | GET -> 0x01 | Message ID 0x0404
    # Opción Uri-Path (número 11) con su longitud: 0xB0 + len (len < 13)
    mensaje = bytes([0x40, 0x01, 0x04, 0x04, 0xB0 + len(ruta_b)]) + ruta_b
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    s.settimeout(ESPERA)
    t0 = time.time()
    try:
        s.sendto(mensaje, (IP_ESP32, 5683))
        datos, _ = s.recvfrom(1024)
        ms = (time.time() - t0) * 1000
        c = datos[1]
        print(f"GET /{ruta:<12} -> {c >> 5}.{c & 0x1F:02d}  "
              f"(Message ID {datos[2]:02x}{datos[3]:02x}, {ms:.0f} ms)")
    except socket.timeout:
        print(f"GET /{ruta:<12} -> sin respuesta en {ESPERA} s")
    finally:
        s.close()


def ignorar_cierre(loop, ctx):
    """Silencia un aviso inofensivo de aiocoap en Windows: un datagrama
    que llega justo cuando el cliente ya se está cerrando."""
    exc = ctx.get("exception")
    if isinstance(exc, AttributeError) and "dispatch_message" in str(exc):
        return
    loop.default_exception_handler(ctx)


async def main():
    asyncio.get_running_loop().set_exception_handler(ignorar_cierre)
    contexto = await Context.create_client_context()

    print("=== P1 / P4: lecturas del sensor simulado ===")
    for i in range(LECTURAS):
        await pedir(contexto, GET, "sensor")
        await asyncio.sleep(1)

    print("\n=== P2: POST /led con payload ON ===")
    await pedir(contexto, POST, "led", b"ON")

    print("\n=== P3: ruta inexistente ===")
    get_sin_token("temperatura")                     # esperado: 4.04

    await asyncio.sleep(0.5)   # deja llegar datagramas rezagados antes de cerrar
    await contexto.shutdown()


asyncio.run(main())
