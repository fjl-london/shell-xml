/*  $Id$

 $Logi$


*/


#include <stdio.h>
#include <string.h>

int main (int argc, char **argv)
{
char *line;
size_t llen;
int ret = 1;
	while ((line = fgetln(stdin, &llen)))
	{
	int quote = 0;

		while (llen--)
		{
		char c;
			if (quote)
			{
				while ((c = *line++) != quote && c != '\n')
					putchar (c);
				quote = ret = 0;
				break;	// Done for this line
			}
			if ((c = *line++) == '"' || c == '\'')
				quote = c;
				
		} 
	}
	
	return 0;
}

