#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include "darp.h"


static int numClients;
int getNumClients() { return numClients;}
void setNumClients(int n) { numClients = n ;}

static int objFuncEvaluation; // Numero de execucoes da funcao objetivo
void addObjFuncEvaluation() { objFuncEvaluation++ ;}

int main(int argc, char **argv)
{
  uint i;
  if (argc < 2 )
    EXIT("Parametro de entrada invalido.");

  RoutesList *solution, *bestSolution;
  bestSolution = createInicialRoutesList (0, 0, 0, 0, 0, 0);
  bestSolution->garage = createEmptVertex();
  bestSolution->total_cost = INF;
  int strSize;

  //printf("\nCusto \tTempo \tIter\n");
  
  for (i = 0; i < 1; i++)
  {
	  srand (time(NULL));
	  objFuncEvaluation = 0;

	  solution = readInputFile(argv[1]);

	  setNumClients(solution->num_clients);

	  //solution = createInicialSolution (solution);
	 
	  solution = createRandomInicialSolution (solution);
	  //distributionHeuristic(solution);
 	 //computeAndPrintSolutionCost(solution);
	 // printRoutesList (solution);
	
		
	  struct timeval initial, final;
	  float time;

	  gettimeofday(&initial, NULL);

	  //localSearch1 para vizinhanca totalmente aleatoria, localSearch2 para vizinhanca 		  
	  // abrangente.
	  if (argc == 3)
	  {
		if (strcmp(argv[2],"-l1") == 0)	{
			//printf("Busca local 1 escolhida");
			solution = iteratedLocalSearch (solution, 500, localSearch1,  50000);
		}
		else if (strcmp(argv[2],"-l2") == 0)  {
			//printf("Busca local 2 escolhida\n");
			solution = iteratedLocalSearch (solution, 500, localSearch2,  50000);
		}
		else  {
			//printf("Busca local padrao escolhida\n");
			solution = iteratedLocalSearch (solution, 500, localSearch1,  50000);
		}
	  }  
	  else {
			//printf("Busca local padrao escolhida\n");
			solution = iteratedLocalSearch (solution, 500, localSearch1,  50000);
	  }

	  gettimeofday(&final, NULL);

	  computeAndPrintSolutionCost(solution);
	  printRoutesList (solution);

	  //solution = postProcessing(solution);
	  
	  //printf("\nApos pos-processamento:\n");
	  //printRoutesList (solution);
	  //computeAndPrintSolutionCost(solution);
	  

	  time = (final.tv_sec - initial.tv_sec);
	  time += (final.tv_usec - initial.tv_usec)/1000000.0;
	  solution->obj_func_eval = objFuncEvaluation;
	  solution->cpu_time = time;
	  
	  
	  //printf("\t%.2f", time);
	  //printf("\t%d\n", objFuncEvaluation);
	  
	if (solution->total_cost < bestSolution->total_cost) {
		bestSolution = freeRoutesList (bestSolution);
		bestSolution = solution;
		solution = NULL;
	}
	else	  
	  	solution = freeRoutesList (solution);

  }
  
  strSize = strlen(argv[1]);
  printf("pr%c%c\t", argv[1][strSize-2], argv[1][strSize-1]);
  
  computeAndPrintSolutionCost(bestSolution);
  
  
  openShell(bestSolution);
  
  
  bestSolution = freeRoutesList (bestSolution);

  return 0;
}

