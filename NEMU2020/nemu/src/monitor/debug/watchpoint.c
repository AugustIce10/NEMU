#include "monitor/watchpoint.h"
#include "monitor/expr.h"

#define NR_WP 32

static WP wp_pool[NR_WP];
static WP *head, *free_;
int init = 0;

void init_wp_pool() {
	int i;
	for(i = 0; i < NR_WP; i ++) {
		wp_pool[i].NO = i;
		wp_pool[i].next = &wp_pool[i + 1];
	}
	wp_pool[NR_WP - 1].next = NULL;

	head = NULL;
	free_ = wp_pool;
}

WP *new_wp()
{
	if(init == 0)
		init_wp_pool();
	WP *p = free_;
	if(p)
	{
		free_ = free_->next;
		p->next = head;
		head = p;
		init ++;
		return p;
	}
	else
	{
		assert(0);
	}
}

int free_wp(int no)
{
	WP *p = head;
	if(head == NULL)
	{
		printf("Watchpoint list is empty. \n");
		assert(0);
	}
	else if(p->NO == no)
	{
		head = head->next;
		p->value = 0;
		p->isused = false;
		p->next = free_;
		free_ = p;
		init --;
		printf("Having alread deleted the %dth watchpoint.\n",no);
		return 1;
	}
	else
	{
		WP *q = head;
		p = p->next;
		while(p != NULL)
		{
			if(p->NO == no)
			{
				q->next = p->next;
				p->value = 0;
				p->isused = false;
				p->next = free_;
				free_ = p;
				printf("Having alread deleted %dth watchpoint.\n",no);
				init --;				
				return 1;
			}
			else
			{
				p = p->next;
				q = q->next;
			}
		}
	}
	printf("The %dth watchpoint is not exist.\n",no);
	return 0;
}

void PrintW()
{
	WP *p = head;
	if(p == NULL)
	{
		printf("There is no watchpoint!\n");
		return ;
	}
	else
	{
		while(p != NULL)
		{
			printf("%d   %s 0x%08x\n",p->NO,p->expr,p->value);
			p = p->next;
		}
		return ;
	}
	return ;
}

bool delete_watchpoint(int no)
{
	if(free_wp(no)) return true;
	else printf("Delete false\n");
	return false;
}

int set_watchpoint(char *e)
{
	WP *p;
	p = new_wp();
	printf("Set watchpoint %d\n", p->NO);
	strcpy(head->expr,e);
	printf("expr = %s\n", p->expr);
	bool success = true;
	p->value = expr(p->expr , &success);
	if(!success)
	{
		printf("Faili to eval\n");
		return 0;
	}
	else
	{
		printf("Old valud = %#x\n", p->value);
	}
	return 1;
}

WP* scan_watchpoint(void)
{
	WP *p = head;
	bool success = true;
	if(p == NULL)
	{
		printf("No Watchpoint\n");
		return false;
	}
	else
	{
		while(p)
		{
			p->newvalue = expr(p->expr, &success);
			if(!success)
				printf("Fail to eval new eval in watchpoint %d\n",p->NO);
			else
			{
				if(p->newvalue != p->value)
				{
					printf("Hit watchpoint %d\n", p->NO);
					printf("Expr = %s\n",p->expr);
					printf("Old value = %#x\n  new value = %#x\n", p->value,p->newvalue);
					p->value = p->newvalue;
					printf("promgram paused\n");
					return p;
				}
			}
		}
	}
	return NULL;
}

/* TODO: Implement the functionality of watchpoint */


