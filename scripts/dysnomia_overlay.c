#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xft/Xft.h>
#include <time.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

#define DYSNOMIA_ZERO 638403877000000000ULL
#define TICKS_PER_DAY (86400ULL * 10000000ULL)
#define TICKS_PER_HOUR (TICKS_PER_DAY / 34ULL)
#define TICKS_PER_MINUTE (TICKS_PER_HOUR / 100ULL)
#define TICKS_PER_SECOND (TICKS_PER_MINUTE / 34ULL)
#define NET_UNIX_EPOCH_TICKS 621355968000000000ULL

void get_dysnomia_time(char *out_time) {
    uint64_t current_ticks = (uint64_t)time(NULL) * 10000000ULL + NET_UNIX_EPOCH_TICKS;
    int64_t ticks = (int64_t)current_ticks - DYSNOMIA_ZERO;
    if (ticks < 0) ticks = 0;

    int day = ticks / TICKS_PER_DAY;
    int hour = (ticks % TICKS_PER_DAY) / TICKS_PER_HOUR;
    int minute = ((ticks % TICKS_PER_DAY) % TICKS_PER_HOUR) / TICKS_PER_MINUTE;
    int second = (((ticks % TICKS_PER_DAY) % TICKS_PER_HOUR) % TICKS_PER_MINUTE) / TICKS_PER_SECOND;

    sprintf(out_time, "[d%04d/%02d%02d%02d]", day, hour, minute, second);
}

int main(int argc, char *argv[]) {
    Display *dpy = XOpenDisplay(NULL);
    if (!dpy) {
        fprintf(stderr, "Error: Cannot open X display\n");
        return 1;
    }

    int screen = DefaultScreen(dpy);
    int screen_width = DisplayWidth(dpy, screen);
    int screen_height = DisplayHeight(dpy, screen);

    // Locate 32-bit TrueColor visual for transparent background
    XVisualInfo vinfo;
    if (!XMatchVisualInfo(dpy, screen, 32, TrueColor, &vinfo)) {
        fprintf(stderr, "Error: No 32-bit ARGB visual found\n");
        return 1;
    }

    Colormap cmap = XCreateColormap(dpy, RootWindow(dpy, screen), vinfo.visual, AllocNone);

    // Window dimensions and positioning (bottom left of monitor)
    int width = 160;
    int height = 24;
    int x = 16;
    int y = screen_height - height - 16;

    XSetWindowAttributes attrs;
    attrs.colormap = cmap;
    attrs.background_pixel = 0; // Fully transparent background
    attrs.border_pixel = 0;
    attrs.override_redirect = True; // Tells WM to skip decorating or positioning this window

    Window win = XCreateWindow(
        dpy, RootWindow(dpy, screen),
        x, y, width, height, 0,
        vinfo.depth, InputOutput, vinfo.visual,
        CWColormap | CWBackPixel | CWBorderPixel | CWOverrideRedirect, &attrs
    );

    // Setup Xft fonts and colors (DeepPink = #FF1493)
    XftFont *font = XftFontOpenName(dpy, screen, "monospace:size=11:bold");
    XRenderColor xr_color;
    xr_color.red = 0xFFFF;
    xr_color.green = 0x1414;
    xr_color.blue = 0x9393;
    xr_color.alpha = 0xFFFF;
    XftColor color;
    XftColorAllocValue(dpy, vinfo.visual, cmap, &xr_color, &color);

    XftDraw *draw = XftDrawCreate(dpy, win, vinfo.visual, cmap);

    XMapWindow(dpy, win);
    XFlush(dpy);

    char time_str[64];
    while (1) {
        // Handle pending X events (clicks, etc. - though we pass them through)
        while (XPending(dpy)) {
            XEvent ev;
            XNextEvent(dpy, &ev);
        }

        // Clear display area
        XClearWindow(dpy, win);

        // Fetch current Dysnomia time and draw text
        get_dysnomia_time(time_str);
        XftDrawStringUtf8(draw, &color, font, 0, 16, (XftChar8 *)time_str, strlen(time_str));
        XFlush(dpy);

        // Tick every 111ms
        usleep(111000);
    }

    // Cleanup (unreached)
    XftDrawDestroy(draw);
    XftColorFree(dpy, vinfo.visual, cmap, &color);
    XftFontClose(dpy, font);
    XDestroyWindow(dpy, win);
    XCloseDisplay(dpy);

    return 0;
}
