#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#define iiiiii 8388608 // = 2^23
#define cccc 32768 // = 2^15
#define vv 128 // = 2^7
#define nreg 32 
#define debug 0

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
	"jmp",   "74xx****",
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
	char * str;
	char ** words;
	char ** identifiersdestination;
	char ** identifierssource;
	int * memlines;
	int * indexesbin;
	char * bin;
	int wordcounter;
	const int ncommands = 29;
	int buf;
	int commandcounter;

	int i;
	int j;
	int k;
	int l;
	int m;
	int n;

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
	str = malloc((sizeof(char) * filesize_source) + 1);
	bin = calloc(filesize_source * 8, sizeof(char));
	

	// initiate str, count words and scan source file
	str[0] = '\0';
	wordcounter = 0;
	for(i = 1; i < filesize_source + 1; ++i){
	  fscanf(source, "%c", &str[i]);
	  if(str[i] == 32 || str[i] == 10)
	    str[i] = '\0';
	  else if(str[i - 1] == '\0')
	    wordcounter++;
	}
	if(ferror(source)){
	  printf("ERROR scaning %s\n", argv[1]);
	  exit(1);
	}

	// close stream to source file
	if(fclose(source)){
		printf("ERROR closing %s\n", argv[1]);
		exit(1);
	}	

	// allocate memory for word list
	words = malloc(sizeof(char *) * wordcounter);
	identifiersdestination = calloc(wordcounter, sizeof(char *));
	identifierssource = calloc(wordcounter, sizeof(char *));
	memlines = malloc(sizeof(int) * wordcounter);
	indexesbin = malloc(sizeof(int) * wordcounter);

	// divide string into words
	j = 0;
	for(i = 0; i < filesize_source; ++i)
		if(str[i - 1] == '\0' && str[i] != '\0')
			words[j++] = &str[i];

	// process input
	l = 0;
	m = 0;
	n = 0;
	commandcounter = 0;
	for(i = 0; i < wordcounter; ++i){
		for(j = 0; j < ncommands; ++j){	
			if(!strcmp(commands[j * 2], words[i])){
				bin[l++] = ((commands[(j * 2) + 1][0] < 87 ?
							(commands[(j * 2) + 1][0] - 48) << 4 :
							(commands[(j * 2) + 1][0] - 87) << 4) + 
						(commands[(j * 2) + 1][1] < 87 ?
						 commands[(j * 2) + 1][1] - 48 : 
						 commands[(j * 2) + 1][1] - 87));
				commandcounter++;
				if(debug == 2 || debug == 3)
					printf("\ncommand %6s #%d :\nop %.2x", words[i], commandcounter, bin[l - 1] & 0xff);
				for(k = 1; k < 4; ++k){
					switch(commands[(j * 2) + 1][k * 2]){
						case 'x' :
						case 'y' :
						case 'z' :
							if(atoi(words[i + 1]) < 0 || atoi(words[i + 1]) >= nreg){
								printf("ERROR you are adressing nonexistent register %d in %s-command command#: %d\n",
										atoi(words[i + 1]), commands[j * 2], commandcounter);
								exit(1);
							}
							sscanf(words[++i], "%d", &bin[l++]);
							if(debug == 2 || debug == 3)
								printf(" rg %.2x", bin[l - 1] & 0xff);
							break;
						case 'i' :
							if((words[i + 1][0] < 58 && words[i + 1][0] > 47) || words[i + 1][0] == 45){
								if(atoi(words[i + 1]) < -iiiiii || atoi(words[i  + 1]) > iiiiii - 1){
									printf("ERROR out of bounds immediate24 %d in %s-command command#: %d\n",
											atoi(words[i + 1]), commands[j * 2], commandcounter);
									exit(1);
								}
								sscanf(words[++i], "%d", &buf);
								bin[l++] = (buf & 0x00ff0000) >> 16;
								bin[l++] = (buf & 0x0000ff00) >> 8;
								bin[l++] = (buf & 0x000000ff);
								k += 2;
								if(debug == 2 || debug == 3){
									printf(" ii %.2x", bin[l - 3] & 0xff);
									printf(" ii %.2x", bin[l - 2] & 0xff);
									printf(" ii %.2x", bin[l - 1] & 0xff);

								}
							}
							else{
								identifierssource[n] = words[++i];
								indexesbin[n++] = l;
								l += 3;
								k += 2;
							}
							break;
						case 'c' :
							if(atoi(words[i + 1]) < -cccc || atoi(words[i  + 1]) > cccc - 1){
								printf("ERROR out of bounds immediate16 %d in %s-command command#: %d\n",
										atoi(words[i + 1]), commands[j * 2], commandcounter);
								exit(1);
							}
							sscanf(words[++i], "%d", &buf);
							bin[l++] = (buf & 0x0000ff00) >> 8;
							bin[l++] = (buf & 0x000000ff);
							k++;
							if(debug == 2|| debug == 3){
								printf(" cc %.2x", bin[l - 2] & 0xff);
								printf(" cc %.2x", bin[l - 1] & 0xff);
							}
							break;	
						case 'v' :
							if(atoi(words[i + 1]) < -vv || atoi(words[i  + 1]) > vv - 1){
								printf("ERROR out of bounds immediate8 %d in %s-command command#: %d\n",
										atoi(words[i + 1]), commands[j * 2], commandcounter);
								exit(1);
							}
							sscanf(words[++i], "%d", &buf);
							bin[l++] = (buf & 0x000000ff);
							if(debug == 2|| debug == 3)
								printf(" vv %.2x", bin[l - 1] & 0xff);
							break;
						case '*' :
						case '0' :
							bin[l++] = 0;
							if(debug == 2 || debug == 3)
								printf(" od %.2x", bin[l - 1] & 0xff);
							break;
					}
				}
				break;
			}
		}
		if(j == ncommands){
			identifiersdestination[m] = words[i];
			memlines[m++] = commandcounter;
		}
	}
	for(i = 0; i < n; ++i){
		for(j = 0; j < m; ++j){	
			if(!strcmp(identifiersdestination[j], identifierssource[i])){
				l = indexesbin[i];
				buf = memlines[j] - ((l + 3) / 4);
				bin[l++] = (buf & 0x00ff0000) >> 16;
				bin[l++] = (buf & 0x0000ff00) >> 8;
				bin[l] = (buf & 0x000000ff);
				break;
			}
		}
		if(j == ncommands){
			printf("ERROR %s is not set\n", identifierssource[i]);
			exit(1);
		}
	}

	// print binary
	if(debug == 1 || debug == 3){
		for(i = 0; i < commandcounter * 4; ++i){
			if(i % 4 == 0)
				printf("\n%4d : ", (i / 4) + 1);
			printf("%.2x", bin[i] & 0xff);
		}
	}

	// open stream to destination file
	destination = fopen(argv[2], "w");
	if(destination == NULL){
		printf("ERROR opening %s\n", argv[2]);
		exit(1);
	}

	// write to destination file
	for(i = 0; i < commandcounter * 4; ++i)
		fprintf(destination, "%c", bin[i] & 0xff);
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
	free(bin);
	free(words);

	return 0;
}
