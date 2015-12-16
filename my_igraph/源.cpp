#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <assert.h>

int main(){
	FILE* file;
	file = fopen("mydata.txt", "r");
	assert(file == NULL);
	return 0;
}