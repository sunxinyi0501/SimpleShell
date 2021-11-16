#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <signal.h>

#include "dl_builtin.h"
#include "get_path.h"
#include "sh.h"




static int dlfun_exit(void *arg);
static int dlfun_cd(void *arg);
static int dlfun_where(void *arga);
static int dlfun_pwd(void *arg);
static int dl_sh_update_cwd(void *arg);
static int dlfun_list(void *arg);
static int dlfun_pid_of_the_shell(void *arg);
static int dlfun_kill(void *arg);
static int dlfun_prompt(void *arg);
static int dlfun_printenv(void *arg);
static int dlfun_setenv(void *arg);
static int dlfun_setvar(void *arg);

static int dl_set_prompt(char *p);

int shell_var_ignoreeof;

static int dl_set_prompt(char *p)
{
	int ret = -2;

	if( p && strlen(p) )
	{
		strncpy(dl_prompt, p, SIZE_DL_PROMPT);
		strcat(dl_prompt, " ");
		dl_prompt[SIZE_DL_PROMPT] = '\0';
		ret = 0;
	}

	return ret;
}

int dl_sh_init(void *arg)
{
	/* signal */
	signal(SIGINT, SIG_IGN);	// Ctrl + C
	signal(SIGTSTP, SIG_IGN);	// Ctrl + Z

	/* "cwd" related */
	dl_sh_update_cwd(NULL);

	/* prompt related */
	memset(dl_prompt, 0, SIZE_DL_PROMPT);
	dl_set_prompt("cisc361");

	return 0;
}
static int dl_sh_update_cwd(void *arg)
{
	memset(cwd_ptr, 0, SIZE_CWD_PTR);
	strncpy(cwd_ptr, getcwd(NULL, 0), SIZE_CWD_PTR);
	printf("cwd_old is (%s).\n", cwd_ptr);
	return 0;
}


struct dl_cmd_st dlcmd[]=
{
	{.cmd="exit",	.func=dlfun_exit},	
	{.cmd="cd",	.func=dlfun_cd},	

	/* Both which and where will start searching for the target from the front of the path/PATH value. */
	{.cmd="where",	.func=dlfun_where},	

	{.cmd="pwd",	.func=dlfun_pwd},	

	//{.cmd="ls",	.func=dlfun_list},	
	{.cmd="list",	.func=dlfun_list},	

	{.cmd="pid",	.func=dlfun_pid_of_the_shell},	

	{.cmd="kill",	.func=dlfun_kill},	

	{.cmd="prompt",	.func=dlfun_prompt},	

	{.cmd="printenv",	.func=dlfun_printenv},	
	{.cmd="setenv",	.func=dlfun_setenv},
	
	{.cmd="set",    .func=dlfun_setvar},

	{.cmd=NULL,	.func=NULL},
};

int dl_checkif_builtin(void *arg)
{
	int index = 0;

char **param = (char**)arg;


	if( NULL == param )
	{
		return 1;
	}
	if( NULL == param[0] )
	{
		return 1;
	}

#if 0
	char *cmd = "exit";
#else
	char *cmd = param[0];
#endif

	for(index = 0; param[index]; index++)
	{
		printf("[%d]\t%s\n", index, param[index]);
	}

	for(index = 0; dlcmd[index].cmd; index++)
	{
		if( 0 == strncmp(cmd, dlcmd[index].cmd, strlen(dlcmd[index].cmd)) )
		{
			if( NULL != dlcmd[index].func )
			{
			printf("CMD is (%s), exec now...\n", dlcmd[index].cmd);
				dlcmd[index].func(arg);
				return 0;
			}
		}
	}
	return 2;
}

static int dlfun_exit(void *arg)
{
	exit(0);
	return 0;
}

static int dlfun_cd(void *arg)
{
	char **param = (char **)arg;

	if( NULL == param[1] ) // "cd  "
	{
		printf("cd home (%s)\n", getenv("HOME"));
		chdir(getenv("HOME"));
	}
	else if( 0 == strncmp(param[1], "-", strlen("-")) ) // "cd -"
	{
		printf("cd previous\n");
		if( strlen(cwd_ptr) )
		{
			chdir(cwd_ptr);
		}
	}
	else
	{
		printf("cd (%s)\n", param[1]);
		chdir(param[1]); // "cd THE_PATH"
	}
	dl_sh_update_cwd(NULL);

	return 0;
}


static int dlfun_where(void *arga)
{
char **arg = (char **)(arga);
			struct pathelement *p, *tmp;
			char *cmd;

			printf("Executing built-in [which]\n");

			if (arg[1] == NULL) {  // "empty" which
				printf("which: Too few arguments.\n");
				return 2;
			}

			p = get_path();
			/***
			  tmp = p;
			  while (tmp) {      // print list of paths
			  printf("path [%s]\n", tmp->element);
			  tmp = tmp->next;
			  }
			 ***/

			cmd =(char*) which(arg[1], p);
			extern char *gch;
			cmd = gch; // ??
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

return 0;
}

static int dlfun_pwd(void *arg)
{
	printf("Current dir is (%s).\n", getcwd(NULL, 0));
	return 0;
}

static int dlfun_list(void *arg)
{
	char **param = (char **)arg;

	/* Debug print */
	//printf("ARG : %s\n", param[0]);
	//printf("ARG : %s\n", param[1]);

	/* Get "argc" */
	int tmp_argc = 0;
	char **tmp_ptr = param;
	while( *tmp_ptr++ )
		tmp_argc++;

	extern int simple_ls_main(int argc, char *argv[]);
	simple_ls_main(tmp_argc, param);

	return 0;
}

static int dlfun_pid_of_the_shell(void *arg)
{
	printf("PID of the shell is (%d)\n", getpid());
	return 0;
}

static int dlfun_kill(void *arg)
{
	char **param = (char **)arg;
	int sig = SIGTERM;
	int pid = 0;

	if( 0 == strncmp(param[1], "-", strlen("-")))
	{
		pid = atoi(param[2]);
		sig = atoi(param[1]+1);
	}
	else
	{
		pid = atoi(param[1]);
	}

	printf("KILL (%d) (%d)\n", sig, pid );
	kill(pid, sig);

	return 0;
}

static int dlfun_prompt(void *arg)
{
	char **param = (char **)arg;

	if ( NULL == param[1] )	// prompt, >>input
	{
		char p[SIZE_DL_PROMPT];
		memset(p, 0, SIZE_DL_PROMPT);
		printf("input prompt prefix: ");
#if 1
		fgets(p, SIZE_DL_PROMPT, stdin);
#else
		gets(p);
#endif
		dl_set_prompt(p);
	}
	else
	{
		dl_set_prompt(param[1]);
	}

	return 0;
}

static int dlfun_printenv(void *arg)
{
	extern char **environ;
	char **ptr = environ;
	char **param = (char **)arg;

	// printenv
	// man 7 environ
	if( NULL == param[1] )
	{
		while( *ptr )
		{
			printf("%s\n", *ptr);
			ptr++;
		}
	}
	// printenv VAR
	else if( NULL == param[2] )
	{
		printf("%s\n", getenv(param[1]));
	}
	// too many arguments
	else
	{
		printf("%s: Too many arguments.\n", param[0]);
	}

	return 0;
}

static int dlfun_setenv(void *arg)
{
	extern char **environ;
	char **ptr = environ;
	char **param = (char **)arg;

	// "setenv" - same as printenv
	if( NULL == param[1] )
	{
		while( *ptr )
		{
			printf("%s\n", *ptr);
			ptr++;
		}
	}
	// "setenv VAR" - set empty
	else if( NULL == param[2] )
	{
		setenv(param[1], "", 1);
	}
	// "setenv VAR VAULE" - set vaule
	else if( NULL == param[3] )
	{
		setenv(param[1], param[2], 1);
	}
	// too many arguments!
	else
	{
		printf("%s: Too many arguments.\n", param[0]);
	}

	return 0;
}



static int dlfun_setvar(void *arg)
{
	char **param = (char **)arg;
	char *pvaule = NULL;

	if( param[1] )
	{
		if( 0 == strncmp(param[1], "ignoreeof", strlen("ignoreeof")) )
		{
			if( NULL != (pvaule = strchr(param[1], (int)'=')) )
			{
				printf("pvl=%s\n", pvaule);
				pvaule++;
				shell_var_ignoreeof = atoi(pvaule);
			}
		}
	}

	return 0;
}

