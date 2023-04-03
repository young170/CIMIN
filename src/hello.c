#include <stdio.h>

int main(int argc, char ** argv) {
	printf("Hello World!\n");

	if (argc != 0) {
		for (int i = 0; i < argc; i++) {
			printf("%s\n", argv[i]);
		}
	}

	printf("===\n");
	while (!feof(stdin)) {
		char c;
		fread(&c, sizeof(char), 1, stdin);

		printf("%c\n", c);
	}

	return 0;
}
