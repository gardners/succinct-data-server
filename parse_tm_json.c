/*
  Parse JSON message list from Text Magic.

*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define IDLE 1
#define KEY 8
#define KEY_START 2
#define KEY_SLASH 3
#define KEY_END 4
#define VAL 9
#define VAL_START 5
#define VAL_SLASH 6
#define VAL_END 7
#define PAIR_END 10
#define VAL_NUMERIC 11

int main(int argc,char **argv)
{
  char key[8192];
  int key_len=0;
  char value[8192];
  int value_len=0;
  int state=IDLE;
  
  FILE *f=stdin;
  if (argc>1) {
    f=fopen(argv[1],"r");
    if (!f) {
      fprintf(stderr,"Could not open '%s' for reading\n",argv[1]);
      exit(-1);
    }
  }

  int line=1;
  int col=1;

  int start_line=1, start_col=1;
  
  while(!feof(f)) {
    int c=fgetc(f);
    if (c=='\n') { col=1; line++;} else col++;

    switch(state) {
    case IDLE:
      if (c!='{') {
	fprintf(stderr,"%s:%d:%d: Expected {, saw '%c'\n",
		argc>1?argv[1]:"stdin",line,col,
		c);
	exit(-1);
      } else state=KEY_START;
      break;
    case KEY_START:
      start_line=line; start_col=col;
      if (c!='\"') {
	fprintf(stderr,"%s:%d:%d: Expected \", saw '%c'\n",
		argc>1?argv[1]:"stdin",line,col,
		c);
	exit(-1);
      } else state=KEY;
      break;
    case KEY:
      if (c=='\\') state=KEY_SLASH;
      else if (c=='\"') state=KEY_END;
      else {
	if (key_len<8000) { key[key_len++]=c; key[key_len]=0; }
	else {
	  fprintf(stderr,"%s:%d:%d: Run-away key field (began at %s:%d:%d)\n",
		  argc>1?argv[1]:"stdin",line,col,
		  argc>1?argv[1]:"stdin",start_line,start_col);
	  exit(-1);
	}
      }
      break;
    case KEY_SLASH:
      if (c=='\"') {
	if (key_len<8000) { key[key_len++]=c; key[key_len]=0; }
	state=KEY;
      } else {
	fprintf(stderr,"%s:%d:%d: Expected \" after \\, saw '%c'\n",
		argc>1?argv[1]:"stdin",line,col,
		c);
	exit(-1);
      }
      break;
    case KEY_END:
      if (c!=':') {
	fprintf(stderr,"%s:%d:%d: Expected : after \", saw '%c'\n",
		argc>1?argv[1]:"stdin",line,col,
		c);
	exit(-1);
      }
      state=VAL_START;
      break;
    case VAL_START:
      start_line=line; start_col=col;
      switch (c) {
      case '0': case '1': case '2': case '3': case '4': case '5':
      case '6': case '7': case '8': case '9':
	value[0]=c; value_len=1;
	state=VAL_NUMERIC;
	break;
      case '"': state=VAL; break;
      default:
	if (c!='\"') {
	  fprintf(stderr,"%s:%d:%d: Expected \", saw '%c'\n",
		  argc>1?argv[1]:"stdin",line,col,
		  c);
	  exit(-1);
	}
	break;
      }
      break;
    case VAL_NUMERIC:      
      switch (c) {
      case '0': case '1': case '2': case '3': case '4': case '5':
      case '6': case '7': case '8': case '9':
	if (value_len<8000) { value[value_len++]=c; value_len++; }
	else {
	  fprintf(stderr,"%s:%d:%d: Run-away value field (began at %s:%d:%d)\n",
		  argc>1?argv[1]:"stdin",line,col,
		  argc>1?argv[1]:"stdin",start_line,start_col);
	  exit(-1);
	}
	break;
      case ',': state=KEY_START; break; 
      default:
	fprintf(stderr,"%s:%d:%d: Expected , after numeric value, saw '%c'\n",
		argc>1?argv[1]:"stdin",line,col,
		c);
	exit(-1);
      }
    case VAL:
      if (c=='\\') state=VAL_SLASH;
      else if (c=='\"') state=VAL_END;
      else {
	if (value_len<8000) { value[value_len++]=c; value[value_len]=0; }
	else {
	  fprintf(stderr,"%s:%d:%d: Run-away value field (began at %s:%d:%d)\n",
		  argc>1?argv[1]:"stdin",line,col,
		  argc>1?argv[1]:"stdin",start_line,start_col);
	  exit(-1);
	}
      }
      break;
    case VAL_SLASH:
      if (c=='\"') {
	if (value_len<8000) { value[value_len++]=c; value[value_len]=0; }
	state=VAL;
      } else {
	fprintf(stderr,"%s:%d:%d: Expected \" after \\, saw '%c'\n",
		argc>1?argv[1]:"stdin",line,col,
		c);
	exit(-1);
      }
      break;
    case VAL_END:
      if (c!=',') {
	fprintf(stderr,"%s:%d:%d: Expected , after closing \", saw '%c'\n",
		argc>1?argv[1]:"stdin",line,col,
		c);
	exit(-1);
      }
      state=PAIR_END;
      break;
    case PAIR_END:
      switch(c) {
      case '\"': state=KEY_START; break;
      default:
	fprintf(stderr,"%s:%d:%d: Expected new value/key pair or parentheses of some sort, saw '%c'\n",
		argc>1?argv[1]:"stdin",line,col,
		c);
	exit(-1);
      }
      break;
    }
  }
  
}
