/* D-042: sustituto de host para firmware/rockbox/firmware/include/file.h
 * -- ese file.h real depende de config.h/fs_defines.h (solo existen
 * dentro del build de Rockbox), asi que un `cc` de host nunca lo
 * encuentra. moonlit_art.c usa open/read/write/close/creat con las
 * MISMAS firmas POSIX que Rockbox expone via FS_PREFIX() en build de
 * simulador/dispositivo -- para el test de host alcanza con las
 * funciones reales del sistema. Solo lo recoge esta compilacion: el
 * `#include "file.h"` de moonlit_art.c resuelve primero contra
 * apps/metro/ (donde no hay file.h), y solo cae aqui via el -I. del
 * Makefile de este directorio. */
#ifndef MOONLIT_TEST_FILE_H
#define MOONLIT_TEST_FILE_H
#include <fcntl.h>
#include <unistd.h>
#endif
