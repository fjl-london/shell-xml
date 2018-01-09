/*  $Id: xml.c,v 1.3 2017/12/01 17:26:02 root Exp root $

 $Log: xml.c,v $
 Revision 1.3  2017/12/01 17:26:02  root
 Redmine version

 Revision 1.2  2017/11/28 13:07:01  fjl
 Almost fixed / bug

 Revision 1.1  2017/11/27 19:09:06  fjl
 Initial revision


*/


#include <stdio.h>
#include <string.h>

typedef struct
{
	char buf[1024];
	int buflen;
	int index;
} tagbuf;
 
int flag_quotedata = 1;	// Put " marks around data from inside a tag
int flag_nopath = 0;	// Don't display tag path - only the data
int flag_setvar = 0;	// "setvar" output mode
char *flag_attribute_prefix = "__";
char *flag_path_separator = "_";


int skip_to_c (char c_to_find)
{
int c;
	while ((c = getchar()) != c_to_find)
		if (c == EOF)
			return 1;
	return 0;
}

void tabs (int num)
{
	while (num--)
		putchar (' ');
}

void init_tag (tagbuf *tb)
{
	tb->buflen=1024;
	tb->index = 0;
	tb->buf[0] = 0;
}


void print_path (tagbuf *tb)
{
int i;
char *ptr;
	if (!(i = tb->index))
		return;
	ptr = tb->buf;
	while (i--) // One less than index so final / not printed.
	{
	int c = *ptr++;
		if (c)
			putchar (c);
		else
			fputs (flag_path_separator,stdout);
	}
}

int read_tag (tagbuf *tb, char *tag, int depth)
{
int c;
int closing, empty_element; // Flags
int havedata;   // True of data exists in tag
int start;		// Index in to tagbuf buffer at start

	depth++;
	start = tb->index;
	if (!flag_nopath && start)	// Don't begin with line of = on its own
	{
		putchar('\n');
		if (flag_setvar) fputs("setvar ",stdout);
		print_path (tb);
		if (flag_setvar) 
			putchar (' ');
		else
			putchar ('=');
	}
        for (;;)
	{
		havedata = closing = empty_element = 0;


		while ((c = getchar()) != '<') // Print data between tags 
			if (c == EOF)
				return 1;
			else
			{
				if (!havedata)
				{
					havedata = 1;
					if (flag_quotedata) putchar('"');
				}
				putchar (c);
			}
                if (flag_quotedata && havedata) putchar('"');

		// When we get here we've just read a '<'

		if ((c = getchar()) == '/')	// Closing tag?
		{
			c = getchar();
			closing = 1;
		}
		else
			if (c == '?')	// Comment tag?
			{
				do
				{
					while ((c = getchar()) != '?')	// Just burn comments
						if (c == EOF)
							return 1;
				}
				while (getchar() != '>');
	
				continue; // Go and find a real tag
			}
	
		while ( c != '>')
		{
        	        if (c == EOF)
        	                return 1;

			if (empty_element) // If we have passed empty element / there should be no more chars
			{
				fprintf(stderr,"Unexpeced characters after empty element '/'\n");
				return 1;
			}

			if (c == ' ')	// Passed initial chars, ' ' = start of attributes
			{
			int ai=0;
			char abuf[128];	// Max size of attribute
			int inquote= 0;

				if (closing)
					fprintf(stderr,"Error - attributes found in closing tag\n");

				while ((c = getchar()) != EOF)
				{
					if (c == '"')
						inquote = !inquote;
 
					if (inquote)	// If inside a quote, just copy characters.
					{
						if (ai<126)
							abuf[ai++]=c;
						continue;
					}
					
					if (empty_element)
					{
                                                if (c != '>') // Next char after '/' should be '>'
                                                {
                                                        fprintf (stderr,"Missing '>' in empty element tag\n");
                                                        return 1;
                                                }
 						break;
					}	

					if (c == '/') // Must be an empty element 
					{
						empty_element = 1;
					 	if ((c = getchar()) == EOF)
						{
                                                        fprintf (stderr,"Unexpected EOF before '>'\n");
							return 1;	
						}	// Important - drops through to print attribute just red
					}

					if (c == ' ' || c == '>')
					{
						abuf[ai] = 0;
						tb->buf[tb->index] = 0;	// Null terminate path
						if (!flag_nopath && tb->index) // Avoid printing blank line
						{
							putchar('\n');
							print_path(tb);
							fputs(flag_attribute_prefix,stdout);
						}
						fputs(abuf,stdout);
						if (c == '>')		// Inner while loop termination
							break;
						ai=0;
					}
					else 
						if (ai<126)
							abuf[ai++]=c;
				}

			}
			else	// Still reading tag name
			{
				if (c == '/') // Strictly speaking there SHOULD have been space before but...
					empty_element = 1;
				else
        	        		tb->buf[tb->index++] = c;
				c = getchar();
			}
        	}
        	tb->buf[tb->index++] = 0;
		if (closing)
		{
			if (strcmp (tag,&tb->buf[start]))
				fprintf(stderr,"Excpected close of '%s' but got '%s'\n",tag,&tb->buf[start]);
			return 0;
		}
	
		if (!empty_element)
			read_tag (tb,&tb->buf[start],depth);

		tb->index = start;
	}

        return 0;
}



int find_tag (char *tag)
{
char found[128];
int i = 0;
int c;
	if (skip_to_c ('<')) return 1;

	while ((c = getchar()) != '>' && i <= 126 )
	{
		if (c == EOF)
			return 1;
		if (c > ' ')
			found[i++] = c;
	}
	found[i]=0;

	printf("Found <%s>\n",found);
	return 0;
}


int main (int argc, char **args)
{
tagbuf tb;

	if (argc < 1)
	{
		fprintf(stderr,"No tag specified\n");
		return 1;
	}

	init_tag (&tb);

	read_tag (&tb,"",0);

	putchar ('\n');

//	while (!find_tag (args[1]))
//		;


	return 0;
}


