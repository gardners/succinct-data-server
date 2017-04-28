/*
  Parse JSON message list from Text Magic.

*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h>

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
#define DONE 12

int flags=0;
char sender[8192];
char receiver[8192];
char text[8192];
char id[8192];

int report_pair(int depth,char *key,char *value)
{
  if (!strcasecmp("sender",key)) { strcpy(sender,value); flags|=1; }
  if (!strcasecmp("receiver",key)) { strcpy(receiver,value); flags|=2; }
  if (!strcasecmp("text",key)) { strcpy(text,value); flags|=4; }
  if (!strcasecmp("id",key)) { strcpy(id,value); flags|=8; }
  if (flags==15) {
    flags=0;
    // fprintf(stderr,"[%s] -> [%s] : [%s]\n",sender,receiver,text);
    if (strstr(text," http://inr.ch/")) {
      // inReach message
      // Inreach chops messages into pieces so that they can put their
      // self-promotion link in.
      // We then need to access this URL and extract the complete text from there
      char cmd[1024];
      char *s=strstr(text," http://inr.ch/");
      s++;
      char *e=s;
      // fprintf(stderr,"e='%s'\n",e);
      while(*e!=' ') e++;
      *e=0;
      unlink("/tmp/fetch.tmp");
      snprintf(cmd,1024,"wget -O /tmp/fetch.tmp -q %s",s);
      // fprintf(stderr,"%s\n",cmd);
      system(cmd);
      FILE *f=fopen("/tmp/fetch.tmp","r");
      char *signature=",\"X\":\"";
      int ofs=0;
      if (f) {
	// Search for ',"X":"', and scrape actual text from immediately after
	int text_len=0;
	while(!feof(f)) {
	  int c=fgetc(f);
	  if (signature[ofs]) {
	    if (c==signature[ofs]) ofs++;
	    else if (c==signature[0]) ofs=1;
	    else ofs=0;
	  } else {
	    if (c=='\"') break;
	    else if (text_len<8000) { text[text_len++]=c; text[text_len]=0; }
	  }
	}
	fclose(f);
	// fprintf(stderr,"Extracted text from inr.ch: [%s]\n",text);
      }      
    }
    fprintf(stdout,"%s:%s:%s:%s\n",id,sender,receiver,text);
  }
  return 0;
}

int main(int argc,char **argv)
{
  char key[8192];
  int key_len=0;
  char value[8192];
  int value_len=0;
  int state=IDLE;

  int array_depth=0;
  
  FILE *f=stdin;
  if (argc>1) {
    f=fopen(argv[1],"r");
    if (!f) {
      fprintf(stderr,"Could not open '%s' for reading\n",argv[1]);
      exit(-1);
    }
  }

  int line=1;
  int col=-1;

  int start_line=1, start_col=-1;
  
  while(!feof(f)) {
    int c=fgetc(f);
    while (c=='\n') { col=-1; line++; c=fgetc(f); }
    col++;

    switch(state) {
    case IDLE:
      if (c!='{') {
	fprintf(stderr,"%s:%d:%d: Expected {, saw '%c'\n",
		argc>1?argv[1]:"stdin",line,col,
		c);
	exit(-1);
      } else {
	array_depth++;
	state=KEY_START; key_len=0; key[0]=0;
      }
      break;
    case KEY_START:
      start_line=line; start_col=col;
      key_len=0; value_len=0;
      if (c!='\"') {
	fprintf(stderr,"%s:%d:%d: Expected \" at start of key name, saw '%c'\n",
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
      value[0]=0; value_len=0;
      switch (c) {
      case '[':
	// Array
	array_depth++;
	state=IDLE;
	break;
      case '0': case '1': case '2': case '3': case '4': case '5':
      case '6': case '7': case '8': case '9':
	value[0]=c; value_len=1;
	state=VAL_NUMERIC;
	break;
      case '"':
	state=VAL;
	break;
      default:
	if (c!='\"') {
	  fprintf(stderr,"%s:%d:%d: Expected numeric value or \" at start of value identifier, saw '%c'\n",
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
      case ',':
	report_pair(array_depth,key,value);
	state=KEY_START;
	key_len=0; key[0]=0;
	break; 
      default:
	fprintf(stderr,"%s:%d:%d: Expected , after numeric value, saw '%c'\n",
		argc>1?argv[1]:"stdin",line,col,
		c);
	exit(-1);
      }
    case VAL:
      if (c=='\\') state=VAL_SLASH;
      else if (c=='\"') {
	report_pair(array_depth,key,value);
	state=VAL_END;
      } else {
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
      switch(c) {
      case '\"': case '/':
	if (value_len<8000) { value[value_len++]=c; value[value_len]=0; }
	state=VAL;
	break;
      default:
	fprintf(stderr,"%s:%d:%d: Expected \" after \\, saw '%c'\n",
		argc>1?argv[1]:"stdin",line,col,
		c);
	exit(-1);
      }
      break;
    case VAL_END:
      switch(c) {
      case -1:
	state=DONE;
	break;
      case ',':
	state=PAIR_END; break;
      case '}': case ']':
	if (array_depth>0) {
	  array_depth--; state=VAL_END;
	}
	else {
	  fprintf(stderr,"%s:%d:%d: Unmatched } after \"\n",
		  argc>1?argv[1]:"stdin",line,col);
	  exit(-1);
	}
	break;
      default:	
	fprintf(stderr,"%s:%d:%d: Expected , after closing \", saw '%c'\n",
		argc>1?argv[1]:"stdin",line,col,
		c);
	exit(-1);
      }
      break;
    case PAIR_END:
      switch(c) {
      case '\"': state=KEY; key_len=0; key[0]=0; break;
      case '{':
	array_depth++;
	break;
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
