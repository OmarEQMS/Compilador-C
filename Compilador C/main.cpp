#include <iostream>
#include <string.h>
using namespace std;

int main(int argc, char *argv[]){
	freopen(argv[1], "r", stdin);
	string line;
	while(getline(cin,line)){
		//Recorrer caracter por caracter de la cadena line
		cout<<line<<"\n";
	}

	return 0;
}