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
