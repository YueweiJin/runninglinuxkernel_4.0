#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SZ_1M (1024 * 1024) 

int main(int argc,char **argv) 
{
	char *p = NULL;

	p = malloc(256 * SZ_1M); 
	memset(p, 0x0, 256 * SZ_1M);
	return 0;
}
