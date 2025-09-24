/*
Proyecto0
Interfaz Grafica Principal
*/

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <time.h>
#include <fcntl.h>
#include <math.h>
#include "src/UI.h"
#include "src/logic.h"

//Variables globales de GTK
GtkWidget *mainWindow, *genesEntry, *genesAmount, *viewPort, 
*errorDialog, *characteristicsWindow, *scrollWindow, *resultWindow, 
*percentagesWindow, *scrollWindow1, *fillBox;
GtkGrid *grid, *characteristicsGrid, *resultGrid, *percentagesGrid;
GtkBuilder	*builder; 

void buildComponents(){
	//getBuilder
	builder = gtk_builder_new_from_file ("proyecto1.glade");

	//getCSS
	GtkCssProvider *provider = gtk_css_provider_new();
	gtk_css_provider_load_from_path(provider, "styles/styles.css", NULL);

	GdkDisplay *display = gdk_display_get_default();
	GdkScreen *screen = gdk_display_get_default_screen(display);

	gtk_style_context_add_provider_for_screen(
		screen,
		GTK_STYLE_PROVIDER(provider),
		GTK_STYLE_PROVIDER_PRIORITY_USER
	);

	//getComponents
	mainWindow = GTK_WIDGET(gtk_builder_get_object(builder, "mainWindow"));
	genesEntry = GTK_WIDGET(gtk_builder_get_object(builder, "propertyEntry"));
	genesAmount = GTK_WIDGET(gtk_builder_get_object(builder, "genesAmount"));
	fillBox = GTK_WIDGET(gtk_builder_get_object(builder, "fillBox"));
	characteristicsWindow = GTK_WIDGET(gtk_builder_get_object(builder, "characteristicsWindow"));
	characteristicsGrid = GTK_GRID(gtk_builder_get_object(builder, "characteristicsGrid"));
	viewPort = GTK_WIDGET(gtk_builder_get_object(builder, "view"));
	grid = GTK_GRID(gtk_builder_get_object(builder, "grid"));
	resultGrid = GTK_GRID(gtk_builder_get_object(builder, "resultGrid"));
	percentagesGrid = GTK_GRID(gtk_builder_get_object(builder, "percentagesGrid"));
	errorDialog = GTK_WIDGET(gtk_builder_get_object(builder, "errorDialog"));
	resultWindow = GTK_WIDGET(gtk_builder_get_object(builder, "resultWindow"));
	scrollWindow = GTK_WIDGET(gtk_builder_get_object(builder, "scrollWindow"));
	scrollWindow1 = GTK_WIDGET(gtk_builder_get_object(builder, "scrollWindow1"));
	percentagesWindow = GTK_WIDGET(gtk_builder_get_object(builder, "percentagesWindow"));


	g_signal_connect(characteristicsWindow, "delete-event", G_CALLBACK(on_window_delete_event), NULL);
	g_signal_connect(genesEntry, "delete-event", G_CALLBACK(on_window_delete_event), NULL);

}

int main(int argc, char *argv[]) { 

	gtk_init(&argc, &argv); // init Gtk

	buildComponents();

	gtk_window_set_title(GTK_WINDOW(mainWindow), "BMC");
	gtk_window_set_title(GTK_WINDOW(resultWindow), "Crossing");

	g_signal_connect(mainWindow, "destroy", G_CALLBACK(gtk_main_quit), NULL);
	g_signal_connect(resultWindow, "delete-event", G_CALLBACK(gtk_widget_hide_on_delete), NULL);
	g_signal_connect(percentagesWindow, "delete-event", G_CALLBACK(gtk_widget_hide_on_delete), NULL);

    gtk_builder_connect_signals(builder, NULL);

    g_object_unref(builder);

	gtk_widget_show_all(mainWindow);

	gtk_main();

	return 0;
}

