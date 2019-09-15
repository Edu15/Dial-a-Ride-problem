#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "darp.h"
#define BUFFER 100

Route *getRoute (RoutesList *rl, int idRota)
{
	Route *rota = rl->first_rt;
	int i;
	for (i = 0; i < idRota; i++) {
		rota = rota->next;
		if (rota == NULL)
			return NULL;
	}
	return rota;
}


void moveVertex (RoutesList *solution, int idRota, int idVerticeA, int idVerticeB)
{
	printf("Na rota %d : movendo o vertice %d para o local do vertice %d.\n", idRota, idVerticeA, idVerticeB);
	//printf("Rota: %d\n", (getRoute(solution, rota))->vehicle);
	
	Route *rota = getRoute(solution, idRota);
	if (rota == NULL) {
		fprintf(stderr, "Identificador incorreto da rota.\n");
		return;
	}
	
	Vertex *verticeA, *verticeB;
	verticeA = takeVertex(rota, idVerticeA);
	verticeB = findVertex(rota, idVerticeB);
	
	insertBeforeVertex(verticeA, verticeB, rota);	
	
}

char *copiarString (char *string)
{
	int tamanho = strlen(string);
	char *copia = (char*) malloc ( (tamanho + 1) * sizeof(char));
	strcpy(copia, string);
	copia[tamanho] = '\0';
	return copia;
}

void undoMove(RoutesList *solution, char *copiaLinhaAnterior)
{
	char *linha = copiaLinhaAnterior;
	char *token = strtok (linha," \n\0");
	
	if (strcmp(token,"mvertex") == 0) {
		int rota, idOrig, idDest;
		rota = atoi(strtok (NULL," \n\0"));	 
		idDest = atoi(strtok (NULL," \n\0"));
		idOrig = atoi(strtok (NULL," \n\0"));
		
		moveVertex(solution, rota, idOrig, idDest);
	}
	
}

/* Shell para a manipulacao manual das rotas programadas*/
/* Comandos:
 * 
 * mvertex <id rota> <id vertice a mover> <id do vertice anterior a nova posicao de insercao>
 * 
 * mclient <id rota origem> <id rota destino> <um dos vertices do cliente a ser movido>
 * 
 */
void openShell (RoutesList *solution)
{
	char linha[BUFFER];
	char *token, *copiaLinhaAnterior = NULL;
	do {
		printf("shell> ");
		fgets(linha, BUFFER, stdin);
		
		
		if (linha[0] != '\n')
		{
			token = strtok (linha," \n\0");
			
			if (linha[0] == 'q') {
				printf("Finalizando shell.\n");
				break;
			}
			else if (strcmp(token,"mvertex") == 0) {
				int rota, idOrig, idDest;
				//char *rota, *idOrig, *idDest;
				rota = atoi(strtok (NULL," \n\0"));	 
				idOrig = atoi(strtok (NULL," \n\0"));
				idDest = atoi(strtok (NULL," \n\0"));
				
				moveVertex(solution, rota, idOrig, idDest);
			}
			else if (strcmp(token,"print") == 0) {
				printRoutesList(solution);
				computeAndPrintSolutionCost(solution);
			}
			else if (strcmp(token,"desfazer") == 0) {
				if (copiaLinhaAnterior == NULL)
					printf("Nada a ser desfeito.\n");
				//else
					//undoMove(solution, copiaLinhaAnterior);
			}
			
			free(copiaLinhaAnterior);
			copiaLinhaAnterior = copiarString(linha);
		}
	} while (1);
	
}
