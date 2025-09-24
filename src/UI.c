#include "UI.h"
#include "logic.h"
#include <math.h>
#include "file_manager.h"

#define SIZE 1

typedef struct {
    char *key;
    gint value;
} Entry;

fileContent* currentContent = NULL;
property *currentProperties;
int currentPropertiesCount;
extern GtkWidget *errorDialog;

GtkWidget *group, *group2;
GtkCssProvider* dynamicProvider;
char **children;
int totalColumns, totalRows;

void setAdjustments(GtkWidget *widget, GtkAlign value, int column, int row){
	gtk_widget_set_halign(widget, value);
	gtk_grid_attach(grid, widget, column, row, 1, 1);
}

// Construye las 2 colecciones
void buildGrid(int amountOfCombinations){

    // Limpiar todos los hijos del grid antes de agregar nuevos
    GList *children, *iter;
    children = gtk_container_get_children(GTK_CONTAINER(grid));
    for (iter = children; iter != NULL; iter = g_list_next(iter)) {
        gtk_widget_destroy(GTK_WIDGET(iter->data));
    }
    g_list_free(children);

	GtkWidget *label, *label2;
	int row;

	/**/
	row = 0;
	while (row < amountOfCombinations) {
		label = gtk_label_new(allCombinations[row]);
		label2 = gtk_label_new(allCombinations[row]);
        
        GtkWidget *radio1, *radio2;
        if(row == 0){
            radio1 = gtk_radio_button_new(NULL);
            radio2 = gtk_radio_button_new(NULL);
            group = radio1;
            group2 = radio2;
        } else{
            radio1 = gtk_radio_button_new_from_widget(GTK_RADIO_BUTTON(group));
            radio2 = gtk_radio_button_new_from_widget(GTK_RADIO_BUTTON(group2));
        }

        // Aplica clase CSS diferente a cada grupo
        GtkStyleContext *context1 = gtk_widget_get_style_context(radio1);
        gtk_style_context_add_class(context1, "radio-parent1");
        GtkStyleContext *context2 = gtk_widget_get_style_context(radio2);
        gtk_style_context_add_class(context2, "radio-parent2");

        // Fenotype Tooltip
        char tooltip[256] = "";
        for (int j = 0; j < currentPropertiesCount; j++) {
            char dom = allCombinations[row][j*2];
            char rec = allCombinations[row][j*2+1];
            //printf("Genotype %d (%c%c): dom=%s, rec=%s\n", j, dom, rec, currentProperties[j].dominant, currentProperties[j].recessive);

            if (dom == currentProperties[j].upperCase || rec == currentProperties[j].upperCase) {
                strcat(tooltip, currentProperties[j].dominant);
            } else {
                strcat(tooltip, currentProperties[j].recessive);
            }
            if (j < currentPropertiesCount - 1) strcat(tooltip, " / ");
        }
        gtk_widget_set_tooltip_text(radio1, tooltip);
        gtk_widget_set_tooltip_text(radio2, tooltip);

		//Adjustment for the scrollView
		gtk_widget_set_margin_start(label, 10);
		gtk_widget_set_hexpand(label2, TRUE);

		//Some adjustments and attach it to the grid
		setAdjustments(label, GTK_ALIGN_START, 0, row);
		setAdjustments(radio1, GTK_ALIGN_START, 1, row);
		setAdjustments(label2, GTK_ALIGN_END, 2, row);
		setAdjustments(radio2, GTK_ALIGN_END, 3, row);

		row++;
		//gtk_style_context_add_class(gtk_widget_get_style_context(group), "radio-custom");

	}
	/**/
	gtk_widget_show_all(GTK_WIDGET(grid));
}

//Esta funcion obtiene la fila/columna del boton seleccionado y devuelve el label correspondiente
GtkWidget* obtainSelectedRadioButton(GtkWidget *radioButton){
	//Obtiene el grupo al que pertenece la referencia que tenemos (El ultimo boton)
	GSList *group = gtk_radio_button_get_group(GTK_RADIO_BUTTON(radioButton)); 
	GSList *iter;

	for (iter = group; iter != NULL; iter = iter->next) {
		GtkWidget *radioButton = GTK_WIDGET(iter->data);
		if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radioButton))) {
			gint row, col;

			gtk_container_child_get(GTK_CONTAINER(grid), radioButton,
									"top-attach", &row,
									"left-attach", &col,
									NULL);
			return gtk_grid_get_child_at(grid, col - 1, row);
		}
	}

}

void on_inserted_char1(GtkEntry *entry, gchar *new_text) {
    const gchar *current_text = gtk_entry_get_text(entry);

    if (!g_ascii_isdigit(new_text[0]) && new_text[0] != ',') {
        g_signal_stop_emission_by_name(entry, "insert-text");
        return;
    }

    // Si es un punto, asegurarnos de que no haya ya uno en el texto
    if (new_text[0] == ',') {
        if (strchr(current_text, ',') != NULL) {
            // Ya hay un punto
            g_signal_stop_emission_by_name(entry, "insert-text");
            return;
        }
    }
}

void clearContainer(GtkContainer *container) {
    gtk_container_foreach(container, (GtkCallback)gtk_widget_destroy, NULL);
}

void clearGrid(GtkGrid *grid) {
    gtk_container_foreach(GTK_CONTAINER(grid), (GtkCallback)gtk_widget_destroy, NULL);
}

//Inserta a los padres/alelos dentro del grid
void insertParents (char** firstAlleles, char** secondAlleles, int amountOfAlleles){
	for (int i = 0; i < amountOfAlleles; i++){
		char *individualAllele = firstAlleles[i];
		char *individualAllele2 = secondAlleles[i];

		//Se crean los labels y se insertan en la posicion correcta
		if(individualAllele[0] != False){
			GtkWidget *newLabel = gtk_label_new(individualAllele);
			gtk_grid_attach(resultGrid, newLabel, i+1, 0, SIZE, SIZE);
			gtk_widget_set_hexpand(newLabel, TRUE);
		} 
		if(individualAllele2[0] != False){
			GtkWidget *newLabel2 = gtk_label_new(individualAllele2);
			gtk_grid_attach(resultGrid, newLabel2, 0, i+1, SIZE, SIZE);
			gtk_widget_set_vexpand(newLabel2, TRUE);
		} 
		
	}
}

//Inserta a los hijos dentro del grid
void insertChildren(){
	int childrenAmount = totalColumns * totalRows;

	PangoFontDescription *font_desc = pango_font_description_from_string("Sans 6");

	for (int row = 0; row < totalRows; row++){
		for (int column = 0; column < totalColumns; column++){
			//Este indice es la representacion de una matriz en una lista
			int matrixIndex = row * totalColumns + column;
			char *genotype = children[matrixIndex];

			//Se crean los labels y se insertan en la posicion correcta
			GtkWidget *newLabel = gtk_label_new(genotype);
            GtkStyleContext *context = gtk_widget_get_style_context(newLabel);

            // Obtener el fenotipo a partir del genotipo
            char *phenotype = getPhenotype(genotype);
            gtk_widget_set_tooltip_text(newLabel, phenotype);

			//Se busca la clase correspondiente segun el id
			gchar *indexColor = g_strdup_printf("cell-%d", matrixIndex);
			gtk_style_context_add_class(context, indexColor);
			gtk_style_context_add_class(context, "texto");

			//Se aplica estilo tambien para el tamano de letra
			double fontSize = log2((double) totalColumns);
			gchar *indexSize = g_strdup_printf("texto%d", (int) fontSize);
			gtk_style_context_add_class(context, indexSize);

			gtk_grid_attach(resultGrid, newLabel, column + 1, row + 1, SIZE, SIZE);

            g_free(phenotype);

		}
	}

	pango_font_description_free(font_desc);
}


//Procedimiento que genera un CSS dinamico segun el modo seleccionado
void generateColorsCSS(int mode) {
	if (!dynamicProvider) {
        dynamicProvider = gtk_css_provider_new();
        gtk_style_context_add_provider_for_screen(
            gdk_screen_get_default(),
            GTK_STYLE_PROVIDER(dynamicProvider),
            GTK_STYLE_PROVIDER_PRIORITY_USER
        );
    }
	gtk_css_provider_load_from_data(dynamicProvider, "", -1, NULL);
    GString *css = g_string_new("");

    for (int row = 0; row < totalRows; row++) {
        for (int column = 0; column < totalColumns; column++) {
            int matrixIndex = row * totalColumns + column;
            char *genotype = children[matrixIndex];

            // Buscar el color correspondiente
			GdkRGBA *newColor;
			if(mode == True){
				newColor = g_hash_table_lookup(colorMapGenotype, genotype);
			} else{
				char *phenotype = genotypeToPhenotype(genotype);
				newColor = g_hash_table_lookup(colorMapPhenotype, phenotype);
			}
            
            
            gchar *className = g_strdup_printf(".cell-%d", matrixIndex);
            gchar *color = gdk_rgba_to_string(newColor);
            g_string_append_printf(css,
                "%s {\n"
                "  background-color: %s;\n"
                "}\n",
                className, color);

            g_free(className);
            g_free(color);
        }
    }

    // Cargar CSS en el provider
    gtk_css_provider_load_from_data(dynamicProvider, css->str, -1, NULL);

	g_string_free(css, TRUE);

}

// Función para cruzar los progenitores al presionar el boton
void cross_parents (GtkButton *btn){
	clearContainer(GTK_CONTAINER(resultGrid));
    gtk_widget_show_all(resultWindow);

	//Se obtiene el label en relacion al boton seleccionado
	GtkWidget *label1 = obtainSelectedRadioButton(group);
	GtkWidget *label2 = obtainSelectedRadioButton(group2);

	const gchar *text1 = gtk_label_get_text(GTK_LABEL(label1));
	const gchar *text2 = gtk_label_get_text(GTK_LABEL(label2));
	
	//Se obtienen todos los alelos de ambos progenitores
	char** firstAlleles = calculateAlleles(text1);
	char** secondAlleles = calculateAlleles(text2);

	int amountOfAlleles = pow(PROPERTYLENGTH, (double) amountOfGenes);
	insertParents(firstAlleles, secondAlleles, amountOfAlleles);

	//Se reciben los hijos en este string
	children = calculateChildren(firstAlleles, secondAlleles);

	//Aritmetica para insertar hijos en la tabla
	totalColumns = calculateArrayElements(firstAlleles, amountOfAlleles);
	totalRows = calculateArrayElements(secondAlleles, amountOfAlleles);

	//Generar colores para cada genotipo
	insertChildren();

	generateColorsCSS(True);

	gtk_widget_show_all(GTK_WIDGET(resultGrid));

	//Se libera la memoria
	freeMemory(firstAlleles, amountOfAlleles);
	freeMemory(secondAlleles, amountOfAlleles);
}

void collect_entries(gpointer key, gpointer value, gpointer user_data) {
    Entry entry;
    entry.key = (char *) key;
    entry.value = *(gint *) value;
    g_array_append_val((GArray *) user_data, entry);
}

//Comparacion entre entries
gint compare_entries_desc(gconstpointer a, gconstpointer b) {
    const Entry *entryA = a;
    const Entry *entryB = b;
    return entryB->value - entryA->value;
}

double obtainPercentage(int amount) {
    int totalAmount = totalColumns * totalRows;
    return ((double) amount / totalAmount) * 100;
}

void addToPercentageColumn(GArray *array, int i, int column){
	Entry entry = g_array_index(array, Entry, i);
	double percentage = obtainPercentage(entry.value);
	gchar *text = g_strdup_printf("%s: %.2f%%", entry.key, percentage);
	GtkWidget *newLabel = gtk_label_new(text);
	gtk_grid_attach(percentagesGrid, newLabel, column, i + 1, SIZE, SIZE);
}

void remove_widget(GtkWidget *widget, gpointer data) {
    gtk_widget_destroy(widget);
}

void insertPercentages(){
	clearContainer(GTK_CONTAINER(percentagesGrid));

	GArray *entriesGenotypes = g_array_new(FALSE, FALSE, sizeof(Entry));
	GArray *entriesPhenotypes = g_array_new(FALSE, FALSE, sizeof(Entry));

	g_hash_table_foreach(counterMapGenotype, collect_entries, entriesGenotypes);
	g_hash_table_foreach(counterMapPhenotype, collect_entries, entriesPhenotypes);

	g_array_sort(entriesGenotypes, compare_entries_desc);
	g_array_sort(entriesPhenotypes, compare_entries_desc);

	for (int i = 0; i < entriesGenotypes->len; i++){
		addToPercentageColumn(entriesGenotypes, i, 0);
	}
	for (int i = 0; i < entriesPhenotypes->len; i++){
		addToPercentageColumn(entriesPhenotypes, i, 1);
	}

	//Insertar los titulos
	GtkWidget *genotypesLabel = gtk_label_new("Genotypes:");
	GtkWidget *phenotypesLabel = gtk_label_new("Phenotypes:");
	gtk_grid_attach(percentagesGrid, genotypesLabel, 0, 0, SIZE, SIZE);
	gtk_grid_attach(percentagesGrid, phenotypesLabel, 1, 0, SIZE, SIZE);
	gtk_widget_set_hexpand(genotypesLabel, TRUE);
	gtk_widget_set_hexpand(phenotypesLabel, TRUE);

	gtk_widget_show_all(GTK_WIDGET(resultGrid));
}

void free_properties(char** props, int count) {
    if (!props) return;

    for (int i = 0; i < count; i++) {
        free(props[i]);
    }
    free(props);
}

// Callback para actualizar la minúscula cuando cambia la mayúscula
void on_upper_entry_changed(GtkEntry *entry, gpointer user_data) {
    GtkEntry *lowerEntry = GTK_ENTRY(user_data);
    const char *text = gtk_entry_get_text(entry);
    if (text[0] >= 'A' && text[0] <= 'Z') {
        char lower[2] = { (char)(text[0] + 32), '\0' };
        gtk_entry_set_text(lowerEntry, lower);
    }
}

// Callback para actualizar la mayúscula cuando cambia la minúscula
void on_lower_entry_changed(GtkEntry *entry, gpointer user_data) {
    GtkEntry *upperEntry = GTK_ENTRY(user_data);
    const char *text = gtk_entry_get_text(entry);
    if (text[0] >= 'a' && text[0] <= 'z') {
        char upper[2] = { (char)(text[0] - 32), '\0' };
        gtk_entry_set_text(upperEntry, upper);
    }
}

// Limpiar grid
void cleanGrid(GtkGrid *grid) {
    GList *children, *iter;
    children = gtk_container_get_children(GTK_CONTAINER(grid));
    for (iter = children; iter != NULL; iter = g_list_next(iter)) {
        gtk_widget_destroy(GTK_WIDGET(iter->data));
    }
    g_list_free(children);
}

//Procedimiento para consistencia de nombres entre filas y columnas
void on_name_changed(GtkEntry *entry, gpointer user_data) {
    GtkEntry *oppositeEntry = GTK_ENTRY(user_data);
    const char *text = gtk_entry_get_text(entry);
    const char *oppositeText = gtk_entry_get_text(oppositeEntry);

    if (g_strcmp0(text, oppositeText) != 0) {
        gtk_entry_set_text(oppositeEntry, text);
    }
}

void addStyleToEntry(GtkWidget* entry){
    //Estilo para texto
    GtkStyleContext *context = gtk_widget_get_style_context(entry);
    gtk_style_context_add_class(context, "texto");

    //Tamano de los entries
    gtk_entry_set_alignment(GTK_ENTRY(entry), 0.5);
    gtk_entry_set_width_chars(GTK_ENTRY(entry), 6);
    gtk_entry_set_max_length(GTK_ENTRY(entry), 5);
    gtk_widget_set_size_request(entry, 5, 5);  
}

void createGenesInput(int n, int totalDigits) {
    // Limpiar el grid
    clearGrid(grid);

    //Primero creamos los genes
    for (int i = 0; i < n; i++){
        // Columnas
        GtkWidget *columnGen = gtk_entry_new();
        GtkWidget *rowGen = gtk_entry_new();

        char defaultName[16];
        sprintf(defaultName, "G%0*d", totalDigits, i);

        gtk_entry_set_text(GTK_ENTRY(columnGen), defaultName);
        gtk_entry_set_text(GTK_ENTRY(rowGen), defaultName);

        //Estilos y alineamiento
        addStyleToEntry(columnGen);
        addStyleToEntry(rowGen);

        gtk_grid_attach(grid, columnGen, i+1, 0, 1, 1);
        gtk_grid_attach(grid, rowGen, 0, i+1, 1, 1);

        //Senales para revisar cambios
        g_signal_connect(columnGen, "changed", G_CALLBACK(on_name_changed), rowGen);
        g_signal_connect(rowGen, "changed", G_CALLBACK(on_name_changed), columnGen);
    }

    //Despues creamos los entries de probabilidades
    char defaultValue[4] = {"0.00"};
    for (int i = 1; i < n+1; i++){
        for (int j = 1; j < n+1; j++){
            GtkWidget *entry = gtk_entry_new();

            if (i >= j){
                gtk_widget_set_sensitive(entry, FALSE);
                gtk_entry_set_text(GTK_ENTRY(entry), "----");
                if (i == j)
                    gtk_entry_set_text(GTK_ENTRY(entry), defaultValue);
            }

            //Estilos y alineamiento
            addStyleToEntry(entry);
            gtk_grid_attach(grid, entry, j, i, 1, 1);

            //Senal para no permitir caracteres
            g_signal_connect(entry, "insert-text", G_CALLBACK(on_inserted_char1), NULL);
        }
    }
    gtk_widget_show_all(GTK_WIDGET(grid));
}

void accept_genes(GtkButton *btn){
	gtk_widget_hide(genesEntry);

    // Obtener la cantidad de propiedades
	const gchar *text = gtk_entry_get_text(GTK_ENTRY(genesAmount));
	int amount = atoi(text);

	if (amount > 0) {
		amountOfGenes = amount;

        //Hacemos el calculo para no repetirlo despues
        int totalDigits = countDigits(amountOfGenes);
        
        // Crear entradas para las características
        createGenesInput(amountOfGenes, totalDigits);
	} else {
		show_error("The total amount of genes has to be higher than 0.");
	}

}

//Esta funcion convierte el valor de un entry en un double
double getValue(GtkEntry* entry, gboolean fill) {
    const gchar* text = gtk_entry_get_text(entry);

	//En el caso de que un espacio se deje en blanco
	if (strcmp(text, "") == 0){
		return INPUTERROR;
	}

    char* endptr;
    errno = 0;
    double value = strtod(text, &endptr);

    if (endptr == text || errno == ERANGE) {
        return INPUTERROR;
    }

    return value;
}

void fillGridTable(double** dataMatrix){
    // Escribir las probabilidades de los genes
    for (int i = 1; i <= amountOfGenes; i++) {
        for (int j = i+1; j <= amountOfGenes; j++) {
            GtkEntry *entry = GTK_ENTRY(gtk_grid_get_child_at(grid, j, i));

            char text[16];
            snprintf(text, sizeof(text), "%.3f", dataMatrix[i-1][j-1]);
            gtk_entry_set_text(entry, text);
        }
    }
}

void getTableData(gboolean fill){
	// Asignación de memoria para la matriz de probabilidades
    double** dataMatrix = malloc(amountOfGenes * sizeof(double*));

	for(int i = 0; i < amountOfGenes; i++)
		dataMatrix[i] = malloc(amountOfGenes * sizeof(double));

	int firstCheck = 2;
	int lastCheck = amountOfGenes - 1;

	for (int row = 1; row <= amountOfGenes; row++){
		for (int column = row; column <= amountOfGenes; column++) {
			GtkEntry *entry = GTK_ENTRY(gtk_grid_get_child_at(grid, column, row));
			double currentItem = getValue(entry, fill); 

			//Revisamos si la casilla esta vacia y el boton no fue presionado
			if (currentItem == INPUTERROR && !fill){
				show_error("Invalid entries, fill in all blank cells or select the option 'Fill'");
				return;
			}

			dataMatrix[row-1][column-1] = currentItem;
		}
	}

    //En caso de que se haya marcado la casilla 'fill'
    if (fill){
        if(calculateValidValue(dataMatrix) == INPUTERROR){
            show_error("Couldn't deduce the remaining probabilities");
            return; 
        }    
    }

	//Variables para registrar el error en caso de que sea necesario
	int columnError;
	int rowError;
	if(checkMatrix(dataMatrix, &columnError, &rowError) == INPUTERROR){
        char errorMsg[100];
        sprintf(errorMsg, "Cell (%d,%d) is incosistent", rowError, columnError);
		show_error(errorMsg);
		return;
	}

    fillGridTable(dataMatrix);
    //printMatrix(dataMatrix);

	freeDoubleMemory(dataMatrix, amountOfGenes);
}

void on_solve_clicked(GtkButton *btn){
    gboolean fillFlag = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(fillBox));
    getTableData(fillFlag);
}

void leave_program (GtkButton *btn){
    gtk_widget_destroy(mainWindow);
}

void on_cancelBtn_clicked(GtkButton *btn) {
    gtk_widget_hide(genesEntry);
}

void on_char_cancel_clicked(GtkButton *btn) {
    gtk_widget_hide(characteristicsWindow);
}

void on_inserted_char(GtkEntry *entry, gchar *new_text) {
    const gchar *current_text = gtk_entry_get_text(entry);

    if (!g_ascii_isdigit(new_text[0])) {
        g_signal_stop_emission_by_name(entry, "insert-text");
        return;
    }
}

void on_errorDialog_response(GtkDialog *dialog, gint response_id, gpointer user_data) {
    gtk_widget_hide(GTK_WIDGET(dialog));
}

void show_error(const char* message) {
	gtk_message_dialog_set_markup(GTK_MESSAGE_DIALOG(errorDialog), message);
	gtk_widget_show(errorDialog);
}

void on_file_set(GtkFileChooserButton *btn, gpointer user_data) {
    char *fileName = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(btn));
    if (!fileName) return;

    if (currentContent) {
        freeFileContent(currentContent);
        currentContent = NULL;
    }

    fileContent* content = loadFile(fileName);

    //Hacemos el calculo para no repetirlo despues
    int totalDigits = countDigits(amountOfGenes);
    
    // Crear entradas para las características
    createGenesInput(amountOfGenes, totalDigits);

    if (content) {
		currentContent = content;

        // Escribir nombres de los genes
        for (int i = 0; i < amountOfGenes; i++) {
            GtkEntry *entry = GTK_ENTRY(gtk_grid_get_child_at(grid, i+1, 0)) ;
            gtk_entry_set_text(entry, content->names[i]);
        }
        
        // Escribir las probabilidades de los genes
        int arrayIndex = 0;
        for (int i = 1; i <= amountOfGenes; i++) {
            for (int j = i+1; j <= amountOfGenes; j++) {
                GtkEntry *entry = GTK_ENTRY(gtk_grid_get_child_at(grid, j, i));
                gtk_entry_set_text(entry, content->probabilities[arrayIndex]);

                arrayIndex++;
            }
        }
    } else {
		show_error("Error loading file, check format");
        gtk_file_chooser_unselect_all(GTK_FILE_CHOOSER(btn));
        gtk_file_chooser_button_set_title(GTK_FILE_CHOOSER_BUTTON(btn), "Select a file");
    }

    /*
	if (content) {
		currentContent = content;

        if (currentProperties) {
            free_properties(currentProperties, currentPropertiesCount);
            currentProperties = NULL;
            currentPropertiesCount = 0;
        }

		// Obtener la cantidad de propiedades
        currentProperties = malloc(content->fileProperties * sizeof(property));
        for (int i = 0; i < content->fileProperties; i++) {
            currentProperties[i].upperCase = content->properties[i].upperCase;
            currentProperties[i].lowerCase = content->properties[i].lowerCase;
            currentProperties[i].dominant = strdup(content->properties[i].dominant);
            currentProperties[i].recessive = strdup(content->properties[i].recessive);
        }
        currentPropertiesCount = content->fileProperties;

        // Generar genotipos y construir el grid
		amountOfGenes = content->fileProperties;
		int amountOfCombinations = pow(COMBINATIONLENGTH, (double) amountOfGenes);
		generateGenotypes(content->properties, amountOfGenes, amountOfCombinations);
		buildGrid(amountOfCombinations);

        for (int i = 0; i < amountOfCombinations; i++) {
            if (currentContent->parent1 && strcmp(allCombinations[i], currentContent->parent1) == 0) {
                GtkWidget *radio1 = gtk_grid_get_child_at(GTK_GRID(grid), 1, i);
                if (radio1) gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio1), TRUE);
            }
            if (currentContent->parent2 && strcmp(allCombinations[i], currentContent->parent2) == 0) {
                GtkWidget *radio2 = gtk_grid_get_child_at(GTK_GRID(grid), 3, i);
                if (radio2) gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio2), TRUE);
            }
        }


    */

    g_free(fileName);
}


void on_save_clicked(GtkButton *btn) {
    if (!amountOfGenes || amountOfGenes == 0) {
        show_error("No data to save.");
    }

    // Construir y mostrar el dialogo de guardar archivo
    GtkWidget *dialog = gtk_file_chooser_dialog_new(
        "Save File",
        GTK_WINDOW(mainWindow),
        GTK_FILE_CHOOSER_ACTION_SAVE,
        "_Cancel", GTK_RESPONSE_CANCEL,
        "_Save", GTK_RESPONSE_ACCEPT,
        NULL
    );
    gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(dialog), TRUE);

    gint res = gtk_dialog_run(GTK_DIALOG(dialog));
    // Verificar si se aceptó el dialogo
    if (res == GTK_RESPONSE_ACCEPT) {
        char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));

        // Verificar y agregar la extensión .txt
        if (!g_str_has_suffix(filename, ".txt")) {
            char *with_ext = g_strconcat(filename, ".txt", NULL);
            g_free(filename);
            filename = with_ext;
        }

        // Guardar la configuración
        int ok = saveConfig(amountOfGenes, filename, grid);

        if (!ok) {
            show_error("Something went wrong saving the file.");
        }
        g_free(filename);
    }
    gtk_widget_destroy(dialog);
}

void on_newBtn_clicked(GtkButton *btn, gpointer user_data) {
	gtk_widget_show_all(genesEntry);
	gtk_window_present(GTK_WINDOW(genesEntry));
}


gboolean on_window_delete_event(GtkWidget *widget, GdkEvent *event, gpointer user_data) {
	gtk_widget_hide(widget);
	return TRUE;
}

void on_crossing_cancelar_clicked(GtkButton *btn, gpointer user_data) {
	gtk_widget_hide(resultWindow);
}

void on_crossing_accept_clicked(GtkButton *btn, gpointer user_data) {
	gtk_widget_hide(resultWindow);
}

void on_genotypeBtn_toggled(GtkToggleButton *button, gpointer user_data) {
	if (gtk_toggle_button_get_active(button)) {
       	generateColorsCSS(True);
        gtk_toggle_button_set_active(button, FALSE);
    }
}

void on_fenotypeBtn_toggled(GtkToggleButton *button, gpointer user_data) {
	if (gtk_toggle_button_get_active(button)) {
       	generateColorsCSS(False);
        gtk_toggle_button_set_active(button, FALSE);
    }
}

void on_percentagesBtn_clicked(GtkButton *button, gpointer user_data) {
	insertPercentages();
	gtk_widget_show_all(percentagesWindow);
    gtk_window_present(GTK_WINDOW(percentagesWindow));
}

void on_back_button_clicked(GtkButton *button, gpointer user_data) {
    gtk_window_close(GTK_WINDOW(resultWindow));
}


void on_back_button2_clicked(GtkButton *button, gpointer user_data) {
    gtk_window_close(GTK_WINDOW(percentagesWindow));
}