#!/usr/bin/env bash
# Genera archivos de audio sinteticos cortos en cada formato nativo que
# Metro debe reproducir (FLAC, MP3, AAC/m4a, ALAC, WAV, AIFF), con tags y
# una letra .lrc de muestra, para probar el reproductor en el simulador
# sin depender de musica real del usuario.
#
# Salida: firmware/test-media/ (no se versiona -- generado on-demand).
#
# Uso: firmware/tools/gen_test_media.sh

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
OUT_DIR="$ROOT_DIR/firmware/test-media"

mkdir -p "$OUT_DIR"

TITLE="Metro Test Tone"
ARTIST="Metro QA"
ALBUM="Fase 4 Fixtures"
DURATION=3
FREQ=440

gen() {
  local ext="$1"; shift
  local out="$OUT_DIR/metro-test.$ext"
  echo "==> Generando $out"
  ffmpeg -y -loglevel error \
    -f lavfi -i "sine=frequency=${FREQ}:duration=${DURATION}" \
    -metadata title="$TITLE" -metadata artist="$ARTIST" -metadata album="$ALBUM" \
    "$@" "$out"
}

gen flac -c:a flac
gen mp3  -c:a libmp3lame -b:a 128k
gen m4a  -c:a aac -b:a 128k
gen alac.m4a -c:a alac
gen wav  -c:a pcm_s16le
gen aiff -c:a pcm_s16be

# F5: solid-color cover art, via ffmpeg PNG -> sips JPEG, not ffmpeg's
# own mjpeg encoder straight to .jpg. -pix_fmt yuvj420p alone (D-030,
# DECISIONS.md) fixes the *chroma sampling factor* mismatch that used
# to corrupt these images outright; it does NOT fix a second, subtler
# ffmpeg-mjpeg-specific issue found verifying F5's Now Playing screen:
# Rockbox's JPEG decoder reads such a file "successfully" (ret>0,
# right dimensions) but drops chroma silently, rendering the album art
# tile as flat gray (RGB565 ~= the source color's luma) instead of the
# real color -- confirmed by decoding the exact same pixels through
# both PIL and sips instead of ffmpeg's mjpeg encoder, both correct.
# ffmpeg still generates the flat-color PNG (no chroma subsampling
# concern for a lossless format); sips does the actual JPEG encode.
gen_cover_jpg() {
  local color="$1" size="$2" out="$3"
  local tmp_dir tmp_png
  tmp_dir="$(mktemp -d)"
  tmp_png="$tmp_dir/cover.png"
  ffmpeg -y -loglevel error -f lavfi -i "color=c=${color}:s=${size}" \
    -frames:v 1 "$tmp_png"
  sips -s format jpeg "$tmp_png" --out "$out" > /dev/null
  rm -rf "$tmp_dir"
}

cat > "$OUT_DIR/metro-test.lrc" <<'EOF'
[ar:Metro QA]
[ti:Metro Test Tone]
[al:Fase 4 Fixtures]
[00:00.00]Instrumental
[00:01.00]Segundo uno
[00:02.00]Segundo dos
EOF

echo "==> Generando fixture SIN caratula (test-media/SinArte)"
# Carpeta propia sin cover.jpg: find_albumart no encuentra arte ni
# embebido ni de carpeta -> ejercita la imagen "Default" (nota gris
# sobre tile gris, D-112) en Cover Flow, el reproductor y Flip-and-Flow.
mkdir -p "$OUT_DIR/SinArte"
ffmpeg -y -loglevel error \
  -f lavfi -i "sine=frequency=330:duration=${DURATION}" \
  -metadata title="Pista sin arte" -metadata artist="Metro QA" \
  -metadata album="Album sin portada" \
  -c:a libmp3lame -b:a 128k "$OUT_DIR/SinArte/metro-test-noart.mp3"

echo "==> Generando $OUT_DIR/cover.jpg"
gen_cover_jpg "0x3366CC" "200x200" "$OUT_DIR/cover.jpg"

echo "==> Generando fixtures de Fotos (test-media/Photos)"
mkdir -p "$OUT_DIR/Photos"
ffmpeg -y -loglevel error -f lavfi -i "testsrc=size=320x240:rate=1" \
  -pix_fmt yuvj420p -frames:v 1 "$OUT_DIR/Photos/photo1.jpg"
ffmpeg -y -loglevel error -f lavfi -i "color=c=0x2244AA:s=320x240" \
  -pix_fmt yuvj420p -frames:v 1 "$OUT_DIR/Photos/photo2.jpg"
ffmpeg -y -loglevel error -f lavfi -i "smptebars=size=160x120:rate=1" \
  -frames:v 1 "$OUT_DIR/Photos/photo3.bmp"
ffmpeg -y -loglevel error -f lavfi -i "color=c=0xAA6622:s=100x100" \
  -frames:v 1 "$OUT_DIR/Photos/photo4_unsupported.png"

echo "==> Generando fixture de Video (test-media/Videos)"
mkdir -p "$OUT_DIR/Videos"
ffmpeg -y -loglevel error -f lavfi -i "testsrc=size=320x240:rate=15:duration=2" \
  -f lavfi -i "sine=frequency=440:duration=2" \
  -c:v mpeg2video -q:v 5 -c:a mp2 "$OUT_DIR/Videos/test.mpg"

# F4: biblioteca musical de prueba para metro_music.c (proveedores sobre
# tagcache_search*) -- 2 artistas, 3 albumes, 12 pistas, 3 generos en el
# layout Artista/Album/archivo (mas dos pistas sueltas mas abajo para
# los otros dos layouts del contrato, ver el comentario junto a ellas).
# Cada pista lleva ALBUMARTIST igual a ARTIST (ejercita la ruta
# principal de metro_music_album_artist_label(): preferir
# tag_albumartist, con tag_artist como respaldo si una pista no lo
# trae -- ningun fixture aqui ejercita ese respaldo, ver el comentario
# [ESTIMADO] junto a esa funcion en metro_music.c). TRACKNUMBER fija el
# orden de disco de cada album (metro_music_songs_of_album() ordena por
# el, no alfabetico).
echo "==> Generando biblioteca musical de prueba (test-media/Music)"
MUSIC_DIR="$OUT_DIR/Music"
rm -rf "$MUSIC_DIR"

gen_track() {
  local artist="$1" album="$2" genre="$3" track="$4" title="$5" freq="$6"
  local dir="$MUSIC_DIR/$artist/$album"
  local out
  mkdir -p "$dir"
  out="$dir/$(printf '%02d' "$track") $title.mp3"
  ffmpeg -y -loglevel error \
    -f lavfi -i "sine=frequency=${freq}:duration=2" \
    -metadata title="$title" -metadata artist="$artist" \
    -metadata album_artist="$artist" -metadata album="$album" \
    -metadata genre="$genre" -metadata track="$track" \
    -c:a libmp3lame -b:a 96k "$out"
}

gen_track "Aura Test Combo" "First Light" "Electronic" 1 "Sunrise" 220
gen_track "Aura Test Combo" "First Light" "Electronic" 2 "Horizon" 247
gen_track "Aura Test Combo" "First Light" "Electronic" 3 "Glow" 262
gen_track "Aura Test Combo" "First Light" "Electronic" 4 "Daybreak" 294

# F5: cover.jpg de carpeta para el album que SI debe mostrar caratula
# real en Now Playing (find_albumart()); "Night Drive" se deja sin
# arte a proposito -- ejercita metro_draw_tile() (F5-nowplaying-noart.png).
gen_cover_jpg "0xCC7733" "300x300" "$MUSIC_DIR/Aura Test Combo/First Light/cover.jpg"

gen_track "Aura Test Combo" "Night Drive" "Synthwave" 1 "Overpass" 330
gen_track "Aura Test Combo" "Night Drive" "Synthwave" 2 "Neon Mile" 349
gen_track "Aura Test Combo" "Night Drive" "Synthwave" 3 "Rearview" 392
gen_track "Aura Test Combo" "Night Drive" "Synthwave" 4 "Static Coast" 440

gen_track "Wheel & Click" "Analog Dreams" "Ambient" 1 "Slow Turn" 165
gen_track "Wheel & Click" "Analog Dreams" "Ambient" 2 "Ferrite" 175
gen_track "Wheel & Click" "Analog Dreams" "Ambient" 3 "Idle Hum" 185
gen_track "Wheel & Click" "Analog Dreams" "Ambient" 4 "Wind Down" 196

# COMPAT_STUDIO.md C8: tagcache escanea todo el disco sin importar la
# profundidad de carpetas (docs/contracts/library-layout-v1.md SS "/Music/",
# Aura-Firmware, "no depende del layout") -- estas dos pistas ejercitan
# los otros dos layouts que Studio puede elegir ademas del por defecto
# (Artista/Album/archivo, ya cubierto arriba): Album/archivo (sin
# carpeta de artista) y Artista/archivo (sin carpeta de album).
mkdir -p "$MUSIC_DIR/Flat Album Test"
ffmpeg -y -loglevel error -f lavfi -i "sine=frequency=210:duration=2" \
  -metadata title="Flat Song" -metadata artist="Flat Artist" \
  -metadata album_artist="Flat Artist" -metadata album="Flat Album Test" \
  -metadata genre="Electronic" -metadata track=1 \
  -c:a libmp3lame -b:a 96k "$MUSIC_DIR/Flat Album Test/01 Flat Song.mp3"

mkdir -p "$MUSIC_DIR/Flat Artist Test"
ffmpeg -y -loglevel error -f lavfi -i "sine=frequency=280:duration=2" \
  -metadata title="Loose Track" -metadata artist="Flat Artist Test" \
  -metadata album_artist="Flat Artist Test" -metadata album="Singles" \
  -metadata genre="Ambient" -metadata track=1 \
  -c:a libmp3lame -b:a 96k "$MUSIC_DIR/Flat Artist Test/01 Loose Track.mp3"

echo "==> Generando lista de prueba (test-media/Playlists)"
mkdir -p "$OUT_DIR/Playlists"
cat > "$OUT_DIR/Playlists/QA Favorites.m3u8" <<'EOF'
/Music/Aura Test Combo/First Light/01 Sunrise.mp3
/Music/Aura Test Combo/Night Drive/02 Neon Mile.mp3
/Music/Wheel & Click/Analog Dreams/03 Idle Hum.mp3
EOF

SIMDISK="$ROOT_DIR/firmware/build-sim/simdisk"
if [[ -d "$SIMDISK" ]]; then
  echo "==> Instalando fixtures en $SIMDISK"
  mkdir -p "$SIMDISK/Photos" "$SIMDISK/Videos"
  cp "$OUT_DIR"/Photos/* "$SIMDISK/Photos/"
  cp "$OUT_DIR"/Videos/*.mpg "$SIMDISK/Videos/"
  # Fixtures de MUSICA: solo bajo peticion explicita
  # (METRO_INSTALL_MUSIC_FIXTURES=1), para no ensuciar el simulador con
  # una biblioteca de prueba cuando alguien solo necesita probar
  # Videos/Fotos, o cuando simdisk/Music ya apunta (symlink manual del
  # dueno del checkout) a una biblioteca real.
  if [[ "${METRO_INSTALL_MUSIC_FIXTURES:-0}" == "1" ]]; then
    mkdir -p "$SIMDISK/Music" "$SIMDISK/Playlists"
    cp -R "$MUSIC_DIR"/. "$SIMDISK/Music/"
    cp "$OUT_DIR/Playlists"/*.m3u8 "$SIMDISK/Playlists/"
    cp "$OUT_DIR"/metro-test.* "$SIMDISK/Music/"
    cp "$OUT_DIR"/cover.jpg "$SIMDISK/Music/"
    # En la RAIZ del disco, no bajo Music/: find_albumart tambien busca
    # cover.jpg en el directorio padre, y Music/cover.jpg "vestiria" al
    # album que precisamente debe quedar sin arte.
    mkdir -p "$SIMDISK/SinArte"
    cp "$OUT_DIR"/SinArte/*.mp3 "$SIMDISK/SinArte/"
  fi
fi

echo "==> Listo: $OUT_DIR"
