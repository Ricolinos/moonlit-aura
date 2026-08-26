/* D-056: sustituto de host para firmware/rockbox/firmware/include/dir.h
 * (mismo criterio que test/file.h, D-042): moonlit_art_sweep() usa
 * opendir/readdir/closedir y `struct DIRENT` -- en el build real
 * filesystem-native.h define DIRENT como dirent; aqui basta <dirent.h>
 * del sistema. */
#ifndef MOONLIT_TEST_DIR_H
#define MOONLIT_TEST_DIR_H
#include <dirent.h>
#define DIRENT dirent
#endif
