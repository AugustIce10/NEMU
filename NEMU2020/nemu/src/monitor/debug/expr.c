#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>

enum {
	NOTYPE = 256, EQ,UEQ,AND,LOR,NOT,HEX,REG,NUMBER

	/* TODO: Add more token types */

};

static struct rule {
	char *regex;
	int token_type;
} rules[] = {

	/* TODO: Add more rules.
	 * Pay attention to the precedence level of different rules.
	 */

	{" +",	NOTYPE},				// spaces
	{"\\+", '+'},					// plus
	{"==", EQ},						// equal
        {"\\b[0-9]+\\b",NUMBER},
        {"-",'-'},
        {"\\*",'*'},
        {"/",'/'},
        {"\\(",'('},
        {"\\)",')'},
	{"!=",UEQ},
	{"&&",AND},
	{"\\|\\|",LOR},
	{"!",NOT},
	{"0[xX][A-Fa-f0-9]{1,8}",HEX},
	{"\\$[a-dA-D][hlHL]|\\$[eE]?(ax|dx|cx|bx|bp|si|di|sp)",REG}
};

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]) )

static regex_t re[NR_REGEX];

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
	int i;
	char error_msg[128];
	int ret;

	for(i = 0; i < NR_REGEX; i ++) {
		ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
		if(ret != 0) {
			regerror(ret, &re[i], error_msg, 128);
			Assert(ret == 0, "regex compilation failed: %s\n%s", error_msg, rules[i].regex);
		}
	}
}

typedef struct token {
	int type;
	char str[32];
} Token;

Token tokens[32];
int nr_token;

static bool make_token(char *e) {
	int position = 0;
	int i;
	regmatch_t pmatch;
	
	nr_token = 0;

	while(e[position] != '\0') {
		/* Try all rules one by one. */
		for(i = 0; i < NR_REGEX; i ++) {
			if(regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
				char *substr_start = e + position;
				int substr_len = pmatch.rm_eo;
                                
				Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s", i, rules[i].regex, position, substr_len, substr_len, substr_start);
				position += substr_len;
                                
				/* TODO: Now a new token is recognized with rules[i]. Add codes
				 * to record the token in the array `tokens'. For certain types
				 * of tokens, some extra actions should be performed.
				 */
                                
				switch(rules[i].token_type) {
				      case 257:
                                        tokens[nr_token].type = 257;
					strcpy(tokens[nr_token].str,"==");
					break;
                                      case 40:
                                        tokens[nr_token].type = 40;
					break;
				      case 41:
					tokens[nr_token].type = 41;
					break;
 				      case 42:
					tokens[nr_token].type = 42;
					break;
				      case 43:
					tokens[nr_token].type = 43;
					break;
				      case 45:
					tokens[nr_token].type = 45;
					break;
				      case 47:
					tokens[nr_token].type = 47;
					break;
				      case 264:
					tokens[nr_token].type = 264;
					strncpy(tokens[nr_token].str,&e[position-substr_len],substr_len);
					break;
				      case 258:
					tokens[nr_token].type = 258;
					strcpy(tokens[nr_token].str,"!=");
					break;
				      case 259:
					tokens[nr_token].type = 259;
					strcpy(tokens[nr_token].str,"&&");
					break;
				      case 260:
					tokens[nr_token].type = 260;
					strcpy(tokens[nr_token].str,"||");
					break;
				      case 261:
					tokens[nr_token].type = 261;
					break;
				      case 262:
					tokens[nr_token].type = 262;
					strncpy(tokens[nr_token].str,&e[position-substr_len],substr_len);
					break;
				      case 263:
					tokens[nr_token].type = 263;
					strncpy(tokens[nr_token].str,&e[position-substr_len],substr_len);
					break;	
                                      default: 
					nr_token--;
                                        break;
				}
				nr_token ++;
				break;
			}
		}

		if(i == NR_REGEX) {
			printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
			return false;
		}
	}

	return true; 
}

bool check_parentheses(int p, int q)
{
	int left = 0;
	int sign = 0;
	if(tokens[p].type == 40)
	{
		left ++ ;
		int i;
		for(i = p+1; i <= q; i++)
		{
			if(tokens[i].type == 40)
				left++;
			else if(tokens[i].type == 41)
			{
				left --;
				if(left == 0 && i != q)
					sign = 1;
				if(left < 0)
					assert(0);
			}
		}
		if(left == 0 && tokens[q].type == 41 && sign != 1)
			return 1;
		else if(left == 0)
			return 0;
		else 
			assert(0);
	}
	else
		return 0;
}

int find_dominant_operator(int p, int q)
{
	int doop = p;
	int j = p;
	int left = 0 ;
	for( ; j <= q ; j++)
	{
		if(tokens[j].type < 262)
		{
			if(tokens[j].type == 40)
			{
				left ++;
				for(j = j + 1; tokens[j].type != 41 || left != 1; j++)
				{
					if(tokens[j].type == 40)    left++;
					if(tokens[j].type == 41)    left--;
				}
				left = 0;
			}
			else if(tokens[j].type == 260)
				doop = j;
			else if(tokens[j].type == 259 && (tokens[doop].type < 260 || tokens[doop].type >= 262))
				doop = j;
			else if(tokens[j].type == 258 && (tokens[doop].type < 259 || tokens[doop].type >= 262))
				doop = j;
			else if(tokens[j].type == 257 && (tokens[doop].type < 258 || tokens[doop].type >= 262))
				doop = j;
			else if(tokens[j].type == '+' && (tokens[doop].type < 256 || tokens[doop].type >= 261))
				doop = j;
			else if(tokens[j].type == '-' && (tokens[j-1].type == 264 || tokens[j-1].type == ')') && j!=p  && (tokens[doop].type < 256 || tokens[doop].type >= 261))
				doop = j;
			else if(tokens[j].type == '/' && (tokens[doop].type == '(' || tokens[doop].type == '*' || tokens[doop].type == '/' || tokens[doop].type >= 261))
				doop = j;
			else if(tokens[j].type == '*' && j!=p && (tokens[j-1].type == 264 || tokens[j-1].type == ')') &&(tokens[doop].type >= 261 ||  tokens[doop].type == '(' || tokens[doop].type == '*' || tokens[doop].type == '/'))
				doop = j;
		}
	}
	return doop;
}

uint32_t expr(char *e, bool *success) {
	if(!make_token(e)) {
		*success = false;
		return 0;
	}

	/* TODO: Insert codes to evaluate the expression. */
	int eval(int p, int q)
	{
		int i=0;
		if(p>q)
			assert(0);
		else if(p == q)
		{
			if(tokens[p].type == 264)
			{
				sscanf(tokens[p].str, "%d", &i);
				return i;
			}
			else if(tokens[p].type == 262)
			{
				int j=0,sl=1,sw=1;
				for (; j < 8 && sl != 0 && sw != 0 ; j++)
				{
					sl = strcmp(tokens[p].str + 1 , regsl[i]);
					sw = strcmp(tokens[p].str + 1 , regsw[i]);
				}
				if(sl == 0)
				{
					i = cpu.gpr[j]._32;
					return i;
				}
				else if(sw == 0)
					return cpu.gpr[j]._16;
				else
				{
					if(strcmp(tokens[p].str , "$al") == 0)
						return reg_b(0);
					if(strcmp(tokens[p].str + 1 , "cl") == 0)
						return reg_b(1);
					if(strcmp(tokens[p].str + 1 , "dl") == 0)
						return reg_b(2);
					if(strcmp(tokens[p].str + 1 , "bl") == 0)
						return reg_b(3);
					if(strcmp(tokens[p].str + 1 , "ah") == 0)
						return reg_b(4);
					if(strcmp(tokens[p].str + 1 , "ch") == 0)
						return reg_b(5);
					if(strcmp(tokens[p].str + 1 , "dh") == 0)
						return reg_b(6);
					if(strcmp(tokens[p].str + 1 , "bh") == 0)
						return reg_b(7);
				}
				if(j == 8)
					assert(0);
			}
			else 
				assert(0);
		}
		else if(check_parentheses(p,q)==true)
			return eval(p+1,q-1);
		else
		{
			int op,val1,val2;
			if((q-p == 1) && tokens[p].type == '-')
				return 0-eval(q,q);
			if(((q-p == 1) || (tokens[p+1].type == '(' && tokens[q].type == ')')) && tokens[p].type == 261)
			{
				i = eval(p+1,q);
				return !i;
			}
			op = find_dominant_operator(p,q);

			val1 = eval(p,op-1);
			val2 = eval(op+1,q);
			switch(tokens[op].type)
			{
				case '+' : return val1 + val2;
				case '-' : return val1 - val2;
				case '*' : return val1 * val2;
				case '/' : return val1 / val2;
				case 257:
					if(val1 == val2)
						return 1;
					else 
						return 0;
				case 258:
					if(val1 != val2)
						return 1;
					else
						return 0;
				case 259:
					if(val1 && val2)
						return 1;
					else
						return 0;
				case 260:
					if(val1 || val2)
						return 1;
					else
						return 0;
				default:assert(0);
			}
		}
		return 0;
	}
	int p = 0;
	int q = nr_token - 1;
	*success = true;
	return eval(p,q);
}

