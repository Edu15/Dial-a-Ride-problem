#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "darp.h"

//ω = [8,0,1,3,1] e β = [1500,1500,1500,1500,1500].
//Definicoes de constantes para calculo de custo.
#define W0 1500 
#define W1 1500 
#define W2 1500 
#define W3 1500 
#define W4 8 
#define W5 0 
#define W6 1
#define W7 3 
#define W8 1

//Calcula o custo de uma solucao com base nos requisitos previamente definidos e
// seus pesos relativos.
double computeSolutionCost (RoutesList *solution)
{
  addObjFuncEvaluation();
  double cost = 0;
  
  //REQUISITOS ESSENCIAIS
  double excess_route_time = computeExcessRouteTime (solution);
  double excess_ride_time = computeExcessRideTime (solution);
  //Computar excessos nos tempos de espera?
  int excess_load = computeExcessLoad (solution);
  double time_windows_violations = computeTimeWindowsViolations(solution);
  
  cost = (W0)*excess_route_time + (W1)*excess_ride_time + (W2)*excess_load + (W3)*time_windows_violations;
  
  //REQUISITOS NAO ESSENCIAIS
  double total_distance_traveled = computeDistanceTraveledVehicles (solution);
  uint num_vehicles_used = computeNumberVehiclesUsed (solution);
  double total_duration_routes = computeTotalDurationRoutes (solution);
  double total_ride_time_clients = computeTotalTimeClients (solution);
  double total_wait_time = computeTotalWaitTime (solution);
  
  cost += (W4)*total_distance_traveled + (W5)*num_vehicles_used + \
  (W6)*total_duration_routes + (W7)*total_ride_time_clients + (W8)*total_wait_time;
  

  solution->total_cost = cost;
  return cost;
}

//Calcula o custo de uma solucao com base nos requisitos previamente definidos e
// seus pesos relativos e tambem imprime os valores calculados de forma detalhada.
void computeAndPrintSolutionCost (RoutesList *solution)
{
  double cost = 0;

  //REQUISITOS ESSENCIAIS
  double excess_route_time = computeExcessRouteTime (solution);
  double excess_ride_time = computeExcessRideTime (solution);
  //Computar excessos nos tempos de espera?
  int excess_load = computeExcessLoad (solution);
  double time_windows_violations = computeTimeWindowsViolations(solution);

  cost =  (W0)*excess_route_time + (W1)*excess_ride_time + (W2)*excess_load + (W3)*time_windows_violations;

  //REQUISITOS NAO ESSENCIAIS
  double total_distance_traveled = computeDistanceTraveledVehicles (solution);
  uint num_vehicles_used = computeNumberVehiclesUsed (solution);
  double total_duration_routes = computeTotalDurationRoutes (solution);
  double total_ride_time_clients = computeTotalTimeClients (solution);
  double total_wait_time = computeTotalWaitTime (solution);

  cost += (W4)*total_distance_traveled + (W5)*num_vehicles_used + \
  (W6)*total_duration_routes + (W7)*total_ride_time_clients + (W8)*total_wait_time;

/*
  printf("\nREQUISITOS ESSENCIAIS\n");
  printf(" Excesso de tempo das rotas: \t%.2lf\n", excess_route_time);
  printf(" Excesso de tempo das viagens: \t%.2lf\n", excess_ride_time);
  printf(" Excesso de capacidade: \t%d\n", excess_load);
  printf(" Violacoes janelas de tempo: \t%.2lf\n", time_windows_violations);
  printf("REQUISITOS NAO ESSENCIAIS\n");
  printf(" Distancia total de viagem: \t%.2lf\n", total_distance_traveled);
  printf(" Numero veiculos usados: \t%d\n", num_vehicles_used);
  printf(" Tempo total das rotas: \t%.2lf\n",total_duration_routes);
  printf(" Tempo total de viagens: \t%.2lf\n", total_ride_time_clients);
  printf(" Tempo total de espera: \t%.2lf\n", total_wait_time);
  printf("        => CUSTO TOTAL: \t%.2lf\n", cost);
  */
  
  printf("%.2lf", cost);
  printf("\t%.2lf", total_distance_traveled);
  printf("\t%.2lf",total_duration_routes);
  printf("\t%.2lf", total_wait_time);
  printf("\t%.2lf", total_ride_time_clients);
  printf("\t%.2f", solution->cpu_time);
  printf("\t%d\n", solution->obj_func_eval);
  
  solution->total_cost = cost;
}


////////////////////

double computeExcessRouteTime (RoutesList *rt_list)
{
  double cost = 0;
  Route *rt = rt_list->first_rt->next;
  
  while (rt != NULL)
  {
    cost+= MAX (0, (rt->last_vx->begService - rt->first_vx->departure) - rt_list->max_route_time);
    rt = rt->next;
  }
  return cost;
}

double computeExcessRideTime (RoutesList *rt_list)
{
  double cost = 0;
  Route *rt = rt_list->first_rt->next;
  Vertex *dropoffVertex, *v = rt->first_vx;
  while (rt != NULL)
  {
    v = rt->first_vx;
    while (v != NULL)
    {
		if (v->demand == (1))
		{
			dropoffVertex = findDropoffVertex(v);
			if (dropoffVertex == NULL)
				printf("Erro DropoffVertex == NULL\nVertex= %d\n", v->id);
			v->rideTime = dropoffVertex->begService - v->departure;
			//printf("R: %.2lf - %.2lf\n", dropoffVertex->begService, v->departure);
		}
		cost+= MAX (0, v->rideTime - rt_list->max_ride_time);
		//printf("> MAX (0, %.2lf - %.2lf) = %.2lf\n", v->rideTime, rt_list->max_ride_time,  MAX (0, v->rideTime - rt_list->max_ride_time));
		v = v->next;
    }
    rt = rt->next;
  }
  return cost;  
}


int computeExcessLoad (RoutesList *rt_list)
{
  int excessLoad = 0;
  Route *rt = rt_list->first_rt->next;
  Vertex *vx = rt->first_vx;
  while (rt != NULL)
  {
    vx = rt->first_vx;
    while (vx != NULL)
    {
      if (vx->load > rt_list->max_load)
	excessLoad += vx->load - rt_list->max_load;
      
      vx = vx->next;
    }
    rt = rt->next;
  }
  return excessLoad;  
}

double computeTimeWindowsViolations (RoutesList *rt_list)
{
  double tw_violations = 0;
  Route *rt = rt_list->first_rt->next;
  Vertex *vx = rt->first_vx;
  while (rt != NULL)
  {
    vx = rt->first_vx;
    while (vx != NULL)
    {
    	/////////TEMP///////////////
    	vx->twViolations = MAX (0, vx->e_tw - vx->begService) + MAX (0, vx->begService - vx->l_tw);
    	////////////////////////
      //tw_violations += MAX (0, vx->e_tw - vx->begService);
      //tw_violations += MAX (0, vx->begService - vx->l_tw);
      tw_violations += vx->twViolations;

      vx = vx->next;
    }
    rt = rt->next;
  }
  return tw_violations;
}

double computeDistanceTraveledVehicles (RoutesList *rt_list)
{
  double distance = 0;
  Route *rt = rt_list->first_rt->next;
  Vertex *vPrev = rt->first_vx;
  Vertex *v = vPrev->next;
  while (rt != NULL)
  {
    v = rt->first_vx;
    while (v != NULL)
    {
      distance += sqrt( pow(v->coord.x - vPrev->coord.x ,2) + pow(v->coord.y - vPrev->coord.y ,2) );
      vPrev = v;
      v = v->next;
    }
    rt = rt->next;
  }
  return distance;   
}

double computeNumberVehiclesUsed (RoutesList *rt_list)
{
  double num_vehicles = 0;
  Route *rt = rt_list->first_rt->next;
  while (rt != NULL)
  {
	  if (rt->num_vertices > 2)
		  num_vehicles++;
    rt = rt->next;
  }
  return num_vehicles;
}

double  computeTotalDurationRoutes (RoutesList *rt_list)
{
  double cost = 0;
  Route *rt = rt_list->first_rt->next;
  
  while (rt != NULL)
  {
    cost+= rt->last_vx->begService - rt->first_vx->departure;
    rt = rt->next;
  }
  return cost;
}


double  computeTotalTimeClients (RoutesList *rt_list)
{
  double time = 0;
  Route *rt = rt_list->first_rt->next;
  Vertex *v = rt->first_vx->next;

  while (rt != NULL)
  {
    v = rt->first_vx->next;
    while (v != NULL)
    {
		time += v->rideTime;
		v = v->next;
    }
    rt = rt->next;
  }
  return time;
}

double  computeTotalWaitTime (RoutesList *rt_list)
{
  double wait_time = 0;
  Route *rt = rt_list->first_rt->next;
  Vertex *vx = rt->first_vx->next;  
  while (rt != NULL)
  {
    vx = rt->first_vx->next;
    while (vx != NULL)
    {
      wait_time += vx->wait;
      vx = vx->next;
    }
    rt = rt->next;
  }
  return wait_time;
}

