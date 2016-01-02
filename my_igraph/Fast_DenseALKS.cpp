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
#include "Fast-DensestSubgraph.h"
#include "Fast_DenseALKS.h"
#include "CTimer.h"
#include <algorithm>
#include <iterator>
#include "print_graph.h"


using namespace std;

void print_vector(igraph_vector_t *v, FILE *f) {
	long int i;
	for (i = 0; i < igraph_vector_size(v); i++) {
		fprintf(f, " %.3f", VECTOR(*v)[i]);
	}
	fprintf(f, "\n");
}


// 对节点子集求稠密子图


/*
// 对节点子集求稠密子图，图的期望密度和节点的期望度数都考虑保留边
int fast_densestSubgraph4(const Mysubgraph *subgraph, const igraph_vector_t *vertices,
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
	igraph_vector_init(&expected_edges, 0);
	igraph_vector_init(&expected_degrees, 0);

	// keep存放子图的节点集合，不包含保留边关联的已删除的节点
	igraph_vector_init(&keep, 0);
	igraph_vector_copy(&keep, vertices);

	// 先得到原图的copy，这里使用不删除节点推导得到的便是原图
	igraph_vector_init(&rm_vertice, 0);
	subgraph_removeVetices2(subgraph, &rm_vertice, &subg_cur);

	// 将节点从新映射到子图上
	for (int i = 0; i < igraph_vector_size(&keep); i++)
	{
		int pre_vid = VECTOR(keep)[i];  // 父图的节点编号
		int cur_vid = VECTOR(*(subg_cur.map_next))[pre_vid] - 1;  // 父图编号映射到子图的编号
		if (cur_vid < 0)
		{
			printf("invalid vertices id\n");
			break;
		}
		VECTOR(keep)[i] = cur_vid;
	}



	int cnt = 0;
	while (true)
	{
		// 当前图的节点个数，边数， 子图的节点个数
		long int no_nodes = igraph_vcount(subg_cur.graph);
		long int no_edges = igraph_ecount(subg_cur.graph);
		long int no_vids = igraph_vector_size(&keep);

		if (no_vids < 2)
		{
			break;
		}

		// 初始化
		mysubgraph_empty(&subg_next, subgraph->graph->directed);
		igraph_vector_resize(&expected_edges, no_edges);
		igraph_vector_resize(&expected_degrees, no_vids);
		igraph_vector_resize(&rm_vertice, 0);

		// 求出每条边的期望权值
		igraph_edges_expected(subg_cur.graph, &expected_edges);

		// 求出节点的期望度数
		vertice_expected_degree(subg_cur.graph, &expected_degrees, igraph_vss_vector(&keep), IGRAPH_OUT, IGRAPH_NO_LOOPS, &expected_edges);
		// 求出图的期望密度
		graph_expected_density(subg_cur.graph, &density_cur, igraph_vss_vector(&keep), IGRAPH_OUT, IGRAPH_NO_LOOPS, &expected_edges);

		// 最大稠密子图
		if (density_cur > max_density)
		{
			max_density = density_cur;
			mysubgraph_copy(&max_dentity_subg, &subg_cur);
			igraph_vector_resize(&max_density_keep_v, igraph_vector_size(&keep));
			igraph_vector_copy(&max_density_keep_v, &keep);
			//printf("max_density : %f\n", max_density);
			//printf("max_keep : %d\n", igraph_vector_size(&keep));
		}


		// 删除期望度数比图的期望密度小的节点
		for (long i = 0; i < igraph_vector_size(&keep); i++)
		{
			long vid = (long)VECTOR(keep)[i];
			if (VECTOR(expected_degrees)[i] < density_cur)
			{
				igraph_vector_push_back(&rm_vertice, vid);
			}
		}

		// 如果没有节点的期望度数比图的期望密度小，则删除期望度数最小的节点
		if (igraph_vector_size(&rm_vertice) == 0)
		{
			long int min_idx = igraph_vector_which_min(&expected_degrees);
			long vid = (long)VECTOR(keep)[min_idx];
			igraph_vector_push_back(&rm_vertice, vid);
		}

		// 删除节点产生新子图
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
			long pre_vid = VECTOR(keep)[i];  // 子图映射到父图id
			// 父图id映射到子图id，如果是-1后是负的说明已经被删除
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

		// 将剩余的节点存放于keep用于下次迭代
		igraph_vector_resize(&keep, igraph_vector_size(&keep2));
		igraph_vector_copy(&keep, &keep2);

		mysubgraph_swap(&subg_cur, &subg_next);
	}


	//printf("keep : %d\n", igraph_vector_size(&max_density_keep_v));

	//igraph_vector_print(&max_density_keep_v);

	// 产生最稠密子图的节点集合，存放于res_keepv中用于返回
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
*/


// 这里经常使用集合并差集，所以使用c++的集合运算
// 考虑保留边


// 这里经常使用集合并差集，所以使用c++的集合运算
// 不考虑保留边


int main()
{
	igraph_t g;
	Mysubgraph subg_next;
	igraph_vector_t expected_edges;
	igraph_vector_t expected_degrees;
	//	igraph_vector_t weights;
	//	igraph_vector_t probabilitys;
	Mysubgraph subg0;
	igraph_warning_handler_t* oldwarnhandler;

	FILE *input = NULL;
	FILE *file_desity_graph = NULL;
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

	igraph_real_t g_density = 0;
	graph_expected_density(&g, &g_density, igraph_vss_all(), IGRAPH_OUT, IGRAPH_NO_LOOPS, &expected_edges);
	printf("density:%f\n", g_density);

	init_mysubgraph(&g, &subg0);

	printf("%d %d\n", (int)igraph_vcount(subg0.graph), (int)igraph_ecount(subg0.graph));

	igraph_real_t density = 0;
	//fast_densestSubgraph(&subg0, &subg_next, &density);
	double a1, b1;
	a1 = clock();
	Fast_DenseALKS(&g, 100, &subg_next, &density);
	printf("vsize : %d density : %f\n", igraph_vcount(subg_next.graph), density);
	b1 = clock();
	printf("\n-----\nclock:%lfs\n", (b1 - a1) / CLOCKS_PER_SEC);

	//file_desity_graph = fopen("result\\density_subgraph_edges.txt", "w");
	file_desity_graph = fopen("result\\density_subgraph.gml", "w");
	// input = fopen("to_TAP_core.txt", "r");
	if (file_desity_graph == NULL)
	{
		printf("read file error");
		return 1;
	}
	//print_data_4col(file_desity_graph, subg_next.graph);
	oldwarnhandler = igraph_set_warning_handler(null_warning_handler);
	igraph_write_graph_gml(subg_next.graph, file_desity_graph, 0, "");

	igraph_destroy(&g);

	destroy_mysubgraph(&subg_next);
	destroy_mysubgraph(&subg0);
	igraph_vector_destroy(&expected_edges);
	igraph_vector_destroy(&expected_degrees);
	fclose(input);
	fclose(file_desity_graph);
	return 0;
}
