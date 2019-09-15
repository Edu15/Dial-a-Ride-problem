#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "darp.h"


void randomMove (RoutesList *neighbor)
{
	int move;
	move = rand()%3;
	switch(move) {
		case 0: {
			reallocate(neighbor);
			break;
		}
		case 1: {
			changePoints(neighbor);
			break;
		}
		case 2: {
			//reorderRoute(neighbor);
			twoOpt(neighbor);
			twoOpt(neighbor);
			break;
		}

	}
}


//Testar antes se a rota nao e vazia.
//Pega e remove vertice
Vertex *takeRandomVertex (Route *rt)
{
  if (rt->first_vx != NULL) 
  {
    Vertex *v = rt->first_vx->next;
    
    if (rt->num_vertices > 2)
    {
		int randNum = rand() % (rt->num_vertices - 2); //Desconsiderando garagens, que nao podem ser removidas.
		int cont = 0;
		while (cont!=randNum) // && v != NULL)
		{
		  cont++;
		  v = v->next;
		}

		if (v != NULL) {
		  v = removeVertexFromRoute (v, rt);
		  return v;
		}
    }
  }

  return NULL;
}


//Retorna endereco do vertice. Nao remove.
Vertex *getRandomVertex (Route *rt)
{
	Vertex *v = NULL;
	if (rt->first_vx != NULL)
	{
		v = rt->first_vx->next;
		if (rt->num_vertices > 2)
		{
			int randNum = rand() % (rt->num_vertices - 2); //Desconsiderando garagens, que nao podem ser removidas.
			int cont = 0;

			while (cont != randNum) // && v != NULL)
			{
			  cont++;
			  v = v->next;
			}
		}
	}
  return v;
}

Route *getRandomRoute(RoutesList *rt_list)
{

	//Escolhendo rota aleatoria
	if (rt_list->num_vehicles != 0)
	{
		int randNum = rand() % rt_list->num_vehicles;
		Route *rt = rt_list->first_rt->next;
		int cont = 0;
		while (cont != randNum)
		{
			cont++;
			rt = rt->next;
		}
		return rt;
	}
	return NULL;
}

//Move um ponto de embarque ou desembarque aleatoriamente para
//outro ponto da mesma rota mantendo a relacao de precedencia.
void reorderRoute (RoutesList *rt_list)
{
	//Escolhendo rota aleatoria
	Route *rt = getRandomRoute(rt_list);
	while (rt->num_vertices <= 4)
		rt = getRandomRoute(rt_list);

	//Escolhendo vertice aleatorio
	Vertex *v2;
	Vertex *v1 = getRandomVertex(rt);

	//printf("pegando vertice %d do veiculo %d\n", v1->id, rt->vehicle);
	if (v1->demand > 0) //Se v1 eh pickup vertex
	{
		//v2 = findVertex (rt, v1->id + rt_list->num_clients);
		v2 = findDropoffVertex (v1); //Retorna vertice de desembarque
		v1 = removeVertexFromRoute (v1, rt);
		//printf("Reinserindo %d antes de %d\n", v1->id, v2->id);
		insertRandomPointRouteBefore (v1, rt, v2);
	}
	else //Se v1 eh dropoff vertex
	{
		v2 = findPickupVertex (v1); //Retorna vertice de embarque
		//printf("Reinserindo %d depois de %d\n", v1->id, v2->id);
		v1 = removeVertexFromRoute (v1, rt);
		insertRandomPointRouteAfter (v1, rt, v2);
	}
	rt->evaluated = FALSE;
}

//Transfere clientes de uma rota para outra aleatoriamente.
//Move um par (pickup e dropoff) aleatorio de uma rota para a outra tambem aleatoriamente.
void reallocate (RoutesList *rt_list)
{
  Route *routeA = getRandomRoute(rt_list);
  while (routeA->num_vertices <= 2)
	  routeA = getRandomRoute(rt_list); //Para evitar de pegar rota sem cliente
  Route *routeB = getRandomRoute(rt_list);
  while (routeB == routeA)
	  routeB = getRandomRoute(rt_list);
  
  Vertex *v1 = getRandomVertex(routeA);
  Vertex *v2;
  if (v1 != NULL)
  {
	  if (v1->demand > 0)
	  {
		  v2 = takeDropoffVertex (routeA, v1); //Pega vertice de desembarque
		  //printf("Realocando vertices %d e %d da rota %d para a rota %d\n", v1->id, v2->id, routeA->vehicle, routeB->vehicle);
		  v1 = removeVertexFromRoute (v1, routeA);
		  insertRandomPointRoute (v1, routeB);
		  insertRandomPointRouteAfter (v2, routeB, v1);
	  }
	  else
	  {
		  v2 = takePickupVertex (routeA, v1); //Pega vertice de embarque
		  //printf("Realocando vertices %d e %d da rota %d para a rota %d\n", v1->id, v2->id, routeA->vehicle, routeB->vehicle);

		  v1 = removeVertexFromRoute (v1, routeA);
		  insertRandomPointRoute (v2, routeB);
		  insertRandomPointRouteAfter (v1, routeB, v2);
	  }
	  routeA->evaluated = FALSE;
	  routeB->evaluated = FALSE;
  }
}

//Pre-requisito: a e b nao devem ser vertice inicial nem final da rota. a e b nao deven ser garagens.
void changeVertices (Vertex *a, Vertex *b)
{

	Vertex *temp = (Vertex*) malloc (sizeof(Vertex));
	temp->next = a->next;
	temp->prev = a->prev;

	a->next = b->next;
	a->prev = b->prev;

	b->next = temp->next;
	b->prev = temp->prev;

	free(temp);

	a->next->prev = a;
	a->prev->next = a;

	b->next->prev = b;
	b->prev->next = b;
}

//Troca clientes trocando pontos de embarque e desembarque
//entre 2 rotas escolhidas aleatoriamente.
void changePoints (RoutesList *rt_list)
{
	//printf("\nCHANGE POINTS\n");
	Vertex *pickupA, *pickupB, *dropoffA, *dropoffB;


	Route *routeA = getRandomRoute(rt_list);
	/*if (routeA->num_vertices < 4) {
		printf("Numero insuficiente de vertices1\n");
		return;
	}*/
	while (routeA->num_vertices < 4)
		routeA = getRandomRoute(rt_list);

	Route *routeB = getRandomRoute(rt_list);
	while (routeB == routeA || routeB->num_vertices < 4)
	  routeB = getRandomRoute(rt_list);

	/*if (routeB->num_vertices < 4)
	{
		printf("Numero insuficiente de vertices2\n");
		return;
	}*/

	Vertex *v = getRandomVertex(routeA);
	//printf("Peguei o vertice %d da rota %d\n", v->id, routeA->vehicle);
	if (v->demand > 0)
	{
		pickupA = v;
		dropoffA = findDropoffVertex(pickupA); //Retorna vertice de desembarque
		//printf("  Encontrei o seu destino: o vertice %d\n", dropoffA->id);
	}
	else
	{
		dropoffA = v;
		pickupA = findPickupVertex (dropoffA); //Retorna vertice de embarque
		//printf("  Ecncontrei a sua origem: o vertice %d\n", pickupA->id);
	}

	v = getRandomVertex(routeB);
	//printf("Peguei o vertice %d da rota %d\n", v->id, routeB->vehicle);
	if (v->demand > 0)
	{
		pickupB = v;
		dropoffB = findDropoffVertex(pickupB); //Retorna vertice de desembarque
		//printf("  Encontrei o seu destino: o vertice %d\n", dropoffB->id);
	}
	else
	{
		dropoffB = v;
		pickupB = findPickupVertex (dropoffB); //Retorna vertice de coleta
		//printf("  Ecncontrei a sua origem: o vertice %d\n", pickupB->id);
	}

	//printf("Trocados os vertices %d e %d da rota %d com %d e %d da rota %d\n", pickupA->id, dropoffA->id, routeA->vehicle, pickupB->id, dropoffB->id, routeB->vehicle);
	changeVertices (pickupA, pickupB);
	changeVertices (dropoffA, dropoffB);
	routeA->evaluated = FALSE;
	routeB->evaluated = FALSE;
}

//2-opt
void twoOpt (RoutesList *rt_list)
{
	boolean movement = FALSE;

	Route *rt = getRandomRoute(rt_list);

	Vertex *v1, *v2;
	uint iterations = 0;

	while (movement == FALSE && iterations < 10)
	{
		//Mantem o loop a procura de uma rota com numero minimo de vertices para fazer o movimento.
		while (rt->num_vertices < 4)
			rt = getRandomRoute(rt_list);

		v1 = getRandomVertex(rt);
		if (v1->next == rt->last_vx)
		{
			v2 = v1->prev;
			if ( (v2->id + rt_list->num_clients) != v1->id)
			{
				v2->next = v1->next;
				v1->prev = v2->prev;
				v2->prev = v1;
				v1->next = v2;

				v1->prev->next = v1;
				v2->next->prev = v2;
				movement = TRUE;
			}
		}
		else
		{
			v2 = v1->next;
			if ( (v1->id + rt_list->num_clients) != v2->id)
			{
				v1->next = v2->next;
				v2->prev = v1->prev;
				v2->next = v1;
				v1->prev = v2;

				v2->prev->next = v2;
				v1->next->prev = v1;
				movement = TRUE;
			}
		}
		iterations++;
		rt->evaluated = FALSE;
		//printf(".");
	}
	//printf ("%d - %d \n", v1->id, v2->id);
}
