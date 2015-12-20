#ifndef GRAPH_PARTITION_H
#define GRAPH_PARTITION_H

#include <set>
#include <vector>
#include "mystr.h"
#include "read_data.h"
#include "expected_degree.h"

typedef struct Mysubgraph
{
	igraph_t *graph;
	igraph_vector_t *invmap;
}Mysubgraph;


// graph是初始化过的图，该函数只是生成一个映射自身的图，包装成Mysubgraph类型
// 记得释放subgraph对象内存
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
		//graph = (Mysubgraph*)malloc(sizeof(Mysubgraph));
		printf("graph cann't null in init_mysubgraph\n");
		return 1;
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

	if (igraph_vector_tail(&rm_vertice) >= no_node || igraph_vector_size(&rm_vertice)>0&&VECTOR(rm_vertice)[0] < 0)
	{
		printf("invalid vertice id to remove\n");
		return 2;
	}

	int j = 0;
	for (long int i = 0; i < no_node; i++)
	{
		if (j < igraph_vector_size(&rm_vertice) && (long)VECTOR(rm_vertice)[j] == i)
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
		long int pre = (long int)VECTOR(*(res->invmap))[i];
		VECTOR(*(res->invmap))[i] = VECTOR(*(g->invmap))[pre];
	}

	igraph_vector_destroy(&ig_map);
	igraph_vector_destroy(&keep);
	igraph_vector_destroy(&rm_vertice);

	return 0;
}
#endif // !1
