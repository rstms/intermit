#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

int main(int argc, char *argv[])
{
    int pipec2p[2];
    int pipep2c[2];
    pid_t cpid;
    char buf;
    int i;
    int len;

    fprintf(stderr, "intermit testing mode inserting random pauses in child program's output\n");

    int delay=1000;
    int percent=0;
    int seed=time(NULL);
    int verbose=0;

    if (argc < 1) {
	fprintf(stderr, "Usage: %s <command>\n", argv[0]);
	exit(EXIT_FAILURE);
    }

    while(argv[1][0] == '-') {
      switch(argv[1][1]) {
        case 's':
	    seed = atoi(argv[1]+2);
	    srandom(seed);
	    break;
        case 'p':
	    percent = atoi(argv[1]+2);
	    break;
	case 'd':
	    delay  = atoi(argv[1]+2);
	    break;
	case 'v':
	    verbose = 1;
      }
      argv++;
    }

    fprintf(stderr, "seed: %d\n", seed);
    fprintf(stderr, "percent: %d\n", percent);
    fprintf(stderr, "delay: %d\n", delay);
    fprintf(stderr, "verbose: %d\n", verbose);

    char **args;
    if (!(args = calloc(sizeof(char *), argc + 1))) {
	perror("calloc");
	exit(EXIT_FAILURE);
    }
    for (i = 0; i < argc - 1; i++) {
	args[i] = argv[i + 1];
    }

    /*
       for (i = 0; i <= argc-1; i++) {
       fprintf(stderr, "args[%d]=%p %s\n", i, args[i], args[i]);
       }
     */

    if (pipe(pipep2c) == -1) {
	perror("pipep2c");
	exit(EXIT_FAILURE);
    }

    if (pipe(pipec2p) == -1) {
	perror("pipec2p");
	exit(EXIT_FAILURE);
    }

    cpid = fork();
    if (cpid == -1) {
	perror("fork");
	exit(EXIT_FAILURE);
    }

    if (cpid == 0) {		/* Child reads from p2c, writes to c2p */
	// child
	if(close(pipep2c[1])==-1){	/* Close unused write end of p2c pipe */
	    perror("close");
	    exit(EXIT_FAILURE);
	}

	if(close(pipec2p[0])==-1){/* Close unused read end of c2p pipe */
	    perror("close");
	    exit(EXIT_FAILURE);
	}

	if (dup2(pipep2c[0], STDIN_FILENO) == -1) {
	    perror("dup2");
	    exit(EXIT_FAILURE);
	}
	
	if(close(pipep2c[0]) == -1) {
	    perror("close");
	    exit(EXIT_FAILURE);
	}

	if (dup2(pipec2p[1], STDOUT_FILENO) == -1) {
	    perror("dup2");
	    exit(EXIT_FAILURE);
	}

	if(close(pipec2p[1]) == -1) {
	    perror("close");
	    exit(EXIT_FAILURE);
	}

	if (execvp(args[0], args) == -1) {
	    perror("execvp");
	    exit(EXIT_FAILURE);
	}
    } else {			/* Parent writes to p2c, reads from c2p */
	// parent
		/* Close unused read end of p2c pipe */
	if(close(pipep2c[0])==-1){
	    perror("close");
	    exit(EXIT_FAILURE);
	}
		/* Close unused write end of c2p pipe */
	if(close(pipec2p[1])==-1){
	    perror("close");
	    exit(EXIT_FAILURE);
	}

	fd_set rfds, efds;
	int retval;

	int stdin_active = 1;
	int child_active = 1;

	char buf[8192];
	int len;
	int nfds = pipec2p[0] + 1;

	while (stdin_active || child_active) {
	    /* Watch stdin (fd 0) and c2p pipe for input. */
	    FD_ZERO(&rfds);
	    if(stdin_active)
	        FD_SET(STDIN_FILENO, &rfds);
	    if(child_active)
	        FD_SET(pipec2p[0], &rfds);

	    /* Watch stdin (fd 0) and c2p pipe for errors. */
	    FD_ZERO(&efds);
	    if(stdin_active)
	        FD_SET(STDIN_FILENO, &efds);
	    if(child_active)
	        FD_SET(pipec2p[0], &efds);

	    retval = select(nfds, &rfds, NULL, &efds, NULL);
	    /* Don't rely on the value of tv now! */
	    if (retval == -1) {
		perror("select");
		exit(EXIT_FAILURE);
	    } else if (retval != 0) {
		if (FD_ISSET(STDIN_FILENO, &rfds)) {
		    // read stdin, write to child
		    if ((len = read(STDIN_FILENO, buf, sizeof(buf))) == -1) {
			perror("stdin read");
			exit(EXIT_FAILURE);
		    } else if (len > 0) {
			if (write(pipep2c[1], buf, len) == -1) {
			    perror("write pipep2c");
			    exit(EXIT_FAILURE);
			}
		    } else if (len == 0) {
			stdin_active = 0;
			if(close(pipep2c[1])==-1) {
			    perror("close");
			    exit(EXIT_FAILURE);
			}
		    }
		}
	        if (FD_ISSET(pipec2p[0], &rfds)) {
		    // read c2p, write to stdout
		    if ((len = read(pipec2p[0], buf, sizeof(buf))) == -1) {
			perror("pipec2p read");
			exit(EXIT_FAILURE);
		    } else if (len > 0) {
			if (random() % 100 <= percent) {
			    if(verbose)
			        fprintf(stderr, "pause\n");
			    usleep(delay * 1000);
			    if(verbose)
			        fprintf(stderr, "resume\n");
			}
			if (write(STDOUT_FILENO, buf, len) == -1) {
			    perror("write pipep2c");
			    exit(EXIT_FAILURE);
			}
		    } else if (len == 0) {
			child_active = 0;
			if(close(STDOUT_FILENO)==-1){
			    perror("close");
			    exit(EXIT_FAILURE);
			}
		    }
		}
	       	if (FD_ISSET(STDIN_FILENO, &efds)) {
		    perror("stdin exception");
		    exit(EXIT_FAILURE);
		}
		if (FD_ISSET(pipec2p[0], &efds)) {
		    perror("pipec2p exception");
		    exit(EXIT_FAILURE);
		}
	    }
	}
    }
}
