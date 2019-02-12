#include <iostream>
#include <vector>
#include <math.h>

#include <fstream>
#include <sstream>
#include <string>
using namespace std;

void ReadFileTokens();
void ReadFileLexico();
void ReadFilePrograma();
void ReadFilePalabrasReservadas();

vector<vector<vector<int>>> matrizLexico;
vector<string> tokens;
vector<string> palabras_reservadas;
string programa;

int* SiguienteEstado(int estadoActual, char caracter) {
	int carac = -1;
	carac = (caracter != '\n') ? (caracter != '\t') ? caracter - 32 + 3 : 2 : 1;
	if (carac >= 98) return new int[1]{ -2 };
	
	int pilaCount = (matrizLexico[estadoActual][carac]).size();
	int pilaCountVacio = (matrizLexico[estadoActual][0]).size();
	int* pilaEstados = new int[pilaCount + pilaCountVacio + 1];
	for (int i = 0; i < pilaCount; i++) {
		pilaEstados[i + 1] = matrizLexico[estadoActual][carac][i];
	}
	for (int i = 0; i < pilaCountVacio; i++) {
		pilaEstados[i + pilaCount + 1] = matrizLexico[estadoActual][0][i];
	}
	pilaEstados[0] = pilaCount + pilaCountVacio;

	return pilaEstados;
}

void main() {
	ReadFilePrograma();
	ReadFileLexico();
	ReadFileTokens();
	ReadFilePalabrasReservadas();

	int estadoActual = 0;
	for (int i = 0; i < programa.length(); i++) {
		int* estados = SiguienteEstado(estadoActual, programa[i]);
		cout << programa[i] << ": ";
		for (int k = 0; k < estados[0]; k++) {
			cout << estados[k + 1] << " ";
		}
		cout << endl;
	}

	system("pause");
}


void ReadFilePalabrasReservadas() {
	ifstream archivo("palabras_reservadas.txt");
	string fila;
	if (archivo.is_open()) {
		while (getline(archivo, fila)) {
			istringstream SSfila(fila);
			palabras_reservadas.push_back(fila);
		}
		archivo.close();
	}
}

void ReadFilePrograma() {
	programa = "";
	ifstream archivo("programa.txt");
	if (archivo.is_open()) {
		char buffer[4096];
		while (archivo.read(buffer, sizeof(buffer)))
			programa.append(buffer, sizeof(buffer));
		programa.append(buffer, archivo.gcount());
		archivo.close();
	}
}

void ReadFileLexico() {
	int i = 0, j = 0, k = 0;
	ifstream archivo("matrizLexico.txt");
	string fila, celda, dato;
	if (archivo.is_open()) {
		i = 0;
		while (getline(archivo, fila)) {
			istringstream SSfila(fila);
			matrizLexico.resize(i + 1);
			j = 0;
			while (getline(SSfila, celda, '\t')) {
				istringstream SScelda(celda);
				matrizLexico[i].resize(j + 1);
				k = 0;
				while (getline(SScelda, dato, ',')) {
					matrizLexico[i][j].resize(k + 1);
					matrizLexico[i][j][k] = stoi(dato);
					k++;
				}
				j++;
			}
			i++;
		}
		archivo.close();
	}
}

void ReadFileTokens() {
	ifstream archivo("tokens.txt");
	string fila;
	if (archivo.is_open()) {
		while (getline(archivo, fila)) {
			istringstream SSfila(fila);
			tokens.push_back(fila);
		}
		archivo.close();
	}
}