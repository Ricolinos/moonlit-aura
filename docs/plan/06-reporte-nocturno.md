# moonlit.aura — Reporte de la corrida nocturna (M10→M12)

**Fecha:** 2026-08-26 · **Estado:** DETENIDO antes de ejecutar ningún hito.

## Verificación de prerrequisitos (M1–M9)

`git log --oneline moonlit-fork-base..HEAD` confirma M1–M9 completos, terminando en:

```
c992cbb6 fix: hub no repite el creciente en la barra de estado (D-044)
ef718d2f feat: splash/about/hub/statusbar/usb use logo; remove provisional wordmark (D-044)
bff717f8 feat: Waning Crescent logo spec + generated masks (D-016)
114b932d feat: pantalla Marea -- Cover Flow vertical, portadas izq/info der (D-029, D-030)
```

Esto sí está en orden. El problema está en el método pedido para M10 en adelante.

## Motivo de la parada

La instrucción dice: *"Cada hito lo ejecuta un subagente nuevo con el prompt de
`docs/plan/prompt-hito.md` (el prompt B) y el número de hito"*.

**`docs/plan/prompt-hito.md` no existe.** Verificado:

```
$ test -f docs/plan/prompt-hito.md && echo existe || echo "NO EXISTE"
NO EXISTE

$ find . -iname "*prompt*hito*" -not -path "*/.git/*"
(sin resultados)

$ git log --all --oneline -- docs/plan/prompt-hito.md
(sin resultados — nunca existió en ningún commit de este repo)
```

No aparece bajo otro nombre en `docs/plans/` (que sí tiene `PROMPT-aura-studio.md` y
`PROMPT-hermanos-gen-fonts.md`, pero ninguno de los dos es un prompt de ejecución de
hitos de este plan — son prompts aparte para Aura-Studio y para los repos hermanos,
ya referenciados por `05-plan-correctivo.md`).

Sin ese archivo no puedo ejecutar el método pedido: no sé qué instrucciones exactas
debe llevar cada subagente de hito más allá de lo que yo mismo podría inferir de
`05-plan-correctivo.md` §II.0 — y las condiciones de parada de esta misma tarea dicen
explícitamente **"no improvises una solución"**. Redactar yo un prompt B a partir de
mi propia lectura del plan sería precisamente eso: una decisión de método tomada en
tu ausencia que no pediste, sobre un archivo que aparentemente ya existía en tu
cabeza (o en otra sesión) y no llegó a este repositorio.

No ejecuté M10. No toqué `apps/metro/` ni ningún otro archivo de código en esta
corrida — el único cambio de esta sesión es este mismo reporte.

## Estado al despertar

- **Hitos hechos:** M1–M9 (sin cambios desde antes de esta corrida).
- **Hito detenido:** M10, antes de empezar — falta `docs/plan/prompt-hito.md`.
- **M12 requiere hardware físico** ("Usuario flashea"; `DEBUGF` en dispositivo real) —
  aunque se hubiera podido llegar hasta ahí, ese hito no es ejecutable por un
  subagente de todos modos y habría necesitado tu intervención igual.
- **Qué necesito de ti:** o (a) el contenido de `docs/plan/prompt-hito.md` (pégalo o
  dime dónde está — quizás quedó en otra sesión o carpeta y no se guardó aquí), o
  (b) confirmar que uso `05-plan-correctivo.md` §II.0 "Reglas del plan" directamente
  como las instrucciones del subagente de cada hito, sin un prompt B aparte.
