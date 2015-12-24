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

	// Mysubgraph�д�ŵ���ָ�룬����Ľ���ֻ�ǽ������Ǵ�ŵĵ�ַ
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
	// ����������ڴ���ǰto�е�graph��invmapδ��ʼ�������Ծ���
	igraph_copy(to->graph, from->graph);
	igraph_vector_copy(to->invmap, from->invmap);
	igraph_vector_copy(to->map_next, from->map_next);
	igraph_vector_copy(to->map_pre, from->map_pre);
	return 0;
}

// �Խڵ��Ӽ��������ͼ��ͼ�������ܶȺͽڵ�����������������Ǳ�����
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

	// �ȵõ�ԭͼ��copy������ʹ�õ���ɾ���ڵ����ԭͼ
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

		// expected_edgesʹ��ǰ�����ʼ��
		// ���ÿ���ߵ�����Ȩֵ
		igraph_edges_expected(subg_cur.graph, &expected_edges);

		// ����ڵ����������
		vertice_expected_degree(subg_cur.graph, &expected_degrees, igraph_vss_all(), IGRAPH_OUT, IGRAPH_NO_LOOPS, &expected_edges);
		// ���ͼ����������
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

/*
// �Խڵ��Ӽ��������ͼ��ͼ�������ܶȺͽڵ���������������Ǳ�����
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

	// �ȵõ�ԭͼ��copy������ʹ�õ���ɾ���ڵ����ԭͼ
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

		// expected_edgesʹ��ǰ�����ʼ��
		// ���ÿ���ߵ�����Ȩֵ
		igraph_edges_expected(subg_cur.graph, &expected_edges);

		// ����ڵ����������
		vertice_expected_degree(subg_cur.graph, &expected_degrees, igraph_vss_vector(&keep), IGRAPH_OUT, IGRAPH_NO_LOOPS, &expected_edges);
		// ���ͼ�������ܶ�
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
			if (VECTOR(expected_degrees)[i] < density_cur)
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

		// swap��СҪ��ͬ
		//igraph_vector_reserve(&keep, igraph_vector_size(&keep2));
		//igraph_vector_copy(&keep, &keep2);
		// ����ͼ�ڵ���ת��Ϊ��ͼ�ڵ���
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
*/

// �Խڵ��Ӽ��������ͼ��ͼ�������ܶȺͽڵ���������������Ǳ�����
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
	igraph_vector_init(&expected_edges, 0);
	igraph_vector_init(&expected_degrees, 0);

	// keep�����ͼ�Ľڵ㼯�ϣ������������߹�������ɾ���Ľڵ�
	igraph_vector_init(&keep, 0);
	igraph_vector_copy(&keep, vertices);

	// �ȵõ�ԭͼ��copy������ʹ�ò�ɾ���ڵ��Ƶ��õ��ı���ԭͼ
	igraph_vector_init(&rm_vertice, 0);
	subgraph_removeVetices2(subgraph, &rm_vertice, &subg_cur);

	// ���ڵ����ӳ�䵽��ͼ��
	for (int i = 0; i < igraph_vector_size(&keep); i++)
	{
		int pre_vid = VECTOR(keep)[i];  // ��ͼ�Ľڵ���
		int cur_vid = VECTOR(*(subg_cur.map_next))[pre_vid] - 1;  // ��ͼ���ӳ�䵽��ͼ�ı��
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
		// ��ǰͼ�Ľڵ������������ ��ͼ�Ľڵ����
		long int no_nodes = igraph_vcount(subg_cur.graph);
		long int no_edges = igraph_ecount(subg_cur.graph);
		long int no_vids = igraph_vector_size(&keep);

		if (no_vids < 2)
		{
			break;
		}

		// ��ʼ��
		mysubgraph_empty(&subg_next, subgraph->graph->directed);
		igraph_vector_resize(&expected_edges, no_edges);
		igraph_vector_resize(&expected_degrees, no_vids);
		igraph_vector_resize(&rm_vertice, 0);

		// ���ÿ���ߵ�����Ȩֵ
		igraph_edges_expected(subg_cur.graph, &expected_edges);

		// ����ڵ����������
		vertice_expected_degree(subg_cur.graph, &expected_degrees, igraph_vss_vector(&keep), IGRAPH_OUT, IGRAPH_NO_LOOPS, &expected_edges);
		// ���ͼ�������ܶ�
		graph_expected_density(subg_cur.graph, &density_cur, igraph_vss_vector(&keep), IGRAPH_OUT, IGRAPH_NO_LOOPS, &expected_edges);

		// ��������ͼ
		if (density_cur > max_density)
		{
			max_density = density_cur;
			mysubgraph_copy(&max_dentity_subg, &subg_cur);
			igraph_vector_resize(&max_density_keep_v, igraph_vector_size(&keep));
			igraph_vector_copy(&max_density_keep_v, &keep);
			//printf("max_density : %f\n", max_density);
			//printf("max_keep : %d\n", igraph_vector_size(&keep));
		}


		// ɾ������������ͼ�������ܶ�С�Ľڵ�
		for (long i = 0; i < igraph_vector_size(&keep); i++)
		{
			long vid = (long)VECTOR(keep)[i];
			if (VECTOR(expected_degrees)[i] < density_cur)
			{
				igraph_vector_push_back(&rm_vertice, vid);
			}
		}

		// ���û�нڵ������������ͼ�������ܶ�С����ɾ������������С�Ľڵ�
		if (igraph_vector_size(&rm_vertice) == 0)
		{
			long int min_idx = igraph_vector_which_min(&expected_degrees);
			long vid = (long)VECTOR(keep)[min_idx];
			igraph_vector_push_back(&rm_vertice, vid);
		}

		// ɾ���ڵ��������ͼ
		subgraph_removeVetices2(&subg_cur, &rm_vertice, &subg_next);
		//printf("%d %d\n", (int)igraph_vcount(subg_next.graph), (int)igraph_ecount(subg_next.graph));

		// swap��СҪ��ͬ
		//igraph_vector_reserve(&keep, igraph_vector_size(&keep2));
		//igraph_vector_copy(&keep, &keep2);

		// ����ͼ�ڵ���ת��Ϊ��ͼ�ڵ���
		int cnt = 0;
		igraph_vector_init(&keep2, 0);
		for (int i = 0; i < igraph_vector_size(&keep); i++)
		{
			long pre_vid = VECTOR(keep)[i];  // ��ͼӳ�䵽��ͼid
			// ��ͼidӳ�䵽��ͼid�������-1���Ǹ���˵���Ѿ���ɾ��
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

		// ��ʣ��Ľڵ�����keep�����´ε���
		igraph_vector_resize(&keep, igraph_vector_size(&keep2));
		igraph_vector_copy(&keep, &keep2);

		mysubgraph_swap(&subg_cur, &subg_next);
	}


	//printf("keep : %d\n", igraph_vector_size(&max_density_keep_v));

	//igraph_vector_print(&max_density_keep_v);

	// �����������ͼ�Ľڵ㼯�ϣ������res_keepv�����ڷ���
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

// �����ܶȲ����Ǳ����ߣ��ڵ�������Ǳ�����
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

	igraph_vector_init(&expected_edges, 0);
	igraph_vector_init(&expected_real_edges, 0);
	igraph_vector_init(&expected_degrees, 0);


	igraph_vector_init(&max_density_keep_v, 0);

	// keep�����ͼ�Ľڵ㼯�ϣ������������߹�������ɾ���Ľڵ�
	igraph_vector_init(&keep, 0);
	igraph_vector_copy(&keep, vertices);

	// �ȵõ�ԭͼ��copy������ʹ�ò�ɾ���ڵ��Ƶ��õ��ı���ԭͼ
	igraph_vector_init(&rm_vertice, 0);
	subgraph_removeVetices2(subgraph, &rm_vertice, &subg_cur);

	// ���ڵ����ӳ�䵽��ͼ��
	for (int i = 0; i < igraph_vector_size(&keep); i++)
	{
		int pre_vid = VECTOR(keep)[i];
		int cur_vid = VECTOR(*(subg_cur.map_next))[pre_vid] - 1;
		if (cur_vid < 0)
		{
			printf("invalid vertices id\n");
			break;
		}
		VECTOR(keep)[i] = cur_vid;
	}

	int pflag = 0;
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
		igraph_vector_resize(&expected_edges, 0);
		igraph_vector_resize(&expected_real_edges, 0);
		igraph_vector_resize(&expected_degrees, 0);
		igraph_vector_resize(&rm_vertice, 0);

		// ��keep�еĽڵ���������ͼ,������������Ȩ
		vertices_rm(igraph_vcount(subg_cur.graph), keep, &rm_vertice_real);
		subgraph_removeVetices2(&subg_cur, &rm_vertice_real, &subg_cur_real);
		igraph_edges_expected(subg_cur_real.graph, &expected_real_edges);

		// expected_edgesʹ��ǰ�����ʼ��
		// ���ÿ���ߵ�����Ȩֵ
		igraph_edges_expected(subg_cur.graph, &expected_edges);

		// ����ڵ����������
		vertice_expected_degree(subg_cur.graph, &expected_degrees, igraph_vss_vector(&keep), IGRAPH_OUT, IGRAPH_NO_LOOPS, &expected_edges);

		// ���ͼ�������ܶ�
		// igraph_real_t cur_density2 = 0;
		// graph_expected_density(subg_cur.graph, &cur_density2, igraph_vss_vector(&keep), IGRAPH_OUT, IGRAPH_NO_LOOPS, &expected_edges);

		// ���ͼ�������ܶ�
		graph_expected_density(subg_cur_real.graph, &density_cur, igraph_vss_all(), IGRAPH_OUT, IGRAPH_NO_LOOPS, &expected_real_edges);

		//printf("max_keep : %d density_cur : %f	density_cur2 : %f\n", igraph_vector_size(&keep), density_cur, cur_density2);

		// ��������ͼ
		if (density_cur > max_density)
		{
			max_density = density_cur;
			mysubgraph_copy(&max_dentity_subg, &subg_cur);
			igraph_vector_resize(&max_density_keep_v, igraph_vector_size(&keep));
			igraph_vector_copy(&max_density_keep_v, &keep);
			//printf("max_density : %f\n", max_density);
			//printf("max_keep : %d\n", igraph_vector_size(&keep));
		}

		// expected_degrees(�����˱�����)��keep�д�ŵ���cur_subg�ڵ���
		// ɾ������������ͼ�������ܶ�С�Ľڵ�
		for (long i = 0; i < igraph_vector_size(&keep); i++)
		{
			long vid = (long)VECTOR(keep)[i];
			if (VECTOR(expected_degrees)[i] < density_cur)
			{
				igraph_vector_push_back(&rm_vertice, vid);
			}
		}

		// ���û�нڵ������������ͼ�������ܶ�С����ɾ������������С�Ľڵ�
		if (igraph_vector_size(&rm_vertice) == 0)
		{
			long int min_idx = igraph_vector_which_min(&expected_degrees);
			long vid = (long)VECTOR(keep)[min_idx];
			igraph_vector_push_back(&rm_vertice, vid);
		}

		// ɾ���ڵ��������ͼ
		subgraph_removeVetices2(&subg_cur, &rm_vertice, &subg_next);

		// ����ͼ�ڵ���ת��Ϊ��ͼ�ڵ���
		int cnt = 0;
		igraph_vector_init(&keep2, 0);
		for (int i = 0; i < igraph_vector_size(&keep); i++)
		{
			long pre_vid = VECTOR(keep)[i];
			// ��ͼidӳ�䵽��ͼid�������-1���Ǹ���˵���Ѿ���ɾ��
			long cur_vid = VECTOR(*(subg_next.map_next))[pre_vid] - 1;
			if (cur_vid < 0)
			{
				//printf("invalid vertices\n");
				cnt++;
			}
			if (cur_vid >= 0)
				igraph_vector_push_back(&keep2, cur_vid);
		}
		//printf("rm:%d rm2:%d\n",cnt, igraph_vector_size(&rm_vertice));
		//printf("density : %f\n", density_cur);

		// ��ʣ��Ľڵ�����keep�����´ε���
		igraph_vector_resize(&keep, igraph_vector_size(&keep2));
		igraph_vector_copy(&keep, &keep2);

		mysubgraph_empty(&subg_cur_real, subg_cur.graph->directed);
		mysubgraph_swap(&subg_cur, &subg_next);
	}

	// �����������ͼ�Ľڵ㼯�ϣ������res_keepv�����ڷ���
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


#endif // !FAST_DENSESTSUBGRAPH_H
