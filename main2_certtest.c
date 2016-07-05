#include "miracl.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <dirent.h>

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"


#define MOD_TAM 1200

#define TYPE_PEM 0
#define TYPE_DER 1
#define TIPO_CUSTOM 3

#define TAM_MODULE 1200
#define NUM_DIGITOS_MIRACL 1200
#define BASE_MIRACL 16
#define TIPO_BIN 1
#define TIPO_ASCII 2

#define setDebug 0

#define NORMAL 0
#define IGUAL 1
#define PERIGO 2
#define NONE 3

#define FX_ini 100
#define FX_fin 400

//site importante
//http//www.mobilefish.com/services/rsa_key_generation/rsa_key_generation.php
//viet spider
//httrack
//compilar com
//gcc -Wall main2_certtest.c -lmiracl -lcrypto -o exe

//warning variavel global
char nomeProOutput[200];


typedef struct t_certificate{
	char filename[300];
	char module[TAM_MODULE];
	int certType;
}t_certificate;




char* mdcComStrings(char* x, char* y,char* result){
	big xd = mirvar(0);
	big yd = mirvar(0);
	big mdcResult = mirvar(0);
	int ret1,ret2;
	
	if(setDebug){
		printf("mdc\n");
		printf("%s\ninput2\n%s\n\n",x,y);
	}
	
	
	big xBig = mirvar(0);
	big yBig = mirvar(0);
	
	ret1 = cinstr(xBig,x);
	ret2 = cinstr(yBig,y);
	
	if(ret1 < 100 || ret2 < 100){
		printf("Cuidado, input de mdc inesperado\n");
		printf("Inputs recebidos\n%s\ninput2\n%s\n\n",x,y);
		
	}
	
	if(setDebug){
		printf("Fazendo o mdc com strings\n");
	}
	
	xgcd(xBig,yBig,xd,yd,mdcResult);
	
        //printf("Passou do xgcd\n");
            
	int len = cotstr(mdcResult,result);
	
	assert(len != 0);
	
	mirkill(xd);
	mirkill(yd);
	mirkill(xBig);
	mirkill(yBig);
	mirkill(mdcResult);
	
        //printf("Passou de todo o mdc\n");
        
	return result;
}


char* readFile(char* filename,char* result)
{
    FILE* file = fopen(filename,"r");
	
	if(setDebug){
		printf("Filename recebido %s\n",filename);
	}
    if(file == NULL)
    {
		printf("nofile\n");	
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long int size = ftell(file);
    rewind(file);

    char* content = calloc(size + 1, 1);

    fread(content,1,size,file);
	
	strcpy(result,content);
	
	fclose(file);
    return content;
}

//retorna erros para serem analizados
int recebeDiretorio(char* diretorio){
	DIR* dirAtual;
	struct dirent *arquivo;
	int i;
	int numNos;
	char prefix[200];
	int ind = 0;
	strcpy(prefix,diretorio);
	char c;
	int barra = 0;
	while((c = prefix[ind]) != '\0'){
		if(c == '/' && prefix[ind+1] == '\0'){
			barra = 1;
		}
		ind++;
	}
	if(!barra){
		char minhabarra[2];
		minhabarra[0] = '/';
		minhabarra[1] = '\0';
		strcat(prefix,minhabarra);
	}
	
	
	if((dirAtual = opendir(diretorio)) == NULL){
		return 1;
	}
	
	numNos = 0;
	while((arquivo = readdir(dirAtual)) != NULL){
		if(strcmp(arquivo->d_name,"..") && strcmp(arquivo->d_name,".")){
			numNos++;
		}
	}
	
	t_certificate *certificadosVetor = (t_certificate*) calloc(numNos,sizeof(t_certificate));
	if(certificadosVetor == NULL){
		printf("erro no calloc\n");
		
	}
	
	
	rewinddir(dirAtual);
	
	char stringModulo[TAM_MODULE];
	char straux[100];
	i = 0;
	while((arquivo = readdir(dirAtual)) != NULL){
		if(strcmp(arquivo->d_name,"..") && strcmp(arquivo->d_name,".")){
			strcpy(certificadosVetor[i].filename,arquivo->d_name);
			strcpy(straux,prefix);
			strcat(straux,arquivo->d_name);
			readFile(straux,stringModulo);
			strcpy(certificadosVetor[i].module,stringModulo);
			i++;
			if(setDebug){
				printf("%s\n",stringModulo);
			}
		}
	}
	
	
	FILE* ptResultado;
	ptResultado = fopen(nomeProOutput,"w");
        
        if(ptResultado == NULL){
            printf("Erro ao criar arquivo de output!\n");
            exit(1);
        }
	
	int j;
	char mdcResultado[TAM_MODULE];
	char moduloA[TAM_MODULE];
	char moduloB[TAM_MODULE];
	int situacaoAtual;
	int tamModA;
	int tamModResult;
	for(i=0;i<numNos;i++){
            if(!setDebug){
                system("clear");
            }
            printf("Progresso: %.2f%%\n",((float)i/(float)numNos) * 100);
		for(j = i + 1;j<numNos;j++){
			//fazer mdc entre i e j
			strcpy(moduloA,certificadosVetor[i].module);
			strcpy(moduloB,certificadosVetor[j].module);
			if(setDebug){
				printf("mdc entre\n%s e %s\n",certificadosVetor[i].filename,certificadosVetor[j].filename);
			}
			
			mdcComStrings(moduloA,moduloB,mdcResultado);
			
			tamModA = strlen(moduloA);
			tamModResult = strlen(mdcResultado);
			
			
			if(!strcmp(mdcResultado,"1")){
				situacaoAtual = NORMAL;
			}else if(tamModResult - tamModA >= -3 && tamModResult - tamModA <= 3){
				situacaoAtual = IGUAL;
				fprintf(ptResultado,"***********************************\nMDC\n%s  x  %s\nModulos iguais\n%s\n\n",certificadosVetor[i].filename,certificadosVetor[j].filename,mdcResultado);
			}else{
				situacaoAtual = PERIGO;
				fprintf(ptResultado,ANSI_COLOR_RED"***********************************\nMDC\n%s  x  %s\nPrimo em comum\n%s\n\n"ANSI_COLOR_RESET,certificadosVetor[i].filename,certificadosVetor[j].filename,mdcResultado);
			}
			
			
		}
		fflush(ptResultado);
	}
	fclose(ptResultado);
	
	return 0;
}






//recebe 2 ou 3 argumentos, sendo que se receber 2 tem q ser help, se for 3 eh a execuçao padrao em 2 certificados
int main(int argc, char** argv){
	int retDir;
	char diretorioOut[200];
	
	miracl *mip = mirsys(NUM_DIGITOS_MIRACL, BASE_MIRACL);
	mip->IOBASE = 16;
	
	if(argc != 3){
		printf("Deve-se passar um diretorio contendo módulos RSA e um nome para output gerado automaticamente na pasta outputs/\n");
		return 1;
	}else{
		strcpy(diretorioOut,"outputs/");
		strcat(diretorioOut,argv[2]);
		strcpy(nomeProOutput,diretorioOut);
		retDir = recebeDiretorio(argv[1]);
	}
	
	if(retDir == 1){
		printf("Diretorio especificado nao existe\n");
		
	}
	
	
	return 0;
}	