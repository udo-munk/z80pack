#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>

void help(char *name)
{
  printf("%s - BINARY to Intel HEX file convertor version 1.00\n"\
         "(c)BCL Vysoke Myto 2001 (benedikt@lphard.cz)\n\n",name);
  printf("Usage: %s [-option] binfile hexfile\n"\
	 " -l  Bytes to read from binary file\n"\
	 " -i  Binary file starting offset\n"\
	 " -o  Output file offset (where HEX data starts)\n"\
	 " -t  Exclude EOF record\n"\
	 " -a  Append to end of existing HEX file\n"\
	 " -q  Quiet mode (no statistics are printed)\n", name);
}

int main(int argc,char *argv[])/*Main routine*/
{
  char *ifile = NULL;
  char *ofile = NULL;
  char c;
  FILE *inp, *outp;
  int ch,csum;
  int ofsa = 0;
  int cnt = 0;
  struct stat statbuf;
  long int foffset = 0;
  long int fsize = 0;
  long int fsub;
  long int fpoint = 0;
  long int adrs = 0;
  unsigned char quiet = 0;
  unsigned char eofrec = 0;
  unsigned char append = 0;

  opterr = 0; //print error message if unknown option

  while ((c = getopt (argc, argv, "l:i:o:taqv")) != -1)
  switch (c) {
    case 'l':
      fsize = atol(optarg);
      break;
    case 'i':
      foffset = atol(optarg);
      break;
    case 'o':
      adrs = atol(optarg);
      break;
    case 't':
      eofrec = 1;
      break;
    case 'a':
      append = 1;
      break;
    case 'q':
      quiet = 1;
      break;
    case 'v':
      printf("%s - BINARY to Intel HEX file convertor version 1.00\n"\
             "(c)BCL Vysoke Myto 2001 (benedikt@lphard.cz)\n",argv[0]);
      return 0;
    case '?':
      help (argv[0]);
      return 1;
  }

  if ((argc - optind) != 2) {
    printf("ERROR: Missing input/output file.\n");
    help(argv[0]);
    return 1;
  }
  ifile = argv[optind];
  ofile = argv[optind+1];
    
  /*Open file check*/
  if((inp = fopen(ifile, "rb")) == NULL){
    printf("ERROR: Cannot open input file.\n");
    return 1;
  }
  fseek (inp, foffset, SEEK_SET);
  
  if (append == 0) {
    if((outp = fopen(ofile, "wt")) == NULL){
      printf("ERROR: Cannot open output file.\n");
      return 1;
    }
  } else {
    if((outp = fopen(ofile, "at")) == NULL){
      printf("ERROR: Cannot re-open output file.\n");
      return 1;
    }
    fseek (outp, 0, SEEK_END);
  }

  fstat(fileno(inp), &statbuf);
  if (quiet == 0) printf("Input file size=%ld\n",(long)statbuf.st_size);
  if (foffset > statbuf.st_size) {
    printf("ERROR: Input offset > input file length\n");
  }
  if ((fsize == 0) || (fsize > (statbuf.st_size - foffset)))
    fsize = statbuf.st_size - foffset;

//  fprintf(outp,":020000020000FC\n");/*Start Header*/
  fsub = fsize - fpoint;
  if (fsub > 0x20) {
  	fprintf(outp,":20%04X00",(unsigned int) adrs); /*Hex line Header*/
    csum = 0x20 + (adrs>>8) + (adrs & 0xFF);
    adrs += 0x20;
  }
  else {
  	fprintf(outp, ":%02X%04X00", (unsigned int) fsub, (unsigned int) adrs);/*Hex line Header*/
    csum = fsub + (adrs>>8) + (adrs & 0xFF);
    adrs += fsub;
  }
  while (fsub > 0){
    ch = fgetc(inp);
    fprintf(outp,"%02X",ch); /*Put data*/
    cnt++; fpoint++;
    fsub = fsize - fpoint;
    csum = ch + csum;
    if((fsub == 0)||(cnt == 0x20)){
      cnt = 0; csum = 0xFF & (~csum + 1);
      fprintf(outp,"%02X\n",csum); /*Put checksum*/
      if(fsub == 0) break;
      if(adrs > 0xFFFF){
		ofsa = 0x1000 + ofsa;
		adrs = 0;
		fprintf(outp,":02000002%04X",ofsa); /*Change offset address*/
		csum = 0x02 + 0x02 + (ofsa>>8) + (ofsa & 0xFF);
		csum = 0xFF & (~csum + 1);
        fprintf(outp,"%02X\n", csum);
      }
      adrs = 0xFFFF & adrs;
	  if (fsub > 0x20) {
  		fprintf(outp,":20%04X00", (unsigned int) adrs); /*Next Hex line Header*/
    	csum = 0x20 + (adrs>>8) + (adrs & 0xFF);
        adrs += 0x20;
      }
      else {
      	if(fsub > 0){
  			fprintf(outp, ":%02X%04X00", (unsigned int) fsub, (unsigned int) adrs); /*Next Hex line Header*/
    		csum = fsub + (adrs>>8) + (adrs & 0xFF);
        	adrs += fsub;
        }
      }
    }
  }
  if (eofrec == 0) fprintf(outp,":00000001FF\n"); /*End footer*/
  fflush (outp);

  fstat(fileno(outp), &statbuf);
  if (quiet == 0) printf("Output file size=%ld\n",(long)statbuf.st_size);

  fclose(inp);
  fclose(outp);
  return 0;
}
