# Changelog

## v0.1.2 — 2026-10-08

- La fase Wi-Fi conserva diez intentos pero amplía su ventana total a unos 30
  segundos para permitir el cambio entre redes guardadas.
- Ajuste motivado por una comprobación física con dos credenciales almacenadas,
  donde la primera asociación superaba el presupuesto anterior.

## v0.1.1 — 2026-10-08

- Verificación inicial dividida en enlace ESP32, Wi-Fi, Internet y sesión.
- Hasta diez intentos automáticos por etapa, con reapertura de UART y progreso
  visible `N/10`.
- Recuperación de una red guardada si el Wi-Fi cae durante la prueba HTTPS.
- Mensajes finales explícitos al agotar los reintentos, sin bloquear la UI.
- Pruebas de regresión para recuperación en el décimo intento y fallo acotado.

## v0.1.0 — 2026-10-08

- Primera publicación independiente de CasioGPT.
- Interfaz de chat nativa para 384×216 y splash screen dedicado.
- Verificación local de API key, ESP32, Wi-Fi e Internet.
- Streaming incremental desde Ollama Cloud mediante el puente `cg50-espmod`.
- Escritura durante la respuesta y cancelación con F6.
- Salida limpia con cancelación de mejor esfuerzo y cierre de UART.
- Cinco capturas reproducibles, benchmark de modelos y detección de secretos.
