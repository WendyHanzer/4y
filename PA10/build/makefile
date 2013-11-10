# Linux
CC=g++

# Compiler flags
LIBS= -lglut -lGLEW -lGL -lassimp -lBulletDynamics -lBulletCollision  -lLinearMath
CXXFLAGS= -g -Wall -std=c++0x -I/usr/include/bullet

# variables
OBJ= main.o  MeshManager.o ShaderManager.o Physics.o

../bin/Physics: $(OBJ)
	$(CC) $(OBJ) -o ../bin/Physics $(LIBS) `Magick++-config --cppflags --cxxflags --ldflags --libs`

main.o: ../src/main.cpp  ../src/MeshManager.h  ../src/ShaderManager.h  ../src/Physics.h
	$(CC) $(CXXFLAGS) -c ../src/main.cpp $(LIBS) `Magick++-config --cppflags --cxxflags --ldflags --libs`

ShaderManager.o: ../src/ShaderManager.h ../src/ShaderManager.cpp ../src/shaderLoader.h
	$(CC) $(CXXFLAGS) -c ../src/ShaderManager.cpp $(LIBS)

MeshManager.o: ../src/MeshManager.h ../src/MeshManager.cpp
	$(CC) $(CXXFLAGS) -c ../src/MeshManager.cpp $(LIBS)

Physics.o:	../src/Physics.h ../src/Physics.cpp ../src/mesh.h
	$(CC) $(CXXFLAGS) -c ../src/Physics.cpp $(LIBS)

clean:
	-rm *.o
