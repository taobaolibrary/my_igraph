#ifndef PRINT_GRAPH_4COL_H
#define PRINT_GRAPH_4COL_H

#include "igraph.h"
#include "graph_partition.h"

int print_data_4col(FILE *output_file, igraph_t *output_graph)
{
	if (output_graph == NULL)
	{
		return 1;
	}

	if (!igraph_cattribute_has_attr(output_graph, IGRAPH_ATTRIBUTE_VERTEX, "vname"))
	{
		return 2;
	}

	if (!igraph_cattribute_has_attr(output_graph, IGRAPH_ATTRIBUTE_EDGE, "weights"))
	{
		return 3;
	}


	if (!igraph_cattribute_has_attr(output_graph, IGRAPH_ATTRIBUTE_EDGE, "probabilitys"))
	{
		return 4;
	}

	igraph_strvector_t vname_vec;

	igraph_strvector_init(&vname_vec, igraph_vcount(output_graph));

	VASV(output_graph, "vname", &vname_vec);

	for (int i = 0; i < igraph_ecount(output_graph); i++)
	{
		igraph_real_t probability = EAN(output_graph, "probabilitys", i);
		igraph_real_t w = EAN(output_graph, "weights", i);
		igraph_integer_t from = 0, to = 0;
		igraph_edge(output_graph, i, &from, &to);
		const char *from_name = VAS(output_graph, "vname", from);
		const char *to_name = VAS(output_graph, "vname", to);
		fprintf_s(output_file, "%s %s %f %f\n", from_name, to_name, w, probability);
	}
	return 0;
}

int print_each_graph(const Mysubgraph *origal_graph, const std::vector< std::vector<int> > *subg_vvec, const std::string filename_prefix, const std::string filetype)
{
	igraph_warning_handler_t* oldwarnhandler;
	if (origal_graph == NULL || subg_vvec == NULL)
	{
		return 1;
	}
	long int no_node = igraph_vcount(origal_graph->graph);
	long int no_edge = igraph_ecount(origal_graph->graph);
	igraph_vector_t remain_vertices;
	igraph_vector_t rm_vertices;
	Mysubgraph subg;
	igraph_real_t max_density = -1;
	igraph_real_t directed = origal_graph->graph->directed;

	init_mysubgraph(&subg, directed);
	igraph_vector_init(&remain_vertices, 0);
	igraph_vector_init(&rm_vertices, 0);
	for (int subg_th = 1; subg_th < (*subg_vvec).size(); subg_th++)
	{
		std::ostringstream tmp_s;
		tmp_s << subg_th;
		FILE * graph_file = NULL;
		std::string filename = "";
		filename = filename_prefix + tmp_s.str();
		filename += "." + filetype;

		fopen_s(&graph_file, filename.c_str(), "w");
		if (graph_file == NULL)
		{
			printf("open file error\n");
			return 1;
		}

		igraph_vector_resize(&rm_vertices, 0);
		igraph_vector_resize(&remain_vertices, 0);
		for each (int vid in (*subg_vvec)[subg_th])
		{
			igraph_vector_push_back(&remain_vertices, vid);
		}

		vertices_rm(no_node, remain_vertices, &rm_vertices);
		subgraph_removeVetices2(origal_graph, &rm_vertices, &subg);

		if (filetype == "txt")
		{
			print_data_4col(graph_file, subg.graph);
		}
		else
		{
			oldwarnhandler = igraph_set_warning_handler(null_warning_handler);
			igraph_write_graph_gml(subg.graph, graph_file, 0, "");
			mysubgraph_empty(&subg, directed);
		}

		fclose(graph_file);
		graph_file = NULL;
	}

	igraph_vector_destroy(&rm_vertices);
	igraph_vector_destroy(&remain_vertices);
	destroy_mysubgraph(&subg);
	return 0;
}


#endif


