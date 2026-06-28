#include <gtk/gtk.h>
#include <time.h>
#include <stdint.h>
#include <stdio.h>

#define DYSNOMIA_ZERO 638403877000000000ULL
#define TICKS_PER_DAY (86400ULL * 10000000ULL)
#define TICKS_PER_HOUR (TICKS_PER_DAY / 34ULL)
#define TICKS_PER_MINUTE (TICKS_PER_HOUR / 100ULL)
#define TICKS_PER_SECOND (TICKS_PER_MINUTE / 34ULL)
#define NET_UNIX_EPOCH_TICKS 621355968000000000ULL

GtkWidget *label_time;

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

static gboolean update_time(gpointer data) {
    char time_str[64];
    get_dysnomia_time(time_str);

    char markup[256];
    sprintf(markup, "<span font_desc='Consolas Bold 11' foreground='DeepPink'>%s</span>", time_str);
    gtk_label_set_markup(GTK_LABEL(label_time), markup);
    return TRUE;
}

static gboolean draw_callback(GtkWidget *widget, cairo_t *cr, gpointer data) {
    cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 0.0);
    cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
    cairo_paint(cr);
    return FALSE;
}

static void realize_callback(GtkWidget *widget, gpointer data) {
    GdkWindow *gdk_win = gtk_widget_get_window(widget);
    if (gdk_win) {
        // Essential: Allow mouse clicks to pass through the fullscreen overlay window
        gdk_window_set_pass_through(gdk_win, TRUE);
    }
}

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Dysnomia Time Overlay");
    
    // Set borderless, input bypass hints
    gtk_window_set_decorated(GTK_WINDOW(window), FALSE);
    gtk_window_set_skip_taskbar_hint(GTK_WINDOW(window), TRUE);
    gtk_window_set_skip_pager_hint(GTK_WINDOW(window), TRUE);
    gtk_window_set_keep_above(GTK_WINDOW(window), TRUE);
    gtk_window_set_accept_focus(GTK_WINDOW(window), FALSE);

    // Fullscreen forces the window to occupy the entire monitor and strips decorations
    gtk_window_fullscreen(GTK_WINDOW(window));

    // CSS styling override to ensure window container has no visual padding or border
    GdkScreen *screen = gtk_widget_get_screen(window);
    GtkCssProvider *css_provider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(css_provider,
        "window, decoration, grid { background: transparent; background-color: transparent; border: none; box-shadow: none; margin: 0; padding: 0; }\n",
        -1, NULL);
    gtk_style_context_add_provider_for_screen(screen,
        GTK_STYLE_PROVIDER(css_provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    // Transparent window visual setup
    GdkVisual *visual = gdk_screen_get_rgba_visual(screen);
    if (visual) {
        gtk_widget_set_visual(window, visual);
    }
    gtk_widget_set_app_paintable(window, TRUE);
    
    g_signal_connect(G_OBJECT(window), "draw", G_CALLBACK(draw_callback), NULL);
    g_signal_connect(G_OBJECT(window), "realize", G_CALLBACK(realize_callback), NULL);

    // Grid occupying fullscreen
    GtkWidget *grid = gtk_grid_new();
    gtk_widget_set_hexpand(grid, TRUE);
    gtk_widget_set_vexpand(grid, TRUE);
    gtk_container_add(GTK_CONTAINER(window), grid);

    label_time = gtk_label_new(NULL);
    // Align clock to the absolute bottom-left corner of the monitor
    gtk_widget_set_halign(label_time, GTK_ALIGN_START);
    gtk_widget_set_valign(label_time, GTK_ALIGN_END);
    // Margin of 16px from left and bottom screen borders
    gtk_widget_set_margin_start(label_time, 16);
    gtk_widget_set_margin_bottom(label_time, 16);
    
    gtk_grid_attach(GTK_GRID(grid), label_time, 0, 0, 1, 1);

    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    update_time(NULL);
    g_timeout_add(111, update_time, NULL);

    gtk_widget_show_all(window);
    gtk_main();

    return 0;
}
