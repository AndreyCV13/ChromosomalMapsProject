#ifndef UI_H
#define UI_H
#include <gtk/gtk.h>
#include "logic.h"

// Variables globales UI
extern GtkWidget *scrollWindow, *percentagesWindow, *scrollWindow1, *resultWindow, *mainWindow, *genesEntry, *genesAmount, *viewPort, *characteristicsWindow;
extern GtkGrid *grid, *characteristicsGrid, *resultGrid, *percentagesGrid;
extern GtkBuilder	*builder; 

void buildGrid(int amountOfCombinations);
void open_main_window (GtkButton *btn);
void leave_program (GtkButton *btn);
void on_inserted_char(GtkEntry *entry, gchar *new_text);
void cross_parents (GtkButton *btn);
char* getPhenotype(const char* genotype);

void on_file_set(GtkFileChooserButton *btn, gpointer user_data);
void show_error(const char* message);
void on_errorDialog_response(GtkDialog *dialog, gint response_id, gpointer user_data);
gboolean on_window_delete_event(GtkWidget *widget, GdkEvent *event, gpointer user_data);
#endif