
#include "plugin.h"

/* Metro (M-059): version 6 -- see mpeg_scale_mode_id/settings.scale_mode
 * below, ported from Aura-Firmware's mpeg_settings.h (D-304, consulted
 * read-only). SETTINGS_MIN_VERSION stays 1 -- an old .cfg without the
 * new field just gets settings.scale_mode's compiled default from
 * init_settings() below (same "missing key -> default" pattern the
 * rest of this struct already relies on, not a new compatibility
 * mechanism). */
#define SETTINGS_VERSION 6
#define SETTINGS_MIN_VERSION 1
#define SETTINGS_FILENAME "mpegplayer.cfg"

#if defined(TOSHIBA_GIGABEAT_F) || defined(SANSA_E200) || defined(SANSA_C200) \
    || defined(IRIVER_H10) || defined(COWON_D2) || defined(PHILIPS_HDD1630) \
    || defined(SANSA_FUZE) || defined(SANSA_E200V2) || defined(SANSA_FUZEV2) \
    || defined(TOSHIBA_GIGABEAT_S) || defined(PHILIPS_SA9200)
#define MPEG_OPTION_DITHERING_ENABLED 1
#endif

#ifndef MPEG_OPTION_DITHERING_ENABLED
#define MPEG_OPTION_DITHERING_ENABLED 0
#endif

enum mpeg_option_id
{
#if MPEG_OPTION_DITHERING_ENABLED
    MPEG_OPTION_DITHERING,
#endif
    MPEG_OPTION_DISPLAY_FPS,
    MPEG_OPTION_LIMIT_FPS,
    MPEG_OPTION_SKIP_FRAMES,
    MPEG_OPTION_SCALE_MODE,
#ifdef HAVE_BACKLIGHT_BRIGHTNESS
    MPEG_OPTION_BACKLIGHT_BRIGHTNESS,
#endif
};

/* Metro (M-059): "ajustar" (con franjas, sin recortar) vs "cubrir"
 * (llena la pantalla, recorta el sobrante) -- mismo concepto que el
 * visor de fotos (R2-F3/DD-10), alternable con SELECT durante la
 * reproduccion o desde este menu. */
enum mpeg_scale_mode_id
{
    MPEG_SCALE_MODE_FIT = 0,
    MPEG_SCALE_MODE_COVER,
};

enum mpeg_audio_option_id
{
    MPEG_AUDIO_TONE_CONTROLS,
    MPEG_AUDIO_CHANNEL_MODES,
    MPEG_AUDIO_CROSSFEED,
    MPEG_AUDIO_EQUALIZER,
    MPEG_AUDIO_DITHERING,
};

enum mpeg_resume_id
{
    MPEG_RESUME_MENU_ALWAYS = 0,
    MPEG_RESUME_MENU_IF_INCOMPLETE,
    MPEG_RESUME_RESTART,
    MPEG_RESUME_ALWAYS,
    MPEG_RESUME_NUM_OPTIONS,
};

enum mpeg_start_id
{
    MPEG_START_RESTART,
    MPEG_START_RESUME,
    MPEG_START_SEEK,
    MPEG_START_SETTINGS,
    MPEG_START_QUIT,
    MPEG_START_EXIT,
};

enum mpeg_setting_id
{
    MPEG_SETTING_DISPLAY_SETTINGS,
    MPEG_SETTING_AUDIO_SETTINGS,
    MPEG_SETTING_PLAY_MODE,
    MPEG_SETTING_CLEAR_RESUMES,
};

enum mpeg_menu_id
{
    MPEG_MENU_SETTINGS,
    MPEG_MENU_QUIT,
};

struct mpeg_settings {
    int showfps;               /* flag to display fps */
    int limitfps;              /* flag to limit fps */
    int skipframes;            /* flag to skip frames */
    int scale_mode;            /* Metro (M-059): fit vs cover, enum mpeg_scale_mode_id */
    int resume_options;        /* type of resume action at start */
    int resume_count;          /* total # of resumes in config file */
    int resume_time;           /* resume time for current mpeg (in half minutes) */
    char resume_filename[MAX_PATH]; /* filename of current mpeg */
#if MPEG_OPTION_DITHERING_ENABLED
    int displayoptions;
#endif
    int play_mode;             /* play single file or all files in directory */
    /* Audio options - simple on/off specification */
    int tone_controls;
    int channel_modes;
    int crossfeed;
    int equalizer;
    int dithering;
    /* Backlight options */
#ifdef HAVE_BACKLIGHT_BRIGHTNESS
    int backlight_brightness;
#endif
};

extern struct mpeg_settings settings;

int mpeg_start_menu(uint32_t duration);
int mpeg_menu(void);

void init_settings(const char* filename);
void save_settings(void);

#ifdef HAVE_BACKLIGHT_BRIGHTNESS
void mpeg_backlight_update_brightness(int value);
#endif

/* R2-F4/DD-11 (M-059): Metro's own bilingual mini string table --
 * strings live in mpeg_settings.c, IDs here so mpegplayer.c/
 * stream_mgr.c can reference them by name for their own splash
 * strings too. metro_str()/metro_language() are the two boundary
 * points a plugin has into Metro's real personalization (which lives
 * in apps/metro/, a separate build/link unit this plugin can't
 * include) -- see metro_load_personalization() in mpegplayer.c. */
enum metro_str_id
{
    MSTR_SETTINGS = 0,
    MSTR_EXIT,
    MSTR_VIDEO_PLAYER,
    MSTR_DISPLAY_OPTIONS,
    MSTR_AUDIO_OPTIONS,
    MSTR_PLAY_MODE,
    MSTR_CLEAR_RESUMES,
    MSTR_SHOW_FPS,
    MSTR_LIMIT_FPS,
    MSTR_SKIP_FRAMES,
    MSTR_SCALE_MODE,
    MSTR_BACKLIGHT_BRIGHTNESS,
    MSTR_DITHERING,
    MSTR_TONE_CONTROLS,
    MSTR_CHANNEL_MODES,
    MSTR_CROSSFEED,
    MSTR_EQUALIZER,
    MSTR_NO,
    MSTR_YES,
    MSTR_OFF,
    MSTR_USE_SOUND_SETTING,
    MSTR_FIT,
    MSTR_COVER,
    MSTR_SINGLE,
    MSTR_ALL,
    MSTR_USE_COMMON_SETTING,
    MSTR_GREYLIB_FAILED,
    MSTR_STREAM_THREAD_FAILED,
    MSTR_OUT_OF_MEMORY,
    MSTR_PCM_FAILED,
    MSTR_AUDIO_THREAD_FAILED,
    MSTR_VIDEO_THREAD_FAILED,
    MSTR_BUFFER_THREAD_FAILED,
    MSTR_PARSER_FAILED,
    MSTR_PLAYBACK_FAILED,
    MSTR_NO_FILE,
    MSTR_UNSUPPORTED_FORMAT,
    MSTR_ERROR_OPENING_FILE,
    /* R2-F4 Zune redesign (M-060 cont.): lowercase page titles for the
     * DISPLAY-48 header -- separate entries from the row labels above
     * because Metro's big titles are all-lowercase and deliberately
     * short (the 48px face fits ~13 characters across 320px), while
     * the same concept as a row label reads in sentence case ("Opciones
     * de pantalla" row -> "pantalla" page). */
    MSTR_TITLE_VIDEO,
    MSTR_TITLE_SETTINGS,
    MSTR_TITLE_DISPLAY,
    MSTR_TITLE_AUDIO,
    MSTR_TITLE_BRIGHTNESS,
    MSTR_COUNT
};

const char *metro_str(int id);
int metro_language(void);
void metro_osd_colors(unsigned *bgcolor, unsigned *fgcolor, unsigned *secondary,
                      unsigned *tertiary, unsigned *accent);

/* R2-F4 Zune redesign (M-060 cont.): Metro's own real list typefaces,
 * loaded once in mpegplayer.c's osd_init() next to the caption font --
 * mpeg_settings.c's menu widget was drawing with plain FONT_UI before,
 * which is why it never actually looked like the rest of Metro despite
 * matching its row geometry. A plugin can't include apps/metro/'s own
 * metro_fonts.c (separate link unit), so these are loaded fresh here,
 * same rb->font_load() mechanism as the caption font. */
int metro_font_caption(void);
int metro_font_list(void);
int metro_font_list_sel(void);
int metro_font_display(void);
int metro_font_title(void);
