#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "darp.h"


//Recebe uma string, le o arquivo de mesmo nome e cria e retorna
// uma estrutura contendo todos os dados lidos.
RoutesList *readInputFile(char *file_name)
{
  FILE *file;
  uint num_vehicles, num_clients;
  double max_duration, max_ride_time, max_route_time;
  uint i, num_vertices, max_load;

  file = fopen(file_name, "r");
  if (file == NULL)
    EXIT("ERRO: Nao foi possivel abrir o arquivo.");


  fscanf (file, "%d %d %lf %d %lf", &num_vehicles, &num_vertices, &max_route_time, &max_load, &max_ride_time);
  num_clients = num_vertices/2;
  //printf ("- - - - - LENDO ARQUIVO DE ENTRADA - - - - -\
    //\n #Veiculos: %d\n #Clientes: %d\n #Garagens: %d\n #Tmax viagem: %d\n", \
    //num_vehicles, num_clients,1,max_ride_time );

  Vertex *garage = createEmptVertex(); //Considerando garagem unica
  fscanf (file, "%d %lf %lf %lf %d", &garage->id, &garage->coord.x, &garage->coord.y, &garage->durService, &garage->demand);
  fscanf (file, "%lf %lf", &garage->e_tw, &garage->l_tw);

  //Armazena dados lidos na estrutura
  RoutesList *inicial_rt_list = createInicialRoutesList (num_vehicles, num_clients, max_duration, max_load, max_ride_time, max_route_time);
  inicial_rt_list->first_rt = inicial_rt_list->last_rt = createEmptRoute(0);
  inicial_rt_list->garage = garage;

  //Leitura dos clientes
  for (i = 0; i < num_vertices; i++)
  {
    Vertex *c = createEmptVertex();
    fscanf (file, "%d %lf %lf %lf %d", &c->id, &c->coord.x, &c->coord.y, &c->durService, &c->demand);
    fscanf (file, "%lf %lf", &c->e_tw, &c->l_tw);

    //printf (" ID: %d Coord: (%lf, %lf) ", c->id, c->coord.x, c->coord.y);
    //printf ("Dur serv: %d Passageiros: %d ", c->duration, c->demand);
    //printf("Time window: [%d, %d]\n", c->e_tw, c->l_tw);

    insertVertexEndRoute (c, inicial_rt_list->first_rt);
  }

  fclose (file);
  return inicial_rt_list;
}


//Recebe uma estrutura de lista de rotas proveniente da funcao de leitura de arquivo
// e retorna uma solucao com rotas e clientes atribuidos de forma aleatoria.
RoutesList *createRandomInicialSolution (RoutesList *rt_list)
{
  uint i, num_pickups;
  uint n = rt_list->num_clients;
  uint m = rt_list->num_vehicles;
  Vertex *pickup, *dropoff;

  //Cria rotas com um vertice Garagem (uma para cada veiculo de 1 a m)
  for (i = 0; i < m; i++)
  {
   Route *new_rt = createEmptRoute (i+1);
   new_rt->evaluated = FALSE;
   insertRoute (new_rt, rt_list);
   Vertex *g = copyGarage(rt_list);
   insertVertexEndRoute (g, new_rt);
   new_rt->num_vertices = 1;
  }

  Route *listRequests = rt_list->first_rt;
  Route *vehicleRoute = rt_list->first_rt->next;
  //Vertex *vertexConnection = NULL; /////////////PRECISA DISSO??????

  num_pickups = n; //Numero de pontos de coleta inicialmente.


  //Preenche rotas#define W0 1
  while (listRequests->num_vertices != 0)
  {
    //printf("\nRequisicoes: %d\n",listRequests->num_vertices );
    pickup = takeRandomPickupVertex (listRequests, num_pickups);
    num_pickups--;
    if (pickup == NULL) {
       freeRoutesList (rt_list);
       EXIT("ERRO: Inconsistencia de solicitacoes.\n(Mais desembarques que embarques)");
     }

    insertRandomPointRoute (pickup, vehicleRoute);

    dropoff = takeVertex (listRequests, (pickup->id + n) ); //Pega vertice de desembarque

    if (dropoff == NULL) {
      freeRoutesList (rt_list);
      EXIT("ERRO: Inconsistencia de solicitacoes.\n(Mais embarques que desembarques)");
    }

    insertRandomPointRouteAfter (dropoff, vehicleRoute, pickup);

    vehicleRoute = vehicleRoute->next;
   if (vehicleRoute == NULL)
      vehicleRoute = rt_list->first_rt->next;
  }

  //Reinsere uma copia da garagem no fim da rota.
  vehicleRoute = rt_list->first_rt->next;
  while (vehicleRoute != NULL)
  {
   Vertex *g = copyGarage(rt_list);
   insertVertexEndRoute (g, vehicleRoute);
   vehicleRoute->num_vertices = vehicleRoute->num_vertices + 1;
   vehicleRoute = vehicleRoute->next;
  }

  return rt_list;
}


int comparVertices(const void *a, const void *b)
{
	Vertex *v1 = *(Vertex**)a;
	Vertex *v2 = *(Vertex**)b;
	//int time_v1 = (v1->e_tw + v1->l_tw)/2;
	//int time_v2 = (v2->e_tw + v2->l_tw)/2;
	//printf ("%d - %d = %d\n", time_v1, time_v2, time_v1 - time_v2);
	//return (time_v1 - time_v2);
	return (v1->e_tw - v2->e_tw);
}

double computeInsertionCost(Vertex *p, Vertex *d, Vertex *lastElem)
{
	double cost = 0;
	
	if (p->demand > 0)	{
		double dist = timeBetweenVertices(p, lastElem); 

		cost = dist;
		cost += 100*(MAX (0, dist - p->l_tw - lastElem->departure));		
	}
	else {
		double dist = timeBetweenVertices(p, d) + timeBetweenVertices(d, lastElem);
		cost = dist;
		cost += 100*(MAX (0, dist - d->l_tw - lastElem->departure));
	}
	return cost;
}




RoutesList *createInicialSolution (RoutesList *rt_list)
{
	uint i;
	uint n = rt_list->num_clients;
	uint m = rt_list->num_vehicles;
	Vertex *v, *v_next, *v_prev;
	Vertex *nodes[n];

	v = rt_list->first_rt->first_vx;
	for (i = 0; i < n; i++) {
		if (v->e_tw == E_OPEN_T_WINDOW && v->l_tw == L_OPEN_T_WINDOW)
		{
			v_next = v->next;
			removeVertexFromRoute (v, rt_list->first_rt);
			insertVertexEndRoute (v, rt_list->first_rt);
			v = v_next;
		}
		else
			v = v->next;
	}
	
	

	v = rt_list->first_rt->first_vx;
	for (i = 0; i < n; i++) {
		nodes[i] = v;
		v = v->next;
	}
	/*
	printf("\n Teste\n");
	for (i = 0; i < n; i++) {
		printf("%d ", nodes[i]->id);
	}*/
	
	
	//Ordenando os pontos pelo valor da janela de tempo
	qsort(nodes, n, sizeof(Vertex*), comparVertices);
	/*
	printf("\n Vetor\n");
	for (i = 0; i < n; i++) {
		printf("%d ", nodes[i]->id);
	}*/
	
	
	//Cria rotas com um vertice Garagem (uma para cada veiculo de 1 a m)
	for (i = 0; i < m; i++)	{
		Route *new_rt = createEmptRoute (i+1);
		new_rt->evaluated = FALSE;
		insertRoute (new_rt, rt_list);
		Vertex *g = copyGarage(rt_list);
		insertVertexEndRoute (g, new_rt);
	}


	Route *listRequests = rt_list->first_rt;
	Route *route = rt_list->first_rt->next;
	
	Vertex *pickup, *dropoff;

	double costInsertion, bestCostInsertion;
	Route *bestRouteInsertion;
	//Adiciona o restante dos clientes as rotas.
	for (i = 0; i < n; i++)
	{
		v = removeVertexFromRoute (nodes[i], listRequests);
		if (v->id > getNumClients()) {
			dropoff = v;
			pickup = takeVertex (listRequests, dropoff->id - getNumClients());
		}
		else	{
			pickup = v;
			dropoff = takeVertex (listRequests, pickup->id + getNumClients());
		}


		route = rt_list->first_rt->next;
		bestCostInsertion = INF;

		//printf("--------------\n");


		while (route != NULL) {
			costInsertion = computeInsertionCost(pickup, dropoff, route->last_vx);

			//printf("cost: %.2lf\n", costInsertion);
			if (costInsertion < bestCostInsertion){
				bestCostInsertion = costInsertion;
				bestRouteInsertion = route;
			}
			route = route->next;


		}

		//printf("melhor rota: %d\n", bestRouteInsertion->vehicle );
		insertVertexEndRoute (pickup, bestRouteInsertion);
		insertVertexEndRoute (dropoff, bestRouteInsertion);

		v = pickup;
		v_prev = v->prev;
		v->arrival = v_prev->departure + timeBetweenVertices(v, v_prev);
		v->begService = MAX(v->arrival, v->e_tw);
		v->departure = v->begService + v->durService;

		v = dropoff;
		v_prev = pickup;
		v->arrival = v_prev->departure + timeBetweenVertices(v, v_prev);
		v->begService = MAX(v->arrival, v->e_tw);
		v->departure = v->begService + v->durService;


	}

	route = rt_list->first_rt->next;
	//Reinsere uma copia da garagem no fim da rota.
	for (i = 0; i < m; i++)	{
		Vertex *g = copyGarage(rt_list);
		insertVertexEndRoute (g, route);
		route = route->next;
	}

	return rt_list;
}






// Retorna a distancia euclidiana entre um par de vertices dado.
// Esse valor tambem e usado como o tempo para se deslocar de um vertice para o outro.
double timeBetweenVertices(Vertex *v1, Vertex* v2)
{
  double cost, dif_x, dif_y;
  dif_x = v1->coord.x - v2->coord.x;
  dif_y = v1->coord.y - v2->coord.y;
  cost = sqrt ( dif_x * dif_x + dif_y * dif_y);

  //printf("Dist %d a %d eh: %d\n", v1->id, v2->id, (uint)round(cost));
  return (cost);
}



// Define o valor das variaveis de tempo para cada vertice.
void computeTimesVertices (Vertex *v, Route *route)
{
	Vertex *vPrev, *pickupVertex, *dropoffVertex;

	vPrev = v;
	v = v->next;

	while (v != NULL)
	{
		v->arrival = vPrev->departure + timeBetweenVertices(v, vPrev);
		v->begService = MAX(v->arrival, v->e_tw);
		v->departure = v->begService + v->durService;
		v->wait = v->begService - v->arrival;
		v->load = vPrev->load + v->demand;
		v->rideTime = 0;

		vPrev = v;
		v = v->next;
	}

}


// Define o valor das variaveis de tempo para cada rota.
void distributionHeuristic (RoutesList *routesList)
{
	Vertex *v;
	Route *route = routesList->first_rt->next;
	while (route != NULL)
	{
		//So processa rotas que foram modificadas ou que acabaram de ser inicializadas
		if (route->evaluated == FALSE)
		{
			v = route->first_vx;

			//Inicializa garagem de origem.
			v->arrival = v->begService = v->departure = MAX (v->e_tw,  v->next->e_tw - timeBetweenVertices(v->next, v));
			v->wait = 0;
			v->durService = 0;
			v->load = 0;

			computeTimesVertices (v, route);

			minimizeWaitingTimes(route);
			minimizeRidingTimes(route);
			route->evaluated = TRUE;
		}
		route = route->next;
	}


}

// Minimiza os tempos de viagem para cada cliente, quando possivel, 
// atrasando os tempos de embarque.
void minimizeRidingTimes(Route *route)
{
	//printf("minimizando tempos de viagem:\n");
	double delay, smallestFTS, forwardTimeSlack;
	Vertex *v, *i, *pickupVertex;

	v = route->first_vx;
	while (v != NULL)
	{

		if (v->demand == (-1) && v->wait > 0)
		{
			//printf("Tempo de espera encontrado em: vertice %d da rota %d (%.2lf)\n", v->id, route->vehicle, v->wait);
			smallestFTS = INF;
			//Encontra e retorna o vertice de embarque
			pickupVertex = findPickupVertex(v);

			i = pickupVertex;
			while (i != v)
			{
				forwardTimeSlack = MAX (0, i->l_tw - i->begService);
				i->forwardTimeSlack = forwardTimeSlack;
				if (forwardTimeSlack < smallestFTS)
					smallestFTS = forwardTimeSlack;
				i = i->next;
			}

			//Se possivel atrasar entao...
			if (smallestFTS != 0.0)
			{
				delay = MIN (smallestFTS, v->wait);
				//printf("Delay: %.2lf\n", delay);
				//Atrasando o vertice de embarque
				//printf("Espera no vertice de origem %d passa de %.2lf para %.2lf\n", pickupVertex->id, pickupVertex->wait, pickupVertex->wait + delay );
				pickupVertex->wait += delay;
				pickupVertex->begService += delay;

				pickupVertex->departure += delay;
				//Atrasando vertices intermediarios
				i = pickupVertex->next;
				while (i != v)
				{
					i->arrival += delay;
					i->begService += delay;
					i->departure += delay;
					i = i->next;
				}
				//Removendo o tempo de espera do desembarque
				v->arrival += delay;
				v->wait -= delay;
			}
		}
		v = v->next;
	}
}

// Atrasa os tempos de chegada e partida dos vertices quando possivel,
// reduzindo assim o tempo das rotas.
void minimizeWaitingTimes(Route *route)
{
	double delay, smallestFTS;
	Vertex *v;
	smallestFTS = INF;
	v = route->first_vx;

	while (v != NULL)
	{
		if (v->wait > 0)
		{

			delay = MIN	(smallestFTS, v->wait); //Define o valor de atraso
			smallestFTS = smallestFTS - delay; //Propaga o atraso restante

			v->wait -= delay;
			v->arrival += delay;

			//printf("Tempo de espera encontrado: %lf, Delay = %lf\n", v->wait, delay);

			Vertex *i = route->first_vx;
			while (i != v)
			{
				i->arrival += delay;
				i->begService += delay;
				i->departure += delay;
				i = i->next;
			}
		}
		v->forwardTimeSlack = MAX (0,  v->l_tw - v->begService);

		if (v->forwardTimeSlack < smallestFTS)
			smallestFTS = v->forwardTimeSlack;

		if (smallestFTS == 0) break;
		v = v->next;
	}

}
