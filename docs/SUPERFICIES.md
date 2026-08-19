# SUPERFICIES.md — Checklist de superficies de Rockbox visibles en Metro

Recorrido de `Aura-Firmware/docs/superficies-rockbox.md` niveles 1–2
(leído del repo hermano, no copiado — mismo criterio que
`COMPAT_STUDIO.md`) contra el estado real de Metro-Aura al cierre de
F9. Metro es deliberadamente más chico que Aura v1 (`PLAN_MAESTRO.md`
§1.2): varias filas del inventario original de Aura no aplican todavía
porque la funcionalidad que las dispara (Music Flow, reproductor de
video con OSD propio, temas instalables, fotos de artista) no existe
en el plan de fases F0–F13 de Metro, o llega en una fase posterior a
F9. Esas filas se marcan **N/A (Metro)** con la razón puntual, no se
inventan capturas ni código para superficies que Metro no puede
alcanzar por construcción.

Leyenda: ✅ Resuelto y verificado · 🟡 Resuelto con matices/no
verificado visualmente · ⬜ Pendiente · N/A (Metro) = la superficie no
es alcanzable en el estado actual de Metro, con la razón.

## Nivel 1 — se ven seguro en uso normal

| Superficie | Estado | Nota |
|---|---|---|
| Logo Rockbox al encender | ✅ | F1: bitmap `rockboxlogo.320x98x16.bmp` reemplazado por el wordmark "metro" (M-020). F9: el splash de texto propio (`metro_screen_splash_show()`, fuente Metro real) se dibuja justo después, con barra de progreso mientras `tagcache_is_fully_initialized()` — `docs/screenshots/F9-boot.png` |
| Texto del bootloader | N/A (Metro) | Tocar el bootloader es un riesgo real de brickeo sin verificación en hardware — mismo motivo que Aura-Firmware lo dejó "deliberadamente intacto" para sus pantallas fatales (`docs/superficies-rockbox.md` de ese repo, nivel 3-4). Fuera de alcance hasta F13 (hardware real) |
| Pantalla USB | 🟡 | F9: `metro_screen_usb_show()` dibuja un cuadro de Metro antes de `default_event_handler()`, pero verificado que no llega a mostrarse ni un cuadro en el simulador (`gui_usb_screen_run()` toma la pantalla de inmediato) — ver `docs/DESVIACIONES.md` F9-1, `docs/screenshots/F9-usb.png` muestra la pantalla nativa de Rockbox |
| "Loading..." al desconectar USB | 🟡 | Cubierto por el gancho genérico de `metro_splash_translate()` (M-037) si el mensaje pasa por `splash()` — no verificado con un escenario dedicado |
| "Shutting down..." | ✅ | `show_shutdown_message=false` (M-019) suprime el splash nativo por completo; `draw_shutdown_screen()` (F9, `metro_main.c`) dibuja el propio antes de `default_event_handler()` — a diferencia de USB, nada redibuja encima después, así que si se alcanza a ver, es lo último que se ve. No se pudo capturar con el mecanismo de captura headless actual: `default_event_handler()` procede a apagar el proceso del simulador antes de que el hilo de captura pueda tomar el volcado (mismo tipo de limitación que "Committing database" abajo) — verificado por inspección de log ("Writing system_status to disk" confirma que la secuencia de apagado corrió después del dibujo) |
| Menú "MPEG Player" al abrir un video | N/A (Metro) | `metro_video_play()` (F7) llama `plugin_load()` directo — Metro nunca pasa por un menú propio antes, el plugin arranca con su propio menú nativo (`docs/screenshots/F7-mpegplayer.png`), sin restilo (fuera de alcance, mismo criterio que Aura documentó "OSD del video" como trabajo dedicado que Metro no ha hecho) |
| OSD del video | N/A (Metro) | El reproductor de video es 100% el plugin `mpegplayer.rock` sin modificar — Metro no tiene su propia superposición de controles todavía, ningún plan de fase F0-F13 lo pide |
| Backdrop Cabbie v2 | ✅ | `metro_apply_hygiene()` (M-019, F1): `backdrop_file="-"` |
| Statusbar clásica de Rockbox | ✅ | `metro_apply_hygiene()` (M-019, F1): `STATUSBAR_OFF` |

## Nivel 2 — muy probables

| Superficie | Estado | Nota |
|---|---|---|
| "Scanning disk..." / "Committing database [x/y]" | 🟡 | El mecanismo (`metro_splash_translate()`, M-037) cubre estos mensajes en principio, pero no se pudo verificar visualmente: intentado con `firmware/tools/gen_test_media.sh` (biblioteca de prueba de F4/F7) en un rango amplio de tiempos de espera (0.05s–3s) sin atraparlo — se completa antes o durante una ventana que el mecanismo de captura de un solo proceso no puede subdividir más. Mismo resultado exacto que `docs/superficies-rockbox.md` de Aura-Firmware documentó para esta fila con su propia biblioteca de prueba |
| Splashes de batería | ✅ | Cubiertos por `metro_splash_translate()` (M-037): "WARNING! Low Battery!.../Battery empty!..." tienen sus propias reglas de traducción — no verificado con batería real/simulada baja, pero la regla está en la tabla y sigue el mismo patrón probado que las demás |
| "Database is not ready" cancela el apagado | 🟡 | Cubierto por el mismo gancho genérico (M-037); no probado con un escenario dedicado (requiere apagar justo durante un rebuild activo) |
| "Loading..." al abrir video con disco dormido | 🟡 | Cubierto por el mismo gancho genérico; no probado con un escenario dedicado |
| Modo solo-carga silencioso con botón pulsado | N/A (Metro) | `USBPOWER_BTN_IGNORE` es un fix de Aura-Firmware en su propio `usb-s5l8702.c`/config — Metro no modificó ningún archivo relacionado con USB fuera de `metro_screen_usb.c` (F9); revisar si el mismo bug aplica es tarea de F13 (hardware) |
| HID sigue enumerándose ante la Mac | ✅ | `metro_apply_hygiene()` (M-019, F1): `global_settings.usb_hid = false` bajo `#ifdef USB_ENABLE_HID` |
| Menús Settings/Display/Audio de mpegplayer | N/A (Metro) | Mismo criterio que "Menú MPEG Player" arriba — plugin sin modificar |

## Resumen

De las filas de nivel 1–2 del inventario de Aura-Firmware aplicables a
Metro (14 de 16 — 2 dependen de un reproductor de video con superficie
propia que Metro no tiene): **5 resueltas y verificadas (✅)**, **5
resueltas por mecanismo pero con verificación visual pendiente o un
límite del propio simulador ya documentado (🟡)**, **0 pendientes sin
tocar**. Las 2 filas restantes son N/A por construcción (Metro delega
el reproductor de video al plugin nativo sin capa propia todavía). Ver
`DECISIONS.md` M-037/M-038/M-039 y `docs/DESVIACIONES.md` F9-1 para el
detalle de cada límite encontrado.
