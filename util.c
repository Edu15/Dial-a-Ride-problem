#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "darp.h"


//Recebe um ponteiro "c" para um vertice ou uma sequencia de vertices
//e libera a memoria alocada para eles.
void freeVertices (Vertex *c)
{
  Vertex *temp;
  while (c != NULL)
  {
    temp = c;
    c = c->next;
    free(temp);
  }
}

//Recebe um ponteiro "r" para o primeiro elemento de uma lista encadeada
//de rotas (de um veiculo) e libera a memoria alocada pela lista.
void freeRoutes (Route *r)
{
  Route *temp;
  while (r != NULL)
  {
    temp = r;
    freeVertices(r->first_vx);    
    r = r->next; 
    free(temp);
  }
}

//Recebe um ponteiro "rt_list" para a estrutura contendo todos os dados do problema,
//libera a memoria alocada para toda a estrutura e retorna um ponteiro nulo.
RoutesList *freeRoutesList (RoutesList *rt_list)
{
  free(rt_list->garage);
  freeRoutes(rt_list->first_rt);
  free(rt_list);
  return NULL;  
}

//Recebe como parametro as informacoes basicas necessarias para a definicao do problema
//e cria uma estrutura para armazenar essas informacoes. Como essa funcao nao cria as rotas,
//os pronteiros para a lista de rotas sao inicializados como nulos.
//Retorna a estrutura criada.
RoutesList *createInicialRoutesList(uint num_vehicles, uint num_clients, double max_duration, uint max_load, double max_ride_time, double max_route_time)
{
  RoutesList *rt_list = (RoutesList*) malloc (sizeof(RoutesList));
  rt_list->num_vehicles = num_vehicles;
  rt_list->num_clients = num_clients;
 
  rt_list->max_duration = max_duration;
  rt_list->max_load = max_load;
  rt_list->max_ride_time = max_ride_time;
  rt_list->max_route_time = max_route_time;
  rt_list->first_rt = NULL;
  rt_list->last_rt = NULL;
  rt_list->total_cost = 0;
  rt_list->obj_func_eval = 0;
  rt_list->cpu_time = 0;
  return rt_list;
}

//Recebe um ponteiro "c" para um vertice e imprime seus dados armazenados.
void printVertices (Vertex *c)
{
  while (c != NULL)
  {
   printf ("ID: %.3d\tD: %d\t", c->id, c->demand);
    //printf (" ID: %.2d Coord: (%.3lf, %.3lf)\t", c->id, c->coord.x, c->coord.y);
    //printf ("Dur serv: %.2lf\tDemanda: %d\t", c->durService, c->demand);
    printf("TW: [%.2lf, %.2lf]\t", c->e_tw, c->l_tw);
    printf("A: %.2lf W: %.2lf B: %.2lf D: %.2lf Load: %d", c->arrival, c->wait, c->begService, c->departure, c->load);
    if (c->rideTime != 0.0)
		printf(" R: %.2lf",  c->rideTime);
		
    //printf(" F: %.2lf\n", c->forwardTimeSlack);
    //printf(" Dist: %2lf\n", c->dist);

    /////////////TEMP////////////
   // printf (" V: %.2lf\n", c->twViolations);
   if ( c->twViolations != 0) printf(" *");
	
   printf("\n");
    c = c->next;
  }
}

void printRoute (Route *rt)
{
	printf("- - - - - Rota %d: - - - - - \n", rt->vehicle);
	printf(" Tamanho: %d vertices \n", rt->num_vertices);
	printVertices (rt->first_vx);
}


void printRoutesList (RoutesList *rt_list)
{
  printf ("- - - - - DETALHES DO PROBLEMA - - - - -\n");
  printf(" #Veiculos: %d\n #Clientes: %d\n #Garagens: %d\n", \
    rt_list->num_vehicles, rt_list->num_clients, 1);
 
  Route *rt = rt_list->first_rt;
  
  while (rt != NULL)
  {
    printRoute(rt);
    rt = rt->next;
  }
}

Route *createEmptRoute (uint vehicle)
{
  Route *rt = (Route*) malloc (sizeof(Route));
  rt->vehicle = vehicle;
  rt->first_vx = NULL;
  rt->last_vx = NULL;
  rt->num_vertices = 0;
  rt->next = NULL;
  return rt;
}

void  insertRoute (Route *rt, RoutesList * rt_list)
{
  if (rt_list->last_rt == NULL)
  {
    if (rt_list->first_rt == NULL)
    {
      rt_list->first_rt = rt;
      rt_list->last_rt = rt;
    }
  } 
  else
  {
    rt_list->last_rt->next = rt;
    rt_list->last_rt = rt;
  }
}

Vertex *createEmptVertex()
{
  Vertex *c = (Vertex*) calloc (1, sizeof(Vertex));
  c->next = NULL;
  c->prev = NULL;

  return c;
}


void insertVertexEndRoute (Vertex *c, Route *rt)
{
   if (rt->last_vx == NULL)
   {
     if (rt->first_vx == NULL)
       rt->first_vx = c;
     
     rt->last_vx = c;
   }
   else
   {
      rt->last_vx->next = c;
      c->prev = rt->last_vx;
      rt->last_vx = c;
   }
   c->next = NULL;
   rt->num_vertices +=1;
}

Vertex *removeVertexBeginningRoute (Route *rt)
{
  Vertex *temp = rt->first_vx;
  rt->first_vx = rt->first_vx->next;
  if (temp == rt->last_vx)
    rt->last_vx = NULL;
  return temp;
}

Vertex *copyGarage (RoutesList *rt_list)
{
  Vertex *g = (Vertex*) malloc (sizeof (Vertex));
  g->id = rt_list->garage->id;
  g->coord.x = rt_list->garage->coord.x;
  g->coord.y = rt_list->garage->coord.y;
  g->e_tw = rt_list->garage->e_tw;
  g->l_tw = rt_list->garage->l_tw;
  g->durService = 0;
  g->demand = 0;
  g->arrival =0;
  g->begService = 0;
  g->departure = 0;
  g->rideTime = 0;
  g->forwardTimeSlack = 0;
  g->next = NULL;
  return g;
}



void plotSolution (RoutesList *rt_list)
{
  FILE *file = fopen("solution.txt", "w");
  
  Route *rt_pointer = rt_list->first_rt->next;
  
  fprintf (file, "%d %.3lf %.3lf\n",rt_list->garage->id, rt_list->garage->coord.x, rt_list->garage->coord.y);
 
  while (rt_pointer != NULL)
  {
    Vertex *vx_pointer = rt_pointer->first_vx->next;
    while (vx_pointer != NULL)
    {
      fprintf (file, "%d %.3lf %.3lf\n",vx_pointer->id, vx_pointer->coord.x, vx_pointer->coord.y);
      vx_pointer = vx_pointer->next;
    }
    rt_pointer = rt_pointer->next;
  }
  fclose(file); 
  
  system(" gnuplot -p -e \"plot \
  'solution.txt' u 1:2:(0.5) with circles linecolor rgb 'white' lw 2 fill solid border lc rgb 'dark-blue' notitle, \
 '' using 1:2:1  with labels tc rgb 'dark-blue' offset (0,0) font 'Arial Bold' title \'Random\' \" ");
  
}


/*
double computeCostVertice (Vertex *origin, Vertex *request)
{
  double cost, dif_x, dif_y, timeRequest, timeOrigin;
  dif_x = request->coord.x - origin->coord.x;
  dif_y = request->coord.y - origin->coord.y;
  cost = sqrt ( dif_x * dif_x + dif_y * dif_y);
  
  timeRequest = request->e_tw; //Escolha com base no inicio da janela de tempo
  timeOrigin = origin->e_tw;
  printf("Calc: %lf + (%lf - %lf)\n", cost,timeRequest,timeOrigin);
  //cost = cost + (timeRequest - timeOrigin);
  
  if (timeRequest >= timeOrigin)
    cost = cost + (timeRequest - timeOrigin);
  else
    cost = INF;
  
  
  return cost;
}
*/

Vertex *removeVertexFromRoute (Vertex *v, Route *route)
{
	if (v == NULL)
		return NULL;

	if (v->prev == NULL)
	{
		route->first_vx = v->next;
		if (route->first_vx != NULL)
			route->first_vx->prev = NULL;
			//printf("Removeu primeiro\n");
	}
	else
		v->prev->next = v->next;

	if (v->next == NULL)
	{
		route->last_vx = v->prev;
		if (route->last_vx != NULL)
			route->last_vx->next = NULL;
			//printf("Removeu ultimo\n");
	}
	else
		v->next->prev = v->prev;

	v->next = NULL;
	v->prev = NULL;
	route->num_vertices -= 1;
	return v;
}
  

Vertex *findVertex (Route *route, uint numVertex)
{
  Vertex *vx = route->first_vx;

  while (vx != NULL)
  {
    if (vx->id == numVertex) {
      return vx;
    }
    vx = vx->next;
  }
  return NULL;
}

Vertex *findDropoffVertex (Vertex *pickup)
{
  uint numVertex = pickup->id + getNumClients();
  //printf("procurando por %d\n", pickup->id);
  Vertex *vx = pickup->next;
  while (vx != NULL)
  {
    if (vx->id == numVertex) {
      return vx;
    }
    vx = vx->next;
  }

  return NULL;
}

Vertex *findPickupVertex (Vertex *dropoff)
{
  uint numVertex = dropoff->id - getNumClients();
  //printf("procurando por %d\n", dropoff->id);
  Vertex *vx = dropoff->prev;

  while (vx != NULL)
  {
    if (vx->id == numVertex) {
      return vx;
    }
    vx = vx->prev;
  }
  return NULL;
}


Vertex *takeVertex (Route *route, uint numVertex)
{
  Vertex *vertex = findVertex (route, numVertex);
  if (vertex == NULL) {
	  fprintf(stderr, "Identificador incorreto do vertice. Vertice nao removido.\n");
	  return;
  }
  vertex = removeVertexFromRoute (vertex, route);

  return  vertex;
}

Vertex *takeDropoffVertex (Route *route, Vertex *pickup)
{
  Vertex *dropoff = findDropoffVertex (pickup);
  dropoff = removeVertexFromRoute (dropoff, route);

  return  dropoff;
}

Vertex *takePickupVertex (Route *route, Vertex *dropoff)
{
  Vertex *pickup = findPickupVertex (dropoff);
  pickup = removeVertexFromRoute (pickup, route);

  return  pickup;
}


//Recebe dois vertices v1 (avulso) e v2 (numa rota) e uma estrutura de rota rt e
//insere v1 antes de v2 na lista de vertices dentro da rota.
void insertBeforeVertex (Vertex *v1, Vertex *v2, Route *rt)
{
  Vertex *previous = v2->prev;
  
  v1->next = v2;
  v2->prev = v1;
  
  if (previous == NULL)
  {
    rt->first_vx = v1;
    v1->prev = NULL;
  }
  else
  {
    previous->next = v1;
    v1->prev = previous;
  }
  rt->num_vertices = rt->num_vertices +1;
}

void insertAfterVertex (Vertex *v1, Vertex *v2, Route *rt)
{
  Vertex *next = v2->next;
  
  v2->next = v1;
  v1->prev = v2;
  	 
  if (next == NULL)
  {
    rt->last_vx = v1;
    v1->next = NULL;
  }
  else
  {
    next->prev = v1;
    v1->next = next;
  }
  rt->num_vertices = rt->num_vertices +1;
}

/*
//Recebe um vertice de desembarque (dropoff), um vertice de embarque (pickup) e uma rota.
//Insere o vertice dropoff na lista de vertices da rota de forma ordenada,
//respeitando o valor inicial da janela de tempo (e_tw) de cada vertice.
void insertDropoffVertice (Vertex *dropoff, Vertex *pickup, Route *vehicleRoute)
{
  Vertex *v = pickup;
  Vertex *bestCandidateOrigin = NULL;
  double cost, bestCost;
  
  //printf("----Best dropoff----\n");
  bestCost = INF;
  while (v != NULL)
  {
    cost = computeCostVertice (v, dropoff);
    //printf("Custo: %lf\n", cost);
    if (cost < bestCost)
    {
      //printf("Atualiza custo\n");
      bestCost = cost;
      bestCandidateOrigin = v;
    }
    v = v->next;
  }
  //printf ("bestCandidateOrigin: %d\n", bestCandidateOrigin->id);
    insertAfterVertex (dropoff, bestCandidateOrigin, vehicleRoute);Vertex *
}
*/


Vertex *takeRandomPickupVertex (Route *listRequests, int num_pickups)
{
  Vertex *v = listRequests->first_vx;
  Vertex *vPrev = v;
  
  int randNum = rand() % (num_pickups);
  int cont = 0;
  while (cont!=randNum && v != NULL)
  {
    vPrev = v;
    v = v->next;
    if (v->demand == 1) cont++;
  }
  
  if (v != NULL) {
    v = removeVertexFromRoute (v, listRequests);
    //printf("Removido vertice : %d\n", v->id); 
  }
  //else 
    //printf("Sem mais pontos de coleta na lista.\n");
   
  return v;
}





void insertRandomPointRoute (Vertex *pickup, Route *vehicleRoute)
{
  Vertex *v = vehicleRoute->first_vx;
  Vertex *vPrev = v;
  int randNum = rand() % (vehicleRoute->num_vertices);
  int cont = 0;
  while (cont!=randNum && v != NULL)
  {
    vPrev = v;
    v = v->next;
    cont++;
  }
  //printf("Inserindo %d depois de %d\n", pickup->id, vPrev->id);
  insertAfterVertex (pickup, vPrev, vehicleRoute);
}

void insertRandomPointRouteAfter (Vertex *dropoff, Route *vehicleRoute, Vertex *pickup)
{
  int cont = 0;
  Vertex *v = pickup;
  Vertex *vPrev = v;
  
  while (v != NULL)
  {
    v = v->next;
    cont++;
  }
  
  //printf("->Contei %d\n", cont);
  int randNum = rand() % cont;
  //printf("->Aleatorio %d\n", randNum);
  
  cont = 0;
  vPrev = v = pickup;
  while (v != NULL && cont!=randNum)
  {
    vPrev = v;
    v = v->next;
    cont++;
  }
  insertAfterVertex (dropoff, vPrev, vehicleRoute);
}

void insertRandomPointRouteBefore (Vertex *pickup, Route *vehicleRoute, Vertex *dropoff)
{ 
  int cont = 0;
  Vertex *v = dropoff;
  //Vertex *vNext = v;
  
  while (v->id != 0 && v != NULL)
  {
    v = v->prev;
    cont++;
  }
  
  //printf("->Contei %d\n", cont);
  int randNum = rand() % cont;
  //printf("->Aleatorio %d\n", randNum);
  cont = 0;
  //vNext = 
  v = dropoff;
  while (v != NULL && cont!=randNum)
  {
    v = v->prev;
    cont++;
  }
  
  insertBeforeVertex (pickup, v, vehicleRoute);
}

Vertex *copyVertex (Vertex *vx)
{
  Vertex *new_vx = (Vertex*) malloc (sizeof (Vertex));
  new_vx->id = vx->id;
  new_vx->coord = vx->coord;
  new_vx->durService = vx->durService;
  new_vx->demand = vx->demand;
  new_vx->e_tw = vx->e_tw;
  new_vx->l_tw = vx->l_tw;
  new_vx->arrival = vx->arrival;
  new_vx->begService = vx->begService;
  new_vx->departure = vx->departure;
  new_vx->wait = vx->wait;
  new_vx->load = vx->load;
  new_vx->rideTime = vx->rideTime;
  new_vx->forwardTimeSlack = vx->forwardTimeSlack;
  new_vx->next = NULL;
  new_vx->prev = NULL;

  ///////TEMP///////
  new_vx->twViolations = vx->twViolations;
  //////////////////
  return new_vx;
}

Route *copyRoute (Route *rt)
{
	Route *new_rt = (Route*) malloc (sizeof(Route));
	new_rt->vehicle = rt->vehicle;
	new_rt->num_vertices = 0;
	new_rt->first_vx = NULL;
	new_rt->last_vx = NULL;
	new_rt->next = NULL;
	new_rt->evaluated = rt->evaluated;

	Vertex *vxPointer = rt->first_vx;

	while (vxPointer != NULL)
	{
		Vertex *new_vx = copyVertex (vxPointer);
		insertVertexEndRoute (new_vx, new_rt);
		vxPointer = vxPointer->next;
	}

	return new_rt;
}

RoutesList *copyRoutesList (RoutesList *rt_list)
{

	RoutesList *new_rt_list = (RoutesList*) malloc (sizeof(RoutesList));
	new_rt_list->num_vehicles = rt_list->num_vehicles;
	new_rt_list->num_clients = rt_list->num_clients;
	new_rt_list->garage = copyVertex(rt_list->garage);
	new_rt_list->max_duration = rt_list->max_duration;
	new_rt_list->max_load = rt_list->max_load;
	new_rt_list->max_ride_time = rt_list->max_ride_time;
	new_rt_list->max_route_time = rt_list->max_route_time;
	new_rt_list->excess_route_time = rt_list->excess_route_time;
	new_rt_list->excess_load = rt_list->excess_load;
	new_rt_list->total_cost = rt_list->total_cost;
	new_rt_list->first_rt = NULL;
	new_rt_list->last_rt = NULL;
	new_rt_list->obj_func_eval = 0;
	new_rt_list->cpu_time = 0;

	Route *rtPointer = rt_list->first_rt;

	while (rtPointer != NULL)
	{
		Route *new_rt = copyRoute(rtPointer);
		insertRoute (new_rt, new_rt_list);
		rtPointer = rtPointer->next;
	}

	return new_rt_list;
}










