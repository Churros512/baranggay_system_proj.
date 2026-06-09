#include <gtk/gtk.h>
#include "database.h"
#include "gui.h"

int main(int argc, char* argv[])
{
    initializeDatabase();
    gtk_init(&argc, &argv);
    show_gui();
    gtk_main();
    return 0;
}
