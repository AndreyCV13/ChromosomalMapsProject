#include "logic.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "UI.h"

int amountOfGenes;
char** allCombinations;
GHashTable* colorMapGenotype, *colorMapPhenotype;
GHashTable *counterMapGenotype, *counterMapPhenotype;
extern int currentPropertiesCount;

// Funcion para obtener el fenotipo a partir del genotipo
char* getPhenotype(const char* genotype) {
    // Asume que genotype tiene longitud 2*currentPropertiesCount
    GString *phenotype = g_string_new("");
    for (int i = 0; i < currentPropertiesCount; i++) {
        char allele1 = genotype[i*2];
        char allele2 = genotype[i*2+1];
        // Si alguno es mayúscula, es dominante
        if (allele1 == currentProperties[i].upperCase || allele2 == currentProperties[i].upperCase) {
            g_string_append(phenotype, currentProperties[i].dominant);
        } else {
            g_string_append(phenotype, currentProperties[i].recessive);
        }
        if (i < currentPropertiesCount - 1) g_string_append(phenotype, " / ");
    }
    return g_string_free(phenotype, FALSE); // Devuelve un string heap, hay que liberar después
}

void freeMemory(char** array, int length){
	for (int i = 0; i < length; i++){
		free(array[i]);
	}
	free(array);
}

void freeDoubleMemory(double** array, int length){
	for (int i = 0; i < length; i++){
		free(array[i]);
	}
	free(array);
}

//Cuenta los digitos sacando el logaritmo en base 10
int countDigits(int num) {
    return (int)log10(num) + 1;
}

int calculateArrayElements(char** array, int arrayLength){
	int amountOfElements = arrayLength;
	for (int i = 0; i < arrayLength; i++){
		if(array[i][0] == False){
			amountOfElements = i;
			return amountOfElements;
		}
	}

	return amountOfElements;
}


// Función para convertir HSV a RGB
void hsv_to_rgb(double h, double s, double v, double* r, double* g, double* b) {
    double c = v * s;
    double x = c * (1 - fabs(fmod(h / 60.0, 2) - 1));
    double m = v - c;

    double r1, g1, b1;

    if (h < 60) {
        r1 = c; g1 = x; b1 = 0;
    } else if (h < 120) {
        r1 = x; g1 = c; b1 = 0;
    } else if (h < 180) {
        r1 = 0; g1 = c; b1 = x;
    } else if (h < 240) {
        r1 = 0; g1 = x; b1 = c;
    } else if (h < 300) {
        r1 = x; g1 = 0; b1 = c;
    } else {
        r1 = c; g1 = 0; b1 = x;
    }

    *r = r1 + m;
    *g = g1 + m;
    *b = b1 + m;
}

// Función para convertir el genotipo en fenotipo
char* genotypeToPhenotype(const char* genotype) {
    GString *phenotype = g_string_new("");

    for (int i = 0; i < amountOfGenes; i++) {
        char allele1 = genotype[i * 2];
        char allele2 = genotype[i * 2 + 1];

        // Determinar si hay dominancia
        char property;

        if (g_ascii_isupper(allele1) || g_ascii_isupper(allele2)) {
            property = g_ascii_toupper(allele1);
        } else {
            property = allele1;
        }

        // Separar cada rasgo en el string final
        g_string_append_c(phenotype, property);

        if (i < amountOfGenes - 1) {
            g_string_append(phenotype, "-");
        }
    }

    return g_string_free(phenotype, FALSE);
}

GdkRGBA* colorGenerator(int index, int total) {
    GdkRGBA *color = g_new(GdkRGBA, 1);

    // Usar ángulo áureo para mejor dispersión
    double hue = fmod(index * 137.508, 360.0);

    // Cambiar saturación y valor en patrones
    double saturation = 0.6 + 0.4 * ((index % 3) / 2.0);
    double value = 0.7 + 0.3 * ((index % 4) / 3.0); 

    double r, g, b;
    hsv_to_rgb(hue, saturation, value, &r, &g, &b);

    color->red = r;
    color->green = g;
    color->blue = b;
    color->alpha = 1.0;

    return color;
}

//Hashcode
guint ASCIIValue(gconstpointer key) {
    const char *genotype = (const char *)key;
	guint sum = 0;
	int index = 0;
	while(genotype[index] != 0){
		sum += genotype[index];
		index++;
	}
	return sum;
}

gboolean genotype_equal(gconstpointer a, gconstpointer b) {
    const char *g1 = (const char *)a;
    const char *g2 = (const char *)b;

    size_t len = strlen(g1);
    if (len != strlen(g2)) return FALSE;

    for (int i = 0; i < len; i += 2) {
        char g1a = g1[i], g1b = g1[i+1];
        char g2a = g2[i], g2b = g2[i+1];

        if (!((g1a == g2a && g1b == g2b) || (g1a == g2b && g1b == g2a))) {
            return FALSE;
        }
    }
    return TRUE;
}

void imprimir_llave(gpointer key, gpointer value, gpointer user_data) {
    char *genotype = (char *)key;
	gint count = *(gint *)value;
    printf("Fenotipo: %s\n", genotype);
	printf("Valor: %d\n", count);
}


property* generateDefaults(){
	//Esto es simplemente como prueba, genera los valores por defectos segun el n ingresado
	int totalProperties = amountOfGenes + 1;
	property* entries = malloc(amountOfGenes * sizeof(property));
	for(int i = 0; i < amountOfGenes; i++){
		property newLetter;
		newLetter.upperCase = (char) (i + 65);
		newLetter.lowerCase = (char) (i + 97);
		entries[i] = newLetter;
	}
	return entries;
}

// Funcion para generar todas las combinaciones de un alelo
char** generateCombinations(property currentLetter){
	char** possibleCombinations = malloc(COMBINATIONLENGTH * sizeof(char*));

	for (int i = 0; i < COMBINATIONLENGTH; i++){
		possibleCombinations[i] = malloc(3 * sizeof(char));
	}

	//Asigna todas las combinaciones dentro de; array possibleCombinations
	//(Se puede hace con fors, pero puede ser poco legible)
	possibleCombinations[0][0] = currentLetter.upperCase;
	possibleCombinations[0][1] = currentLetter.upperCase;
	possibleCombinations[0][2] = '\0';

	possibleCombinations[1][0] = currentLetter.upperCase;
	possibleCombinations[1][1] = currentLetter.lowerCase;
	possibleCombinations[1][2] = '\0';

	possibleCombinations[2][0] = currentLetter.lowerCase;
	possibleCombinations[2][1] = currentLetter.lowerCase;
	possibleCombinations[2][2] = '\0';
	
	return possibleCombinations;
}

// Funcion para calcular los hijos a partir de los alelos
char** calculateChildren(char** firstAlleles, char** secondAlleles){
	//Se inicializan los HashMaps de colores
	colorMapGenotype = g_hash_table_new_full(ASCIIValue, genotype_equal, g_free, g_free);
	colorMapPhenotype = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_free);
	counterMapGenotype = g_hash_table_new_full(ASCIIValue, genotype_equal, g_free, g_free);
	counterMapPhenotype = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_free);

	//Array para guardar a todos los hijos
	int amountOfAlleles = pow(PROPERTYLENGTH, (double) amountOfGenes);
	int totalColumns = calculateArrayElements(firstAlleles, amountOfAlleles);
	int totalRows = calculateArrayElements(secondAlleles, amountOfAlleles);

	int childrenAmount = totalColumns * totalRows;
	char** children = malloc(childrenAmount * sizeof(char*));

	for (int i = 0; i < childrenAmount; i++){
		children[i] = malloc((PROPERTYLENGTH * amountOfGenes + 1) * sizeof(char));
	}

	//String para guardar el genotipo resultante de cada alelo
	char* genotype = calloc((PROPERTYLENGTH * amountOfGenes + 1), sizeof(char));
	int index = 0;
	for (int row = 0; row < totalRows; row++){
		for (int column = 0; column < totalColumns; column++){
			char *currentMother = firstAlleles[column];
			char *currentFather = secondAlleles[row];
			for (int i = 0; i < amountOfGenes; i++){
				genotype[index++] = *currentMother++;
				genotype[index++] = *currentFather++;
			}
			//Este indice es la representacion de una matriz en una lista
			int matrixIndex = row * totalColumns + column;
			strcpy(children[matrixIndex], genotype);

			//Se ingresa tambien en un HashMap para modificar colores en la UI
			//Y el contador
			if (!g_hash_table_contains(colorMapGenotype, genotype)) {
				GdkRGBA* newColor = colorGenerator(matrixIndex, childrenAmount);
				g_hash_table_insert(colorMapGenotype, g_strdup(genotype), newColor);

				// Asignar contador inicial 1
				gint *newCounter = g_new(gint, 1);
				*newCounter = 1;
				g_hash_table_insert(counterMapGenotype, g_strdup(genotype), newCounter);
			} else{
				gint *value = g_hash_table_lookup(counterMapGenotype, genotype);
        		(*value)++;
			}

			//Se insertan los fenotipos
			//Y el contador
			char *phenotype = genotypeToPhenotype(genotype);
			if (!g_hash_table_contains(colorMapPhenotype, phenotype)) {
				GdkRGBA* newColor = colorGenerator(matrixIndex, childrenAmount);
				g_hash_table_insert(colorMapPhenotype, g_strdup(phenotype), newColor);
				
				// Asignar contador inicial 1
				gint *newCounter = g_new(gint, 1);
				*newCounter = 1;	
				g_hash_table_insert(counterMapPhenotype, g_strdup(phenotype), newCounter);
			} else{
				gint *value = g_hash_table_lookup(counterMapPhenotype, phenotype);
				(*value)++;
			}

			//Reiniciamos el indice
			index = 0;

		}
	}

	//Liberar memoria
	free(genotype);

	return children;

}

char** calculateAlleles(const gchar *currentGenotype){
	//Array donde se obtienen las caracteristicas de cada genotipo
	char **allProperties = malloc(amountOfGenes * sizeof(char*));
	for (int i = 0; i < amountOfGenes; i++){
		allProperties[i] = calloc(COMBINATIONLENGTH, sizeof(char));
	}

	//Aqui se separan las propiedades "AA", "BB"
	for (int i = 0; i < amountOfGenes; i++){
		for (int j = 0; j < PROPERTYLENGTH; j++){
			allProperties[i][j] = *currentGenotype++;
		}
	}

	//Array para guardar todos los genotipos no repetidos
	int amountOfAlleles = pow(PROPERTYLENGTH, (double) amountOfGenes);
	char **allAlleles = malloc(amountOfAlleles * sizeof(char*));
	for (int i = 0; i < amountOfAlleles; i++){
		allAlleles[i] = malloc((amountOfGenes + 1) * sizeof(char));
		allAlleles[i][0] = False;
	}

	int alleleLastIndex= amountOfGenes - 1;
	//String para guardar el genotipo resultante de cada alelo
	char* allele = malloc((amountOfGenes + 1) * sizeof(char));

	//Array para ir accesando las combinaciones de cada propiedad i = [0,0]... [0,1]... [1,0]
	int* indices = calloc(amountOfGenes, sizeof(int));
	int done = False;
	int currentArrayElement = 0;
	while (!done) {
		// Guardar la combinación actual dentro de genotype
		for (int i = 0; i < amountOfGenes; i++) {
			allele[i] = allProperties[i][indices[i]];
		}

		//Termina en \0
		allele[amountOfGenes] = '\0';

		for (int i = 0; i < amountOfAlleles; i++) {
			if(strcmp(allAlleles[i], allele) == 0){
				break;
			} else if (allAlleles[i][0] == False){
				strcpy(allAlleles[currentArrayElement++], allele);
				break;
			}
		}

		// Incrementar los índices para la próxima combinación
		for (int i = alleleLastIndex; i >= 0; i--) {
			if (indices[i] < PROPERTYLENGTH - 1) {
				indices[i]++;
				break;
			} else {
				indices[i] = 0;
				if (i == 0) {
					done = True;
				}
			}
		}
	}

	free(allele);
	free(indices);
	freeMemory(allProperties, amountOfGenes);

	return allAlleles;
}

void saveAllCombinations(char*** propertyCombinations, int amountOfCombinations){

	int sizeOfCombination = 2 * amountOfGenes + 1;
	//Array para guardar todas las combinaciones de genotipos
	//Guarda memoria para el tamano de las combinaciones ("AABB" = 4)
	allCombinations = malloc(amountOfCombinations * sizeof(char*));
	for (int i = 0; i < amountOfCombinations; i++){
		allCombinations[i] = malloc(sizeOfCombination * sizeof(char));
	}

	//Se guardan las combinaciones dentro de cada indice de allCombinations
	int* indices = calloc(amountOfGenes, sizeof(int));
	int done = False;
	int combinationIndex = 0;

	while (!done) {
		// Guardar la combinación actual en un string
		char* combination = allCombinations[combinationIndex];

		int pos = 0;
		for (int i = 0; i < amountOfGenes; i++) {
			char* value = propertyCombinations[i][indices[i]];

			// Copia caracter por caracter
			while (*value != '\0') {
				combination[pos++] = *value++;
			}
		}

		// Terminar con '\0'
		combination[pos] = '\0';
		combinationIndex++;

		// Incrementar los índices para la próxima combinación
		for (int i = amountOfGenes - 1; i >= 0; i--) {
			if (indices[i] < COMBINATIONLENGTH - 1) {
				indices[i]++;
				break;
			} else {
				indices[i] = 0;
				if (i == 0) {
					done = True;
				}
			}
		}
	}

	free(indices);
}

// Funcion para generar todos los genotipos posibles a partir de los alelos
void generateGenotypes(property* properties, int amountOfGenes, int amountOfCombinations) {
    char*** propertyCombinations = malloc(amountOfGenes * sizeof(char**));
    for (int i = 0; i < amountOfGenes; i++) {
        propertyCombinations[i] = generateCombinations(properties[i]);
    }
    saveAllCombinations(propertyCombinations, amountOfCombinations);
    for (int i = 0; i < amountOfGenes; i++) {
        freeMemory(propertyCombinations[i], (int)COMBINATIONLENGTH);
    }
}


double calculateValidValue(){
	return INPUTERROR;
}

double maxValue(double values[3]) {
    int selectedIndex = 0;
	double maxValue = 0.0;
    for (int i = 0; i < 3; i++) {
        if (values[i] > maxValue) {
			maxValue = values[i];
            selectedIndex = i;
        }
    }
	if (maxValue > 0.5)
		return INPUTERROR;
    return selectedIndex;
}

int checkInputs(double values[3], int selectedIndex){
	double sum = 0;
	double maxValue = values[selectedIndex];
		
	for (int i = 0; i < 3; i++){
		if (i != selectedIndex) 
			sum += values[i];
	}	

	return (maxValue - MARGINERROR < sum && maxValue + MARGINERROR > sum);
}

//Revisa que las distancias sean correctas
int checkMatrix(double** dataMatrix, int* columnError, int* rowError){
	int lastIndex = amountOfGenes - 2;
	if (lastIndex <= 0)
		return 0;

	double check[3];

	for (int row = 0; row < lastIndex; row++){
		check[0] = dataMatrix[row][row+1];
		for (int column = row + 2; column < amountOfGenes; column++){
			check[1] = dataMatrix[row][column];
			check[2] = dataMatrix[row+1][column];
			int selectedIndex = maxValue(check);
			
			
			if (selectedIndex == INPUTERROR){
				*columnError = column;
				*rowError = row;
				return INPUTERROR;
			}

			if(!checkInputs(check, selectedIndex)){
				*columnError = column;
				*rowError = row;
				return INPUTERROR;
			}
				
		}
	}
	return 0;
}