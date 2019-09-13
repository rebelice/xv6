#include <stdio.h>

extern void bye(char *s);

int main(void* argc, void** argv)
{
	printf("Hello, %s\n", (char *)argv[1]);
	bye(argv[1]);
	return 0;
}