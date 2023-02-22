
#include "path_seq.h"


struct sequeue init(struct sequeue seq)
{
    seq.front = seq.rear = (struct node *)malloc(sizeof(struct node)); //????????? ?????????
    seq.rear->next = NULL;
    seq.rear->prev = NULL;
    return seq;
}
struct sequeue pop(struct sequeue seq)
{

		struct node* tist = seq.front->next;
//    seq.front = seq.front->next;
		
		
		free(seq.front);
		seq.front = tist;
    return seq;
}
struct sequeue rear_pop(struct sequeue seq)
{

    seq.rear = seq.rear->prev;
    return seq;
}
struct sequeue push(struct sequeue seq, char * data)
{

    struct node *temp;
    temp = (struct node *)malloc(sizeof(struct node));
    seq.rear->next = temp;
    temp->prev = seq.rear;
    seq.rear = temp;
    strcpy(seq.rear->data,data);


    return seq;
}

void Sequeue_display(void *handle,struct sequeue s)
{
    struct node *p;
    p = s.rear;
    while (p != s.front)
    {

        s.display(handle,p->data);
        p = p->prev;
    }
}
void Sequeue_display_from_rear(void *handle,struct sequeue s)
{
    struct node *p;
    p = s.front->next;
    while (p != s.rear->next)
    {

        s.display(handle,p->data);
        p = p->next;
    }
}
void merge_dir(struct sequeue s,char *buf)
{
				  struct node *p;
					p = s.front->next;
					p = p->next;
					while (p != s.rear->next)
					{
						sprintf(buf,"%s/%s",buf,p->data);
						p = p->next;
					}
}