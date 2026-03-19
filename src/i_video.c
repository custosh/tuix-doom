// i_video.c - Doom video API implemented using TUIX renderer

#include <stdlib.h>
#include <string.h>

#include "doomtype.h"
#include "i_video.h"
#include "i_system.h"
#include "v_video.h"
#include "v_diskicon.h"
#include "tables.h"
#include "w_wad.h"
#include "z_zone.h"

#include "i_video_tuix.h"

static TuixFinalBuffer tuix_buffer;
static TuixPixel *tuix_pixels;

static int initialized = 0;
static int usegamma = 0;

static unsigned char palette[256][3];

//
// Convert Doom framebuffer → TUIX pixels
//

static void ConvertFrame(void)
{
    int size = SCREENWIDTH * SCREENHEIGHT;

    for (int i = 0; i < size; i++)
    {
        int p = I_VideoBuffer[i];

        TuixRGBTuple c;

        c.r = palette[p][0];
        c.g = palette[p][1];
        c.b = palette[p][2];

        tuix_pixels[i].styles.bg = c;
        tuix_pixels[i].styles.fg = c;
        tuix_pixels[i].sym[0] = ' ';
        tuix_pixels[i].sym[1] = '\0';
    }
}

//
// Init
//

void I_InitGraphics(void)
{
    if (initialized)
        return;

    printf("Initializing TUIX video...\n");
    tuix_pixels = calloc(SCREENWIDTH * SCREENHEIGHT, sizeof(TuixPixel));

    tuix_buffer.width = SCREENWIDTH;
    tuix_buffer.height = SCREENHEIGHT;
    tuix_buffer.pixels = tuix_pixels;
    tuix_buffer.full_redraw = 1;

    I_VideoBuffer = malloc(SCREENWIDTH * SCREENHEIGHT);

    memset(I_VideoBuffer, 0, SCREENWIDTH * SCREENHEIGHT);

    byte *doompal = W_CacheLumpName("PLAYPAL", PU_CACHE);
    I_SetPalette(doompal);

    initialized = 1;

    I_AtExit(I_ShutdownGraphics, true);
}

//
// Shutdown
//

void I_ShutdownGraphics(void)
{
    if (!initialized)
        return;

    free(I_VideoBuffer);
    free(tuix_pixels);

    initialized = 0;
}

//
// Frame begin
//

void I_StartFrame(void)
{
}

//
// Finish update
//

void I_FinishUpdate(void)
{
    if (!initialized)
        return;

    V_DrawDiskIcon();

    ConvertFrame();

    tuix_render_streaming(&tuix_buffer, NULL, NULL);

    V_RestoreDiskBackground();
}

//
// NoBlit
//

void I_UpdateNoBlit(void)
{
}

//
// Start tic
//

// Start tic: implementation provided by input module (avoid duplicate symbol)

//
// Read screen
//

void I_ReadScreen(pixel_t *scr)
{
    memcpy(scr, I_VideoBuffer, SCREENWIDTH * SCREENHEIGHT);
}

//
// Palette
//

void I_SetPalette(byte *doompalette)
{
    for (int i = 0; i < 256; i++)
    {
        palette[i][0] = gammatable[usegamma][*doompalette++] & ~3;
        palette[i][1] = gammatable[usegamma][*doompalette++] & ~3;
        palette[i][2] = gammatable[usegamma][*doompalette++] & ~3;
    }

    tuix_buffer.full_redraw = 1;
}

//
// Palette index lookup
//

int I_GetPaletteIndex(int r, int g, int b)
{
    int best = 0;
    int bestdiff = 0x7fffffff;

    for (int i = 0; i < 256; i++)
    {
        int dr = r - palette[i][0];
        int dg = g - palette[i][1];
        int db = b - palette[i][2];

        int diff = dr*dr + dg*dg + db*db;

        if (diff < bestdiff)
        {
            bestdiff = diff;
            best = i;
        }
    }

    return best;
}

//
// Stubs required by Doom
//

void I_DisplayFPSDots(boolean dots_on) {}
void I_SetWindowTitle(const char *title) {}
void I_InitWindowTitle(void) {}
void I_RegisterWindowIcon(const unsigned int *icon, int w, int h) {}
void I_InitWindowIcon(void) {}
void I_GraphicsCheckCommandLine(void) {}
void I_CheckIsScreensaver(void) {}
void I_BindVideoVariables(void) {}