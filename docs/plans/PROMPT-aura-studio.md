# Prompt aparte — `[Aura Studio]` (D-017; no se ejecuta desde este repo)

Se entrega tal cual a una sesión que corra en `Aura-Studio/`. Fuente del
requisito: `CONTRATO-moonlit-studio.md` v1 §C.

```
[Aura Studio] Añadir la familia de firmware "moonlit" (repo ricolinos/moonlit-aura).
Solo lectura de moonlit-aura/CONTRATO-moonlit-studio.md v1. Cambios, todos en Aura-Studio:
- Models/FirmwareFamily.swift:28 → case moonlit; :35-40 configValue "moonlit"; :43-49 displayName "moonlit.aura";
  :56-62 releaseRepository "ricolinos/moonlit-aura"; :66-68 installable += .moonlit; :75-81 bundleSubdirectory "moonlit";
  :88-93 installedTreeSentinel "/.rockbox/fonts/moonlit-body-18.fnt"; :105-110 dormantTreeName ".firmware-moonlit";
  :114-121 parse case "moonlit".
- Views/ExtrasView.swift:75-97 entrada de UI (patrón Metro).
- scripts/fetch-firmware.sh:49-57,157 bloque moonlit. project.yml:72,119-121 recurso de bundle moonlit.
- FIRMWARE_VERSION.example: bloque moonlit.* (mismo formato que metro.*, líneas 13-20).
- Limpieza de convivencia entre familias (la que hoy borra metrocache/, photocache/ y cfcache/ al cambiar
  de familia en un mismo iPod): añadir /.rockbox/aura/moonlitcache/ a esa lista (CONTRATO-moonlit-studio.md §A.4).
No requieren cambio: GitHubReleaseChecker, AuraUpdateChecker, BundledArtifacts (genéricos por familia).
Registrar como ST-NNN. Verificar con un iPod que tenga moonlit instalado (C20).
```

Las citas `archivo:línea` son las del informe de lectura de fase 3
(`docs/plan/02-investigacion.md` §4) sobre `Aura-Studio` en esa fecha;
la sesión que lo ejecute debe reverificarlas.
