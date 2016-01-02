#ifndef FAST_DENSEALKS_H
#define FAST_DENSEALKS_H

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
#include "print_graph.h"
#include "Fast-DensestSubgraph.h"
#include <algorithm>
#include <iterator>
#include <string>
#include <sstream>

int max_dense_kv_subg(const Mysubgraph *origal_graph, const std::vector< std::vector<int> > *subg_vvec, const int in_gsize
	,Mysubgraph *res, std::vector< std::vector<int> > *out_subg_vvec_kv = NULL)
{
	if (origal_graph == NULL || subg_vvec == NULL || res == NULL)
	{
		return 1;
	}

	long int no_node = igraph_vcount(origal_graph->graph);
	long int no_edge = igraph_ecount(origal_graph->graph);
	igraph_vector_t remain_vertices;
	igraph_vector_bool_t vertices_flag;
	std::vector< std::vector<int> > subg_vvec_kv;
	igraph_vector_t rm_vertices;
	igraph_vector_t expected_edges;
	Mysubgraph subg;
	Mysubgraph subg_max_density;
	igraph_real_t max_density = -1;
	igraph_real_t directed = origal_graph->graph->directed;

	init_mysubgraph(&subg, directed);
	init_mysubgraph(&subg_max_density, directed);
	init_mysubgraph(res, directed);

	igraph_vector_init(&remain_vertices, 0);
	igraph_vector_bool_init(&vertices_flag, no_node);

	// 填充到k个节点
	for (int subg_th = 0; subg_th < subg_vvec->size(); subg_th++)
	{
		std::vector<int> subg_kv((*subg_vvec)[subg_th].begin(), (*subg_vvec)[subg_th].end());
		int vsize = (*subg_vvec)[subg_th].size();
		igraph_vector_bool_null(&vertices_flag);
		for each (int vid in subg_kv)
		{
			if (vid >= no_node)
			{
				printf("invalid id\n");
				break;
			}
			VECTOR(vertices_flag)[vid] = 1;
		}

		igraph_vector_resize(&remain_vertices, 0);
		for (int i = 0; i < no_node; i++)
		{
			if (VECTOR(vertices_flag)[i] == 0)
			{
				igraph_vector_push_back(&remain_vertices, i);
			}
		}
		igraph_vector_shuffle(&remain_vertices);

		// printf("in_gsize : %d	vsize : %d\n", in_gsize, vsize);
		for (int i = 0; i < in_gsize - vsize; i++)
		{
			int vid = (int)VECTOR(remain_vertices)[i];
			subg_kv.push_back(vid);
		}
		subg_vvec_kv.push_back(subg_kv);
		subg_kv.clear();
	}


	for each (std::vector<int> vvec in subg_vvec_kv)
	{
		printf("vsize :%d\n", vvec.size());
	}

	printf("-----------------------\n");
	// 获取期望密度最大的k稠密子图
	igraph_real_t density;
	igraph_vector_init(&rm_vertices, 0);
	igraph_vector_init(&expected_edges, 0);
	for (int subg_th = 0; subg_th < subg_vvec_kv.size(); subg_th++)
	{
		igraph_vector_resize(&rm_vertices, 0);
		igraph_vector_resize(&expected_edges, 0);
		igraph_vector_resize(&remain_vertices, 0);
		for each (int vid in subg_vvec_kv[subg_th])
		{
			igraph_vector_push_back(&remain_vertices, vid);
		}
		vertices_rm(no_node, remain_vertices, &rm_vertices);
		subgraph_removeVetices2(origal_graph, &rm_vertices, &subg);
		

		igraph_edges_expected(subg.graph, &expected_edges);
		graph_expected_density(subg.graph, &density, igraph_vss_all(), IGRAPH_OUT, IGRAPH_NO_LOOPS, &expected_edges);

		printf("vsize : %d density : %f\n", igraph_vcount(subg.graph), density);
		if (density > max_density)
		{
			max_density = density;
			mysubgraph_copy(&subg_max_density, &subg);
		}

		mysubgraph_empty(&subg, directed);
		// 		igraph_vector_resize(subg.invmap, 0);
		// 		igraph_vector_resize(subg.map_pre, 0);
		// 		igraph_vector_resize(subg.map_next, 0);
		// 		igraph_empty(subg.graph, 0, directed);
	}

	printf("-----------------------\n");
	// 原稠密子图密度
	igraph_vector_init(&rm_vertices, 0);
	igraph_vector_init(&expected_edges, 0);
	for (int subg_th = 0; subg_th < (*subg_vvec).size(); subg_th++)
	{
		igraph_vector_resize(&rm_vertices, 0);
		igraph_vector_resize(&expected_edges, 0);
		igraph_vector_resize(&remain_vertices, 0);
		for each (int vid in (*subg_vvec)[subg_th])
		{
			igraph_vector_push_back(&remain_vertices, vid);
		}

		printf("reamin_vertices : %d \n", igraph_vector_size(&remain_vertices));

		vertices_rm(no_node, remain_vertices, &rm_vertices);
		printf("rm_vertices : %d \n", igraph_vector_size(&rm_vertices));
		subgraph_removeVetices2(origal_graph, &rm_vertices, &subg);

		igraph_edges_expected(subg.graph, &expected_edges);
		graph_expected_density(subg.graph, &density, igraph_vss_all(), IGRAPH_OUT, IGRAPH_NO_LOOPS, &expected_edges);

		printf("vsize : %d density : %f\n", igraph_vcount(subg.graph), density);
		mysubgraph_empty(&subg, directed);
	}

	mysubgraph_copy(res, &subg_max_density);

	// 返回节点列表
	if (out_subg_vvec_kv != NULL)
	{
		out_subg_vvec_kv->clear();
		for each (std::vector<int> vvec_kv in subg_vvec_kv)
		{
			std::vector<int> tmp(vvec_kv.begin(), vvec_kv.end());
			out_subg_vvec_kv->push_back(vvec_kv);
		}
	}

	// 释放内存
	for (int subg_th = 0; subg_th < subg_vvec_kv.size(); subg_th++)
	{
		std::vector<int> vecint_empty(0);
		subg_vvec_kv[subg_th].swap(vecint_empty);
	}
	subg_vvec_kv.clear();

	igraph_vector_destroy(&rm_vertices);
	igraph_vector_destroy(&expected_edges);
	igraph_vector_destroy(&remain_vertices);
	igraph_vector_bool_destroy(&vertices_flag);
	destroy_mysubgraph(&subg_max_density);
	destroy_mysubgraph(&subg);
	return 0;
}

// subg_density暂时还没返回值
int Fast_DenseALKS(const igraph_t *input_graph, const int in_gsize, Mysubgraph *res, igraph_real_t *subg_density)
{
	// 存放稠密子图节点
	std::vector< std::vector<int> > subg_vvec(0);
	// 存放k稠密子图节点
	std::vector< std::vector<int> > subg_vec_kv(0);

	std::vector<int> cur_m(0);
	std::vector<int> empty_vec(0);

	// 原图
	Mysubgraph origal_graph;
	Mysubgraph pre_subg;
	Mysubgraph cur_subg;
	Mysubgraph cur_density_subg;
	igraph_vector_t origal_keepv;
	igraph_vector_t cur_keepv;
	igraph_vector_t cur_density_vertices;
	igraph_vector_t rm_vertices;

	if (input_graph == NULL)
	{
		printf("input graph is null in Fast_DenseALKS\n");
		return 1;
	}


	init_mysubgraph(input_graph, &origal_graph);
	igraph_bool_t directed = origal_graph.graph->directed;

	init_mysubgraph(&pre_subg, directed);
	init_mysubgraph(&cur_subg, directed);
	init_mysubgraph(&cur_density_subg, directed);
	init_mysubgraph(res, directed);

	mysubgraph_copy(&pre_subg, &origal_graph);


	int times = 1;
	subg_vvec.push_back(std::vector<int>(0));

	while (subg_vvec[times - 1].size() < in_gsize)
	{
		std::vector<int> pre_m = subg_vvec[times - 1];

		printf("-----------------times : %d vsize : %d \n", times, pre_m.size());

		igraph_vector_init(&rm_vertices, 0);
		for each (int vid in pre_m)
		{
			igraph_vector_push_back(&rm_vertices, vid);
		}

		//		printf("times : %d rmvsize : %d \n", times, igraph_vector_size(&rm_vertices));
		subgraph_removeVetices2(&origal_graph, &rm_vertices, &cur_subg);
		printf("times : %d curkeepvsize : %d \n", times, igraph_vcount(cur_subg.graph));
		//		printf("times : %d prekeepvsize : %d \n", times, igraph_vcount(pre_subg.graph));
		// 获取子图映射父图的节点编号
		igraph_vector_init(&origal_keepv, 0);
		for (int i = 0; i < igraph_vector_size(cur_subg.invmap); i++)
		{
			igraph_vector_push_back(&origal_keepv, VECTOR(*(cur_subg.invmap))[i]);
			//printf("%d  ", (int)VECTOR(*(cur_subg.invmap))[i]);
		}
		//printf("\n");
		//printf("cur_size : %d\n", igraph_vector_size(cur_subg.invmap));

		// 将原图的节点编号映射到子图的节点编号
		igraph_vector_init(&cur_keepv, 0);
		for (int i = 0; i < igraph_vector_size(&origal_keepv); i++)
		{
			long origal_vid = VECTOR(origal_keepv)[i];
			long pre_subg_vid = VECTOR(*(pre_subg.map_next))[origal_vid] - 1;
			if (pre_subg_vid >= 0)
			{
				igraph_vector_push_back(&cur_keepv, pre_subg_vid);
			}
			else
			{
				printf("1 : invalid vertices id %d\n", pre_subg_vid);
			}
		}
		igraph_real_t cur_density;
		igraph_vector_init(&cur_density_vertices, 0);
		//fast_densestSubgraph2(&pre_subg, &cur_keepv, &cur_density_subg, &cur_density, &cur_density_vertices);

		fast_densestSubgraph2(&pre_subg, &cur_keepv, &cur_density_subg, &cur_density, &cur_density_vertices);

		// 		printf("---------------------------\n");
		// 		for (int i = 0; i < igraph_vector_size(pre_subg.invmap); i++)
		// 		{
		// 			printf("%d  ", (int)VECTOR(*(pre_subg.invmap))[i]);
		// 		}
		// 		printf("\n");
		// 		printf("---------\n");
		// 		igraph_vector_print(&cur_keepv);
		// 		printf("---------------------------\n");

		printf("cur_density : %f\n", cur_density);
		printf("times : %d hsize : %d hkeepvsize : %d\n", times,
		igraph_vcount(cur_density_subg.graph), igraph_vector_size(&cur_density_vertices));


		// 将这次的稠密子图节点集合与上次的图节点集合并
		std::set<int> cur_m_set(pre_m.begin(), pre_m.end());
		for (int i = 0; i < igraph_vector_size(&cur_density_vertices); i++)
		{
			int vid = VECTOR(cur_density_vertices)[i];
			int origal_vid = VECTOR(*(cur_density_subg.invmap))[vid];
			if (origal_vid >= 0)
			{
				cur_m_set.insert(origal_vid);
				//printf("%d  ", origal_vid);
			}
			else
			{
				printf("2 : invalid vertices id\n");
			}
		}
		//printf("\n");

		printf("cur_vsize :　%d\n\n", cur_m_set.size());

		for each (int vid in cur_m_set)
		{
			cur_m.push_back(vid);
		}

		if (cur_m.size() > in_gsize)
		{
			break;
		}

		subg_vvec.push_back(cur_m);
		cur_m.clear();

		times++;

		mysubgraph_swap(&cur_subg, &pre_subg);
		mysubgraph_empty(&cur_subg, directed);
		// 		igraph_empty(cur_subg.graph, 0, directed);
		// 		igraph_vector_resize(cur_subg.invmap, 0);
		// 		igraph_vector_resize(cur_subg.map_next, 0);
		// 		igraph_vector_resize(cur_subg.map_pre, 0);
	}

	// 	fast_densestSubgraph(&sub_graph, &subgi, &density);
	// 
	// 	vector<int> subg_vpre = subg_vec[times - 1];
	// 	vector<int> subg_vcur(0);
	// 	vector<int> subg_next(0);
	// 	for (int i = 0; i < igraph_vector_size(subgi.invmap); i++)
	// 	{
	// 		subg_vcur.push_back((int)VECTOR(*(subgi.invmap))[i]);
	// 	}
	// 	vecint_union(subg_vpre, subg_vcur, &subg_next);
	// 	subg_vec.push_back(subg_next);

	max_dense_kv_subg(&origal_graph, &subg_vvec, in_gsize, res, &subg_vec_kv);
	print_each_graph(&origal_graph, &subg_vvec, "result\\original_dense_", "gml");
	print_each_graph(&origal_graph, &subg_vec_kv, "result\\dense_", "gml");


	igraph_vector_destroy(&cur_keepv);
	destroy_mysubgraph(&origal_graph);
	destroy_mysubgraph(&pre_subg);
	destroy_mysubgraph(&cur_subg);
	return 0;
}

// subg_density暂时还没返回值
int Fast_DenseALKS2(const igraph_t *input_graph, const int in_gsize, Mysubgraph *res, igraph_real_t *subg_density)
{
	// 存放稠密子图节点
	std::vector< std::vector<int> > subg_vvec(0);
	// 存放k稠密子图节点
	std::vector< std::vector<int> > subg_vec_kv(0);

	std::vector<int> cur_m(0);
	std::vector<int> empty_vec(0);

	// 原图
	Mysubgraph origal_graph;
	//	Mysubgraph pre_subg;
	Mysubgraph cur_subg;
	Mysubgraph cur_density_subg;
	//	igraph_vector_t origal_keepv;
	//	igraph_vector_t cur_keepv;
	//	igraph_vector_t cur_density_vertices;
	igraph_vector_t rm_vertices;



	if (input_graph == NULL)
	{
		printf("input graph is null in Fast_DenseALKS\n");
		return 1;
	}


	init_mysubgraph(input_graph, &origal_graph);
	igraph_bool_t directed = origal_graph.graph->directed;

	//	init_mysubgraph(&pre_subg, directed);
	init_mysubgraph(&cur_subg, directed);
	init_mysubgraph(&cur_density_subg, directed);
	init_mysubgraph(res, directed);

	//	mysubgraph_copy(&pre_subg, &origal_graph);


	int times = 0;
	subg_vvec.push_back(std::vector<int>(0));

	while (subg_vvec[times].size() < in_gsize)
	{
		std::vector<int> pre_m = subg_vvec[times];

		printf("times : %d vsize : %d \n", times, pre_m.size());

		igraph_vector_init(&rm_vertices, 0);
		for each (int vid in pre_m)
		{
			igraph_vector_push_back(&rm_vertices, vid);
		}

		//		printf("times : %d rmvsize : %d \n", times, igraph_vector_size(&rm_vertices));
		subgraph_removeVetices2(&origal_graph, &rm_vertices, &cur_subg);
		//		printf("times : %d curkeepvsize : %d \n", times, igraph_vcount(cur_subg.graph));

		igraph_real_t cur_density;
		//igraph_vector_init(&cur_density_vertices, 0);
		//fast_densestSubgraph2(&pre_subg, &cur_keepv, &cur_density_subg, &cur_density, &cur_density_vertices);

		fast_densestSubgraph(&cur_subg, &cur_density_subg, &cur_density);

		//printf("times : %d hsize : %d hkeepvsize : %d\n", times,
		//igraph_vcount(cur_density_subg.graph), igraph_vector_size(&cur_density_vertices));


		// 将这次的稠密子图节点集合与上次的图节点集合并
		std::set<int> cur_m_set(pre_m.begin(), pre_m.end());
		//cur_m_set.clear();
		// 		for each (int var in pre_m)
		// 		{
		// 			cur_m_set.insert(var);
		// 		}

		printf("times : %d vsize : %d \n", times, cur_m_set.size());
		for (int i = 0; i < igraph_vector_size(cur_density_subg.invmap); i++)
		{
			int origal_vid = VECTOR(*(cur_density_subg.invmap))[i];
			if (origal_vid >= 0)
			{
				cur_m_set.insert(origal_vid);
				if (times == 0)
				{
					printf("%d  ", origal_vid);
				}
			}
			else
			{
				printf("2 : invalid vertices id\n");
			}
		}

		if (times == 0)
		{
			printf("\n");
		}

		printf("times : %d vsize : %d \n\n", times, cur_m_set.size());

		for each (int vid in cur_m_set)
		{
			cur_m.push_back(vid);
		}

		if (cur_m.size() > in_gsize)
		{
			break;
		}

		subg_vvec.push_back(cur_m);
		cur_m.clear();

		times++;


		mysubgraph_empty(&cur_subg, directed);
		//mysubgraph_swap(&cur_subg, &pre_subg);
		// 		igraph_empty(cur_subg.graph, 0, directed);
		// 		igraph_vector_resize(cur_subg.invmap, 0);
		// 		igraph_vector_resize(cur_subg.map_next, 0);
		// 		igraph_vector_resize(cur_subg.map_pre, 0);

		mysubgraph_empty(&cur_density_subg, directed);
		// 		igraph_empty(cur_density_subg.graph, 0, directed);
		// 		igraph_vector_resize(cur_density_subg.invmap, 0);
		// 		igraph_vector_resize(cur_density_subg.map_next, 0);
		// 		igraph_vector_resize(cur_density_subg.map_pre, 0);
	}

	for each (std::vector<int> var in subg_vvec)
	{
		printf("msize : %d\n", var.size());
	}

	dense_vertivce(&origal_graph, &subg_vvec, in_gsize, res);


	//	igraph_vector_destroy(&cur_keepv);
	destroy_mysubgraph(&origal_graph);
	//	destroy_mysubgraph(&pre_subg);
	destroy_mysubgraph(&cur_subg);
	return 0;
}


#endif // !FAST_DENSEALKS_H
