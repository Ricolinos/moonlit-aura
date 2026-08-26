#include "plugin.h"
#include "lib/helper.h"
#include "lib/configfile.h"

#include "mpegplayer.h"
#include "mpeg_settings.h"

struct mpeg_settings settings;

/* R2-F4/DD-11 (M-059): port + restyle of Aura-Firmware's own mpegplayer
 * patch (D-304..D-309, consulted read-only as a mechanism guide --
 * apps/plugins/mpegplayer/{mpeg_settings.c,mpegplayer.c,stream_mgr.c}
 * of that repo). Two things Aura's version did that this one does NOT
 * copy verbatim, kept Metro's own way instead:
 *   - Selection highlight: Aura draws a rounded "pill" (its own Apple2026
 *     design language); Metro draws flat rectangles, same geometry as
 *     metro_draw_rows() (pitch 28px, x=12, no rounding) -- see
 *     metro_menu_draw() below.
 *   - Colors: Aura's aura_load_personalization() supports fully custom
 *     runtime themes (a whole theme.cfg file per style) because Aura
 *     has installable themes; Metro doesn't (M-012) -- metro_load_personalization()
 *     (mpegplayer.c) only ever picks among the 4 compiled MD3 accent
 *     presets in moonlit_tokens.h plus the compiled night/dawn surface
 *     tones (moonlit D-035), matching exactly what moonlit_palette.c
 *     already does for the rest of the app.
 *
 * The per-target MPEG_START_TIME_* button block (~400 lines, one #elif
 * per Rockbox target) that used to fill the top of this file, and
 * get_start_time()/show_start_menu()/draw_slider()/display_thumb_image()/
 * show_loading()/increment_time()/resume_options(), are gone -- dead
 * code once the interactive start menu is removed (mpeg_start_menu()
 * below always resolves directly, same simplification Aura's own D-06x
 * made first). vo_draw_frame_thumb()/stretch_image_plane() in
 * video_out_rockbox.c stay (unused by anything here now, but
 * stretch_image_plane() gets a real second caller from the new "cubrir"
 * mode there). */

/* metro_osd_colors()/metro_language() (declared in mpeg_settings.h):
 * mpegplayer.c reads aura.cfg ONCE, at osd_init() time
 * (metro_load_personalization() lives there, next to the osd struct it
 * fills) -- these are cheap accessors into that already-loaded state,
 * not a re-read per menu redraw. Same split Aura-Firmware's own patch
 * uses (aura_osd_colors(), consulted read-only) for the same reason:
 * this menu widget redraws on every button press, re-opening and
 * re-parsing a file that often would be pure waste. */

/* --- bilingual mini string table ----------------------------------------
 *
 * Not metro_lang.c (that's apps/metro/, a separate build/link unit a
 * plugin can't include) -- a small table of just what this plugin
 * itself ever shows, same idea in miniature, chosen by metro_language()
 * (mpegplayer.c, same aura.cfg read as the colors above). enum
 * metro_str_id itself lives in mpeg_settings.h -- mpegplayer.c/
 * stream_mgr.c need the symbolic IDs too, for their own splash strings. */
static const char *const metro_str_es[MSTR_COUNT] = {
    [MSTR_SETTINGS]              = "Ajustes",
    [MSTR_EXIT]                  = "Salir",
    /* Lowercase on purpose -- used as the page eyebrow (caption line at
     * the very top), which Metro's own screens render lowercase
     * ("videos", "musica" -- see metro_draw_header() call sites). */
    [MSTR_VIDEO_PLAYER]          = "reproductor de video",
    [MSTR_DISPLAY_OPTIONS]       = "Opciones de pantalla",
    [MSTR_AUDIO_OPTIONS]         = "Opciones de audio",
    [MSTR_PLAY_MODE]             = "Modo de reproduccion",
    [MSTR_CLEAR_RESUMES]         = "Borrar todas las reanudaciones",
    [MSTR_SHOW_FPS]              = "Mostrar FPS",
    [MSTR_LIMIT_FPS]             = "Limitar FPS",
    [MSTR_SKIP_FRAMES]           = "Omitir fotogramas",
    [MSTR_SCALE_MODE]            = "Modo de ajuste",
    [MSTR_BACKLIGHT_BRIGHTNESS]  = "Brillo de la luz de fondo",
    [MSTR_DITHERING]             = "Tramado",
    [MSTR_TONE_CONTROLS]         = "Controles de tono",
    [MSTR_CHANNEL_MODES]         = "Configuracion de canales",
    [MSTR_CROSSFEED]             = "Crossfeed",
    [MSTR_EQUALIZER]             = "Ecualizador",
    [MSTR_NO]                    = "No",
    [MSTR_YES]                   = "Si",
    [MSTR_OFF]                   = "Desactivado",
    [MSTR_USE_SOUND_SETTING]     = "Usar ajuste de sonido",
    [MSTR_FIT]                   = "Ajustar",
    [MSTR_COVER]                 = "Cubrir",
    [MSTR_SINGLE]                = "Uno",
    [MSTR_ALL]                   = "Todos",
    [MSTR_USE_COMMON_SETTING]    = "Usar ajuste general",
    [MSTR_GREYLIB_FAILED]        = "Fallo al iniciar greylib",
    [MSTR_STREAM_THREAD_FAILED]  = "No se pudo crear el hilo del gestor de flujo",
    [MSTR_OUT_OF_MEMORY]         = "Memoria insuficiente",
    [MSTR_PCM_FAILED]            = "No se pudo inicializar el PCM",
    [MSTR_AUDIO_THREAD_FAILED]   = "No se pudo crear el hilo de audio",
    [MSTR_VIDEO_THREAD_FAILED]   = "No se pudo crear el hilo de video",
    [MSTR_BUFFER_THREAD_FAILED]  = "No se pudo crear el hilo de buffer",
    [MSTR_PARSER_FAILED]         = "Fallo al iniciar el analizador",
    [MSTR_PLAYBACK_FAILED]       = "Error al reproducir",
    [MSTR_NO_FILE]               = "Sin archivo",
    [MSTR_UNSUPPORTED_FORMAT]    = "Formato no compatible",
    [MSTR_ERROR_OPENING_FILE]    = "Error al abrir el archivo: %d",
    [MSTR_TITLE_VIDEO]           = "video",
    [MSTR_TITLE_SETTINGS]        = "ajustes",
    [MSTR_TITLE_DISPLAY]         = "pantalla",
    [MSTR_TITLE_AUDIO]           = "audio",
    [MSTR_TITLE_BRIGHTNESS]      = "brillo",
};

static const char *const metro_str_en[MSTR_COUNT] = {
    [MSTR_SETTINGS]              = "Settings",
    [MSTR_EXIT]                  = "Exit",
    [MSTR_VIDEO_PLAYER]          = "video player",
    [MSTR_DISPLAY_OPTIONS]       = "Display options",
    [MSTR_AUDIO_OPTIONS]         = "Audio options",
    [MSTR_PLAY_MODE]             = "Play mode",
    [MSTR_CLEAR_RESUMES]         = "Clear all resumes",
    [MSTR_SHOW_FPS]              = "Show FPS",
    [MSTR_LIMIT_FPS]             = "Limit FPS",
    [MSTR_SKIP_FRAMES]           = "Skip frames",
    [MSTR_SCALE_MODE]            = "Scale mode",
    [MSTR_BACKLIGHT_BRIGHTNESS]  = "Backlight brightness",
    [MSTR_DITHERING]             = "Dithering",
    [MSTR_TONE_CONTROLS]         = "Tone controls",
    [MSTR_CHANNEL_MODES]         = "Channel configuration",
    [MSTR_CROSSFEED]             = "Crossfeed",
    [MSTR_EQUALIZER]             = "Equalizer",
    [MSTR_NO]                    = "No",
    [MSTR_YES]                   = "Yes",
    [MSTR_OFF]                   = "Off",
    [MSTR_USE_SOUND_SETTING]     = "Use sound setting",
    [MSTR_FIT]                   = "Fit",
    [MSTR_COVER]                 = "Cover",
    [MSTR_SINGLE]                = "Single",
    [MSTR_ALL]                   = "All",
    [MSTR_USE_COMMON_SETTING]    = "Use common setting",
    [MSTR_GREYLIB_FAILED]        = "greylib init failed!",
    [MSTR_STREAM_THREAD_FAILED]  = "Could not create stream manager thread!",
    [MSTR_OUT_OF_MEMORY]         = "Out of memory",
    [MSTR_PCM_FAILED]            = "Could not initialize PCM!",
    [MSTR_AUDIO_THREAD_FAILED]   = "Cannot create audio thread!",
    [MSTR_VIDEO_THREAD_FAILED]   = "Cannot create video thread!",
    [MSTR_BUFFER_THREAD_FAILED]  = "Cannot create buffering thread!",
    [MSTR_PARSER_FAILED]         = "Parser init failed!",
    [MSTR_PLAYBACK_FAILED]       = "Playback failed",
    [MSTR_NO_FILE]               = "No File",
    [MSTR_UNSUPPORTED_FORMAT]    = "Unsupported format",
    [MSTR_ERROR_OPENING_FILE]    = "Error opening file: %d",
    [MSTR_TITLE_VIDEO]           = "video",
    [MSTR_TITLE_SETTINGS]        = "settings",
    [MSTR_TITLE_DISPLAY]         = "display",
    [MSTR_TITLE_AUDIO]           = "audio",
    [MSTR_TITLE_BRIGHTNESS]      = "brightness",
};

const char *metro_str(int id)
{
    if (id < 0 || id >= MSTR_COUNT)
        return "";
    return (metro_language() == 1) ? metro_str_en[id] : metro_str_es[id];
}

static struct configdata config[] =
{
    {TYPE_INT, 0, 2, { .int_p = &settings.showfps }, "Show FPS", NULL},
    {TYPE_INT, 0, 2, { .int_p = &settings.limitfps }, "Limit FPS", NULL},
    {TYPE_INT, 0, 2, { .int_p = &settings.skipframes }, "Skip frames", NULL},
    {TYPE_INT, 0, 1, { .int_p = &settings.scale_mode }, "Scale mode", NULL},
    {TYPE_INT, 0, INT_MAX, { .int_p = &settings.resume_count }, "Resume count",
     NULL},
    {TYPE_INT, 0, MPEG_RESUME_NUM_OPTIONS,
     { .int_p = &settings.resume_options }, "Resume options", NULL},
#if MPEG_OPTION_DITHERING_ENABLED
    {TYPE_INT, 0, INT_MAX, { .int_p = &settings.displayoptions },
     "Display options", NULL},
#endif
    {TYPE_INT, 0, 2, { .int_p = &settings.tone_controls }, "Tone controls",
     NULL},
    {TYPE_INT, 0, 2, { .int_p = &settings.channel_modes }, "Channel modes",
     NULL},
    {TYPE_INT, 0, 2, { .int_p = &settings.crossfeed }, "Crossfeed", NULL},
    {TYPE_INT, 0, 2, { .int_p = &settings.equalizer }, "Equalizer", NULL},
    {TYPE_INT, 0, 2, { .int_p = &settings.dithering }, "Dithering", NULL},
    {TYPE_INT, 0, 2, { .int_p = &settings.play_mode }, "Play mode", NULL},
#ifdef HAVE_BACKLIGHT_BRIGHTNESS
    {TYPE_INT, -1, INT_MAX, { .int_p = &settings.backlight_brightness },
     "Backlight brightness", NULL},
#endif
};

static void mpeg_settings(void);

/* --- Metro's own list widget, replacing rb->do_menu()/rb->set_option()/
 * rb->set_int_ex() ---------------------------------------------------
 *
 * Those are 100% native Rockbox widgets (own "back" icon, own selection
 * highlight, own font) that never read a single color from aura.cfg.
 * Geometry matches metro_draw_rows() (apps/metro/metro_draw.c) exactly
 * -- pitch 28px, x=12, header caption at y=4 -- so this menu looks like
 * any other Metro list, not a foreign plugin screen. */
#define METRO_ROW_PITCH  28
#define METRO_ROW_LEFT_X 12
#define METRO_ROWS_FIRST_Y 84
#define METRO_HEADER_Y   4
#define METRO_TITLE_Y    28

/* Battery glyph geometry copied 1:1 from metro_draw_battery()
 * (apps/metro/metro_draw.c, F10) -- a plugin can't link that module,
 * so the 18x9 outline + 2x4 nub is redrawn here with rb-> calls. */
#define METRO_BATTERY_W     18
#define METRO_BATTERY_H     9
#define METRO_BATTERY_NUB_W 2
#define METRO_BATTERY_NUB_H 4

static void metro_page_battery(int x_right, int y, unsigned color)
{
    int level = rb->battery_level();
    int body_x = x_right - METRO_BATTERY_NUB_W - METRO_BATTERY_W;
    int nub_x = x_right - METRO_BATTERY_NUB_W;
    int nub_y = y + (METRO_BATTERY_H - METRO_BATTERY_NUB_H) / 2;
    int fill_w;

    rb->lcd_set_foreground(color);
    rb->lcd_drawrect(body_x, y, METRO_BATTERY_W, METRO_BATTERY_H);
    rb->lcd_fillrect(nub_x, nub_y, METRO_BATTERY_NUB_W, METRO_BATTERY_NUB_H);

    if (level > 100)
        level = 100;
    if (level > 0)
    {
        fill_w = (METRO_BATTERY_W - 4) * level / 100;
        if (fill_w > 0)
            rb->lcd_fillrect(body_x + 2, y + 2, fill_w, METRO_BATTERY_H - 4);
    }
}

/* The top of every page: eyebrow caption at the left, clock, battery --
 * metro_draw_header() (apps/metro/metro_draw.c) replicated with rb->
 * calls, same positions (12,4 / width-40 / width-4). The eyebrow is
 * always "reproductor de video": the plugin is one place inside the
 * larger app, and the eyebrow names the place, like "videos" does on
 * the hub. */
static void metro_page_chrome(const char *title)
{
    unsigned bg, fg, secondary, tertiary, accent;
    struct tm *now;

    metro_osd_colors(&bg, &fg, &secondary, &tertiary, &accent);
    (void)tertiary; (void)accent;

    rb->lcd_set_background(bg);
    rb->lcd_set_drawmode(DRMODE_SOLID);
    rb->lcd_clear_display();

    rb->lcd_set_drawmode(DRMODE_FG);
    rb->lcd_setfont(metro_font_caption());
    rb->lcd_set_foreground(secondary);
    rb->lcd_putsxy(METRO_ROW_LEFT_X, METRO_HEADER_Y,
                   metro_str(MSTR_VIDEO_PLAYER));

    now = rb->get_time();
    if (now != NULL)
    {
        char timebuf[8];
        int w;
        rb->snprintf(timebuf, sizeof(timebuf), "%02d:%02d",
                     now->tm_hour, now->tm_min);
        rb->lcd_getstringsize(timebuf, &w, NULL);
        rb->lcd_putsxy(LCD_WIDTH - 40 - w, METRO_HEADER_Y, timebuf);
    }

    metro_page_battery(LCD_WIDTH - 4, METRO_HEADER_Y, secondary);

    /* The signature Metro element: the big lowercase page title in the
     * display face (48px), same position the hub draws its pivots
     * (metro_draw_pivots(): x=12, y=28). This -- not the row geometry
     * -- is what M-059's first restyle was missing entirely. */
    rb->lcd_setfont(metro_font_display());
    rb->lcd_set_foreground(fg);
    rb->lcd_putsxy(METRO_ROW_LEFT_X, METRO_TITLE_Y, title);
}

static void metro_menu_draw(const char *title, const char *const *labels,
                            const char *const *values, int count, int sel)
{
    unsigned bg, fg, secondary, tertiary, accent;
    int y, i;

    metro_osd_colors(&bg, &fg, &secondary, &tertiary, &accent);
    (void)accent; /* not used here -- selected row uses fg, see DD-11 */
    (void)bg;

    metro_page_chrome(title);

    /* R2-F1/DD-1 (M-051)'s own rule, restated here since a plugin can't
     * include metro_draw.c: every glyph transparent (DRMODE_FG), never
     * the DRMODE_SOLID default. Rows replicate metro_draw_rows()
     * (apps/metro/metro_draw.c) exactly: from y=84, pitch 28, x=12,
     * title in the list face (listsel when selected, fg vs secondary),
     * current value right-aligned at width-12 in caption/tertiary,
     * nudged +4 to sit on the list face's baseline -- same fonts, same
     * colors, same offsets, so this reads as one more Metro list, not
     * a plugin's imitation of one. */
    y = METRO_ROWS_FIRST_Y;

    for (i = 0; i < count; i++)
    {
        bool selected = (i == sel);

        rb->lcd_set_drawmode(DRMODE_FG);
        rb->lcd_setfont(selected ? metro_font_list_sel() : metro_font_list());
        rb->lcd_set_foreground(selected ? fg : secondary);
        rb->lcd_putsxy(METRO_ROW_LEFT_X, y, labels[i]);

        if (values && values[i])
        {
            int vw;
            rb->lcd_setfont(metro_font_caption());
            rb->lcd_getstringsize(values[i], &vw, NULL);
            rb->lcd_set_foreground(tertiary);
            rb->lcd_putsxy(LCD_WIDTH - METRO_ROW_LEFT_X - vw, y + 4, values[i]);
        }

        y += METRO_ROW_PITCH;
    }

    rb->lcd_update();
}

/* Devuelve el indice elegido (0..count-1), o -1 si el usuario cancelo
 * con MENU o por un evento de sistema (USB, apagado -- mpeg_sysevent()
 * distingue el segundo caso para que el llamador no siga navegando). */
static int metro_menu_pick(const char *title, const char *const *labels,
                           const char *const *values, int count, int start_sel)
{
    int sel = (start_sel >= 0 && start_sel < count) ? start_sel : 0;

    rb->button_clear_queue();
    mpeg_sysevent_clear();

    while (1)
    {
        int button;

        metro_menu_draw(title, labels, values, count, sel);

        button = mpeg_button_get(TIMEOUT_BLOCK);

        if (mpeg_sysevent() != 0)
            return -1;

        switch (button)
        {
        case BUTTON_SCROLL_FWD:
        case BUTTON_SCROLL_FWD | BUTTON_REPEAT:
            sel = (sel + 1) % count;
            break;

        case BUTTON_SCROLL_BACK:
        case BUTTON_SCROLL_BACK | BUTTON_REPEAT:
            sel = (sel - 1 + count) % count;
            break;

        case BUTTON_SELECT:
            return sel;

        case BUTTON_MENU:
            return -1;

        default:
            break;
        }
    }
}

#ifdef HAVE_BACKLIGHT_BRIGHTNESS /* Only used for this atm */
static void metro_adjust_draw(const char *title, const char *value_text)
{
    unsigned bg, fg, secondary, tertiary, accent;

    metro_osd_colors(&bg, &fg, &secondary, &tertiary, &accent);
    (void)bg; (void)fg; (void)secondary; (void)tertiary;

    /* R2-F4 Zune redesign (M-060 cont.): same page anatomy as every
     * other screen (chrome + big lowercase title), value left-aligned
     * under it in the title face and the user's accent -- the one spot
     * a Metro settings page paints accent, the "current value" (same
     * role the accent plays in metro_widgets' dialogs). */
    metro_page_chrome(title);

    rb->lcd_set_drawmode(DRMODE_FG);
    rb->lcd_setfont(metro_font_title());
    rb->lcd_set_foreground(accent);
    rb->lcd_putsxy(METRO_ROW_LEFT_X, METRO_ROWS_FIRST_Y + 16, value_text);

    rb->lcd_update();
}

/* Ajustador numerico simple -- solo lo usa el brillo de la luz de
 * fondo, el unico ajuste de este menu que no es una eleccion entre
 * unas pocas opciones fijas. IZQUIERDA/DERECHA cambian el valor de a
 * uno (aplicado en vivo via live_apply), SELECT confirma, MENU/evento
 * de sistema cancela y restaura el valor original. */
static bool metro_menu_adjust_int(const char *title, int *value, int min, int max,
                                  const char* (*formatter)(char*, size_t, int, const char*),
                                  void (*live_apply)(int))
{
    int v = *value;
    int orig = v;

    rb->button_clear_queue();
    mpeg_sysevent_clear();

    while (1)
    {
        char buf[32];
        const char *text;
        int button;

        text = formatter(buf, sizeof(buf), v, NULL);

        if (live_apply)
            live_apply(v);

        metro_adjust_draw(title, text);

        button = mpeg_button_get(TIMEOUT_BLOCK);

        if (mpeg_sysevent() != 0)
        {
            if (live_apply)
                live_apply(orig);
            return false;
        }

        switch (button)
        {
        case BUTTON_LEFT:
        case BUTTON_LEFT | BUTTON_REPEAT:
            if (v > min) v--;
            break;

        case BUTTON_RIGHT:
        case BUTTON_RIGHT | BUTTON_REPEAT:
            if (v < max) v++;
            break;

        case BUTTON_SELECT:
            *value = v;
            return true;

        case BUTTON_MENU:
            if (live_apply)
                live_apply(orig);
            return false;

        default:
            break;
        }
    }
}
#endif /* HAVE_BACKLIGHT_BRIGHTNESS */

#ifdef HAVE_BACKLIGHT_BRIGHTNESS /* Only used for this atm */
void mpeg_backlight_update_brightness(int value)
{
    if (value >= 0)
    {
        value += MIN_BRIGHTNESS_SETTING;
        backlight_brightness_set(value);
    }
    else
    {
        backlight_brightness_use_setting();
    }
}

static void backlight_brightness_function(int value)
{
    mpeg_backlight_update_brightness(value);
}

static const char* backlight_brightness_formatter(char *buf, size_t length,
                                                  int value, const char *input)
{
    (void)input;

    if (value < 0)
        return metro_str(MSTR_USE_COMMON_SETTING);
    else
        rb->snprintf(buf, length, "%d", value + MIN_BRIGHTNESS_SETTING);
    return buf;
}
#endif /* HAVE_BACKLIGHT_BRIGHTNESS */

/* Sync a particular audio setting to global or mpegplayer forced off */
static void sync_audio_setting(int setting, bool global)
{
    switch (setting)
    {
    case MPEG_AUDIO_TONE_CONTROLS:
    #ifdef AUDIOHW_HAVE_BASS
        rb->sound_set(SOUND_BASS, (global || settings.tone_controls)
            ? rb->global_settings->bass
            : rb->sound_default(SOUND_BASS));
    #endif
    #ifdef AUDIOHW_HAVE_TREBLE
        rb->sound_set(SOUND_TREBLE, (global || settings.tone_controls)
            ? rb->global_settings->treble
            : rb->sound_default(SOUND_TREBLE));
    #endif

    #ifdef AUDIOHW_HAVE_EQ
        for (int band = 0;; band++)
        {
            int setting = rb->sound_enum_hw_eq_band_setting(band, AUDIOHW_EQ_GAIN);

            if (setting == -1)
                break;

            rb->sound_set(setting, (global || settings.tone_controls)
                    ? rb->global_settings->hw_eq_bands[band].gain
                    : rb->sound_default(setting));
        }
    #endif /* AUDIOHW_HAVE_EQ */
        break;

    case MPEG_AUDIO_CHANNEL_MODES:
        rb->sound_set(SOUND_CHANNELS, (global || settings.channel_modes)
                ? rb->global_settings->channel_config
                : SOUND_CHAN_STEREO);
        break;

    case MPEG_AUDIO_CROSSFEED:
        rb->dsp_set_crossfeed_type((global || settings.crossfeed) ?
                                   rb->global_settings->crossfeed :
                                   CROSSFEED_TYPE_NONE);
        break;

    case MPEG_AUDIO_EQUALIZER:
        rb->dsp_eq_enable((global || settings.equalizer) ?
                          rb->global_settings->eq_enabled : false);
        break;

    case MPEG_AUDIO_DITHERING:
        rb->dsp_dither_enable((global || settings.dithering) ?
                              rb->global_settings->dithering_enabled : false);
       break;
    }
}

/* Sync all audio settings to global or mpegplayer forced off */
static void sync_audio_settings(bool global)
{
    static const int setting_index[] =
    {
        MPEG_AUDIO_TONE_CONTROLS,
        MPEG_AUDIO_CHANNEL_MODES,
        MPEG_AUDIO_CROSSFEED,
        MPEG_AUDIO_EQUALIZER,
        MPEG_AUDIO_DITHERING,
    };
    unsigned i;

    for (i = 0; i < ARRAYLEN(setting_index); i++)
    {
        sync_audio_setting(setting_index[i], global);
    }
}

/* Return the desired resume action.
 *
 * Metro (M-059), mismo criterio que Aura-Firmware ya establecio
 * primero (D-06x, consultado read-only): entra directo reproduciendo
 * (o retomando desde donde quedo), sin el menu interactivo "MPEG
 * Player: Play from beginning / Resume / Set resume time / Settings /
 * Quit" que Rockbox stock mostraba siempre antes de cada video --
 * Metro no tiene un equivalente propio de ese menu ni quiere
 * exponerlo. settings.resume_options se ignora a proposito (no solo se
 * le cambia el default): asi el comportamiento es correcto incluso si
 * un mpegplayer.cfg viejo (de una instalacion Rockbox stock previa) ya
 * tenia guardado MPEG_RESUME_MENU_ALWAYS. resume_time=0 (video nunca
 * visto) reproduce igual desde el principio via MPEG_START_SEEK. */
int mpeg_start_menu(uint32_t duration)
{
    (void)duration;
    mpeg_sysevent_clear();
    return MPEG_START_SEEK;
}

int mpeg_menu(void)
{
    const char *const items[] = { metro_str(MSTR_SETTINGS), metro_str(MSTR_EXIT) };
    int result;

    result = metro_menu_pick(metro_str(MSTR_TITLE_VIDEO), items, NULL, 2, 0);

    switch (result)
    {
    case MPEG_MENU_SETTINGS:
        mpeg_settings();
        break;

    default:
        break;
    }

    if (mpeg_sysevent() != 0)
        result = MPEG_MENU_QUIT;

    return result;
}

/* R2-F4 Zune redesign (M-060 cont.): value rows cycle in place on
 * SELECT, with the current value always visible as the row's
 * right-aligned subtitle -- the exact interaction model of the app's
 * own settings screen (metro_screen_settings.c, general_on_select():
 * language/animations/graphics all cycle in place there). M-059's
 * first version opened a separate two-row picker page per value,
 * which is a Rockbox pattern, not a Metro one. */
static void display_options(void)
{
    const char *const items[] = {
        metro_str(MSTR_SHOW_FPS),
        metro_str(MSTR_LIMIT_FPS),
        metro_str(MSTR_SKIP_FRAMES),
        metro_str(MSTR_SCALE_MODE),
#ifdef HAVE_BACKLIGHT_BRIGHTNESS
        metro_str(MSTR_BACKLIGHT_BRIGHTNESS),
#endif
    };
    const char *const yesno[] = { metro_str(MSTR_NO), metro_str(MSTR_YES) };
    const char *const scalemodes[] = { metro_str(MSTR_FIT), metro_str(MSTR_COVER) };
#ifdef HAVE_BACKLIGHT_BRIGHTNESS
    char bright_buf[32];
#endif
    int selected = 0;
    int result;
    bool menu_quit = false;

    while (!menu_quit)
    {
        const char *values[ARRAYLEN(items)];

        values[MPEG_OPTION_DISPLAY_FPS] = yesno[settings.showfps ? 1 : 0];
        values[MPEG_OPTION_LIMIT_FPS] = yesno[settings.limitfps ? 1 : 0];
        values[MPEG_OPTION_SKIP_FRAMES] = yesno[settings.skipframes ? 1 : 0];
        values[MPEG_OPTION_SCALE_MODE] =
            scalemodes[settings.scale_mode == MPEG_SCALE_MODE_COVER ? 1 : 0];
#ifdef HAVE_BACKLIGHT_BRIGHTNESS
        values[MPEG_OPTION_BACKLIGHT_BRIGHTNESS] =
            backlight_brightness_formatter(bright_buf, sizeof(bright_buf),
                                           settings.backlight_brightness, NULL);
#endif

        result = metro_menu_pick(metro_str(MSTR_TITLE_DISPLAY), items, values,
                                 ARRAYLEN(items), selected);
        if (result >= 0)
            selected = result;

        switch (result)
        {
        case MPEG_OPTION_DISPLAY_FPS:
            settings.showfps = !settings.showfps;
            break;

        case MPEG_OPTION_LIMIT_FPS:
            settings.limitfps = !settings.limitfps;
            break;

        case MPEG_OPTION_SKIP_FRAMES:
            settings.skipframes = !settings.skipframes;
            break;

        case MPEG_OPTION_SCALE_MODE:
            settings.scale_mode =
                (settings.scale_mode == MPEG_SCALE_MODE_FIT)
                    ? MPEG_SCALE_MODE_COVER : MPEG_SCALE_MODE_FIT;
            vo_update_scale_mode();
            break;

#ifdef HAVE_BACKLIGHT_BRIGHTNESS
        case MPEG_OPTION_BACKLIGHT_BRIGHTNESS:
        {
            int v = settings.backlight_brightness;
            mpeg_backlight_update_brightness(v);
            metro_menu_adjust_int(metro_str(MSTR_TITLE_BRIGHTNESS), &v, -1,
                                  MAX_BRIGHTNESS_SETTING - MIN_BRIGHTNESS_SETTING,
                                  backlight_brightness_formatter,
                                  backlight_brightness_function);
            settings.backlight_brightness = v;
            mpeg_backlight_update_brightness(-1);
            break;
        }
#endif /* HAVE_BACKLIGHT_BRIGHTNESS */

        default:
            menu_quit = true;
            break;
        }

        if (mpeg_sysevent() != 0)
            menu_quit = true;
    }
}

static void audio_options(void)
{
    const char *const items[] = {
        metro_str(MSTR_TONE_CONTROLS),
        metro_str(MSTR_CHANNEL_MODES),
        metro_str(MSTR_CROSSFEED),
        metro_str(MSTR_EQUALIZER),
        metro_str(MSTR_DITHERING),
    };
    const char *const off_setting[] = {
        metro_str(MSTR_OFF), metro_str(MSTR_USE_SOUND_SETTING)
    };
    int selected = 0;
    int result;
    bool menu_quit = false;

    while (!menu_quit)
    {
        const char *values[ARRAYLEN(items)];

        values[MPEG_AUDIO_TONE_CONTROLS] = off_setting[settings.tone_controls ? 1 : 0];
        values[MPEG_AUDIO_CHANNEL_MODES] = off_setting[settings.channel_modes ? 1 : 0];
        values[MPEG_AUDIO_CROSSFEED] = off_setting[settings.crossfeed ? 1 : 0];
        values[MPEG_AUDIO_EQUALIZER] = off_setting[settings.equalizer ? 1 : 0];
        values[MPEG_AUDIO_DITHERING] = off_setting[settings.dithering ? 1 : 0];

        result = metro_menu_pick(metro_str(MSTR_TITLE_AUDIO), items, values,
                                 ARRAYLEN(items), selected);
        if (result >= 0)
            selected = result;

        switch (result)
        {
        case MPEG_AUDIO_TONE_CONTROLS:
            settings.tone_controls = !settings.tone_controls;
            sync_audio_setting(MPEG_AUDIO_TONE_CONTROLS, false);
            break;

        case MPEG_AUDIO_CHANNEL_MODES:
            settings.channel_modes = !settings.channel_modes;
            sync_audio_setting(MPEG_AUDIO_CHANNEL_MODES, false);
            break;

        case MPEG_AUDIO_CROSSFEED:
            settings.crossfeed = !settings.crossfeed;
            sync_audio_setting(MPEG_AUDIO_CROSSFEED, false);
            break;

        case MPEG_AUDIO_EQUALIZER:
            settings.equalizer = !settings.equalizer;
            sync_audio_setting(MPEG_AUDIO_EQUALIZER, false);
            break;

        case MPEG_AUDIO_DITHERING:
            settings.dithering = !settings.dithering;
            sync_audio_setting(MPEG_AUDIO_DITHERING, false);
            break;

        default:
            menu_quit = true;
            break;
        }

        if (mpeg_sysevent() != 0)
            menu_quit = true;
    }
}

static void clear_resume_count(void)
{
    settings.resume_count = 0;
    configfile_save(SETTINGS_FILENAME, config, ARRAYLEN(config),
                    SETTINGS_VERSION);
}

static void mpeg_settings(void)
{
    const char *const items[] = {
        metro_str(MSTR_DISPLAY_OPTIONS),
        metro_str(MSTR_AUDIO_OPTIONS),
        metro_str(MSTR_PLAY_MODE),
        metro_str(MSTR_CLEAR_RESUMES),
    };
    const char *const single_all[] = { metro_str(MSTR_SINGLE), metro_str(MSTR_ALL) };
    char resume_buf[8];
    int selected = 0;
    int result;
    bool menu_quit = false;

    while (!menu_quit)
    {
        const char *values[ARRAYLEN(items)];

        values[MPEG_SETTING_DISPLAY_SETTINGS] = NULL;
        values[MPEG_SETTING_AUDIO_SETTINGS] = NULL;
        values[MPEG_SETTING_PLAY_MODE] = single_all[settings.play_mode ? 1 : 0];
        /* The stored-resume count doubles as this action's value --
         * it also confirms the action worked (drops to 0 in place). */
        rb->snprintf(resume_buf, sizeof(resume_buf), "%d",
                     settings.resume_count);
        values[MPEG_SETTING_CLEAR_RESUMES] = resume_buf;

        result = metro_menu_pick(metro_str(MSTR_TITLE_SETTINGS), items, values,
                                 ARRAYLEN(items), selected);
        if (result >= 0)
            selected = result;

        switch (result)
        {
        case MPEG_SETTING_DISPLAY_SETTINGS:
            display_options();
            break;

        case MPEG_SETTING_AUDIO_SETTINGS:
            audio_options();
            break;

        case MPEG_SETTING_PLAY_MODE:
            settings.play_mode = !settings.play_mode;
            break;

        case MPEG_SETTING_CLEAR_RESUMES:
            clear_resume_count();
            break;

        default:
            menu_quit = true;
            break;
        }

        if (mpeg_sysevent() != 0)
            menu_quit = true;
    }
}

void init_settings(const char* filename)
{
    /* Set the default settings */
    settings.showfps = 0;     /* Do not show FPS */
    settings.limitfps = 1;    /* Limit FPS */
    settings.skipframes = 1;  /* Skip frames */
    settings.scale_mode = MPEG_SCALE_MODE_FIT; /* Metro (M-059) */
    settings.play_mode = 0;   /* Play single video */
    settings.resume_options = MPEG_RESUME_MENU_ALWAYS; /* unused, see mpeg_start_menu() */
    settings.resume_count = 0;
#ifdef HAVE_BACKLIGHT_BRIGHTNESS
    settings.backlight_brightness = -1; /* Use default setting */
#endif
#if MPEG_OPTION_DITHERING_ENABLED
    settings.displayoptions = 0; /* No visual effects */
#endif
    settings.tone_controls = false;
    settings.channel_modes = false;
    settings.crossfeed = false;
    settings.equalizer = false;
    settings.dithering = false;

    if (configfile_load(SETTINGS_FILENAME, config, ARRAYLEN(config),
                        SETTINGS_MIN_VERSION) < 0)
    {
        /* Generate a new config file with default values */
        configfile_save(SETTINGS_FILENAME, config, ARRAYLEN(config),
                        SETTINGS_VERSION);
    }

    rb->strlcpy(settings.resume_filename, filename, MAX_PATH);

    /* get the resume time for the current mpeg if it exists */
    if ((settings.resume_time = configfile_get_value
         (SETTINGS_FILENAME, filename)) < 0)
    {
        settings.resume_time = 0;
    }

#if MPEG_OPTION_DITHERING_ENABLED
    rb->lcd_yuv_set_options(settings.displayoptions);
#endif

    /* Set our audio options */
    sync_audio_settings(false);
}

void save_settings(void)
{
    unsigned i;
    for (i = 0; i < ARRAYLEN(config); i++)
    {
        configfile_update_entry(SETTINGS_FILENAME, config[i].name,
                                *(config[i].int_p));
    }

    /* If this was a new resume entry then update the total resume count */
    if (configfile_update_entry(SETTINGS_FILENAME, settings.resume_filename,
                                settings.resume_time) == 0)
    {
        configfile_update_entry(SETTINGS_FILENAME, "Resume count",
                                ++settings.resume_count);
    }

    /* Restore audio options */
    sync_audio_settings(true);
}
