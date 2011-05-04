/* lib.c
 * generic library code extracted from dawn.c
 * by Mike Kramlich */

typedef unsigned char byte;
typedef byte bool;
typedef signed short int int_var;
typedef unsigned short int u16int;
typedef signed short int s16int;
typedef signed long int s32int;
typedef unsigned long int u32int;
typedef signed long int slon;
typedef unsigned long ulon;

float get_rnd_percent() {
	return ( (float) rand() / (float) RAND_MAX);
}

byte get_rnd_byte_with_max(byte max) {
	return (byte)(get_rnd_percent() * (float)max);
}

bool get_rnd_bool() {
	if (rand() < (RAND_MAX / 2)) return TRUE;
	else return FALSE;
}

u16int get_rnd_u16int_with_notinclusive_max(u16int max) {
	return (u16int)(get_rnd_percent() * (float)max);
}

u32int get_rnd_u32int_with_notinclusive_max(u32int max) {
	return (u32int)(get_rnd_percent() * (float)max);
}

byte get_rnd_byte_with_minmax(byte min, byte max) {
	byte diff = max - min;
	return min + get_rnd_byte_with_max(diff);
}

bool rollforsuccess(u16int numerator, u16int denominator) {
	if (get_rnd_u16int_with_notinclusive_max(denominator) <= (numerator - 1)) return TRUE;
	else return FALSE;
}

bool rollforsuccess32(u32int numerator, u32int denominator) {
	if (get_rnd_u32int_with_notinclusive_max(denominator) <= (numerator - 1)) return TRUE;
	else return FALSE;
}

s16int get_dist(s16int x1, s16int y1, s16int x2, s16int y2) {
	s16int xdist, ydist;
	
	xdist = abs(x2 - x1);
	ydist = abs(y2 - y1);
	if (xdist > ydist) {
		return xdist;
	} else return ydist;
}

s16int get_dist2(s16int x1, s16int y1, s16int x2, s16int y2) {
	s16int xdist, ydist;
	
	xdist = x2 - x1;
	ydist = y2 - y1;
	return (s16int) sqrt((double)xdist*(double)xdist + (double)ydist*(double)ydist);
}

int fgetline(char *line, int max, FILE *stream) {
	if (fgets(line, max, stream) == NULL) return 0;
	else return strlen(line);
}

int getline(char *line, int max) {
	if (fgets(line, max, stdin) == NULL) return 0;
	else return strlen(line);
}

int getlinechomped(char *line, int max) {
	int len = getline(line, max);
	/* if last char is a newline then chomp it... */
	if (len > 0) if (line[len-1] == '\n') line[len-1] = '\0';
	return len-1;
}

int fgetlinechomped(char *line, int max, FILE *stream) {
	int len = fgetline(line, max, stream);
	/* if last char is a newline then chomp it... */
	if (len > 0) if (line[len-1] == '\n') line[len-1] = '\0';
	return len-1;
}

bool streql(const char *src, const char *tar) {
	if ((strlen(src) == strlen(tar)) && !strncmp(src,tar,strlen(tar))) {
		return TRUE;
	} else return FALSE;
}

bool strbegin(const char *haystack, const char *needle) {
	char *index;
	
	index = strstr(haystack, needle);
	if (index == NULL) return FALSE;
	else if (index == haystack) return TRUE;
	else return FALSE;
}

int get_nth_token_as_str(char *str, int token_index, char *retval) {
    char *token;
    int i;
    char mystr[MAX_SAFE_STRLEN+1];

    strncpy(mystr, str, MAX_SAFE_STRLEN);
    for (i = 0 ; i <= token_index ; i++) {
        token = strtok(mystr," ");
        if (token == NULL) {
            /* error/failure */
            strcpy(retval,"");
            return;
        }
    }
    /* token now points at 'token_index'-th token in str */
	strncpy(retval, token, MAX_SAFE_STRLEN);
}

int get_nth_token_as_int(char *str, int token_index) {
    char *token;
    int i;
    char mystr[MAX_SAFE_STRLEN+1];

    strncpy(mystr, str, MAX_SAFE_STRLEN);
    for (i = 0 ; i <= token_index ; i++) {
        token = strtok(mystr," ");
        if (token == NULL) return -1; /* error/failure */
    }
    /* token now points at 'token_index'-th token in str */
    return atoi(token);
}

