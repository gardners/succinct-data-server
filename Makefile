COPT=-g -Wall

parse_tm_json:	parse_tm_json.c Makefile
	$(CC) $(COPT) -o parse_tm_json parse_tm_json.c $(LOPT)
