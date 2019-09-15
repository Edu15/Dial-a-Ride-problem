#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#define EXIT(msg) {fprintf(stdout,"\n%s \n...\n",msg); exit(1);}
#define INF 9999999999

//Define o que e considerado uma janela de tempo aberta
#define E_OPEN_T_WINDOW 0
#define L_OPEN_T_WINDOW 1440

//#define MAX(a,b) ((a) > (b) ? a : b )
#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))
#define FREE(p) {if (p != NULL) free(p); p == NULL; }

#ifndef _DARP_H_
#define _DARP_H_

typedef unsigned int uint;
typedef enum {FALSE=0, TRUE} boolean;


typedef struct coordinates {
  double x;
  double y;
} Coordinates;

typedef struct vertex {
  uint id;
  Coordinates coord; //Coordenadas da localizacao do cliente
  double durService; //Duracao do servicoroute->last_vx
  int demand; //Demanda -> se positivo: pickup, se negativo: delivery
  double e_tw; //Earliest time time for start of service
  double l_tw; //Latest time for start of service
  
  double arrival; //A: horario de chegada no vertice
  double begService; //B: horario do incicio do servico
  double departure; //D: horario de partida do vertice
  double wait; //W: espera entre chegada do carro e inicio do servico
  int load; //Q: carga do veiculo apos termino do sevico
  double rideTime; //R: tempo de viagem do cliente (computado no destino)
  double forwardTimeSlack; //F: maior tempo possivel de atraso no atendimento do vertice
  /////////TEMP/////////
  double twViolations;
  /////////////////////
  //double dist;
  struct vertex *next;
  struct vertex *prev;
} Vertex;


typedef struct route {
  uint vehicle;
  uint num_vertices;

  boolean evaluated;
  //uint Q; //Capacidade do veiculo
  //duracao da rota
  Vertex *first_vx;
  Vertex *last_vx;
  struct route *next;
} Route;
  

//Lista de todas as rotas. Armazena todas as informacoes do problema.
typedef struct routesList{ 
  uint num_vehicles;
  uint num_clients;
  //uint num_depots;
  Vertex *garage;
  double max_duration; //T: duracao maxima da rota
  uint max_load;
  double max_ride_time; //R: max tempo de viagem cliente
  double max_route_time;
  //Requisitos nao essenciais
    //sum dij: distancia total percorrida pelos veiculos
    //numero de veiculos usados para atender a todos os clientes
    //tempo total de duracao das rotas
    //tempo total de viagem dos clientes
    //tempo total de espera dos veiculos
  double total_cost; //Usado para armazenar o valor do custo total da solução, depois de calculada
  int obj_func_eval; //Numero de avaliacoes da funcao objetivo.
  float cpu_time;
  
//Requisitos nao essenciais
  double excess_route_time; //tempo que excede o tempo maximo de duracao das rotas
  double excess_ride_time;//tempo total que excede os tempos máximos de viagem permitidos para os clientes
  uint excess_load;//o tempo total que excede os tempos máximos de espera permitidos para cada local nas rotas, o excesso na capacidade dos veículos 
  //total dos tempos que violam as janelas de tempo.
  Route *first_rt;
  Route *last_rt;
} RoutesList;


/******************* FUNCOES ******************/

//Essenciais
RoutesList *readInputFile(char *file_name);
RoutesList *createRandomInicialSolution (RoutesList *rt_list);
RoutesList *createInicialSolution (RoutesList *rt_list);
double computeInsertionCost(Vertex *p, Vertex *d, Vertex *lastElem);
double timeBetweenVertices(Vertex *v1, Vertex* v2);
void computeTimesVertices (Vertex *v, Route *route);
void distributionHeuristic (RoutesList *rt_list);
void minimizeRidingTimes(Route *route);
void minimizeWaitingTimes(Route *route);

//Print
void printVertices (Vertex *c);
void printRoute (Route *rt);
void printRoutesList (RoutesList *rt_list);
Vertex *insertVertexRandom (Route *rt);

//Free
void freeVertices (Vertex *c);
void freeRoutes (Route *r);
RoutesList *freeRoutesList (RoutesList *rt_list);

//Create
RoutesList *createInicialRoutesList(uint num_vehicles, uint num_clients, double max_duration, uint max_load, double max_ride_time, double max_route_time);
Route *createEmptRoute (uint vehicle);
Vertex *createEmptVertex();
RoutesList *createRandomInicialSolution (RoutesList *rt_list);

//Insert
void insertRoute (Route *rt, RoutesList * rt_list);
void insertVertexEndRoute (Vertex *c, Route *rt);
void insertBeforeVertex (Vertex *v1, Vertex *v2, Route *rt);
void insertRandomPointRoute (Vertex *pickup, Route *vehicleRoute);
void insertRandomPointRouteAfter (Vertex *dropoff, Route *vehicleRoute, Vertex *pickup);
void insertRandomPointRouteBefore (Vertex *pickup, Route *vehicleRoute, Vertex *dropoff);

//Remove
Vertex *removeVertexFromRoute (Vertex *v, Route *route);
Vertex *removeVertexBeginningRoute (Route *rt);

//Find, get: Retornam ponteiro
Vertex *findVertex (Route *route, uint numVertex);
Vertex *findDropoffVertex (Vertex *pickup);
Vertex *findPickupVertex (Vertex *dropoff);
Route *getRandomRoute(RoutesList *rt_list);
Vertex *getRandomVertex (Route *rt);
Route *getRoute (RoutesList *rl, int idRota);

//Take: Acha, remove e retorna o ponteiro
Vertex *takeVertex (Route *route, uint numVertex);
Vertex *takeDropoffVertex (Route *route, Vertex *pickup);
Vertex *takePickupVertex (Route *route, Vertex *dropoff);
Vertex *takeRandomPickupVertex (Route *listRequests, int num_pickups);
Vertex *takeRandomVertex (Route *rt);

//Geracao de vizinhos
void changeVertices (Vertex *a, Vertex *b);
void reallocate (RoutesList *rt_list);
void reorderRoute (RoutesList *rt_list);
void changePoints (RoutesList *rt_list);
void perturbate (RoutesList *rt_list);
void randomMove (RoutesList *neighbor);
void twoOpt (RoutesList *rt_list);

//Funcoes custo
double computeSolutionCost (RoutesList *solution);
double computeExcessRouteTime (RoutesList *rt_list);
double computeExcessRideTime (RoutesList *rt_list);
int computeExcessLoad (RoutesList *rt_list);
double computeTimeWindowsViolations (RoutesList *rt_list);
double computeDistanceTraveledVehicles (RoutesList *rt_list);
double computeNumberVehiclesUsed (RoutesList *rt_list);
double  computeTotalDurationRoutes (RoutesList *rt_list);
double  computeTotalTimeClients (RoutesList *rt_list);
double  computeTotalWaitTime (RoutesList *rt_list);

//Copy
Vertex *copyGarage (RoutesList *rt_list);
Vertex *copyVertex (Vertex *vx);
Route *copyRoute (Route *rt);
RoutesList *copyRoutesList (RoutesList *rt_list);

//Local Search
RoutesList *localSearch1(RoutesList *starting_solution, int num_iterations);
RoutesList *localSearch2(RoutesList *starting_solution, int num_iterations);
//RoutesList *iteratedLocalSearch(RoutesList *starting_solution, int iterations, int subIterations);
RoutesList *iteratedLocalSearch(RoutesList *starting_solution, int maxIterations, RoutesList *(*localSearch)(RoutesList*, int),int subIterations);
RoutesList *getBestNeighborByReallocating(RoutesList *solution);
RoutesList *getBestNeighborByReordering(RoutesList *solution);
RoutesList *getBestNeighborByChangingPoints(RoutesList *solution);
void perturbate (RoutesList *rt_list);


//simulatedAnnealing
RoutesList *simulatedAnnealing (RoutesList *starting_solution, int iterations, double temperature, double decay);

//postProcessing
//RoutesList *postProcessing(RoutesList *solution);
void openShell (RoutesList *solution);

#endif
