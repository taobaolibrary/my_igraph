#define  _CRT_SECURE_NO_WARNINGS

#include <igraph.h>
#include <stdio.h>
#include <iostream>
#include <assert.h>  
#include "mystr.h"
#include "read_data.h"

using namespace std;

int main()
{
	vector<string> edges(0); 
	vector<double> wights(0); 
	vector<double> probabilitys(0);
	map<string, int> mymap;
	igraph_t g;
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

	read_data_4col(input, &edges, &wights, &probabilitys);
	strvec_to_map(edges, &mymap);
	init_graph(&g, edges, wights, probabilitys);

	oldwarnhandler = igraph_set_warning_handler(null_warning_handler);
	igraph_write_graph_gml(&g, stdout, 0, "");

	//igraph_write_graph_graphml(&g, stdout, /*prefixattr=*/ 1);

	int i = 0;
	for each (string var in edges)
	{
		if (i == 0)
		{
			cout << "(" << var << ", ";
			i++;
		}
		else if (i == 1)
		{
			cout << var << ")  ";
			i--;
		}
		
	}

	for each (double var in wights)
	{
		cout << var << "  ";
	}

	for each (double var in probabilitys)
	{
		cout << var << "  ";
	}

	printf("\n");
	for each (string var in edges)
	{
		map<string, int>::iterator map_itr = mymap.find(var);
		if (map_itr != mymap.end()){
			cout << '(' << map_itr->first << ',' << map_itr->second << ')' << ',';
		}
	}
	printf("\n");


	fclose(input);
	return 0;
}
