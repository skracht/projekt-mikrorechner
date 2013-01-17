#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>

int main(int args, char** argv){
	FILE * source;
	FILE * destination;

	int filesize_source;
	char * bin;

	int i;

	// open stream to source file
	source = fopen(argv[1], "r");
	if(source == NULL){
		printf("ERROR opening %s\n", argv[1]);
		exit(1);
	}

	// get filesize of source file	
	fseek(source, 0L, SEEK_END);
	filesize_source = ftell(source);
	fseek(source, 0L, SEEK_SET);

	// allocate memory	
	bin = malloc(sizeof(char) * filesize_source);

	// scan source file
	for(i = 0; i < filesize_source; ++i)
		fscanf(source, "%c", &bin[i]);
	if(ferror(source)){
		printf("ERROR scaning %s\n", argv[1]);
		exit(1);
	}

	// close stream to source file
	if(fclose(source)){
		printf("ERROR closing %s\n", argv[1]);
		exit(1);
	}	

	// open stream to destination file
	destination = fopen(argv[2], "w");
	if(destination == NULL){
		printf("ERROR opening %s\n", argv[2]);
		exit(1);
	}

	// write to destination file
	fprintf(destination, "\"");
	for(i = 0; i < filesize_source  * 8; ++i){
		/*if(i % 8 == 0)
			fprintf(destination, "%c", ' ');
		if(i % 32 == 0)
			fprintf(destination, "%c", '\n');
		*/
		if(i % 32 == 0 && i > 0)
			fprintf(destination, "\",\n\"");
		fprintf(destination, "%c", ((bin[i / 8] >> (7 - (i % 8))) & 0x1) + 48);
	}
	fprintf(destination, "\"");
	fprintf(destination, "%c", '\n');
	if(ferror(destination)){
		printf("ERROR writing %s\n", argv[2]);
		exit(1);
	}

	// close stream to destination file
	if(fclose(destination)){
		printf("ERROR closing %s\n", argv[2]);
		exit(1);
	}

	// free allocated memory
	free(bin);

	return 0;
}
