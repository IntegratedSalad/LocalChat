#include "linked_list.h"
#include <stdlib.h>
#include <string.h>

// Probably not needed, as it will be instantiated directly in class as its property.
LinkedList_str* LinkedList_str_Constructor(LinkedListElement_str* head_p)
{
    LinkedList_str* ll_p = NULL;
    ll_p = malloc(sizeof(LinkedList_str));
    memset(ll_p, 0, sizeof(LinkedList_str));
    if (head_p != NULL)
    {
        ll_p->head_p = head_p;
    }
    return ll_p;
}

/* This DOES NOT deallocate each and every element. */
void LinkedList_str_Destructor(LinkedList_str** list_p)
{
    if (list_p == NULL || *list_p == NULL) return;

    free((*list_p)->head_p);
    (*list_p)->head_p = NULL;
    free(*list_p);
    *list_p = NULL;
}

LinkedListElement_str* LinkedList_str_FindTail(const LinkedList_str* ll_p)
{
    LinkedListElement_str* head_p = ll_p->head_p;
    if (head_p == NULL) return NULL;

    LinkedListElement_str* prev_p = head_p;
    for (; head_p != NULL; head_p = head_p->next_p)
    {
        prev_p = head_p;
    }
    // prev_p->next == NULL!
    return prev_p;
}

void LinkedList_str_AddElement(LinkedList_str* ll_p,
                               LinkedListElement_str* ll_element_p)
{
    if (ll_p->head_p == NULL)
    {
        ll_p->head_p = ll_element_p;
        return;
    }

    LinkedListElement_str* tail_p = LinkedList_str_FindTail(ll_p);
    if (tail_p != NULL)
    {
        tail_p->next_p = ll_element_p;
    }
}

void LinkedList_str_RemoveElement(LinkedList_str* ll_p, // provide address of the pointer
                                  LinkedListElement_str* ll_element_p)
{
    LinkedListElement_str* head_p = ll_p->head_p;
    if (head_p == NULL) return;

    if (ll_element_p == head_p) // element_p is at head_p
    {
        ll_p->head_p->next_p = ll_element_p->next_p;
        LinkedListElement_str_Destructor(&ll_p->head_p);
        return;
    }

    LinkedListElement_str* prev_p = head_p;
    for (; (head_p != NULL) || head_p == ll_element_p; head_p = head_p->next_p)
    {
        prev_p = head_p;
    }

    prev_p->next_p = NULL;
    LinkedListElement_str_Destructor(&prev_p);
}

void LinkedList_str_ClearList(LinkedList_str* ll_p)
{
    LinkedListElement_str* head_p = ll_p->head_p;
    if (head_p == NULL) return;

    LinkedListElement_str* prev_p = head_p;
    for (; head_p->next_p != NULL; head_p = head_p->next_p) // it will end on one before the end
    {
        prev_p = head_p;
        LinkedListElement_str_Destructor(&head_p);
    }
    LinkedListElement_str_Destructor(&prev_p);
}

LinkedListElement_str* LinkedListElement_str_Constructor(char data[MAX_DATA_SIZE],
                                                         LinkedListElement_str* next)
{
    // Every linked list element has to be put on heap.
    LinkedListElement_str* ll_element_p = NULL;
    ll_element_p = malloc(sizeof(LinkedListElement_str));
    if (data != NULL)
    {
        memcpy(ll_element_p->data, data, MAX_DATA_SIZE);
    }
    return ll_element_p;
}

void LinkedListElement_str_Destructor(LinkedListElement_str** ll_element_p)
{
    free(*ll_element_p);
    *ll_element_p = NULL;
}
