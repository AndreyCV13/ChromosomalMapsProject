#ifndef LOGIC_H
#define LOGIC_H
#include <gtk/gtk.h>
#include <math.h>

#define INPUTERROR -1
#define MARGINERROR 0.02
#define True 1
#define False 0
#define PROPERTYLENGTH 2.0
#define COMBINATIONLENGTH 3.0

//Variables globales
extern int amountOfGenes;
extern char** allCombinations;
extern GHashTable *colorMapGenotype, *colorMapPhenotype;
extern GHashTable *counterMapGenotype, *counterMapPhenotype;
extern char **children;
extern int totalColumns, totalRows;

//Posiblemente se le agrege la descripcion
typedef struct {
  char upperCase;
  char lowerCase;
  char* dominant;
  char* recessive;
}property;

extern property* currentProperties;
extern int currentPropertiesCount;

void freeMemory(char** array, int length);
void freeDoubleMemory(double** array, int length);
int countDigits(int num);
void printMatrix(double** dataMatrix);
void generateGenotypes(property* properties, int amountOfGenes, int amountOfCombinations);
int calculateArrayElements(char** array, int arrayLength);
char** calculateChildren(char** firstAlleles, char** secondAlleles);
char** calculateAlleles(const gchar *currentGenotype);
char* genotypeToPhenotype(const char* genotype);
guint ASCIIValue(gconstpointer key);
int calculateValidValue(double** dataMatrix);
int checkMatrix(double** dataMatrix, int* columnError, int* rowError);

#endif