#include "Utility.h"

// Checks if binary were correctly generated
// Return value: None (void)
void binarioNaTela(char *nomeArquivoBinario) {
	unsigned long i, cs;
	unsigned char *mb;
	size_t fl;
	FILE *fs;
	if(nomeArquivoBinario == NULL || !(fs = fopen(nomeArquivoBinario, "rb"))) {
		fprintf(stderr, "ERRO AO ESCREVER O BINARIO NA TELA (função binarioNaTela): não foi possível abrir o arquivo que me passou para leitura. Ele existe e você tá passando o nome certo? Você lembrou de fechar ele com fclose depois de usar?\n");
		return;
	}
	fseek(fs, 0, SEEK_END);
	fl = ftell(fs);
	fseek(fs, 0, SEEK_SET);
	mb = (unsigned char *) malloc(fl);
	fread(mb, 1, fl, fs);

	cs = 0;
	for(i = 0; i < fl; i++) {
		cs += (unsigned long) mb[i];
	}
	printf("%lf\n", (cs / (double) 100));
	free(mb);
	fclose(fs);
}

// Transform input from stdin formatting into CSV-like
// Return value: None (void)
void tranformToCsvFormat(char *inputString) {
	bool isQuote = false;
	for(int i = 0; inputString[i] != '\0'; i++) {
		if(inputString[i] == '"') {
			isQuote = !isQuote;
			for(int j = i; inputString[j] != '\0'; j++) {
				inputString[j] = inputString[j+1];
			}
		}
		if(inputString[i] == ' ' && !isQuote) {
			inputString[i] = ',' ;
		}
	}
}
