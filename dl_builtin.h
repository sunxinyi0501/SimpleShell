/*
 * 2020.10.02
 * Simple Shell
 * dramalife@live.com
 */
#ifndef _DL_BUILTIN_H_
#define _DL_BUILTIN_H_


struct dl_cmd_st
{
	char *cmd;
	int (*func)(void *);
};


/* cd */
#define SIZE_CWD_PTR	128
char cwd_ptr[SIZE_CWD_PTR];
#define SIZE_DL_PROMPT	SIZE_CWD_PTR
char dl_prompt[SIZE_DL_PROMPT];


extern int dl_checkif_builtin(void *arg);
extern int dl_sh_init(void *arg);
extern int dl_process_wildcard(char **des);
#endif
