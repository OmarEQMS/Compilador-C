#include <iostream>
#include <vector>
#include <math.h>

#include <fstream>
#include <sstream>
#include <string>
#include <iomanip>
using namespace std;

void ReadFileTokens();
void ReadFileLexico();
void ReadFilePrograma();
void ReadFilePalabrasReservadas();
void PrintTableLines(int tokenS, int textoS, int lS, int cS);
void PrintTable(int op);


struct Token { public: string token; int estado; };
struct simbol {public: string token; string texto; int line; int column;};
vector<vector<vector<int>>> matrizLexico;
vector<Token> tokens;
vector<string> palabras_reservadas;
string programa;
int Line, Column;
vector<simbol> AllTokens;
vector<simbol> identificadores;
int* SiguienteEstado(int estadoActual, char caracter) {
	int carac = -1;
	carac = (caracter != '\n') ? (caracter != '\t') ? caracter - 32 + 3 : 2 : 1;
	if ((carac >= 98)|| (carac < 0)) {  return new int[1]{ -2 }; }
	
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
		for (int i = 0; i < tokens.size(); i++) {
			if (estado == tokens[i].estado) {
				token = i;
				if (tokens[i].token == "IDENTIFICADOR") { //Si es identificdor, busco si es palabra reservada
					for (int j = 0; j < palabras_reservadas.size(); j++) {
						if (texto == palabras_reservadas[j]) { token = 0; } //Palabra Reservada
					}
				}
			}
		}
		if (token != -1) cout << tokens[token].token << ": " << texto << endl;
		if(token!=-1){
			int aux_column  = Column - texto.length()+1;
			simbol simbol_aux = {tokens[token].token, texto, Line, aux_column};
			AllTokens.push_back(simbol_aux);
			if(simbol_aux.token == "IDENTIFICADOR")
				identificadores.push_back(simbol_aux);
		}
	} else if (exitStatus == -2) {
		cout << "NO VALID : " << texto << endl;
	}
	return (token!=-1);
}

void AutomataLexico(string texto, int index, int estado) {
	int* estados = SiguienteEstado(estado, programa[index]);
	for (int k = 0; k < estados[0]; k++) {
		if (index >= programa.length()-1) {	//Llegue al final, exitStatus es -1 y el estado es al que hubiera ido
			ValidarEstadoToken(texto + programa[index], estados[k + 1], -1);
		} else if (estados[k + 1] < 0) {			//Si el estado es menor a 0, verifico estado final y comienzo desde ahi
			if(ValidarEstadoToken(texto, estado, estados[k + 1]))
				AutomataLexico("", index, 0);
		}else {
			Column++;                               //Si no es menor a 0, me sigo moviendo									
			if (estados[k + 1] == 0) {				//Si regreso a 0 vuelvo a comenzar
				if(programa[index]=='\n'){
					Line = Line + 1;
					Column=0;
				}
				AutomataLexico("", index + 1, estados[k + 1]);
			} else {
				if(programa[index]=='\n'){
					Line = Line + 1;
					Column=0;
				}
				AutomataLexico(texto + programa[index], index + 1, estados[k + 1]);
			}
		}
	}
}

int main() {
	ReadFilePrograma();
	ReadFileLexico();
	ReadFileTokens();
	ReadFilePalabrasReservadas();
	AutomataLexico("", 0, 0);
	PrintTable(1);
	cout<<"\n\n";
	PrintTable(0);
	//system("pause");
	return 0;
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
			tokens.resize(i + 1);
			j = 0;
			while (getline(SSfila, celda, '\t')) {
				if (j == 0) { if (celda != "") { tokens[i].token = celda;} else { j = 3; } }
				if (j == 1) { tokens[i].estado = stoi(celda); }
				j++;
			}	
			if(j==2){ i++; }
		}
		archivo.close();
	}
}
void PrintTableLines(int tokenS, int textoS, int lS, int cS){
	cout<<"+";
	for(int i = 0; i<tokenS; i++)
		cout<<"-";
	cout<<"+";
	for(int i = 0; i<textoS; i++)
		cout<<"-";
	cout<<"+";
	for(int i = 0; i<lS; i++)
		cout<<"-";
	cout<<"+";
	for(int i = 0; i<cS; i++)
		cout<<"-";
	cout<<"+\n";
}
void PrintTable(int op){ // 0 para todos los elementos, 1 para solo los identificadores
	int tokenS=25,textoS=25,lS=6,cS=6;
	vector<simbol> Elements;
	if(op == 0)
		Elements = AllTokens;
	else 
		Elements = identificadores;
	PrintTableLines(tokenS,textoS,lS,cS);
	cout<<"|"<<right<<setw(tokenS)<<"TIPO"<<"|"<<setw(textoS)<<"TOKEN"<<"|"<<setw(lS)<<"LINE"<<"|"<<setw(cS)<<"COLUMN"<<"|\n";
	for(simbol &item : Elements){
		PrintTableLines(tokenS,textoS,lS,cS);
		cout<<"|"<<right<<setw(tokenS)<<item.token<<"|"<<setw(textoS)<<item.texto<<"|"<<setw(lS)<<item.line<<"|"<<setw(cS)<<item.column<<"|\n";
	}
	PrintTableLines(tokenS,textoS,lS,cS);
}
