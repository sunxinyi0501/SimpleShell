#include "get_path.h"

char *gch;
char *which(char *command, struct pathelement *p)
{
	
	char cmd[64], *ch;
	int  found;

	found = 0;
	while (p) {       
		sprintf(cmd, "%s/%s", p->element, command);
		if (access(cmd, X_OK) == 0) {
			found = 1;
			break;
		}
		p = p->next;
	}
	if (found) {
		ch = malloc(strlen(cmd)+1);

		strncpy(ch, cmd, (strlen(cmd)+1)<64?(strlen(cmd)+1):64);
		
		gch = ch;
		return ch;
	}
	else
	{
		printf("return NULL \n");
		return (char *) NULL;
	}
}
