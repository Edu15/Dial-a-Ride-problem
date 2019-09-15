#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "darp.h"



/*
//Retorna o melhor vizinho de uma solucao com base na analise de um 
// numero definido de movimentos aleatorios.
RoutesList *getBestNeighbor(RoutesList *solution, int num_neighbors)
{
	RoutesList *neighbor, *best_neighbor = copyRoutesList (solution);
	int i = 0;
	//printf("\nGerando Vizinhos:\n");
	do
	{
		neighbor = copyRoutesList (solution);
		randomMove(neighbor);
		distributionHeuristic (neighbor);
		neighbor->total_cost = computeSolutionCost(neighbor);

		//printf("%.2lf < %.2lf ?\n",neighbor->total_cost, best_neighbor->total_cost);
		if (neighbor->total_cost < best_neighbor->total_cost)
		{
			//printf("Novo melhor custo vizinho: %.2lf\n",neighbor->total_cost );
			best_neighbor = freeRoutesList(best_neighbor);
			best_neighbor = neighbor;
			neighbor = NULL;
		}
		else
			neighbor = freeRoutesList(neighbor);

		i++;
	}
	while (i < num_neighbors);

	solution = freeRoutesList(solution);
	return best_neighbor;
}
*/

RoutesList *getBestNeighborByReallocating(RoutesList *solution)
{
	RoutesList *neighbor = solution;
	RoutesList *best_neighbor = copyRoutesList (solution);

	//Selecionando rotas e vertices que participarao dos movimentos
	Route *routeA = getRandomRoute(neighbor);
	while (routeA->num_vertices <= 2)
	  routeA = getRandomRoute(neighbor); //Para evitar de pegar rota sem cliente
	Route *routeB = getRandomRoute(neighbor);
	while (routeB == routeA)
	  routeB = getRandomRoute(neighbor);

	Vertex *v2, *v1 = getRandomVertex(routeA);

	if (v1->demand > 0)
		v2 = findDropoffVertex(v1);
	else	{
		v2 = v1;
		v1 = findPickupVertex(v2);
	}
	v2 = removeVertexFromRoute (v2, routeA);
	v1 = removeVertexFromRoute (v1, routeA);
	routeA->evaluated = FALSE;

	//Gerando e computando movimentos


	Vertex *i, *j, *lastVx = routeB->last_vx;
	i = routeB->first_vx;
	//int cont = 0;

	while (i != lastVx)
	{
		insertAfterVertex (v1, i, routeB);
		j = v1;
		while (j != lastVx)
		{
			insertAfterVertex (v2, j, routeB);
			routeB->evaluated = FALSE;
			distributionHeuristic (neighbor);

			//Reduz numero de combinacoes computadas
			if (v2->arrival - v1->departure > neighbor->max_ride_time)
			{
				v2 = removeVertexFromRoute (v2, routeB);
				break;
			}
			neighbor->total_cost = computeSolutionCost(neighbor);
			if (neighbor->total_cost < best_neighbor->total_cost)
			{
				//printf("custo vizinho: %.2lf < %.2lf\n",neighbor->total_cost, best_neighbor->total_cost);
				best_neighbor = freeRoutesList(best_neighbor);
				best_neighbor = copyRoutesList (neighbor);
			}

			v2 = removeVertexFromRoute (v2, routeB);
			j = j->next;
			//cont++;
		}
		v1 = removeVertexFromRoute (v1, routeB);
		i = i->next;
	}
	//printf("%d vizinhos\n", cont);

	freeVertices(v1);
	freeVertices(v2);
	neighbor = freeRoutesList(neighbor);
	return best_neighbor;
}


RoutesList *getBestNeighborByReordering(RoutesList *solution)
{
	RoutesList *neighbor = solution;
	RoutesList *best_neighbor = copyRoutesList (solution);

	//Escolhendo rota aleatoria
	Route *route = getRandomRoute(neighbor);
	while (route->num_vertices <= 4)
		route = getRandomRoute(neighbor);

	//Escolhendo vertice aleatorio
	Vertex *v2, *v1 = getRandomVertex(route);

	if (v1->demand > 0) //Se v1 eh pickup vertex
	{
		v2 = findDropoffVertex (v1); //Retorna vertice de desembarque
		v1 = removeVertexFromRoute (v1, route);
		Vertex *v = route->first_vx;

		while (v != v2)
		{

				insertAfterVertex (v1, v, route);
				route->evaluated = FALSE;
				distributionHeuristic (neighbor);
				if (v2->arrival - v1->departure < solution->max_ride_time) //Evita computar custo desnecessariamente
				{
					neighbor->total_cost = computeSolutionCost(neighbor);
					if (neighbor->total_cost < best_neighbor->total_cost)
					{
						//printf("custo vizinho: %.2lf < %.2lf\n\n",neighbor->total_cost, best_neighbor->total_cost);
						best_neighbor = freeRoutesList(best_neighbor);
						best_neighbor = copyRoutesList (neighbor);
					}
				}
				v1 = removeVertexFromRoute (v1, route);
				v = v->next;
		}
	}
	else //Se v1 eh dropoff vertex
	{
		v2 = findPickupVertex (v1); //Retorna vertice de embarque
		v1 = removeVertexFromRoute (v1, route);
		Vertex *v = route->last_vx;

		while (v != v2)
		{
			insertBeforeVertex (v1, v, route);
			route->evaluated = FALSE;
			distributionHeuristic (neighbor);
			neighbor->total_cost = computeSolutionCost(neighbor);
			if (neighbor->total_cost < best_neighbor->total_cost)
			{
				//printf("custo vizinho: %.2lf < %.2lf\n\n",neighbor->total_cost, best_neighbor->total_cost);
				best_neighbor = freeRoutesList(best_neighbor);
				best_neighbor = copyRoutesList (neighbor);
			}
			v1 = removeVertexFromRoute (v1, route);
			v = v->prev;
		}
	}

	freeVertices(v1);
	neighbor = freeRoutesList(neighbor);
	return best_neighbor;
}


RoutesList *getBestNeighborByChangingPoints(RoutesList *solution)
{
	Vertex *pickupA, *pickupB, *dropoffA, *dropoffB;
	RoutesList *neighbor = solution;
	RoutesList *best_neighbor = copyRoutesList (solution);

	Route *routeA = getRandomRoute(neighbor);
	while (routeA->num_vertices < 4)
			routeA = getRandomRoute(neighbor);

	Route *routeB = getRandomRoute(neighbor);
	while (routeB == routeA || routeB->num_vertices < 4)
	  routeB = getRandomRoute(neighbor);

	Vertex *v = getRandomVertex(routeA);

	if (v->demand > 0) {
		pickupA = v;
		dropoffA = findDropoffVertex(pickupA);
	}
	else {
		dropoffA = v;
		pickupA = findPickupVertex (dropoffA);
	}

	Vertex *j, *lastVx = routeB->last_vx;
	j = routeB->first_vx->next;

	while (j != lastVx)
	{
		if (j->demand > 0) {
			pickupB = j;
			dropoffB = findDropoffVertex(pickupB);

			//Troca vertices
			changeVertices (pickupA, pickupB);
			changeVertices (dropoffA, dropoffB);
			//printf("Trocando %d por %d e %d por %d\n", pickupA->id, pickupB->id, dropoffA->id, dropoffB->id);
			routeA->evaluated = FALSE;
			routeB->evaluated = FALSE;

			distributionHeuristic (neighbor);
			neighbor->total_cost = computeSolutionCost(neighbor);
			if (neighbor->total_cost < best_neighbor->total_cost)
			{
				//printf("custo vizinho: %.2lf < %.2lf\n\n",neighbor->total_cost, best_neighbor->total_cost);
				best_neighbor = freeRoutesList(best_neighbor);
				best_neighbor = copyRoutesList (neighbor);
			}

			//Destroca vertices
			changeVertices (pickupA, pickupB);
			changeVertices (dropoffA, dropoffB);
		}
		j = j->next;
	}

	neighbor = freeRoutesList(neighbor);
	return best_neighbor;
}

/*double randomBetween0And1 ()
{
  float x = rand()%1000000;
  x = x/1000000;
  //printf("Random entre 0 e 1: %f", x);
  return (double) x;
}*/


RoutesList *localSearch2(RoutesList *starting_solution, int num_iterations)
{
	//printf("........BUSCA LOCAL INICIADA........\n");
	RoutesList *solution = starting_solution;
	distributionHeuristic (solution);
	solution->total_cost = computeSolutionCost(solution);
	//printf("Custo inicial: %.2lf\n", solution->total_cost);

	double previous_cost;
	int numIterWithoutImprovement = 0;
	int move, i = 0;
	//double move;

	do
	{
		previous_cost = solution->total_cost;
		//solution = getBestNeighbor(solution, 100);

		move = rand()%3;
		switch(move) {
			case 0: {
				solution = getBestNeighborByReallocating(solution);
				break;
			}
			case 1: {
				solution = getBestNeighborByReordering(solution);
				break;
			}
			case 2: {
				solution = getBestNeighborByChangingPoints(solution);
				break;
			}
		}

		//move = randomBetween0And1();

		//if (move < 0.4)
		//	solution = getBestNeighborByReallocating(solution);
		//else if (move < 0.8)
		//	solution = getBestNeighborByReordering(solution);
		//else
		//	solution = getBestNeighborByChangingPoints(solution);

		//solution = getBestNeighborByReallocating(solution);
		//solution = getBestNeighborByReordering(solution);

		if (previous_cost == solution->total_cost)
			numIterWithoutImprovement++;
		else
			numIterWithoutImprovement = 0;

		i++;
	}
	while (i < num_iterations && numIterWithoutImprovement < 120);

	//printf("RESULTADO - Melhor custo: %.2lf\n\n", solution->total_cost);
	return solution;
}





RoutesList *localSearch1(RoutesList *starting_solution, int num_interactions)
{
	//printf("........BUSCA LOCAL INICIADA........\n");

	RoutesList *current_solution = starting_solution;
	distributionHeuristic (current_solution);
	double current_cost = computeSolutionCost(starting_solution);
	//printf("Custo inicial: %.2lf\n", current_cost);

	RoutesList *neighbor = copyRoutesList (current_solution);
	double neighbor_cost = current_cost;

	int numIterWithoutImprovement = 0;
	int i = 0;
	do
	{
		randomMove(neighbor);
		distributionHeuristic (neighbor);
		neighbor_cost = computeSolutionCost(neighbor);

		if (neighbor_cost < current_cost)
		{
			//printf(" Custo melhor encontrado: %.2lf\n", neighbor_cost);
			freeRoutesList (current_solution);
			current_solution = neighbor;
			current_cost = neighbor_cost;
			numIterWithoutImprovement = 0;
		}
		else
		{
			freeRoutesList (neighbor);
			numIterWithoutImprovement++;
		}

		neighbor = copyRoutesList (current_solution);
		i++;
	} while (i < num_interactions && numIterWithoutImprovement < 5000);

	//printf("RESULTADO - Melhor custo: %.2lf\n", current_cost);

	neighbor = freeRoutesList (neighbor);
	return current_solution;
}


void perturbate (RoutesList *rt_list)
{
	//reorderRoute(rt_list);
	//reallocate(rt_list);
	//changePoints(rt_list);
	uint i, iterations = rt_list->num_vehicles;
	for (i = 0; i < iterations; i++)
		twoOpt(rt_list);

}



RoutesList *iteratedLocalSearch(RoutesList *starting_solution, int maxIterations, RoutesList *(*localSearch)(RoutesList*, int),int subIterations)
{
	//printf("....BUSCA LOCAL ITERADA INICIADA....\n\n");
	RoutesList *best_solution = localSearch(starting_solution, subIterations);

	distributionHeuristic (best_solution); //realmente necessario?
	best_solution->total_cost = computeSolutionCost(best_solution);

	RoutesList *current_solution = copyRoutesList (best_solution);
	current_solution->total_cost = best_solution->total_cost;

	//double previous_cost;
	int numIterWithoutImprovement = 0;
	int i = 1;
	while (i < maxIterations && numIterWithoutImprovement < 10)
	{
		//previous_cost = current_solution->total_cost;
		perturbate(current_solution);
		current_solution = localSearch(current_solution, subIterations);
		//printf("numIterWithoutImprovement = %d\n", numIterWithoutImprovement);
		distributionHeuristic (current_solution);
		current_solution->total_cost = computeSolutionCost(current_solution);
		//printf("RESULTADO - Melhor custo: %.2lf\n", current_solution->total_cost);

		/*if ( abs(previous_cost - current_solution->total_cost) < 0.001 )
		{
			printf("======|%.2lf - %.2lf| < 0.001\n\n",previous_cost, current_solution->total_cost );
			//break;
		}*/

		if (current_solution->total_cost < best_solution->total_cost)
		{
			best_solution->total_cost = current_solution->total_cost;
			best_solution = freeRoutesList(best_solution);
			best_solution = copyRoutesList (current_solution);
			numIterWithoutImprovement = 0;
		}
		else
		{
			current_solution = freeRoutesList(current_solution);
			current_solution = copyRoutesList (best_solution);
			numIterWithoutImprovement++;
		}

		i++;
	}

	current_solution = freeRoutesList (current_solution);
	//printf("RESULTADO ILS - Melhor custo: %.2lf", best_solution->total_cost);
	//printf("%.2lf", best_solution->total_cost);
	return best_solution;
}


