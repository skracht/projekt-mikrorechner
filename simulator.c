#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <stdlib.h>
#define nreg 32 
#define debug 4
#define N & 0xff
#define R(a) reg[ram[(reg[30] * 4) + a] & 0xff]
#define V(a) (ram[(reg[30] * 4) + a] & 0xff)
#define S(a) ram[(reg[30] * 4) + a]
#define C ((ram[(reg[30] * 4) + 2] << 8) + (ram[(reg[30] * 4) + 3] & 0xff))
#define I ((ram[(reg[30] * 4) + 1] << 16) + ((ram[(reg[30] * 4) + 2] & 0xff) << 8) + (ram[(reg[30] * 4) + 3] & 0xff))
#define WORD (((ram[reg[30] * 4] & 0xff) << 24) + ((ram[(reg[30] * 4) + 1] & 0xff) << 16) + ((ram[(reg[30] * 4) + 2] & 0xff) << 8) + (ram[(reg[30] * 4) + 3] & 0xff))
#define pc reg[30]
#define sp reg[31]

const char commands[] = {
	0x20, // mov   20xxyy**
	0x21, // add   21xxyyzz
	0x22, // sub   22xxyyzz
	0x30, // and   30xxyyzz
	0x31, // or    31xxyyzz
	0x32, // xor   32xxyyzz
	0x33, // not   33xx****
	0x43, // lslc  43xx****
	0x44, // lsrc  44xx****
	0x45, // asrc  45xx****
	0x50, // cmpe  50xxyy**
	0x51, // cmpne 51xxyy**
	0x52, // cmpgt 52xxyy**
	0x53, // cmplt 53xxyy**
	0x60, // ldw   60xxyyvv
	0x61, // stw   61xxyyvv
	0x70, // br    70iiiiii
	0x71, // jsr   71iiiiii
	0x72, // bt    72iiiiii
	0x73, // bf    73iiiiii
	0x74, // jmp   74xx****
	0x80, // movi  80xxcccc
	0x81, // addi  81xxcccc
	0x82, // subi  82xxcccc
	0x83, // andi  83xxcccc
	0x84, // bseti 84xxcccc
	0x85, // bclri 85xxcccc
	0x90, // nop   90******
	0xf0  // halt  f0******
};

const char * commandformats[] = {
	"     mov : r%d = r%d",                      "20xxyy**",
	"     add : r%d = r%d + r%d",                "21xxyyzz",
	"     sub : r%d = r%d - r%d",                "22xxyyzz",
	"     and : r%d = r%d & r%d",                "30xxyyzz",
	"      or : r%d = r%d | r%d",                "31xxyyzz",
	"     xor : r%d = r%d ^ r%d",                "32xxyyzz",
	"     not : r%d = ~r%d",                     "33xx****",
	"    lslc : r%d = r%d << 1",                 "43xx****",
	"    lsrc : r%d = r%d >>> 1",                "44xx****",
	"    asrc : r%d = r%d >> 1",                 "45xx****",
	"    cmpe : cmpflag = (r%d == r%d) ? 1 : 0", "50xxyy**",
	"   cmpne : cmpflag = (r%d != r%d) ? 1 : 0", "51xxyy**",
	"   cmpgt : cmpflag = (r%d > r%d) ? 1 : 0",  "52xxyy**",
	"   cmplt : cmpflag = (r%d < r%d) ? 1 : 0",  "53xxyy**",
	"     ldw : r%d = ram[r%d + (%d << 1)]",     "60xxyyvv",
	"     stw : ram[r% + (%d << 1)] = r%d",      "61xxyyvv",
	"      br : PC += %d + 1",                   "70iiiiii",
	"     jsr : TODO",                           "71iiiiii", // TODO
        "      bt : if(%d) PC += %d + 1 else PC++",  "72iiiiii",
	"      bf : if(%d) PC += %d + 1 else PC++",  "73iiiiii",
	"     jmp : PC = r%d + 1",                   "74xx****",
	"    movi : r%d = %d",                       "80xxcccc",
	"    addi : r%d += %d",                      "81xxcccc",
	"    subi : r%d -= %d",                      "82xxcccc",
	"    andi : r%d = r%d & %d",                 "83xxcccc",
	"   bseti : r%d = r%d | (1 << %d)",          "84xxcccc",
	"   bclri : r%d = r%d & ~(1 << %d)",         "85xxcccc",
	"        nop",                               "90******",
	"        halt",                              "f0******"
};

const int commandargn[] = {
	002, // mov   20xxyy**
	003, // add   21xxyyzz
	003, // sub   22xxyyzz
	003, // and   30xxyyzz
	003, // or    31xxyyzz
	003, // xor   32xxyyzz
	001, // not   33xx****
	001, // lslc  43xx****
	001, // lsrc  44xx****
	001, // asrc  45xx****
	012, // cmpe  50xxyy**
	012, // cmpne 51xxyy**
	012, // cmpgt 52xxyy**
	012, // cmplt 53xxyy**
       1021, // ldw   60xxyyvv
       1022, // stw   61xxyyvv
	100, // br    70iiiiii
	100, // jsr   71iiiiii
	110, // bt    72iiiiii
	110, // bf    73iiiiii
	001, // jmp   74xx****
	101, // movi  80xxcccc
	101, // addi  81xxcccc
	101, // subi  82xxcccc
	101, // andi  83xxcccc
	101, // bseti 84xxcccc
	101, // bclri 85xxcccc
	000, // nop   90******
	000  // halt  f0******
};

int main(int args, char** argv){
	FILE * source;
	FILE * destination;

	int filesize_source;
	int ramsize = 1000; // ramsize in bytes
	char * ram;
	int reg[nreg];
	memset(reg, '\0', 4 * nreg);
	const int ncommands = 29;
	int commandcounter;
	int halt;
	int cmpflag;
	int printflag;
	int printnregs;
	int printpc;
	int sbuf[14];
	char * command;

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
	ram = malloc(sizeof(char) * ramsize);

	// scan source file
	for(i = 0; i < filesize_source; ++i){
		fscanf(source, "%c", &ram[i]);
		if(ferror(source)){
			printf("ERROR scaning %s\n", argv[1]);
			exit(1);
		}
	}

	// close stream to source file
	if(fclose(source)){
		printf("ERROR closing %s\n", argv[1]);
		exit(1);
	}	

	// process input
	commandcounter = 0;
	cmpflag = 0;
	pc = 0;
	halt = 0;
	while(!halt){
	  commandcounter++;
	  if(debug == 4){
	    printflag = 0;
	    printnregs = 0;
	    printpc = 0;
	    printf("\ncommand# : %d\n", commandcounter);
	    for(i = 0; i < 29; ++i){
	      if(V(0) == (commands[i]N)){
		switch(commandargn[i]){
		case 002 :
		  printf(commandformats[i * 2], V(1), V(2));
		  printnregs = 2;
		  break;
		case 003 :
		  printf(commandformats[i * 2], V(1), V(2), V(3));
		  printnregs = 3;
		  break;
		case 001 :
		  printf(commandformats[i * 2], V(1), V(1)); // the single argument register is needed twice in output
		  printnregs = 1;
		  break;
		case 012 :
		  printf(commandformats[i * 2], V(1), V(2));
		  printnregs = 2;
		  printflag = 1;
		  break;
		case 1021 :
		  printf(commandformats[i * 2], V(1), V(2), V(3));// TODO
		  break;
		case 1022 :
		  printf(commandformats[i * 2], V(2), V(3), V(1));
		  break;
		case 100 :
		  printf(commandformats[i * 2], I);
		  printpc = 1;
		  break;
		case 110 :
		  printf(commandformats[i * 2], cmpflag, I);
		  printpc = 1;
		  break;
		case 101 :
		  printf(commandformats[i * 2], V(1), C);
		  printnregs = 1;
		  break;
		case 000 :
		  printf(commandformats[i * 2]);
		  break;
		}
		printf("  ||  binary : %8.8x-%s\n", WORD, commandformats[(i * 2) + 1]);
		break;
	      }
	    }
	    for(i = 0; i < printnregs; ++i){
	      sbuf[i] = V(i + 1);
	      sbuf[i+3] = R(i + 1);
	    }
	    if(printflag)
	      sbuf[10] = cmpflag;
	    if(printpc)
	      sbuf[12] = pc;
	  }
	  switch(V(0)){
			case 0x20 : // mov   20xxyy**
				R(1) = R(2);
				break;
			case 0x21 : // add   21xxyyzz
				R(1) = R(2) + R(3);
				break;
			case 0x22 : // sub   22xxyyzz
				R(1) = R(2) - R(3);
				break;
			case 0x30 : // and   30xxyyzz
				R(1) = R(2) & R(3);
				break;
			case 0x31 : // or    31xxyyzz
				R(1) = R(2) | R(3);
				break;
			case 0x32 : // xor   32xxyyzz
				R(1) = R(2) ^ R(3);
				break;
			case 0x33 : // not   33xx****
				R(1) = ~R(1);
				break;
			case 0x43 : // lslc  43xx****
				R(1) = R(1) << 1;
				break;
			case 0x44 : // lsrc  44xx****
				R(1) = (R(1) >> 1) & 0x7fffffff;
				break;
			case 0x45 : // asrc  45xx****
				R(1) = R(1) >> 1;
				break;
			case 0x50 : // cmpe  50xxyy**
				cmpflag = (R(1) == R(2)) ? 1 : 0;
				break;
			case 0x51 : // cmpne 51xxyy**
				cmpflag = (R(1) == R(2)) ? 0 : 1;
				break;
			case 0x52 : // cmpgt 52xxyy**
				cmpflag = (R(1) > R(2)) ? 1 : 0;
				break;
			case 0x53 : // cmplt 53xxyy**
				cmpflag = (R(1) < R(2)) ? 1 : 0;
				break;
			case 0x60 : // ldw   60xxyyvv
			  R(1) = ram[R(2) + (S(3) << 1)]; // TODO S(x) correct in this context?
				break;
			case 0x61 : // stw   61xxyyvv
				ram[R(2) + (S(3) << 1)] = R(1);
				break;
			case 0x70 : // br    70iiiiii
				pc += I;
				break;
			case 0x71 : // jsr   71iiiiii
				// TODO
				break;
			case 0x72 : // bt    72iiiiii
				if(cmpflag)
					pc += I;
				break;
			case 0x73 : // bf    73iiiiii
				if(!cmpflag)
					pc += I;
				break;
			case 0x74 : // jmp   74xx****
				pc = R(1);
				break;
			case 0x80 : // movi  80xxcccc
				R(1) = C;
				break;
			case 0x81 : // addi  81xxcccc
				R(1) += C;
				break;
			case 0x82 : // subi  82xxcccc
				R(1) -= C;
				break;
			case 0x83 : // andi  83xxcccc
				R(1) = R(1) & C;
				break;
			case 0x84 : // bseti 84xxcccc
				R(1) = R(1) | (1 << C);
				break;
			case 0x85 : // bclri 85xxcccc
				R(1) = R(1) & ~(1 << C);
				break;
			case 0x90 : // nop   90******
				break;
			case 0xf0 : // halt  f0******
				halt = 1;
				printf("\n SUCCESS\n\n");
				break;
			default :
				printf("ERROR expected op-code in command %d", commandcounter);
				exit(1);
	  }
	  pc += 1;
	  if(debug == 4){
		  for(i = 0; i < printnregs; ++i)
			  printf("%d %d %d", R(i - 3), i, V(i - 3));//sbuf[i + 6] = R(i - 3); // the pc got incremented before, so R(i + 1) has to be actually R(i  - 3)
		  if(printflag)
			  sbuf[11] = cmpflag;
		  if(printpc)
			  sbuf[13] = pc;
		  for(i = 0; i < printnregs; ++i)
			  printf("     r%2.2d : %8.8x/%8.8d -> %8.8x/%8.8d\n",
					  sbuf[i], sbuf[i + 3], sbuf[i + 3], sbuf[i + 6], sbuf[i + 6]);
		  if(printflag)
			  printf(" cmpflag : %d ---> %d\n", sbuf[10], sbuf[11]);
		  if(printpc)
			  printf("      pc : %8.8x/%8.8d -> %8.8x/%8.8d\n",sbuf[12], sbuf[12], sbuf[13], sbuf[13]);
		  scanf("%c");
	  }
	}

	// print regs
	if(debug == 1 || debug == 3 || debug == 4){
		for(i = 0; i < nreg; ++i){
			if(i % 4 == 0)
				printf("\n");
			printf("|r%2.2d: %8.8x-%6.6d|", 
					(i / 4) + (8 * (i % 4)), 
					reg[(i / 4) + (8 * (i % 4))], 
					reg[(i / 4) + (8 * (i % 4))]);
		}
		printf("\n");
	}

	// print binary
	if(debug == 2 || debug == 3){
		for(i = 0; i < filesize_source; ++i){
			if(i % 4 == 0)
				printf("\n%4d : ", (i / 4) + 1);
			printf("%.2x", ram[i]N);
		}
		printf("\n\n");
	}

	// open stream to destination file
	destination = fopen(argv[2], "w");
	if(destination == NULL){
		printf("ERROR opening %s\n", argv[2]);
		exit(1);
	}

	// write to destination file
	for(i = 0; i < nreg; ++i){
		if(i % 4 == 0)
			fprintf(destination, "\n");
		fprintf(destination, "|r%2.2d: %8.8x-%6.6d|", 
				(i / 4) + (8 * (i % 4)), 
				reg[(i / 4) + (8 * (i % 4))], 
				reg[(i / 4) + (8 * (i % 4))]);
	}
	fprintf(destination, "\n");
	//for(i = 0; i < ramsize; ++i)
	//	fprintf(destination, "%c", ram[i]N);
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
	free(ram);

	return 0;
}
