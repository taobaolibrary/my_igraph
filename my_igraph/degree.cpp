#define  _CRT_SECURE_NO_WARNINGS

#include <igraph.h>
#include <stdio.h>
#include <iostream>
#include <assert.h>  
#include "mystr.h"
#include "read_data.h"
#include "expected_degree.h"

using namespace std;


void print_vector(igraph_vector_t *v, FILE *f) {
	long int i;
	for (i = 0; i < igraph_vector_size(v); i++) {
		fprintf(f, " %.3f", VECTOR(*v)[i]);
	}
	fprintf(f, "\n");
}

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
	print_vector(&expected_edges,stdout);

	igraph_vector_init(&expected_degrees, igraph_ecount(&g));
	vertice_expected_degree(&g, &expected_degrees, igraph_vss_all(), IGRAPH_OUT, IGRAPH_NO_LOOPS, &expected_edges);
	print_vector(&expected_degrees, stdout);

	igraph_vector_init(&weights, igraph_ecount(&g));
	igraph_vector_init(&probabilitys, igraph_ecount(&g));
	EANV(&g, "weights", &weights);
	EANV(&g, "probabilitys", &probabilitys);
	vertice_expected_degree(&g, &expected_degrees, igraph_vss_all(), IGRAPH_OUT, IGRAPH_NO_LOOPS, &weights, &probabilitys);
	print_vector(&expected_degrees, stdout);

	vertice_expected_degree(&g, &expected_degrees, igraph_vss_all(), IGRAPH_OUT, IGRAPH_NO_LOOPS);
	print_vector(&expected_degrees, stdout);

	// oldwarnhandler = igraph_set_warning_handler(null_warning_handler);
	// igraph_write_graph_gml(&g, stdout, 0, "");

	fclose(input);
	return 0;
}
