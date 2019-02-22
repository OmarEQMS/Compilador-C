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
struct Identificadores { 
public: 
	string identificador; string tipo; int linea; int columna; int indexCodigo;
	Identificadores() {}
	Identificadores(string id, string tip, int lin, int col, int index) { identificador = id; tipo = tip; linea = lin; columna = col; indexCodigo = index; }
};
struct CodigoToken { 
public: 
	int token; string texto; int linea; int columna; 
	CodigoToken() {}
	CodigoToken(int tok, string tex, int lin, int col) { token = tok; texto = tex; linea = lin; columna = col; }
};
struct ResultAutomata { 
public: 
	int longitud; int token; string texto; int linea; int columna; bool exito;
	ResultAutomata() {} 
	ResultAutomata(int tok, int lon, string tex, int lin, int col, bool ex) { longitud = lon; token = tok; texto = tex; linea = lin; columna = col; exito = ex; }
};

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
	int* pilaEstados = new int[pila + pilaVacio + 2];
	for (int i = 0; i < pila; i++) {
		pilaEstados[i + 2] = matrizLexico[estadoActual][carac][i];
	}
	for (int i = 0; i < pilaVacio; i++) {
		if (matrizLexico[estadoActual][0][i] != -1) { //Me voy a vacio solo si es estado bien
			pilaEstados[pilaVacioValid + pila + 2] = matrizLexico[estadoActual][0][i];
			pilaVacioValid++;
		}
	}
	pilaEstados[0] = pila + pilaVacioValid;
	pilaEstados[1] = pila; //Para poder ditingir cuales son epsilon y cuales no
	return pilaEstados;
}

int ValidarEstadoToken(string texto, int estado, int exitStatus) { //Regreso el token
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
		//if (token != -1) cout << tablaTokens[token].token << ": " << texto << endl;
	}	
	return token;
}

ResultAutomata AutomataLexico(string texto, int longitud, int index, int estado, int master_slave, int linea, int columna) { //Regresa la longitud del texto
	int* estados = SiguienteEstado(estado, programa[index]);
	vector<ResultAutomata> resultados;
	if (estados[0] == -2) {
		cout << "ERROR Caracter no valido en " << linea << ", " << columna << endl;
		resultados.push_back(*(new ResultAutomata(0, longitud + 1, "", linea, columna + 1, false))); //caracter invalido, pero resultado al final del caracter (\n si es valido)
	}
	for (int k = 0; k < estados[0]; k++) {
		if (estados[k + 2] <= 0) {			//Si el estado es menor o iguala 0
			int token = ValidarEstadoToken(texto, estado, estados[k + 2]);
			if (token != -1) {				//encontre un token, lo guardo en resultados
				resultados.push_back(*(new ResultAutomata(token, longitud, texto, linea, columna, true)));
			} else if(estados[k + 2]==0){ //Espacios o saltos de linea, avanzo sin detectar, no se recomienda tener un stack
				int sigLong = longitud + 1;
				int sigCol = (programa[index] == '\n') ? 0 : columna + 1;
				int sigLin = (programa[index] == '\n') ? linea + 1 : linea;
				resultados.push_back(*(new ResultAutomata(0, sigLong, "", sigLin, sigCol, false))); //Genero resultado
			} else { //De seguro hubo un error
				cout << "ERROR Detectado en " << linea << ", " << columna << endl;
				int sigLong = longitud + 1;
				int sigCol = (programa[index] == '\n') ? 0 : columna + 1;
				int sigLin = (programa[index] == '\n') ? linea + 1 : linea;
				resultados.push_back(*(new ResultAutomata(0, sigLong, "", sigLin, sigCol, false))); //Genero resultado
			}
		}else {										//Si no es menor a 0, me sigo moviendo
			int sigIndex = index + 1;
			int sigLong = longitud + 1;
			int sigCol = (programa[index] == '\n') ? 0 : columna + 1;
			int sigLin = (programa[index] == '\n') ? linea + 1 : linea;
			if (k >= estados[1]) { //Significa que estoy haciendo cerradura epsilon
				sigIndex = index; sigLong = longitud; sigCol = columna; sigLin = linea;
			}
			//Sigo recorriendo
			resultados.push_back(AutomataLexico(texto + programa[index], sigLong, sigIndex, estados[k + 2], 1, sigLin, sigCol));
		}
	}
	//Obtengo el token mas largo de resultados
	int mejor = -1;
	for (int i = 0; i < resultados.size(); i++) {
		if (resultados[i].exito) {
			if(mejor!=-1){
				if (resultados[i].longitud > resultados[mejor].longitud) { mejor = i; }
			} else {
				mejor = i;
			}
		}
	}
	if (mejor == -1) { mejor = 0; } //Si no hay exito, tomo el primero (caracter invalido o espacio/salto/tab) //Caracter invalido o espacio o linea

	//Si soy master, guardo el token y sigo, si soy slave, retorno el token
	if (master_slave == 0) {
		if (resultados[mejor].exito) {
			codigoTokenizado.push_back(*(new CodigoToken(resultados[mejor].token, resultados[mejor].texto, linea, columna)));
			if (tablaTokens[resultados[mejor].token].token == "IDENTIFICADOR") { //Si es identificador lo agrego a la tabla
				tablaIdentificadores.push_back(*(new Identificadores(resultados[mejor].texto, "", linea, columna, codigoTokenizado.size()-1)));
			}
		}
		if (programa[index + resultados[mejor].longitud] != '\0') { //Nulo siempre da -1 en automata, tengo que checar el final de el token actual
			AutomataLexico("", 0, index + resultados[mejor].longitud, 0, 0, resultados[mejor].linea, resultados[mejor].columna);
		}
		return *(new ResultAutomata()); //Realmente esto no importa
	} else {
		return resultados[mejor];
	}
	
}

void main() {
	ReadFilePrograma();
	ReadFileLexico();
	ReadFileTokens();
	ReadFilePalabrasReservadas();

	AutomataLexico("", 0, 0, 0, 0, 0, 0);

	cout << endl << "Codigo Tokenizado: " << endl;
	for (int i = 0; i < codigoTokenizado.size(); i++) {
		cout << "   " << tablaTokens[codigoTokenizado[i].token].token << " (" << codigoTokenizado[i].linea << ", " << codigoTokenizado[i].columna << "): " << codigoTokenizado[i].texto << endl;
	}
	cout << endl;

	cout << "Tabla de tablaIdentificadores" << endl;
	for (int i = 0; i < tablaIdentificadores.size(); i++) {
		cout << "   " << tablaIdentificadores[i].identificador << " (" << tablaIdentificadores[i].linea << ", " << tablaIdentificadores[i].columna << ")(" << tablaIdentificadores[i].indexCodigo << "): " << tablaIdentificadores[i].tipo << endl;
	}
	cout << endl;

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