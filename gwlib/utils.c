/*
 * utils.c - generally useful, non-application specific functions for Gateway
 *
 */

#include <ctype.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <termios.h>
#include <locale.h>

#include "gwlib.h"


/*
 * new datatype functions
 */



MultibyteInt get_variable_value(Octet *source, int *len)
{
    MultibyteInt retval = 0;
    
    for(*len=1;; (*len)++, source++) {
	retval = retval * 0x80 + (*source & 0x7F);
	if (*source < 0x80)	/* if the continue-bit (high bit) is not set */
	    break;
    }
    return retval;
}


int write_variable_value(MultibyteInt value, Octet *dest)
{
    int i, loc = 0;
    Octet revbuffer[20];	/* we write it backwards */
    
    for (;;) {
	revbuffer[loc++] = (value & 0x7F) + 0x80;	
	if (value >= 0x80)
	    value = value >> 7;
	else
	    break;
    }
    for(i=0; i < loc; i++)		/* reverse the buffer */
	dest[i] = revbuffer[loc-i-1];
    
    dest[loc-1] &= 0x7F;	/* remove trailer-bit from last */

    return loc;
}

Octet reverse_octet(Octet source)
{
    Octet	dest;
    dest = (source & 1) <<7;
    dest += (source & 2) <<5;
    dest += (source & 4) <<3;
    dest += (source & 8) <<1;
    dest += (source & 16) >>1;
    dest += (source & 32) >>3;
    dest += (source & 64) >>5;
    dest += (source & 128) >>7;
    
    return dest;
}



int get_and_set_debugs(int argc, char **argv,
		       int (*find_own) (int index, int argc, char **argv))
{
    int i, ret = -1;
    int debug_lvl = -1;
    int file_lvl = DEBUG;
    char *log_file = NULL;
    char *debug_places = NULL;
    
    for(i=1; i < argc; i++) {
	if (strcmp(argv[i],"-v")==0 ||
	    strcmp(argv[i],"--verbosity")==0) {

	    if (i+1 < argc) {
		debug_lvl = atoi(argv[i+1]);
		i++;
	    } else
		fprintf(stderr, "Missing argument for option %s\n", argv[i]); 
	} else if (strcmp(argv[i],"-F")==0 ||
		   strcmp(argv[i],"--logfile")==0) {
	    if (i+1 < argc && *(argv[i+1]) != '-') {
		log_file = argv[i+1];
		i++;
	    } else
		fprintf(stderr, "Missing argument for option %s\n", argv[i]); 
	} else if (strcmp(argv[i],"-V")==0 ||
		   strcmp(argv[i],"--fileverbosity")==0) {
	    if (i+1 < argc) {
		file_lvl = atoi(argv[i+1]);
		i++;
	    } else
		fprintf(stderr, "Missing argument for option %s\n", argv[i]); 
	} else if (strcmp(argv[i],"-D")==0 ||
		   strcmp(argv[i],"--debug")==0) {
	    if (i+1 < argc) {
		debug_places = argv[i+1];
		i++;
	    } else
		fprintf(stderr, "Missing argument for option %s\n", argv[i]); 
	} else if(*argv[i] != '-')
	    break;
	else {
	    if (find_own != NULL) {
		ret = find_own(i, argc, argv);
	    }
	    if (ret < 0) {
		fprintf(stderr, "Unknown option %s, exiting.\n", argv[i]);
		panic(0, "Option paring failed");
	    }
	    else
		i += ret;	/* advance additional args */
	}
    }
    if (debug_lvl > -1)
	set_output_level(debug_lvl);
    if (debug_places != NULL)
        set_debug_places(debug_places);
    if (log_file != NULL)
	open_logfile(log_file, file_lvl);

    info(0, "Debug_lvl = %d, log_file = %s, log_lvl = %d",
	  debug_lvl, log_file ? log_file : "<none>", file_lvl);
    if (debug_places != NULL)
	    info(0, "Debug places: `%s'", debug_places);
    
    return i;
}

void print_std_args_usage(FILE *stream)
{
    fprintf(stream,
	   " -v <level>     set stderr output level. 0 = DEBUG, 4 = PANIC\n"
	   " -F <logfile>   set logfile name\n"
	   " -V <level>     set logfile output level. Defaults to DEBUG\n"
	   " -D <places>    set DEBUG places, like \"wap.* -wap.wsp.*\"\n"
	   " --verbosity, --logfile, --fileverbosity   aliased arguments\n");
}


int check_ip(char *accept_string, char *ip, char *match_buffer)
{
    char *p, *t, *start;

    t = accept_string;
    
    while(1) {
	for(p = ip, start = t;;p++, t++) {
	    if ((*t == ';' || *t == '\0') && *p == '\0')
		goto found;

	    if (*t == '*') {
		t++;
		while(*p != '.' && *p != ';' && *p != '\0')
		    p++;

		if (*p == '\0')
		    goto found;
		continue;
	    }
	    if (*p == '\0' || *t == '\0' || *t != *p)
		break;		/* not matching */

	}
	for(; *t != ';'; t++)		/* seek next IP */
	    if (*t == '\0')
		goto failed;
	t++;
    }
failed:    
    debug("gwlib", 0, "Could not find match for <%s> in <%s>", ip, accept_string);
    return 0;
found:
    if (match_buffer != NULL) {
	for(p=match_buffer; *start != '\0' && *start != ';'; p++, start++)
	    *p = *start;
	*p = '\0';
	debug("gwlib", 0, "Found and copied match <%s>", match_buffer);
    }
    return 1;
}


int is_allowed_ip(char *allow_ip, char *deny_ip, Octstr *ip)
{
    if (ip == NULL)
	return -1;

    if (deny_ip == NULL || *deny_ip == '\0')
	return 1;

    if (allow_ip != NULL && check_ip(allow_ip, octstr_get_cstr(ip), NULL) == 1)
	return 1;

    if (check_ip(deny_ip, octstr_get_cstr(ip), NULL) == 1)
	return 0;

    return 1;
}


int normalize_number(char *dial_prefixes, Octstr **number)
{
    char *t, *p, *official, *start;
    int len, official_len;
    
    if (dial_prefixes == NULL || dial_prefixes[0] == '\0')
        return 0;

    t = official = dial_prefixes;
    official_len = 0;

    gw_assert(number != NULL);
    
    while(1) {

        for(p = octstr_get_cstr(*number), start = t, len = 0; ; t++, p++, len++)
	{
            if (*t == ',' || *t == ';' || *t == '\0') {
                if (start != official) {
                    Octstr *nstr;
		    long n;
		    
		    n = official_len;
		    if (strlen(official) < n)
		    	n = strlen(official);
                    nstr = octstr_create_from_data(official, n);
                    octstr_insert_data(nstr, official_len,
                                           octstr_get_cstr(*number) + len,
                                           octstr_len(*number) - len);
                    octstr_destroy(*number);
                    *number = nstr;
                }
                return 1;
            }
            if (*p == '\0' || *t != *p)
                break;          /* not matching */
        }
        for(; *t != ',' && *t != ';' && *t != '\0'; t++, len++)
            ;
        if (*t == '\0') break;
        if (start == official) official_len = len;
        if (*t == ';') official = t+1;
        t++;
    }
    return 0;
}





long decode_network_long(unsigned char *data) {
        return (data[0] << 24) | (data[1] << 16) | (data[2] << 8) | data[3];
}


void encode_network_long(unsigned char *data, unsigned long value) {
        data[0] = (value >> 24) & 0xff;
        data[1] = (value >> 16) & 0xff;
        data[2] = (value >> 8) & 0xff;
        data[3] = value & 0xff;
}

/* Something that does the same as GNU cfmakeraw. We don't use cfmakeraw
   so that we always know what it does, and also to reduce configure.in
   complexity. */

void kannel_cfmakeraw (struct termios *tio){
    /* Block until a charactor is available, but it only needs to be one*/
    tio->c_cc[VMIN]    = 1;
    tio->c_cc[VTIME]   = 0;

    /* GNU cfmakeraw sets these flags so we had better too...*/

    /* Control modes,*/
    tio->c_cflag      &= ~(CSIZE|PARENB); /* Allow parity bits and size*/
    tio->c_cflag      |= CS8;             /* Wow, eight bit bytes!!! */

    /* Input Flags,*/

    /* Break on SIGINT, the FSF set both ICRNL (translate CR -> NL) and 
     * IGNCR (which cancels out  ICRNL) - leave this in case something 
     * barfs, INLCR - translate NL to CR,IXON - enable Xon/Xoff, ISTRIP - 
     * strip the eighth bit (do we really want to be doing this?), IGNBRK - 
     * ignore break condition.*/
    tio->c_iflag      &= ~(BRKINT|ICRNL|IGNCR|IGNBRK|INLCR|IXON|ISTRIP|IGNBRK);

    /* Other flags,*/

    /* ECHO - echo input chars, ECHONL - always echo NL, even if ECHO is off
     * ICANON - enable stuff like KILL, EOF, EOL etc in the stream, IEXTEN - 
     * enable implementation dependant stuff, ISIG - generate signals in 
     * response to chars like INTR, SUSP etc.*/
    tio->c_lflag      &= ~(ECHO|ECHONL|ICANON|IEXTEN|ISIG);

    /* Output flags,*/
    /* Enable Implementation defined stuff on the output stream*/
    tio->c_oflag      &= ~OPOST;
}

Octstr *rfc2068_date_format(unsigned long unixtime) {
	struct tm tm;
	unsigned char buffer[30];
	size_t len;
	Octstr *oldlocale;

	/* Make sure strftime behaves */
	oldlocale = gw_getlocale(LC_TIME);
	setlocale(LC_TIME, "C");

	tm = gw_gmtime((time_t) unixtime);
	len = strftime(buffer, sizeof(buffer),
		"%a, %d %b %Y %H:%M:%S GMT", &tm);
	if (len == 0) {
		warning(0, "strftime failed for %lu.", unixtime);
		return NULL;
	}

	if (oldlocale)
		setlocale(LC_TIME, octstr_get_cstr(oldlocale));
	octstr_destroy(oldlocale);

	return octstr_create(buffer);
}
