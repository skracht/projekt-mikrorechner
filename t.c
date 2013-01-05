#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#define nreg 32 
#define debug 0
#define idmaxlen 20

const char * commands[] = {
	"mov",   "20xxyy**",
	"add",   "21xxyyzz",
	"sub",   "22xxyyzz",
	"and",   "30xxyyzz",
	"or",    "31xxyyzz",
	"xor",   "32xxyyzz",
	"not",   "33xx****",
	"lslc",  "43xx****",
	"lsrc",  "44xx****",
	"asrc",  "45xx****",
	"cmpe",  "50xxyy**",
	"cmpne", "51xxyy**",
	"cmpgt", "52xxyy**",
	"cmplt", "53xxyy**",
	"ldw",   "60xxyyvv",
	"stw",   "61xxyyvv",
	"br",    "70iiiiii",
	"jsr",   "71iiiiii",
	"bt",    "72iiiiii",
	"bf",    "73iiiiii",
	"jmp",   "74iiiiii",
	"movi",  "80xxcccc",
	"addi",  "81xxcccc",
	"subi",  "82xxcccc",
	"andi",  "83xxcccc",
	"bseti", "84xxcccc",
	"bclri", "85xxcccc",
	"nop",   "90******",
	"halt",  "f0******"
};

int main(int args, char** argv){
	FILE * source;
	FILE * destination;

	int filesize_source;
	char buf;
	char * str;
	int str_size;
	char * assembler;
	int assembler_size = 1000; //TODO
	const int ncommands = 29; 
	char id[(nreg - 2) * (idmaxlen + 1)];
	int nid;
	memset(id, '\0', (nreg - 2) * (idmaxlen + 1));
	char itoabuf[12];

	int i;
	int j;
	int k;
	int l;

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
	str = malloc(sizeof(char) * filesize_source);
	assembler = malloc(sizeof(char) * assembler_size); // TODO
	assembler[0] = '\0';

	// scan source file
	str_size = 0;
	for(i = 0; i < filesize_source; ++i){
		fscanf(source, "%c", &buf);
		if(buf != 32 && buf != 10)
		  str[str_size++] = buf;
		// hack for "int " blank
		else
		  if(str[str_size - 3] == 'i' && str[str_size - 2] == 'n' && str[str_size - 1] == 't')
		    str[str_size++] = '\0';
	}
	if(ferror(source)){
	  printf("ERROR scaning %s\n", argv[1]);
	  exit(1);
	}/*
	for(i = 0; i < str_size; ++i)
	  printf("%c", str[i]); // */

	// close stream to source file
	if(fclose(source)){
		printf("ERROR closing %s\n", argv[1]);
		exit(1);
	}	

	// process input
	nid = 0;
	k = 0;
	for(i = 0; i < str_size; ++i){
	  // search for High-level programming language elements
	  switch(str[i]){
	  // int
	  case 'i' :
	    if(str[i + 1] == 'n' && str[i + 2] == 't' && str[i + 3] == '\0'){
	      i += 4;
	      j = 0;
	      while(str[i] != '=' && str[i] != ';'){
		id[j++ + ((idmaxlen + 1) * nid)] = str[i++];
		if(j > idmaxlen){
		  printf("ERROR name of identifier is too long (max %d letters)", idmaxlen);
		  exit(1);
		}
	      }
	      nid++;
	      if(str[i] == '='){
		k += sprintf(&assembler[k], "movi %d", nid - 1);
		i++;
		assembler[k++] = 32;
		while(str[i] != ';')
		  assembler[k++] = str[i++];
		assembler[k++] = '\n';
	      }
	      break;
	    }
	    // if
	    if(str[i + 1] == 'f' && str[i + 2] == '('){
	      i += 3;
	      
	      printf("if\n");
	    }
	    break;
	  // for
	  case 'f' :
	    if(str[i + 1] == 'o' && str[i + 2] == 'r' && str[i + 3] == '('){
	      i += 4;
	      
	      printf("for\n");
	    }
	    break;
	  // while
	  case 'w' :
	    if(str[i + 1] == 'h' && str[i + 2] == 'i' && str[i + 3] == 'l' && str[i + 4] == 'e' && str[i + 5] == '('){
	      i += 6;
	      
	      printf("while\n");
	    }
	    break;
	  }
	  for(j = 0; j < nid; ++j){
	    l = 0;
	    while(id[j] != '\0')
	      if(str[i + l] != id[((idmaxlen + 1) * j) + l++])
		break; // TODO
	  }
	  
	  // simple commands
	  switch(str[i]){
	  }
	    
	}

	for(i = 0; i < k; ++i) 
	  printf("%c", assembler[i]);
	// */

	// open stream to destination file
	destination = fopen(argv[2], "w");
	if(destination == NULL){
		printf("ERROR opening %s\n", argv[2]);
		exit(1);
	}

	// write to destination file
	for(i = 0; i < k; ++i)
		fprintf(destination, "%c", assembler[i]);
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
	free(str);
	free(assembler);

	return 0;
}
