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

struct TablaToken { public: string token; int estado; };
struct Identificadores { public: string identificador; string tipo; int linea; int columna; };
struct CodigoToken { public: string token; string texto; int linea; int columna; };

vector<vector<vector<int>>> matrizLexico;
vector<TablaToken> tablaTokens;
vector<string> palabras_reservadas;
string programa;

vector<CodigoToken> codigoTokenizado;
vector<Identificadores> tablaIdentificadores;

int* SiguienteEstado(int estadoActual, char caracter) {
	int carac = -1;
	carac = (caracter != '\0') ? (caracter != '\n') ? (caracter != '\t') ? caracter - 32 + 4 : 3: 2 : 1;
	if ((carac >= 99)|| (carac < 0)) {  return new int[1]{ -2 }; }
	
	int pila = (matrizLexico[estadoActual][carac]).size();
	int pilaVacio = (matrizLexico[estadoActual][0]).size();
	int pilaVacioValid = 0;
	int* pilaEstados = new int[pila + pilaVacio + 1];
	for (int i = 0; i < pila; i++) {
		pilaEstados[i + 1] = matrizLexico[estadoActual][carac][i];
	}
	for (int i = 0; i < pilaVacio; i++) {
		if (matrizLexico[estadoActual][0][i] != -1) {
			pilaEstados[pilaVacioValid + pila + 1] = matrizLexico[estadoActual][0][i];
			pilaVacioValid++;
		}
	}
	pilaEstados[0] = pila + pilaVacioValid;
	return pilaEstados;
}

bool ValidarEstadoToken(string texto, int estado, int exitStatus) {
	int token = -1;
	if (exitStatus == -1) {
		for (int i = 0; i < tablaTokens.size(); i++) {
			if (estado == tablaTokens[i].estado) {
				token = i;
				if (tablaTokens[i].token == "IDENTIFICADOR") { //Si es identificdor, busco si es palabra reservada
					for (int j = 0; j < palabras_reservadas.size(); j++) {
						if (texto == palabras_reservadas[j]) { token = 0; } //Palabra Reservada
					}
				}
			}
		}
		if (token != -1) cout << tablaTokens[token].token << ": " << texto << endl;
	}	
	return (token!=-1);
}

void AutomataLexico(string texto, int index, int estado) {
	int* estados = SiguienteEstado(estado, programa[index]);
	if (estados[0] == -2) {
		cout << "ERROR: Caracter no valido: " << programa[index] << endl;
	}
	for (int k = 0; k < estados[0]; k++) {
		if (estados[k + 1] < 0) {			//Si el estado es menor a 0, verifico estado final y comienzo desde ahi
			if (ValidarEstadoToken(texto, estado, estados[k + 1])) {
				if(programa[index] != '\0') AutomataLexico("", index, 0);
			}
		}else {										//Si no es menor a 0, me sigo moviendo
			if (estados[k + 1] == 0) {				//Si regreso a 0 vuelvo a comenzar
				AutomataLexico("", index + 1, estados[k + 1]);
			} else {
				AutomataLexico(texto + programa[index], index + 1, estados[k + 1]);
			}
		}
	}
}

void main() {
	ReadFilePrograma();
	ReadFileLexico();
	ReadFileTokens();
	ReadFilePalabrasReservadas();

	AutomataLexico("", 0, 0);

	system("pause");
}


void ReadFilePalabrasReservadas() {
	ifstream archivo("palabras_reservadas.txt");
	string fila;
	if (archivo.is_open()) {
		while (getline(archivo, fila)) {
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
		programa = programa + '\0';
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
	int i = 0, j = 0;
	ifstream archivo("tokens.txt");
	string fila, celda;
	if (archivo.is_open()) {
		i = 0;
		while (getline(archivo, fila)) {
			istringstream SSfila(fila);
			tablaTokens.resize(i + 1);
			j = 0;
			while (getline(SSfila, celda, '\t')) {
				if (j == 0) { if (celda != "") { tablaTokens[i].token = celda;} else { j = 3; } }
				if (j == 1) { tablaTokens[i].estado = stoi(celda); }
				j++;
			}	
			if(j==2){ i++; }
		}
		archivo.close();
	}
}