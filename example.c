#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include "linenoise.h"
#define MIN(a,b) (((a)<(b))?(a):(b))

void completion(const char *buf, linenoiseCompletions *lc) {
    if (!strncmp(buf, "ciao", strlen(buf)))
        linenoiseAddCompletion(lc,"ciao");
    if (!strncmp(buf, "hello there", strlen(buf)))
        linenoiseAddCompletion(lc,"hello there");
    if (!strncmp(buf, "hello", strlen(buf)))
        linenoiseAddCompletion(lc,"hello");
    
}

char *hints(const char *buf, int *color, int *bold) {
    if (!strcasecmp(buf,"hello")) {
        *color = 35;
        *bold = 0;
        return " World\r\nMundo\r\nWelt\r\nМир\r\n";
    }
    return NULL;
}

int main(int argc, char **argv) {
    char *line;
    char *prgname = argv[0];
    int async = 0;

    while(argc > 1) {
        argc--;
        argv++;
        if (!strcmp(*argv,"--async")) {
            async = 1;
        } else {
            fprintf(stderr, "Usage: %s [--async]\n", prgname);
            exit(1);
        }
    }

    /* Set the completion callback. This will be called every time the
     * user uses the <tab> key. */
    linenoiseSetCompletionCallback(completion);
    linenoiseSetHintsCallback(hints);

    /* Load history from file. The history file is just a plain text file
     * where entries are separated by newlines. */
    linenoiseHistoryLoad("history.txt"); /* Load the history at startup */

    /* Now this is the main loop of the typical linenoise-based application.
     * The call to linenoise() will block as long as the user types something
     * and presses enter.
     *
     * The typed string is returned as a malloc() allocated string by
     * linenoise, so the user needs to free() it. */

    while(1) {
        if (!async) {
            line = linenoise("hello> ");
            if (line == NULL) break;
        } else {
            /* Asynchronous mode using the multiplexing API: wait for
             * data on stdin, and simulate async data coming from some source
             * using the select(2) timeout. */
            struct linenoiseState ls;
            char buf[1024];
            linenoiseEditStart(&ls,-1,-1,buf,sizeof(buf),"hello> ");
            while(1) {
		fd_set readfds;
		struct timeval tv;
		int retval;

		FD_ZERO(&readfds);
		FD_SET(ls.ifd, &readfds);
		tv.tv_sec = 1; // 1 sec timeout
		tv.tv_usec = 0;

		retval = select(ls.ifd+1, &readfds, NULL, NULL, &tv);
		if (retval == -1) {
		    perror("select()");
                    exit(1);
		} else if (retval) {
		    line = linenoiseEditFeed(&ls);
                    /* A NULL return means: line editing is continuing.
                     * Otherwise the user hit enter or stopped editing
                     * (CTRL+C/D). */
                    if (line != linenoiseEditMore) break;
		} else {
		    // Timeout occurred
                    static int counter = 0;
                    linenoiseHide(&ls);
		    printf("Async output %d.\n", counter++);
                    linenoiseShow(&ls);
		}
            }
            linenoiseEditStop(&ls);
            if (line == NULL) exit(0); /* Ctrl+D/C. */
        }

        /* Do something with the string. */
        if (line[0] != '\0' && line[0] != '/') {
            printf("echo: '%s'\n", line);
            linenoiseHistoryAdd(line); /* Add to the history. */
            linenoiseHistorySave("history.txt"); /* Save the history on disk. */
        } else if (!strncmp(line,"/historylen",11)) {
            /* The "/historylen" command will change the history len. */
            int len = atoi(line+11);
            linenoiseHistorySetMaxLen(len);
        } else if (line[0] == '/') {
            printf("Unreconized command: %s\n", line);
        }
        free(line);
    }
    return 0;
}
