#ifndef READ_DATA_H
#define READ_DATA_H

#include <vector>
#include <map>
#include <string>
#include <set>
#include <assert.h>
#include "mystr.h"

/*
* edges c++ 字符串列表, 存储边
* wights c++ double型列表，存放权值
* probability c++ double型列表存放边概率
*/

int read_data_4col(FILE *input, std::vector<std::string> *edges, std::vector<double> *wights, std::vector<double> *probabilitys){
	/* turn on attribute handling */
	if (input == NULL) return 1;
	if (edges == NULL) return 2;

	std::string str = "";

	char tmp_s[256];

	while (fgets(tmp_s, 256, input)!=NULL)
	{
		std::string s = tmp_s;
		// 去掉字符串前后的空格
		trim(s);
		// 划分字符串成字符串列表
		std::vector<std::string> strvec = split(s);
		int v_size = strvec.size();

		for each (std::string var in strvec)
		{
			assert(var != "" || var != " ");
		}

		if (v_size >= 2)
		{
			edges->push_back(strvec[0]);
			edges->push_back(strvec[1]);
		}
		if (v_size >= 3 && wights != NULL)
		{
			std::string s_num = strvec[2];
			assert(isNum(s_num) == true);
			wights->push_back(stringToNum<double>(s_num));
		}

		if (v_size >= 4 && probabilitys != NULL)
		{
			//printf("%d\n", probabilitys->size());
			std::string s_num = strvec[3];
			assert(isNum(s_num) == true);
			probabilitys->push_back(stringToNum<double>(s_num));
		}
		strvec.clear();
	}
	return 0;
}

int init_graph(igraph_t *g, std::vector<std::string> edges,
	std::vector<double> weights, std::vector<double> probabilitys, const bool directed){

	assert(edges.size() % 2 == 0);
	/*assert(g != NULL);*/

	igraph_vector_t evec;
	igraph_vector_t wvec;
	igraph_vector_t pvec;

	/* turn on attribute handling */
	igraph_i_set_attribute_table(&igraph_cattribute_table);
	/* Same graph, but forcing undirected mode */
	//igraph_read_graph_ncol(&g, input, 0, 1, IGRAPH_ADD_WEIGHTS_NO, 0);

	std::map<std::string, int> vmap;
	strvec_to_map(edges, &vmap);

	// 初始化图节点
	igraph_empty(g, vmap.size(), directed);
	for each (std::pair<std::string, int> var in vmap){
		SETVAS(g, "vname", (int)var.second, var.first.data());
	}

	// 将节点的名字编号
	igraph_vector_init(&evec, edges.size());
	for (int i = 0; i < edges.size(); i++)
	{
		std::map<std::string, int>::iterator tmp = vmap.find(edges[i]);
		VECTOR(evec)[i] = tmp->second;
		//printf("(%s,%d)", tmp->first.data(), tmp->second);
	}

	// 类型转换
	igraph_vector_init(&wvec, weights.size());
	for (int i = 0; i < weights.size(); i++)
	{
		VECTOR(wvec)[i] = weights[i];
	}

	igraph_vector_init(&pvec, probabilitys.size());
	for (int i = 0; i < probabilitys.size(); i++)
	{
		VECTOR(pvec)[i] = probabilitys[i];
	}

	// 初始化边
	igraph_add_edges(g, &evec, 0);

	// 给边添加属性
	// 给边添加属性
	if (weights.size() == edges.size() / 2)
	{
		SETEANV(g, "weights", &wvec);
	}
	else
	{
		printf("weights数量不等于边数不符合\n");
	}

	if (probabilitys.size() == edges.size() / 2)
	{
		SETEANV(g, "probabilitys", &pvec);
	}
	else
	{
		printf("probabilitys数量不等于边数不符合\n");
	}
	return 0;
}

int init_mygraph(FILE *input, igraph_t *g, const bool derected)
{
	std::vector<std::string> edges(0);
	std::vector<double> wights(0);
	std::vector<double> probabilitys(0);
	std::map<std::string, int> mymap;
	read_data_4col(input, &edges, &wights, &probabilitys);
	strvec_to_map(edges, &mymap);
	init_graph(g, edges, wights, probabilitys, derected);
	edges.clear();
	wights.clear();
	probabilitys.clear();
	mymap.clear();
	return 0;
}

void null_warning_handler(const char *reason, const char *file,
	int line, int igraph_errno) {
}



#endif
