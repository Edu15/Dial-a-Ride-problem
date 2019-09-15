#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "darp.h"


RoutesList *variableNeighborhoodSearch (RoutesList *starting_solution, int maxIterations, int lsIterations)
{
	RoutesList *s, *s1, *s2, s_best;
	printf("....BUSCA EM VIZINHANCA VARIAVEL INICIADA....\n\n");
	RoutesList *s_init = localSearch(starting_solution, lsIterations);

	distributionHeuristic (s_init);
	s_init->total_cost = computeSolutionCost(s_init);

	s = copyRoutesList (s_init);
	s_best = copyRoutesList(s_init);
	
	int k = 1;

	//double previous_cost;
	int numIterWithoutImprovement = 0;
	int i = 1;
	while (i < maxIterations && numIterWithoutImprovement < 10)
	{
		s1 = copyRoutesList (s);
		
		randomMove(s1);
				
		distributionHeuristic (s);
		s->total_cost = computeSolutionCost(s);
		
		distributionHeuristic (s1);
		s1->total_cost = computeSolutionCost(s1);
		

		if (s1->total_cost < 1.02*s->total_cost) {
			s1 = localSearch(s1, lsIterations);
			numIterWithoutImprovement = 0;
		}
			
		s2 = copyRoutesList (s1);   //s2 <- s1
		
		//if f(s'') < f(s)
				if (s2->total_cost >= 1.05*s->total_cost) {
					s2 = localSearch(s2, lsIterations);
				}
			s = freeRoutesList(s);
			s = s2;
			k = 0;
		
		if (s2->total_cost < s_best->total_cost) {
			s_best = freeRoutesList(s_best);
			s_best = s2;
			k = k+1;
		}
		
		numIterWithoutImprovement++;
		i++;
	}
	
	printf("RESULTADO VNS - Melhor custo: %.2lf", s_best->total_cost);
	return s_best;
}
