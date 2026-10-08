# Compilar, probar e instalar

## Dependencias

- PrizmSDK para fx-CG50 en `/opt/prizmsdk-linux` o `$FXCGSDK`.
- GCC compatible con C11 para las pruebas de host.
- Python 3 y Pillow para convertir los renders de prueba.
- El comando global `casio` es opcional y solo se necesita para instalación
  verificada sobre una calculadora conectada.

## Flujo reproducible

```bash
make test
make addin
make checksums
```

`make test` compila la UI con AddressSanitizer y UndefinedBehaviorSanitizer,
genera cinco pantallas de 384×216 y revisa que no se haya incluido ninguna
credencial. `make addin` usa PrizmSDK y deja el resultado en
`dist/CASIOGPT.g3a`.

Los scripts detectan UID 0. Cuando se ejecutan como root, la compilación se
reanuda como el usuario `kali` para producir los mismos artefactos y evitar
archivos de build propiedad de root. Los comandos funcionan bajo ambas
identidades.

## Instalación verificada

Con la fx-CG50 en modo USB Flash:

```bash
make install
```

El instalador recompila el add-in, guarda una copia del `CASIOGPT.g3a` anterior,
escribe mediante un archivo temporal, sincroniza, compara SHA-256 y desmonta la
unidad. Si existe `casiogpt_api.txt`, también lo instala sin mostrar su contenido.
Puede indicarse otra ruta con `CASIOGPT_API_FILE=/ruta/privada/clave.txt`.

El firmware ESP32 no se compila ni se flashea desde este repositorio: usa la
versión compatible publicada por
[`cg50-espmod`](https://github.com/samilososami/cg50-espmod).
