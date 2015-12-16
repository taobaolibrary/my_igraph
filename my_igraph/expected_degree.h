#ifndef EXPECTED_DEGREE_H
#define EXPECTED_DEGREE_H

#include <igraph.h>
#include <stdio.h>
#include <iostream>
#include <assert.h>  

int igraph_edges_expected(const igraph_t *graph, igraph_vector_t *res) {
	igraph_vector_t weights;
	igraph_vector_t probabilitys;
	long int no_of_nodes = igraph_vcount(graph);
	long int no_of_edges = igraph_ecount(graph);

	igraph_vector_init(&weights, 0);
	igraph_vector_init(&probabilitys, 0);

	EANV(graph, "weights", &weights);

	if (igraph_vector_size(&weights) != igraph_ecount(graph)) {
		IGRAPH_ERROR("Invalid weight vector length", IGRAPH_EINVAL);
		return 1;
	}

	EANV(graph, "probabilitys", &probabilitys);

	if (igraph_vector_size(&probabilitys) != igraph_ecount(graph)) {
		IGRAPH_ERROR("Invalid probabilitys vector length", IGRAPH_EINVAL);
		return 1;
	}

	IGRAPH_CHECK(igraph_vector_resize(res, no_of_edges));
	igraph_vector_null(res);

	for (int j = 0; j < no_of_edges; j++) {
		VECTOR(*res)[j] = VECTOR(weights)[j] * VECTOR(probabilitys)[j];
	}

	// 	printf("�ڵ�����%d\n", igraph_vcount(graph));
	// 	printf("������%d\n",igraph_vector_size(&weights));
	// 	printf("������%d\n", igraph_ecount(graph));

	igraph_vector_destroy(&weights);
	igraph_vector_destroy(&probabilitys);
	return 0;
}

int vertice_expected_degree(const igraph_t *graph, igraph_vector_t *res,
	const igraph_vs_t vids, igraph_neimode_t mode,
	igraph_bool_t loops, const igraph_vector_t *weights, const igraph_vector_t *probabilitys) {

	long int no_of_nodes = igraph_vcount(graph);
	igraph_vit_t vit;
	long int no_vids;
	igraph_vector_t neis;
	long int i;

	if (!weights || !probabilitys)
		return igraph_degree(graph, res, vids, mode, loops);

	if (igraph_vector_size(weights) != igraph_ecount(graph)) {
		IGRAPH_ERROR("Invalid weight vector length", IGRAPH_EINVAL);
	}

	if (igraph_vector_size(probabilitys) != igraph_ecount(graph)) {
		IGRAPH_ERROR("Invalid probabilitys vector length", IGRAPH_EINVAL);
	}

	IGRAPH_CHECK(igraph_vit_create(graph, vids, &vit));
	IGRAPH_FINALLY(igraph_vit_destroy, &vit);
	no_vids = IGRAPH_VIT_SIZE(vit);

	IGRAPH_VECTOR_INIT_FINALLY(&neis, 0);
	IGRAPH_CHECK(igraph_vector_reserve(&neis, no_of_nodes));
	IGRAPH_CHECK(igraph_vector_resize(res, no_vids));
	igraph_vector_null(res);

	if (loops) {
		for (i = 0; !IGRAPH_VIT_END(vit); IGRAPH_VIT_NEXT(vit), i++) {
			long int vid = IGRAPH_VIT_GET(vit);
			long int j, n;
			IGRAPH_CHECK(igraph_incident(graph, &neis, (igraph_integer_t)vid, mode));
			n = igraph_vector_size(&neis);
			for (j = 0; j < n; j++) {
				long int edge = (long int)VECTOR(neis)[j];
				VECTOR(*res)[i] += VECTOR(*weights)[edge] * VECTOR(*probabilitys)[edge];
			}
		}
	}
	else {
		for (i = 0; !IGRAPH_VIT_END(vit); IGRAPH_VIT_NEXT(vit), i++) {
			long int vid = IGRAPH_VIT_GET(vit);
			long int j, n;
			IGRAPH_CHECK(igraph_incident(graph, &neis, (igraph_integer_t)vid, mode));
			n = igraph_vector_size(&neis);
			for (j = 0; j < n; j++) {
				long int edge = (long int)VECTOR(neis)[j];
				long int from = IGRAPH_FROM(graph, edge);
				long int to = IGRAPH_TO(graph, edge);
				if (from != to) {
					VECTOR(*res)[i] += VECTOR(*weights)[edge] * VECTOR(*probabilitys)[edge];
				}
			}
		}
	}

	igraph_vit_destroy(&vit);
	igraph_vector_destroy(&neis);
	IGRAPH_FINALLY_CLEAN(2);

	return 0;
}

int vertice_expected_degree(const igraph_t *graph, igraph_vector_t *res,
	const igraph_vs_t vids, igraph_neimode_t mode,
	igraph_bool_t loops, const igraph_vector_t *expected_edge) {

	long int no_of_nodes = igraph_vcount(graph);
	igraph_vit_t vit;
	long int no_vids;
	igraph_vector_t neis;
	long int i;

	if (!expected_edge)
		return igraph_degree(graph, res, vids, mode, loops);

	if (igraph_vector_size(expected_edge) != igraph_ecount(graph)) {
		IGRAPH_ERROR("Invalid weight vector length", IGRAPH_EINVAL);
	}

	if (mode != IGRAPH_OUT && mode != IGRAPH_IN && mode != IGRAPH_ALL) {
		IGRAPH_ERROR("degree calculation failed", IGRAPH_EINVMODE);
	}

	if (!igraph_is_directed(graph)) {
		mode = IGRAPH_ALL;
	}

	IGRAPH_CHECK(igraph_vit_create(graph, vids, &vit));
	IGRAPH_FINALLY(igraph_vit_destroy, &vit);

	IGRAPH_CHECK(igraph_vit_create(graph, vids, &vit));
	IGRAPH_FINALLY(igraph_vit_destroy, &vit);
	no_vids = IGRAPH_VIT_SIZE(vit);

	IGRAPH_VECTOR_INIT_FINALLY(&neis, 0);
	IGRAPH_CHECK(igraph_vector_reserve(&neis, no_of_nodes));
	IGRAPH_CHECK(igraph_vector_resize(res, no_vids));
	igraph_vector_null(res);

	if (loops) {
		for (i = 0; !IGRAPH_VIT_END(vit); IGRAPH_VIT_NEXT(vit), i++) {
			long int vid = IGRAPH_VIT_GET(vit);
			long int j, n;
			IGRAPH_CHECK(igraph_incident(graph, &neis, (igraph_integer_t)vid, mode));
			n = igraph_vector_size(&neis);
			for (j = 0; j < n; j++) {
				long int edge = (long int)VECTOR(neis)[j];
				VECTOR(*res)[i] += VECTOR(*expected_edge)[edge];
			}
		}
	}
	else {
		for (i = 0; !IGRAPH_VIT_END(vit); IGRAPH_VIT_NEXT(vit), i++) {
			long int vid = IGRAPH_VIT_GET(vit);
			long int j, n;
			IGRAPH_CHECK(igraph_incident(graph, &neis, (igraph_integer_t)vid, mode));
			n = igraph_vector_size(&neis);
			for (j = 0; j < n; j++) {
				long int edge = (long int)VECTOR(neis)[j];
				long int from = IGRAPH_FROM(graph, edge);
				long int to = IGRAPH_TO(graph, edge);
				if (from != to) {
					VECTOR(*res)[i] += VECTOR(*expected_edge)[edge];
				}
			}
		}
	}

	igraph_vit_destroy(&vit);
	igraph_vector_destroy(&neis);
	IGRAPH_FINALLY_CLEAN(2);

	return 0;
}

int vertice_expected_degree(const igraph_t *graph, igraph_vector_t *res,
	const igraph_vs_t vids, igraph_neimode_t mode, igraph_bool_t loops)
{
	igraph_vector_t expected_edge;
	igraph_vector_init(&expected_edge, igraph_ecount(graph));
	igraph_edges_expected(graph, &expected_edge);
	vertice_expected_degree(graph, res, vids, mode, loops, &expected_edge);
	igraph_vector_destroy(&expected_edge);
	return 0;
}

#endif
