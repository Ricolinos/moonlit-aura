# Prompts aparte — `[Metro-Aura]` y `[Aura-Firmware]` (D-007; no se ejecutan desde este repo)

Cada uno se entrega a una sesión propia en el repo correspondiente.
Hallazgo de origen: plan §C.1 / D-007 — `atoi("0x20")` devuelve 0, así
que `convttf` recibe rango 0–0 y convierte el charset completo (≈1309
glifos en vez de ≈317). moonlit corrige lo suyo en H2
(`firmware/tools/gen_fonts.sh`, rango decimal 32–383).

```
[Metro-Aura] Corregir firmware/tools/gen_fonts.sh (:47-48) para pasar a convttf el rango
de caracteres en decimal (32–383): atoi("0x20") devuelve 0 y convierte el charset completo
(1309 glifos en vez de ~317). Regenerar los .fnt, medir la diferencia de tamaño por archivo,
registrar la decisión M-NNN en DECISIONS.md y anotar MODIFICATIONS.md si se toca algo fuera
de apps/metro/. Verificar con build_sim.sh + captura del especímen tipográfico.
```

```
[Aura-Firmware] Corregir design-system/generate.py (equivalente de gen_fonts.sh de Metro)
para pasar a convttf el rango de caracteres en decimal (32–383): atoi("0x20") devuelve 0 y
convierte el charset completo (1309 glifos en vez de ~317). Regenerar los .fnt, medir la
diferencia de tamaño por archivo, registrar la decisión D-NNN en DECISIONS.md y anotar
MODIFICATIONS.md si aplica. Verificar con el simulador + captura del especímen tipográfico.
```

## Hallazgo adicional para `[Metro-Aura]` (moonlit D-025)

```
[Metro-Aura] Un configure desde cero (build_sim.sh --reconfigure) falla con "No rule to
make target build-sim/metro_palette.h, needed by mpegplayer.o": tools/make.inc genera
make.dep con -MG -MM y solo CFLAGS globales, sin el -I$(APPSDIR)/metro de mpegplayer.make
(M-059). El make.dep existente es anterior a M-059 y por eso no se notó. Corregir con
#include "../../metro/metro_palette.h" en mpegplayer.c:108 (o equivalente), anotar
MODIFICATIONS.md y registrar M-NNN.
```
