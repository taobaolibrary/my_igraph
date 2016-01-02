#define  _CRT_SECURE_NO_WARNINGS
#include <igraph.h>
#include <stdio.h>
#include <iostream>
#include <assert.h>  
#include <set>
#include <vector>
#include "mystr.h"
#include "read_data.h"

#include "print_graph_4col.h"

// int print_data_4col(FILE *output_file ,igraph_t *output_graph)
// {
// 	if (output_graph == NULL)
// 	{
// 		return 1;
// 	}
// 
// 	if (!igraph_cattribute_has_attr(output_graph, IGRAPH_ATTRIBUTE_VERTEX, "vname"))
// 	{
// 		return 2;
// 	}
// 
// 	if (!igraph_cattribute_has_attr(output_graph, IGRAPH_ATTRIBUTE_EDGE, "weights"))
// 	{
// 		return 3;
// 	}
// 
// 
// 	if (!igraph_cattribute_has_attr(output_graph, IGRAPH_ATTRIBUTE_EDGE, "probabilitys"))
// 	{
// 		return 4;
// 	}
// 
// 	igraph_strvector_t vname_vec;
// 
// 	igraph_strvector_init(&vname_vec, igraph_vcount(output_graph));
// 
// 	VASV(output_graph, "vname", &vname_vec);
// 
// 	for (int i = 0; i < igraph_ecount(output_graph); i++)
// 	{
// 		igraph_real_t probability = EAN(output_graph, "probabilitys", i);
// 		igraph_real_t w = EAN(output_graph, "weights", i);
// 		igraph_integer_t from = 0, to = 0;
// 		igraph_edge(output_graph, i, &from, &to);
// 		const char *from_name = VAS(output_graph, "vname", from);
// 		const char *to_name = VAS(output_graph, "vname", from);
// 		fprintf(output_file, "%s %s %f %f\n", from_name, to_name, w, probability);
// 	}
// 	return 0;
// }

int main()
{
	igraph_t g;
	FILE *input;
	FILE *output_graph;
	/* Same graph, but forcing undirected mode */
	input = fopen("mydata.txt", "r");
	output_graph = fopen("density_graph.txt", "w");
	// input = fopen("to_TAP_core.txt", "r");
	if (input == NULL)
	{
		printf("read file error");
		return 1;
	}

	init_mygraph(input, &g, 0);
	print_data_4col(output_graph, &g);
	igraph_destroy(&g);
	fclose(input);
	return 0;
}

