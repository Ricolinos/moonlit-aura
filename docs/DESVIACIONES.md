# DESVIACIONES.md — Registro de desviaciones respecto a `PLAN_MAESTRO.md`

Cada entrada: fase · qué decía el plan · qué se encontró en la práctica · qué se hizo. Las decisiones de diseño (`M-NNN`) no se cambian por preferencia — solo se corrigen hechos falsos o se documentan ajustes técnicos necesarios para que el plan funcione.

---

## F0-1 — `tools/configure` no estaba en la lista de 27 archivos modificados por Aura

**Plan decía** (`PLAN_MAESTRO.md` §0.2, citando `INVESTIGACION.md` D.4): que `tools/configure` probablemente era idéntico al upstream porque no aparecía en la lista de 27 archivos de `MODIFICATIONS.md` de Aura-Firmware — quedó marcado `NO RESUELTO` en la investigación.

**Qué se encontró**: al hacer `diff -rq` completo del árbol de Aura-Firmware contra una copia limpia del upstream `0726ec93` (verificación previa a sembrar F0), aparecieron **28** archivos distintos, no 27. Los dos no documentados en `MODIFICATIONS.md` de Aura-Firmware:
- `tools/configure`: sí tiene una modificación real (D-007) — detección del gcc de Homebrew más reciente disponible con fallback 16→15→14→13, en vez de fijar `gcc-16` a secas. Es una laguna real de la documentación de Aura-Firmware (no se toca ese repo desde aquí).
- `apps/plugins/mpegplayer/mpegplayer.h`: agrega la declaración de `aura_osd_colors()` (D-306/D-307) — pertenece al mecanismo de OSD de mpegplayer, ya clasificado en el backlog de `PLAN_MAESTRO.md` (ítem 1), no afecta F0.

**Qué se hizo**: se agregó `tools/configure` a la lista de archivos portados en F0 (ahora 10, no 9) — es exactamente el tipo de fix de build que la Fase Cero busca. `mepgplayer.h` no se porta en F0 (el backlog que lo necesita no es v1); se revisará su diff cuando/si se ejecuta ese ítem del backlog.

**Impacto en `PLAN_MAESTRO.md`**: §0.2 pasa de 9 a 10 archivos portados.

---

## F0-2 — Comentarios de atribución GPL en los archivos portados no se reescriben

**Plan decía** (§0.2, columna "Marca inline"): sustituir `Aura (D-NNN)` por `Metro (from Aura D-NNN)` dentro de cada archivo portado.

**Qué se decidió**: los 10 archivos se portan **byte-idénticos** a Aura-Firmware, sin tocar sus comentarios. Los comentarios narrativos que dicen "Aura (D-293): ..." son, en varios de estos archivos, el propio aviso de modificación GPL v2 §2a (qué se cambió, cuándo, por quién) — reescribirlos para decir "Metro" atribuiría a Metro-Aura una modificación que en realidad hizo Aura-Firmware. La atribución correcta (que este archivo llegó modificado desde Aura-Firmware, con referencia cruzada a sus decisiones D-NNN) queda en `MODIFICATIONS.md` de este repo, que es el mecanismo estándar para documentar el historial de modificaciones sin alterar el código fuente heredado.

**Impacto en `PLAN_MAESTRO.md`**: ninguno en el resultado final (mismo comportamiento); la columna "Marca inline" de §0.2 queda reemplazada por "sin cambios en el archivo; atribución en MODIFICATIONS.md".
