#ifndef GRAPH_PARTITION_H
#define GRAPH_PARTITION_H

#include <set>
#include <vector>
#include <algorithm>
#include "mystr.h"
#include "read_data.h"
#include "expected_degree.h"

typedef struct Mysubgraph
{
	igraph_t *graph;
	igraph_vector_t *invmap;
	// 父图映射子图
	igraph_vector_t *map_next;
	// 子图映射父图
	igraph_vector_t *map_pre;
}Mysubgraph;


// graph是初始化过的图，该函数只是生成一个映射自身的图，包装成Mysubgraph类型
// 记得释放subgraph对象内存
int init_mysubgraph(Mysubgraph *graph, const igraph_bool_t directed);
int init_mysubgraph(const igraph_t *graph, Mysubgraph *res)
{
	igraph_vector_t ig_map;
	igraph_vector_init(&ig_map, 0);

	init_mysubgraph(res, graph->directed);

	igraph_induced_subgraph_map(graph, res->graph, igraph_vss_all(),
		IGRAPH_SUBGRAPH_AUTO, res->map_next, res->map_pre);

	igraph_vector_reserve(res->invmap, igraph_vector_size(res->map_pre));
	igraph_vector_copy(res->invmap, res->map_pre);
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
	graph->map_pre = (igraph_vector_t*)malloc(sizeof(igraph_vector_t));
	graph->map_next = (igraph_vector_t*)malloc(sizeof(igraph_vector_t));

	igraph_vector_init(graph->invmap, 0);
	igraph_vector_init(graph->map_pre, 0);
	igraph_vector_init(graph->map_next, 0);
	igraph_empty(graph->graph, 0, directed);
	return 0;
}

int mysubgraph_empty(Mysubgraph *graph, const igraph_bool_t directed)
{
	if (graph == NULL)
	{
		return 1;
	}
	if (graph->graph != NULL)
	{
		igraph_empty(graph->graph, 0, directed);
	}
	if (graph->invmap != NULL)
	{
		igraph_vector_resize(graph->invmap, 0);
	}
	if (graph->map_next != NULL)
	{
		igraph_vector_resize(graph->map_next, 0);

	}
	if (graph->map_pre != NULL)
	{
		igraph_vector_resize(graph->map_pre, 0);

	}
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

int subgraph_removeVetices2(const Mysubgraph  *g, const igraph_vector_t *vertices, Mysubgraph *res)
{
	if (g == NULL || g->graph == NULL || res == NULL)
	{
		return 1;
	}
	//igraph_vector_t ig_map;
	igraph_vector_t keep;
	igraph_vector_t rm_vertice;
	long int no_node = igraph_vcount(g->graph);

	//igraph_vector_init(&ig_map, 0);

	
	igraph_vector_init(&rm_vertice, igraph_vector_size(vertices));
	if (igraph_vector_size(vertices) != 0)
	{
		igraph_vector_copy(&rm_vertice, vertices);
		igraph_vector_sort(&rm_vertice);
	}
	

	igraph_vector_init(&keep, 0);

	if (igraph_vector_tail(&rm_vertice) >= no_node || igraph_vector_size(&rm_vertice) > 0 && VECTOR(rm_vertice)[0] < 0)
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

	//igraph_vector_print(&keep);
	//igraph_vector_print(&rm_vertice);
	//igraph_vector_destroy(&rm_vertice);

	init_mysubgraph(res, g->graph->directed);
	igraph_induced_subgraph_map(g->graph, res->graph, igraph_vss_vector(&keep),
		IGRAPH_SUBGRAPH_AUTO, res->map_next, res->map_pre);

	igraph_vector_resize(res->invmap, igraph_vector_size(res->map_pre));
	//printf("%d %d %d\n", igraph_vector_size(res->map_pre), igraph_vector_size(res->map_next), igraph_vector_size(res->invmap));

	// 上面得出的invmap是到g的映射，需要根据g->invmap进行映射转换，映射到最初的图。 
	for (int i = 0; i < igraph_vector_size(res->map_pre); i++)
	{
		long int pre = (long int)VECTOR(*(res->map_pre))[i];
		VECTOR(*(res->invmap))[i] = VECTOR(*(g->invmap))[pre];
	}

	//igraph_vector_destroy(&ig_map);
	igraph_vector_destroy(&keep);
	igraph_vector_destroy(&rm_vertice);
	return 0;
}


int vecint_union(std::vector<int> v1, std::vector<int> v2, std::vector<int> *res)
{
	if (res == NULL)
	{
		return 1;
	}
	res->resize(v1.size() + v2.size());
	std::vector<int>::iterator it;
	std::sort(v1.begin(), v1.end());
	std::sort(v2.begin(), v2.end());
	it = std::set_union(v1.begin(), v1.end(),
		v2.begin(), v2.end(), res->begin());
	res->resize(it - res->begin());
	return 0;
}

int vertices_remain(std::vector<int> v1, int no_node, std::vector<int> *res)
{
	if (res == NULL)
	{
		return 1;
	}
	res->clear();
	sort(v1.begin(), v1.end());
	int j = 0;
	for (int i = 0; i < no_node; i++)
	{
		if (j < v1.size() && i == v1[i])
		{
			j++;
			continue;
		}
		res->push_back(i);
	}

	return 0;
}

int vertices_rm(int no_nodes, const igraph_vector_t keep, igraph_vector_t *res)
{
	igraph_vector_bool_t real_keep_flag;
	if (res == NULL)
	{
		return 1;
	}
	igraph_vector_init(res, 0);
	igraph_vector_bool_init(&real_keep_flag, no_nodes);
	igraph_vector_bool_null(&real_keep_flag);
	igraph_vector_init(res, 0);

	// 确定真实要删除的节点
	for (int i = 0; i < igraph_vector_size(&keep); i++)
	{
		int vid = VECTOR(keep)[i];
		VECTOR(real_keep_flag)[vid] = 1;
	}

	for (int i = 0; i < no_nodes; i++)
	{
		if (VECTOR(real_keep_flag)[i] == 0)
		{
			igraph_vector_push_back(res, i);
		}
	}

	igraph_vector_bool_destroy(&real_keep_flag);
	return 0;
}


#endif // !1

