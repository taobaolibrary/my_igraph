#define  _CRT_SECURE_NO_WARNINGS

#include <igraph.h>
#include <stdio.h>
#include <iostream>
#include <assert.h>  
#include <set>
#include <vector>
#include "mystr.h"
#include "read_data.h"
#include "expected_degree.h"
#include "graph_partition.h"
#include <algorithm>
#include <iterator>

using namespace std;

// typedef struct Mysubgraph
// {
// 	igraph_t *graph;
// 	igraph_vector_t *invmap;
// }Mysubgraph;

void print_vector(igraph_vector_t *v, FILE *f) {
	long int i;
	for (i = 0; i < igraph_vector_size(v); i++) {
		fprintf(f, " %.3f", VECTOR(*v)[i]);
	}
	fprintf(f, "\n");
}

// 去掉顶点期望度数小于期望密度峰值的节点
// 删除节点的话，图删除关联边。节点集合删除节点
// int graph_partition(const igraph_t *graph, const igraph_vector_t expected_degrees, const igraph_real_t g_density, std::set<long int> *subg_vertice)
// {
// 	std::set<long int> reduce_set;
// 	std::set<long int> result_set;
// 	reduce_set.clear();
// 
// 	if (subg_vertice == NULL || subg_vertice->size() == 0)
// 	{
// 		return 0;
// 	}
// 
// 	for each (long int node_id in (*subg_vertice))
// 	{
// 		if (node_id < 0 || node_id >igraph_vector_size(&expected_degrees))
// 		{
// 			printf("节点编号出错，位置：graph_partition 函数");
// 			return 1;
// 		}
// 	}
// 
// 	for each (long int node_id in (*subg_vertice))
// 	{
// 		if (VECTOR(expected_degrees)[node_id] < g_density);
// 		{
// 			reduce_set.insert(node_id);
// 		}
// 
// 	}
// 
// 	std::set_difference(subg_vertice->begin(), subg_vertice->end(), reduce_set.begin(), reduce_set.end(), result_set.begin());
// 
// 	if (reduce_set.size() == 0)
// 	{
// 		std::set<long int>::iterator min_itr = subg_vertice->begin();
// 		long int min_density_id = *min_itr;
// 		for (min_itr++; min_itr != subg_vertice->end(); min_itr++)
// 		{
// 			if (VECTOR(expected_degrees)[(*min_itr)] > VECTOR(expected_degrees)[min_density_id])
// 			{
// 				VECTOR(expected_degrees)[(*min_itr)] = VECTOR(expected_degrees)[min_density_id];
// 			}
// 		}
// 		subg_vertice->erase(min_itr);
// 	}
// 	
// 	return 0;
// }

// graph是初始化过的图，该函数只是生成一个映射自身的图，包装成Mysubgraph类型
// 记得释放subgraph对象内存
/*
int init_mysubgraph(Mysubgraph *graph, const igraph_bool_t directed);
int init_mysubgraph(igraph_t *graph, Mysubgraph *res)
{
	igraph_vector_t ig_map;
	igraph_vector_init(&ig_map, 0);

	init_mysubgraph(res, graph->directed);

	igraph_induced_subgraph_map(graph, res->graph, igraph_vss_all(), 
		IGRAPH_SUBGRAPH_AUTO, &ig_map, res->invmap);

	igraph_vector_destroy(&ig_map);
	return 0;
}

int init_mysubgraph(Mysubgraph *graph, const igraph_bool_t directed)
{
	if (graph == NULL)
	{
		graph = (Mysubgraph*)malloc(sizeof(Mysubgraph));
	}

	graph->graph = (igraph_t*)malloc(sizeof(igraph_t));
	graph->invmap = (igraph_vector_t*)malloc(sizeof(igraph_vector_t));

	igraph_vector_init(graph->invmap, 0);
	igraph_empty(graph->graph, 0, directed);
	return 0;
}

// 释放Mysubgraph中的内存
int destroy_mysubgraph(Mysubgraph *g)
{
	if (g == NULL)
	{
		return 0;
	}

	if (g->graph != NULL)
	{
		igraph_destroy(g->graph);
		free(g->graph);
		g->graph = NULL;
	}

	if (g->invmap != NULL)
	{
		igraph_vector_destroy(g->invmap);
		free(g->invmap);
		g->invmap = NULL;
	}
	return 0;
}

// 子图删除节点以及关联边，节点映射到g映射的图
int subgraph_removeVetices(const Mysubgraph  *g, const igraph_vector_t *vertices, Mysubgraph *res)
{
	if (g == NULL || g->graph == NULL || res == NULL)
	{
		return 1;
	}
	igraph_vector_t ig_map;
	igraph_vector_t keep;
	igraph_vector_t rm_vertice;
	long int no_node = igraph_vcount(g->graph);

	igraph_vector_init(&ig_map, 0);

	igraph_vector_init(&rm_vertice, igraph_vector_size(vertices));
	igraph_vector_copy(&rm_vertice, vertices);
	igraph_vector_sort(&rm_vertice);

	igraph_vector_init(&keep, 0);

	if (igraph_vector_tail(&rm_vertice) >= no_node || VECTOR(rm_vertice)[0] < 0)
	{
		printf("invalid vertice id to remome");
	}

	int j = 0;
	for (long int i = 0; i < no_node; i++)
	{
		if ((long)VECTOR(rm_vertice)[j] == i)
		{
			j++;
			continue;
		}
		igraph_vector_push_back(&keep, i);
	}

//	igraph_vector_print(&keep);

	init_mysubgraph(res, g->graph->directed);
	igraph_induced_subgraph_map(g->graph, res->graph, igraph_vss_vector(&keep),
		IGRAPH_SUBGRAPH_AUTO, &ig_map, res->invmap);

	// 上面得出的invmap是到g的映射，需要根据g->invmap进行映射转换，映射到最初的图。 
	for (int i = 0; i < igraph_vector_size(res->invmap); i++)
	{
		long int pre = VECTOR(*(res->invmap))[i];
		VECTOR(*(res->invmap))[i] = VECTOR(*(g->invmap))[pre];
	}

	return 0;
}
*/

int main()
{
	igraph_t g;
	igraph_vector_t expected_edges;
	igraph_vector_t expected_degrees;
	igraph_vector_t weights;
	igraph_vector_t probabilitys;
	igraph_warning_handler_t* oldwarnhandler;

	FILE *input;
	/* Same graph, but forcing undirected mode */
	input = fopen("mydata.txt", "r");
	// input = fopen("to_TAP_core.txt", "r");
	if (input == NULL)
	{
		printf("read file error");
		return 1;
	}

	init_mygraph(input, &g, 0);

	// expected_edges使用前必须初始化
	igraph_vector_init(&expected_edges, igraph_ecount(&g));
	igraph_edges_expected(&g, &expected_edges);
	/*print_vector(&expected_edges, stdout);*/

	igraph_vector_init(&expected_degrees, igraph_ecount(&g));
	vertice_expected_degree(&g, &expected_degrees, igraph_vss_all(), IGRAPH_OUT, IGRAPH_NO_LOOPS, &expected_edges);
// 	print_vector(&expected_degrees, stdout);

// 	igraph_vector_init(&weights, igraph_ecount(&g));
// 	igraph_vector_init(&probabilitys, igraph_ecount(&g));
// 	EANV(&g, "weights", &weights);
// 	EANV(&g, "probabilitys", &probabilitys);
// 	vertice_expected_degree(&g, &expected_degrees, igraph_vss_all(), IGRAPH_OUT, IGRAPH_NO_LOOPS, &weights, &probabilitys);
// 	print_vector(&expected_degrees, stdout);
// 
// 	vertice_expected_degree(&g, &expected_degrees, igraph_vss_all(), IGRAPH_OUT, IGRAPH_NO_LOOPS);
// 	print_vector(&expected_degrees, stdout);

	igraph_real_t g_density = 0;
	graph_expected_density(&g, &g_density, igraph_vss_all(), IGRAPH_OUT, IGRAPH_NO_LOOPS, &expected_edges);
	printf("density:%f\n", g_density);

	Mysubgraph subg0;
	Mysubgraph subg_next;
	igraph_vector_t rm_vertice;
	igraph_vector_t n_d;
	igraph_strvector_t node_names;
	char * s;

	igraph_vector_init_int(&rm_vertice, 1, 172);
	
// 	igraph_vector_init(&n_d, 1);
// 	igraph_degree(&g, &n_d, igraph_vss_all(), IGRAPH_ALL, 0);
// 
// 	igraph_strvector_init(&node_names, 0);
// 	VASV(&g, "vname", &node_names);

// 	for (int i = 0; i < igraph_strvector_size(&node_names); i++) {
// 		igraph_strvector_get(&node_names, i, &s);
// 		if (strcmp("HHT1", s) == 0)
// 		{
// 			printf("HHT1 : %d\n", i);
// 		}
// 	}

//	printf("%s\n", VAS(&g, "vname", 2039));


// 	int num = 0;
// 	for (int i = 0; i < igraph_vector_size(&n_d); i++)
// 	{
// 		if (VECTOR(n_d)[i] > 15 && num < 10)
// 		{
// 			igraph_strvector_get(&node_names, i, &s);
// 			printf("id :　%d => name : %s => degree : %d\n", i ,s , (int)VECTOR(n_d)[i]);
// 			num++;
// 		}
// 	}



	oldwarnhandler = igraph_set_warning_handler(null_warning_handler);
	/*igraph_write_graph_gml(&g, stdout, 0, "");*/

	init_mysubgraph(&g, &subg0);

	printf("%d %d\n", (int)igraph_vcount(subg0.graph), (int)igraph_ecount(subg0.graph));

	/*print_vector(subg0.invmap, stdout);*/

//	igraph_write_graph_gml(subg0.graph, stdout, 0, "");
// 	igraph_vector_print(subg0.invmap);

	igraph_vector_print(&rm_vertice);
	subgraph_removeVetices(&subg0, &rm_vertice, &subg_next);

	printf("%d %d\n", (int)igraph_vcount(subg_next.graph), (int)igraph_ecount(subg_next.graph));

	/*printf("%d\n", igraph_vcount(subg_next.graph));*/

	//igraph_write_graph_gml(subg_next.graph, stdout, 0, "");
	//igraph_vector_print(subg_next.invmap);

	//subgraph_removeVetices(&subg0, &rm_vertice, &subg_next)

	// oldwarnhandler = igraph_set_warning_handler(null_warning_handler);
	// igraph_write_graph_gml(&g, stdout, 0, "");

	fclose(input);
	return 0;
}
