#*************************************************
# Executable name : ProyectoProgramado0
# Version : 1.0
# Created date : August 29, 2025
# Authors : 
#	Andrey Calvo Vargas
# 	Andrés Alexander Feng Wu
# Description : simple makefile
#*************************************************
name = proyecto1
Compiler = gcc
CFLAGS = -Wno-format -Wno-deprecated-declarations -Wno-format-security `pkg-config --cflags gtk+-3.0 cairo`
LDFLAGS = `pkg-config --libs gtk+-3.0 cairo` -lm -export-dynamic

SRC = proyecto1.c src/logic.c src/UI.c src/file_manager.c
OBJ = objects/proyecto1.o objects/logic.o objects/UI.o objects/file_manager.o

$(name): $(OBJ)
	$(Compiler) -o $(name) $(OBJ) $(LDFLAGS)

# Crear objects
objects:
	mkdir -p objects

objects/proyecto1.o: proyecto1.c | objects
	$(Compiler) $(CFLAGS) -c $< -o $@

objects/logic.o: src/logic.c | objects
	$(Compiler) $(CFLAGS) -c $< -o $@

objects/UI.o: src/UI.c | objects
	$(Compiler) $(CFLAGS) -c $< -o $@

objects/file_manager.o: src/file_manager.c | objects
	$(Compiler) $(CFLAGS) -c $< -o $@

clean:
	rm -rf objects $(name)




