#pragma once

#define MAX_DATA_SIZE 256

#ifdef __cplusplus // prevent name-mangling
extern "C" {
#endif

// For now, supports only char 
typedef struct LinkedListElement_str
{
    struct LinkedListElement_str* next_p;
    char data[MAX_DATA_SIZE];
} LinkedListElement_str;

typedef struct LinkedList_str
{
    LinkedListElement_str* head_p;
    // tail_p
} LinkedList_str;

/* Linked List functions */
LinkedList_str* LinkedList_str_Constructor(LinkedListElement_str* head_p);
void LinkedList_str_Destructor(LinkedList_str** list_p);
void LinkedList_str_AddElement(LinkedList_str* ll_p,
                               LinkedListElement_str* ll_element_p);
void LinkedList_str_RemoveElement(LinkedList_str* ll_p,
                                  LinkedListElement_str* ll_element_p);
void LinkedList_str_ClearList(LinkedList_str* ll_p);
LinkedListElement_str* LinkedList_str_FindTail(const LinkedList_str* ll_p);
// IsEmpty

/* LinkedListElement functions */
LinkedListElement_str* LinkedListElement_str_Constructor(char data[MAX_DATA_SIZE],
                                                         LinkedListElement_str* next_p);
void LinkedListElement_str_Destructor(LinkedListElement_str** llelement_p);

#ifdef __cplusplus
}
#endif