#ifndef FAST_DENSESTSUBGRAPH_H
#define FAST_DENSESTSUBGRAPH_H

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


int mysubgraph_swap(Mysubgraph *subgraph1, Mysubgraph *subgraph2)
{
	Mysubgraph tmp = { NULL, NULL };

	// Mysubgraph中存放的是指针，这里的交换只是交换的是存放的地址
	tmp = *subgraph1;
	*subgraph1 = *subgraph2;
	*subgraph2 = tmp;

	tmp.graph = NULL;
	tmp.invmap = NULL;
	return 0;
}

int mysubgraph_copy(Mysubgraph *to, Mysubgraph *from)
{
	if (to == NULL || from == NULL)
	{
		return 1;
	}
	// 这里出错，由于传入前to中的graph和invmap未初始化。现以纠正
	igraph_copy(to->graph, from->graph);
	igraph_vector_copy(to->invmap, from->invmap);
	igraph_vector_copy(to->map_next, from->map_next);
	igraph_vector_copy(to->map_pre, from->map_pre);
	return 0;
}

int fast_densestSubgraph(const Mysubgraph *subgraph, Mysubgraph *res, igraph_real_t *subg_density)
{
	igraph_vector_t expected_edges;
	igraph_vector_t expected_degrees;
	igraph_vector_t rm_vertice;
	Mysubgraph subg_cur;
	igraph_real_t density_cur = 0;
	Mysubgraph subg_next;
	igraph_real_t density_next = 0;
	Mysubgraph max_dentity_subg;
	igraph_real_t max_density = -1;

	if (subgraph == NULL || subgraph->graph == NULL || subgraph->invmap == NULL)
	{
		printf("subgraph is null in fast_densestSubgraph\n");
		return 1;
	}

	init_mysubgraph(&subg_cur, subgraph->graph->directed);
	init_mysubgraph(&subg_next, subgraph->graph->directed);
	init_mysubgraph(&max_dentity_subg, subgraph->graph->directed);
	init_mysubgraph(res, subgraph->graph->directed);

	// 先得到原图的copy，这里使用当不删除节点便是原图
	igraph_vector_init(&rm_vertice, 0);
	subgraph_removeVetices(subgraph, &rm_vertice, &subg_cur);

	int cnt = 0;
	while (true)
	{
		long int no_nodes = igraph_vcount(subg_cur.graph);
		long int no_edges = igraph_ecount(subg_cur.graph);

		if (no_nodes < 2)
		{
			break;
		}

		init_mysubgraph(&subg_next, subgraph->graph->directed);
		igraph_vector_init(&expected_edges, no_edges);
		igraph_vector_init(&expected_degrees, no_edges);
		igraph_vector_init(&rm_vertice, 0);

		// expected_edges使用前必须初始化
		// 求出每条边的期望权值
		igraph_edges_expected(subg_cur.graph, &expected_edges);

		// 求出节点的期望度数
		vertice_expected_degree(subg_cur.graph, &expected_degrees, igraph_vss_all(), IGRAPH_OUT, IGRAPH_NO_LOOPS, &expected_edges);
		// 求出图的期望度数
		graph_expected_density(subg_cur.graph, &density_cur, igraph_vss_all(), IGRAPH_OUT, IGRAPH_NO_LOOPS, &expected_edges);

		if (density_cur > max_density)
		{
			max_density = density_cur;
			mysubgraph_copy(&max_dentity_subg, &subg_cur);
		}

		for (long i = 0; i < no_nodes; i++)
		{
			if (VECTOR(expected_degrees)[i] < density_cur)
			{
				igraph_vector_push_back(&rm_vertice, i);
			}
		}

		if (igraph_vector_size(&rm_vertice) == 0)
		{
			long int min_idx = igraph_vector_which_min(&expected_degrees);
			igraph_vector_push_back(&rm_vertice, min_idx);
		}

		subgraph_removeVetices(&subg_cur, &rm_vertice, &subg_next);
//		printf("%d %d\n", (int)igraph_vcount(subg_next.graph), (int)igraph_ecount(subg_next.graph));

		mysubgraph_swap(&subg_cur, &subg_next);
	}

	mysubgraph_copy(res, &max_dentity_subg);
	*subg_density = max_density;
	printf("max_den_v : %d => density : %f \n", igraph_vcount(max_dentity_subg.graph), max_density);

	igraph_vector_destroy(&expected_edges);
	igraph_vector_destroy(&expected_degrees);
	igraph_vector_destroy(&rm_vertice);
	destroy_mysubgraph(&subg_cur);
	destroy_mysubgraph(&subg_next);
	destroy_mysubgraph(&max_dentity_subg);
	return 0;
}

// 对节点子集求稠密子图，图的期望密度和节点的期望度数都考虑保留边
int fast_densestSubgraph2(const Mysubgraph *subgraph, const igraph_vector_t *vertices,
	Mysubgraph *res, igraph_real_t *subg_density, igraph_vector_t *res_keepv)
{
	igraph_vector_t expected_edges;
	igraph_vector_t expected_degrees;
	igraph_vector_t rm_vertice;
	Mysubgraph subg_cur;
	igraph_real_t density_cur = 0;
	Mysubgraph subg_next;
	igraph_real_t density_next = 0;
	Mysubgraph max_dentity_subg;
	igraph_real_t max_density = -1;
	igraph_vector_t keep;
	igraph_vector_t keep2;
	igraph_vector_t max_density_keep_v;

	if (subgraph == NULL || subgraph->graph == NULL || subgraph->invmap == NULL || vertices == NULL)
	{
		printf("subgraph is null in fast_densestSubgraph\n");
		return 1;
	}

	init_mysubgraph(&subg_cur, subgraph->graph->directed);
	init_mysubgraph(&subg_next, subgraph->graph->directed);
	init_mysubgraph(&max_dentity_subg, subgraph->graph->directed);
	init_mysubgraph(res, subgraph->graph->directed);

	igraph_vector_init(&max_density_keep_v, 0);
	igraph_vector_init(&keep, 0);
	igraph_vector_copy(&keep, vertices);

	// 先得到原图的copy，这里使用当不删除节点便是原图
	igraph_vector_init(&rm_vertice, 0);
	subgraph_removeVetices2(subgraph, &rm_vertice, &subg_cur);

	int cnt = 0;
	while (true)
	{
		long int no_nodes = igraph_vcount(subg_cur.graph);
		long int no_edges = igraph_ecount(subg_cur.graph);
		long int no_vids = igraph_vector_size(&keep);

		//printf("n:%d	e:%d\n", no_nodes, no_edges);
		//igraph_vector_print(&keep);

		if (no_vids < 2)
		{
			break;
		}

		init_mysubgraph(&subg_next, subgraph->graph->directed);
		igraph_vector_init(&expected_edges, no_edges);
		//		igraph_vector_init(&expected_degrees, no_edges);
		igraph_vector_init(&expected_degrees, no_vids);
		igraph_vector_init(&rm_vertice, 0);

		// expected_edges使用前必须初始化
		// 求出每条边的期望权值
		igraph_edges_expected(subg_cur.graph, &expected_edges);

		// 求出节点的期望度数
		vertice_expected_degree(subg_cur.graph, &expected_degrees, igraph_vss_vector(&keep), IGRAPH_OUT, IGRAPH_NO_LOOPS, &expected_edges);
		// 求出图的期望密度
		graph_expected_density(subg_cur.graph, &density_cur, igraph_vss_vector(&keep), IGRAPH_OUT, IGRAPH_NO_LOOPS, &expected_edges);

		if (density_cur > max_density)
		{
			max_density = density_cur;
			mysubgraph_copy(&max_dentity_subg, &subg_cur);
			igraph_vector_resize(&max_density_keep_v, igraph_vector_size(&keep));
			igraph_vector_copy(&max_density_keep_v, &keep);
			//printf("max_density : %f\n", max_density);
			//printf("max_keep : %d\n", igraph_vector_size(&keep));
		}


		for (long i = 0; i < igraph_vector_size(&keep); i++)
		{
			long vid = (long)VECTOR(keep)[i];
			if (VECTOR(expected_degrees)[vid] < density_cur)
			{
				igraph_vector_push_back(&rm_vertice, vid);
			}
		}


		if (igraph_vector_size(&rm_vertice) == 0)
		{
			long int min_idx = igraph_vector_which_min(&expected_degrees);
			long vid = (long)VECTOR(keep)[min_idx];
			igraph_vector_push_back(&rm_vertice, vid);
		}

		subgraph_removeVetices2(&subg_cur, &rm_vertice, &subg_next);
		//printf("%d %d\n", (int)igraph_vcount(subg_next.graph), (int)igraph_ecount(subg_next.graph));

		// swap大小要相同
		//igraph_vector_reserve(&keep, igraph_vector_size(&keep2));
		//igraph_vector_copy(&keep, &keep2);
		// 将父图节点编号转化为子图节点编号
		int cnt = 0;
		igraph_vector_init(&keep2, 0);
		for (int i = 0; i < igraph_vector_size(&keep); i++)
		{
			long pre_vid = VECTOR(keep)[i];
			long cur_vid = VECTOR(*(subg_next.map_next))[pre_vid] - 1;
			if (cur_vid < 0)
			{
				//printf("invalid vertices\n");
				cnt++;
			}
			if (cur_vid >= 0)
				igraph_vector_push_back(&keep2, cur_vid);
		}
		//printf("rm:%d\n",cnt);
		//printf("density : %f\n", density_cur);

		igraph_vector_resize(&keep, igraph_vector_size(&keep2));
		igraph_vector_copy(&keep, &keep2);

		mysubgraph_swap(&subg_cur, &subg_next);
	}


	//printf("keep : %d\n", igraph_vector_size(&max_density_keep_v));

	//igraph_vector_print(&max_density_keep_v);

	if (res_keepv != NULL)
	{
		igraph_vector_reserve(res_keepv, igraph_vector_size(&max_density_keep_v));
		igraph_vector_copy(res_keepv, &max_density_keep_v);
	}
	mysubgraph_copy(res, &max_dentity_subg);
	*subg_density = max_density;
	printf("max_den_v : %d => density : %f \n", igraph_vcount(max_dentity_subg.graph), max_density);

	igraph_vector_destroy(&keep);
	igraph_vector_destroy(&keep2);
	igraph_vector_destroy(&max_density_keep_v);
	igraph_vector_destroy(&expected_edges);
	igraph_vector_destroy(&expected_degrees);
	igraph_vector_destroy(&rm_vertice);
	destroy_mysubgraph(&subg_cur);
	destroy_mysubgraph(&subg_next);
	destroy_mysubgraph(&max_dentity_subg);
	return 0;
}



#endif // !FAST_DENSESTSUBGRAPH_H
