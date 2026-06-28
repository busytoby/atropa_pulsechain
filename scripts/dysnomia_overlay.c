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

    // Output only the [dXXXX/HHMMSS] format without the prefix space
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
        // Enable input events pass-through (click-through overlay)
        gdk_window_set_pass_through(gdk_win, TRUE);
    }
}

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Dysnomia Time Overlay");
    gtk_window_set_decorated(GTK_WINDOW(window), FALSE);
    gtk_window_set_skip_taskbar_hint(GTK_WINDOW(window), TRUE);
    gtk_window_set_keep_above(GTK_WINDOW(window), TRUE);
    
    // Set as a Dock type window and refuse focus for desktop overlay behavior
    gtk_window_set_type_hint(GTK_WINDOW(window), GDK_WINDOW_TYPE_HINT_DOCK);
    gtk_window_set_accept_focus(GTK_WINDOW(window), FALSE);

    // Support transparency
    GdkScreen *screen = gtk_widget_get_screen(window);
    GdkVisual *visual = gdk_screen_get_rgba_visual(screen);
    if (visual) {
        gtk_widget_set_visual(window, visual);
    }
    gtk_widget_set_app_paintable(window, TRUE);
    
    g_signal_connect(G_OBJECT(window), "draw", G_CALLBACK(draw_callback), NULL);
    g_signal_connect(G_OBJECT(window), "realize", G_CALLBACK(realize_callback), NULL);

    // Layout
    GtkWidget *grid = gtk_grid_new();
    gtk_widget_set_valign(grid, GTK_ALIGN_CENTER);
    gtk_container_add(GTK_CONTAINER(window), grid);

    label_time = gtk_label_new(NULL);
    // Align to the left (start) of the monitor
    gtk_widget_set_hexpand(label_time, TRUE);
    gtk_widget_set_halign(label_time, GTK_ALIGN_START);
    // Margin of 16px to prevent touching the absolute left screen border
    gtk_widget_set_margin_start(label_time, 16);
    gtk_grid_attach(GTK_GRID(grid), label_time, 0, 0, 1, 1);

    // Size hint to stretch across screen at the bottom
    gtk_window_set_default_size(GTK_WINDOW(window), 1920, 22);

    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    update_time(NULL);
    g_timeout_add(111, update_time, NULL);

    gtk_widget_show_all(window);
    gtk_main();

    return 0;
}
