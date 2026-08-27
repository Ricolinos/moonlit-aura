/* Host stand-in for Rockbox's dir.h (D-056 moonlit_art_sweep(); D-059
 * moonlit_master_art_ensure_dir()). Only what the pure modules use:
 * opendir/readdir/closedir straight from POSIX, plus Rockbox's
 * one-argument mkdir() and dir_exists(). The mkdir macro only affects
 * the module being compiled against this header -- test sources
 * include the module .h files, never this one, so their own two-
 * argument POSIX mkdir() calls are untouched. */
#ifndef MOONLIT_TEST_DIR_H
#define MOONLIT_TEST_DIR_H
#include <dirent.h>
#include <stdbool.h>
#include <sys/stat.h>
#define DIRENT dirent

static inline bool dir_exists(const char *path)
{
    struct stat st;
    return stat(path, &st) == 0 && S_ISDIR(st.st_mode);
}

static inline int moonlit_host_mkdir1(const char *path)
{
    return mkdir(path, 0777);
}
#define mkdir(path) moonlit_host_mkdir1(path)
#endif
