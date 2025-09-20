#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "file_manager.h"
#include "logic.h"
#include <gtk/gtk.h>

char** generateCombinations(property currentLetter);

// Funcion para validar los genotipos de los padres
static int validate_genotype(property* properties, int fileProperties, const char* parent1, const char* parent2) {
    if (!parent1 || strlen(parent1) != (size_t)(fileProperties * 2)) {
        //printf("El primer genotipo no tiene la longitud correcta\n");
        return 0;
    }

    // Validar combinaciones para el primer genotipo
    for (int i = 0; i < fileProperties; i++) {
        property prop = properties[i];
        char** combos = generateCombinations(prop);
        int match = 0;
        for (int j = 0; j < COMBINATIONLENGTH; j++) {

            if (strncmp(&parent1[i*2], combos[j], 2) == 0) {
                match = 1;
                break;
            }
        }
        freeMemory(combos, COMBINATIONLENGTH);
        if (!match) {
            //printf("Primer genotipo tiene una combinación inválida para %d\n", i+1);
            return 0;
        }
    }

    // Validar combinaciones para el segundo genotipo
    if (parent2 && strlen(parent2) > 0) {
        if (strlen(parent2) != (size_t)(fileProperties * 2)) {
            //printf("El segundo genotipo no tiene la longitud correcta\n");
            return 0;
        }
        for (int i = 0; i < fileProperties; i++) {
            property prop = properties[i];
            char** combos = generateCombinations(prop);
            int match = 0;
            for (int j = 0; j < COMBINATIONLENGTH; j++) {
                if (strncmp(&parent2[i*2], combos[j], 2) == 0) {
                    match = 1;
                    break;
                }
            }
            freeMemory(combos, COMBINATIONLENGTH);
            if (!match) {
                //printf("Segundo genotipo tiene una combinación inválida para %d\n", i+1);
                return 0;
            }
        }
    }
    return 1;
}

static void free_gen(char** properties, int count) {
    // Liberar la memoria de cada propiedad
    for (int i = 0; i < count; i++) {
        free(properties[i]);
    }
    free(properties);
}

static int parse_gen_value(const char* line, char* prop) {
    if (strcmp(line, "null") == 0) {
        strcpy(prop, "null");
        return 1;
    }

    char* endptr;
    errno = 0;
    double val = strtod(line, &endptr);

    if (endptr == line || errno == ERANGE) {
        return 0;
    }
    snprintf(prop, 8, "%s", line); 
    return 1;
}

static void cleanup_and_close(FILE* file, char** probabilities, int count) {
    // Liberar la memoria de las propiedades
    free_gen(probabilities, count);
    fclose(file);
}

fileContent* loadFile(const char* filename) {
    // Verifica que el archivo tenga extensión .txt
    const char* ext = strrchr(filename, '.');
    if (!ext || strcmp(ext, ".txt") != 0) {
        //printf( "El archivo debe ser un .txt\n");
        return NULL;
    }

    // Abre el archivo para lectura
    FILE* file = fopen(filename, "r");
    if (!file) {
        //printf("Error al abrir archivo \"%s\"\n", filename);
        return NULL;
    }

    // Lee la cantidad de propiedades
    int fileProperties;
    if (fscanf(file, "%d\n", &fileProperties) != 1) {
        //printf("Cantidad de propiedades inválida\n");
        fclose(file);
        return NULL;
    }

    amountOfGenes = fileProperties;

    // Verifica que la cantidad de genes sea mayor a 0
    if (fileProperties <= 0) {
        //printf("La cantidad de genes debe de ser mayor a 0\n");
        fclose(file);
        return NULL;
    }

    // Asignación de memoria para los nombres de los genes
    char** names = malloc(fileProperties * sizeof(char*));
    for(int i = 0; i < fileProperties; i++) {
        names[i] = malloc(8 * sizeof(char));
        if (!names[i]) {
            //printf("Error al asignar memoria para las probabilidades\n");
            cleanup_and_close(file, names, i);
            return NULL;
        }
    }

    // Lectura de los nombres
    for (int i = 0; i < fileProperties; i++) {
        if (!fscanf(file, " \"%7[^\"]\" ", names[i])) {
            //printf("Error al leer la línea de la característica %d\n", i + 1);
            cleanup_and_close(file, names, fileProperties);
            return NULL;
        }
    }

    // Asignación de memoria para la matriz de probabilidades
    int amountOfEntries = (fileProperties * fileProperties - fileProperties) / 2;
    char** probabilities = malloc(amountOfEntries * sizeof(char*));
    for(int i = 0; i < amountOfEntries; i++) {
        probabilities[i] = malloc(8 * sizeof(char));
        if (!probabilities[i]) {
            //printf("Error al asignar memoria para las probabilidades\n");
            cleanup_and_close(file, probabilities, i);
            return NULL;
        }
    }

    // Lectura de las probabilidades
    char line[8];
    for (int i = 0; i < amountOfEntries; i++) {
        if (!fscanf(file, "%s", line)) {
            //printf("Error al leer la línea de la característica %d\n", i + 1);
            cleanup_and_close(file, probabilities, amountOfEntries);
            return NULL;
        }
        if (!parse_gen_value(line, probabilities[i])) {
            //printf("Error de formato en la característica %d\n", i + 1);
            cleanup_and_close(file, probabilities, amountOfEntries);
            return NULL;
        }
    }
    
    fileContent* content = malloc(sizeof(fileContent));
    if (!content) {
        return NULL;
    }

    content->amountOfEntries = amountOfEntries;
    content->names = names;
    content->probabilities = probabilities;
    
    fclose(file);
    return content;
}

void freeFileContent(fileContent* content) {
    // Liberar la memoria del contenido del archivo
    if (!content) return;
    free_gen(content->names, amountOfGenes);
    free_gen(content->probabilities, content->amountOfEntries);
    free(content);
}

int saveConfig(int totalGenes, const char *filename, GtkGrid *grid) {
    if (!filename) return 0;

    FILE *f = fopen(filename, "w");
    if (!f) return 0;

    // Escribir número de genes
    fprintf(f, "%d\n", totalGenes);

    // Escribir nombres de los genes
    for (int i = 1; i <= totalGenes; i++) {
        GtkEntry *entry = GTK_ENTRY(gtk_grid_get_child_at(grid, i, 0)) ;
        const char *text = gtk_entry_get_text(entry);

        fprintf(f, "\"%s\" ", text);
    }

    fprintf(f, "\n");
    
    // Escribir las probabilidades de los genes
    for (int i = 1; i <= totalGenes; i++) {
        for (int j = i+1; j <= totalGenes; j++) {
            GtkEntry *entry = GTK_ENTRY(gtk_grid_get_child_at(grid, j, i));
            const char *text = gtk_entry_get_text(entry);

            fprintf(f, "%s ", text[0] != '\0' ? text : "null");
        }
        fprintf(f, "\n");
    }

    fclose(f);
    return 1;
}