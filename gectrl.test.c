#include "gbdfed.h"
#include "gectrl.h"

gbdfed_options_t options;



int main(int argc, char* argv[])
{
    bdf_bitmap_t image = {
        .bpp = 4,
        0
    };

    options.colors[0] = 0;
    options.colors[1] = 175;
    options.colors[2] = 207;
    options.colors[3] = 239;

    /*
     * Set the default colors for 4 bits per pixel.
     */
    options.colors[4] = 0;
    options.colors[5] = 31;
    options.colors[6] = 63;
    options.colors[7] = 95;
    options.colors[8] = 127;
    options.colors[9] = 159;
    options.colors[10] = 167;
    options.colors[11] = 175;
    options.colors[12] = 183;
    options.colors[13] = 191;
    options.colors[14] = 199;
    options.colors[15] = 207;
    options.colors[16] = 215;
    options.colors[17] = 223;
    options.colors[18] = 231;
    options.colors[19] = 239;

    bdf_setup();

    gtk_init(&argc, &argv);

    GtkWidget * window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    GtkWidget * gectrl = gecontrol_newv(gtk_label_new(""), &image, options.colors);
    gtk_container_add(GTK_CONTAINER(window), gectrl);

    gtk_widget_show_all(window);

    gtk_main();

    return 0;

}
