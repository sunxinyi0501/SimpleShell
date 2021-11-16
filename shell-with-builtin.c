#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include "sh.h"
#include "dl_builtin.h"

	int
main(int argc, char **argv, char **envp)
{
	char	buf[MAXLINE];
	char	buf_origin[MAXLINE];
	char    *arg[MAXARGS];  // an array of tokens
	char    *ptr;
	char    *pch;
	pid_t	pid;
	int	status, i, arg_no;

	dl_sh_init(NULL);

	printf("%s %s--> ",dl_prompt, getcwd(NULL,0));	/* print prompt (printf requires %% to print %) */

again:
	while (fgets(buf, MAXLINE, stdin) != NULL) {
		memcpy(buf_origin, buf, MAXLINE);
		if (strlen(buf) == 1 && buf[strlen(buf) - 1] == '\n')
			goto nextprompt;  // "empty" command line

		if (buf[strlen(buf) - 1] == '\n')
			buf[strlen(buf) - 1] = 0; /* replace newline with null */
		// parse command line into tokens (stored in buf)
		arg_no = 0;
		pch = strtok(buf, " ");
		while (pch != NULL && arg_no < MAXARGS)
		{
			arg[arg_no] = pch;
			arg_no++;
			pch = strtok (NULL, " ");
		}
		arg[arg_no] = (char *) NULL;

		if (arg[0] == NULL)  // "blank" command line
			goto nextprompt;

		/* print tokens
		   for (i = 0; i < arg_no; i++)
		   printf("arg[%d] = %s\n", i, arg[i]);
		 */

		if (strcmp(arg[0], "pwd") == 0) { // built-in command pwd 
			printf("Executing built-in [pwd]\n");
			ptr = getcwd(NULL, 0);
			printf("%s\n", ptr);
			free(ptr);
		}
		else if (strcmp(arg[0], "which") == 0) { // built-in command which
			struct pathelement *p, *tmp;
			char *cmd;

			printf("Executing built-in [which]\n");

			if (arg[1] == NULL) {  // "empty" which
				printf("which: Too few arguments.\n");
				goto nextprompt;
			}

			p = get_path();
			/***
			  tmp = p;
			  while (tmp) {      // print list of paths
			  printf("path [%s]\n", tmp->element);
			  tmp = tmp->next;
			  }
			 ***/

			cmd = which(arg[1], p);
			if (cmd) {
				printf("%s\n", cmd);
				free(cmd);
			}
			else               // argument not found
				printf("%s: Command not found\n", arg[1]);

			while (p) {   // free list of path values
				tmp = p;
				p = p->next;
				free(tmp->element);
				free(tmp);
			}
		} 
		else if( 0 == dl_checkif_builtin(arg) )
		{
			//printf("Is my builtin cmd!\n");
		}
		else 
		{
			if ((pid = fork()) < 0) {
				printf("fork error");
			} else if (pid == 0) {		/* child */
				printf("EXEC : (%s)(%s)\n", buf, buf_origin);

				if( dl_process_wildcard(arg) )
				{
					//execlp(buf, buf, (char *)0);
					execlp(arg[0], arg[0], arg[1], (char *)0);
				}

				printf("couldn't execute: %s", buf);
				exit(127);
			}

			/* parent */
			if ((pid = waitpid(pid, &status, 0)) < 0)
				printf("waitpid error");
			/**
			  if (WIFEXITED(status)) S&R p. 239 
			  printf("child terminates with (%d)\n", WEXITSTATUS(status));
			 **/
		}

nextprompt:
		printf("%s %s--> ", dl_prompt,getcwd(NULL,0));
	}
	extern int shell_var_ignoreeof;
	if ( shell_var_ignoreeof-->0 )
        {		printf("`Use \"exit\" to leave tcsh.\n");

                 	goto again;
	}
        free(argv);
	free(envp);
	exit(0);
}
