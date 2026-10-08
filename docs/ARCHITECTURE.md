# Arquitectura y límites de confianza

## Responsabilidades

La fx-CG50 ejecuta `CASIOGPT.g3a` y conserva la interacción visible: teclado,
campo de texto, conversación, desplazamiento y estados de envío. La XIAO
ESP32-C3 ejecuta el firmware común de
[`cg50-espmod`](https://github.com/samilososami/cg50-espmod) y se encarga de:

- conexión Wi-Fi y redes guardadas;
- TLS y petición HTTPS a Ollama Cloud;
- historial breve de conversación;
- decodificación incremental del NDJSON;
- entrega ordenada de fragmentos a la calculadora.

## Transporte

El enlace físico es UART 9600 8N1. Cada frame contiene un payload ASCII, un
checksum FNV-1a de 32 bits y un identificador monotónico de petición. Las cargas
grandes —clave y prompt— se envían por bloques. La respuesta se recupera por
offset para que un reintento sea idempotente.

El protocolo CasioGPT usa las operaciones `GPT_NEW`, `GPT_BEGIN`, `GPT_KEY`,
`GPT_PROMPT`, `GPT_RUN`, `GPT_GET` y `GPT_CANCEL`. La especificación completa y
el firmware autoritativo están en el repositorio base; `include/wire.h` mantiene
únicamente la capa de framing que necesita el add-in para compilar.

## Credenciales

`casiogpt_api.txt` se lee desde la raíz de la calculadora. No se incluye en Git,
en el `.g3a` ni en los artefactos de release. Durante una consulta viaja por
UART a la RAM de la ESP32 y se limpia al terminar o cancelar. Esto evita una
credencial embebida, pero no convierte el enlace físico en un canal cifrado:
quien tenga acceso al hardware y pueda observar la UART podría leerla.

Las contraseñas Wi-Fi son responsabilidad de `cg50-espmod` y se guardan en NVS
para permitir reconexión. En la configuración actual esa NVS no está cifrada.

## Fallos y recuperación

- La UI nunca espera bloqueada a una única lectura UART.
- La comprobación inicial separa enlace ESP32, asociación Wi-Fi, salida HTTPS e
  inicio de sesión; cada fase transitoria dispone de hasta diez intentos.
- Cada intento de enlace reabre la UART local y cada frame tiene retransmisión
  interna acotada, sin congelar el bucle gráfico.
- La pantalla muestra el intento actual (`N/10`) para distinguir una
  recuperación en curso de un bloqueo.
- Si HTTPS detecta que el Wi-Fi cayó, CasioGPT vuelve una vez a la fase de
  asociación antes de declarar el fallo definitivo.
- Offsets inesperados y frames dañados se rechazan.
- F6 envía una cancelación y el cierre del add-in realiza una cancelación de
  mejor esfuerzo antes de cerrar UART.
- La detección local de secretos falla el test si encuentra una API key en
  fuentes, documentación o binarios.

La batería de host fuerza nueve fallos y recuperación en el décimo intento,
además del caso de agotamiento completo. Esto valida el estado y el protocolo;
la estabilidad eléctrica del enlace soldado solo puede confirmarse en hardware.
