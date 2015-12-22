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
#include <algorithm>
#include <iterator>

using namespace std;

void print_vector(igraph_vector_t *v, FILE *f) {
	long int i;
	for (i = 0; i < igraph_vector_size(v); i++) {
		fprintf(f, " %.3f", VECTOR(*v)[i]);
	}
	fprintf(f, "\n");
}

int vecint_union(vector<int> v1, vector<int> v2, vector<int> *res)
{
	if (res == NULL)
	{
		return 1;
	}
	res->resize(v1.size() + v2.size());
	vector<int>::iterator it;
	sort(v1.begin(), v1.end());
	sort(v2.begin(), v2.end());
	it = set_union(v1.begin(), v1.end(),
		v2.begin(), v2.end(), res->begin());
	res->resize(it - res->begin());
	return 0;
}

int vertices_remain(vector<int> v1, int no_node, vector<int> *res)
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


// 对节点子集求稠密子图

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


// 期望密度不考虑保留边，节点度数考虑保留边
int fast_densestSubgraph3(const Mysubgraph *subgraph, const igraph_vector_t *vertices,
	Mysubgraph *res, igraph_real_t *subg_density, igraph_vector_t *res_keepv)
{
	igraph_vector_t expected_edges;
	igraph_vector_t expected_real_edges;
	igraph_vector_t expected_degrees;
	igraph_vector_t rm_vertice;
	igraph_vector_t rm_vertice_real;
//	igraph_vector_bool_t real_keep_flag;
	Mysubgraph subg_cur;
	igraph_real_t density_cur = 0;
	Mysubgraph subg_next;
	Mysubgraph subg_cur_real;
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
	init_mysubgraph(&subg_cur_real, subgraph->graph->directed);
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
		igraph_vector_init(&expected_real_edges, igraph_vcount(subg_cur_real.graph));
		//		igraph_vector_init(&expected_degrees, no_edges);
		igraph_vector_init(&expected_degrees, no_vids);
		igraph_vector_init(&rm_vertice, 0);


		igraph_vector_resize(&rm_vertice, 0);
		// 用keep中的节点编号生成子图
		vertices_rm(igraph_vcount(subg_cur.graph), keep, &rm_vertice_real);
		subgraph_removeVetices2(&subg_cur, &rm_vertice_real, &subg_cur_real);
		igraph_vector_init(&expected_edges, igraph_ecount(subg_cur_real.graph));
		igraph_edges_expected(subg_cur_real.graph, &expected_real_edges);

		// expected_edges使用前必须初始化
		// 求出每条边的期望权值
		igraph_edges_expected(subg_cur.graph, &expected_edges);

		// 求出节点的期望度数
		vertice_expected_degree(subg_cur.graph, &expected_degrees, igraph_vss_vector(&keep), IGRAPH_OUT, IGRAPH_NO_LOOPS, &expected_edges);
		// 求出图的期望密度
		igraph_real_t cur_density2 = 0;
		graph_expected_density(subg_cur.graph, &cur_density2, igraph_vss_vector(&keep), IGRAPH_OUT, IGRAPH_NO_LOOPS, &expected_edges);
		// 求出图的期望密度
		graph_expected_density(subg_cur_real.graph, &density_cur, igraph_vss_all(), IGRAPH_OUT, IGRAPH_NO_LOOPS, &expected_real_edges);

		//printf("density_cur : %f	density_cur2 : %f\n", density_cur, cur_density2);
		if (density_cur > max_density)
		{
			max_density = density_cur;
			mysubgraph_copy(&max_dentity_subg, &subg_cur);
			igraph_vector_resize(&max_density_keep_v, igraph_vector_size(&keep));
			igraph_vector_copy(&max_density_keep_v, &keep);
			printf("max_density : %f\n", max_density);
			printf("max_keep : %d\n", igraph_vector_size(&keep));
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
		igraph_empty(subg_cur_real.graph, 0, subg_cur.graph->directed);
		igraph_vector_resize(subg_cur_real.invmap, 0);
		igraph_vector_resize(subg_cur_real.map_pre, 0);
		igraph_vector_resize(subg_cur_real.map_next, 0);
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
	igraph_vector_destroy(&rm_vertice_real);
	//igraph_vector_bool_destroy(&real_keep_flag);
	destroy_mysubgraph(&subg_cur_real);
	destroy_mysubgraph(&subg_cur);
	destroy_mysubgraph(&subg_next);
	destroy_mysubgraph(&max_dentity_subg);
	return 0;
}



// 这里经常使用集合并差集，所以使用c++的集合运算
int Fast_DenseALKS(const igraph_t *input_graph, const int in_gsize, Mysubgraph *res, igraph_real_t *subg_density)
{
	// 存放稠密子图节点
	vector< vector<int> > subg_vvec(0);
	// 存放k稠密子图节点
	vector< vector<int> > subg_vec_kv(0);

	vector<int> cur_m(0);
	vector<int> empty_vec(0);

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
	subg_vvec.push_back(vector<int>(0));

	while ( subg_vvec[times-1].size() < in_gsize )
	{
		vector<int> pre_m = subg_vvec[times - 1];

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
		}
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

		printf("times : %d hsize : %d hkeepvsize : %d\n",times ,
			igraph_vcount(cur_density_subg.graph), igraph_vector_size(&cur_density_vertices));


		// 将这次的稠密子图节点集合与上次的图节点集合并
		set<int> cur_m_set(pre_m.begin(), pre_m.end());
		for (int i = 0; i < igraph_vector_size(&cur_density_vertices); i++)
		{
			int vid = VECTOR(cur_density_vertices)[i];
			int origal_vid = VECTOR(*(pre_subg.invmap))[vid];
			if (origal_vid >= 0)
			{
				cur_m_set.insert(vid);
			}
			else
			{
				printf("2 : invalid vertices id\n");
			}
		}

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
		igraph_empty(cur_subg.graph, 0, directed);
		igraph_vector_resize(cur_subg.invmap, 0);
		igraph_vector_resize(cur_subg.map_next, 0);
		igraph_vector_resize(cur_subg.map_pre, 0);
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

	
	igraph_vector_destroy(&cur_keepv);
	destroy_mysubgraph(&origal_graph);
	destroy_mysubgraph(&pre_subg);
	destroy_mysubgraph(&cur_subg);
	return 0;
}


// 这里经常使用集合并差集，所以使用c++的集合运算
int Fast_DenseALKS2(const igraph_t *input_graph, const int in_gsize, Mysubgraph *res, igraph_real_t *subg_density)
{
	// 存放稠密子图节点
	vector< vector<int> > subg_vvec(0);
	// 存放k稠密子图节点
	vector< vector<int> > subg_vec_kv(0);

	vector<int> cur_m(0);
	vector<int> empty_vec(0);

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
	subg_vvec.push_back(vector<int>(0));

	while (subg_vvec[times].size() < in_gsize)
	{
		vector<int> pre_m = subg_vvec[times];

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
		set<int> cur_m_set(pre_m.begin(), pre_m.end());
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
			}
			else
			{
				printf("2 : invalid vertices id\n");
			}
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

		//mysubgraph_swap(&cur_subg, &pre_subg);
		igraph_empty(cur_subg.graph, 0, directed);
		igraph_vector_resize(cur_subg.invmap, 0);
		igraph_vector_resize(cur_subg.map_next, 0);
		igraph_vector_resize(cur_subg.map_pre, 0);

		igraph_empty(cur_density_subg.graph, 0, directed);
		igraph_vector_resize(cur_density_subg.invmap, 0);
		igraph_vector_resize(cur_density_subg.map_next, 0);
		igraph_vector_resize(cur_density_subg.map_pre, 0);
	}

	for each (vector<int> var in subg_vvec)
	{
		printf("msize : %d\n", var.size());
	}


//	igraph_vector_destroy(&cur_keepv);
	destroy_mysubgraph(&origal_graph);
//	destroy_mysubgraph(&pre_subg);
	destroy_mysubgraph(&cur_subg);
	return 0;
}


int main()
{
	igraph_t g;
	Mysubgraph subg_next;
	igraph_vector_t expected_edges;
	igraph_vector_t expected_degrees;
	//	igraph_vector_t weights;
	//	igraph_vector_t probabilitys;
	Mysubgraph subg0;
	//	igraph_warning_handler_t* oldwarnhandler;

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

	igraph_real_t g_density = 0;
	graph_expected_density(&g, &g_density, igraph_vss_all(), IGRAPH_OUT, IGRAPH_NO_LOOPS, &expected_edges);
	printf("density:%f\n", g_density);

	init_mysubgraph(&g, &subg0);

	printf("%d %d\n", (int)igraph_vcount(subg0.graph), (int)igraph_ecount(subg0.graph));

	igraph_real_t density = 0;
	//fast_densestSubgraph(&subg0, &subg_next, &density);
	Fast_DenseALKS(&g, 100, &subg_next, &density);

	igraph_destroy(&g);
	// 这里报错
	destroy_mysubgraph(&subg_next);
	destroy_mysubgraph(&subg0);
	igraph_vector_destroy(&expected_edges);
	igraph_vector_destroy(&expected_degrees);
	fclose(input);
	return 0;
}
