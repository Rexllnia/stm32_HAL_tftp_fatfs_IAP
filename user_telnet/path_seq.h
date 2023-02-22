#include "stdlib.h"
#include "string.h"
#include "stdio.h"
struct node
{
    char data[100];
    
    struct node *next;
    struct node *prev;
	
};
struct sequeue
{
    struct node *rear;  //????
    struct node *front; //????
		void	(*display)(void *,char *data);
};
struct sequeue init(struct sequeue seq);
struct sequeue pop(struct sequeue seq);
struct sequeue rear_pop(struct sequeue seq);
struct sequeue push(struct sequeue seq, char * data);
struct sequeue Sequeue_invert(struct sequeue s);
void Sequeue_display(void *handle,struct sequeue s);
void Sequeue_display_from_rear(void *handle,struct sequeue s);
void merge_dir(struct sequeue s,char *buf);