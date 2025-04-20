#include <gtest/gtest.h>
#include "linked_list.h"

#define TEST_DATA_SIZE   8
#define MAX_NUM_ELEMENTS 255

/*
    Purpose:
    Test if the allocation and deallocation works
    properly for Linked List.
*/
TEST(LinkedListTestBasic, InitEmptyListDeinitEmptyList)
{
    LinkedList_str* ll_p = LinkedList_str_Constructor(NULL);

    ASSERT_NE(ll_p, nullptr);

    LinkedList_str_Destructor(&ll_p);

    ASSERT_EQ(ll_p, nullptr);
}

TEST(LinkedListTestBasic, InitEmptyElementDeinitEmptyElement)
{
    LinkedListElement_str* ll_element_p = LinkedListElement_str_Constructor(NULL, NULL);

    ASSERT_NE(ll_element_p, nullptr);   

    LinkedListElement_str_Destructor(&ll_element_p);

    ASSERT_EQ(ll_element_p, nullptr);
}

/* 
    Purpose:
    Test if adding and removing element works
    head_p of the new linked list will point to the newly added element.
    It is worth noting, that ll_element_p is not NULL after test.
    It isn't because the ll_p->head_p is not NULL; it's simply because
    ll_element_p is a local variable, which doesn't have to be set NULL in this case.
    Setting ll_p->head_p to NULL prevents UAF vulnerability.
*/
TEST(LinkedListTestBasic, AddElementRemoveElement)
{
    LinkedList_str* ll_p = LinkedList_str_Constructor(NULL);
    LinkedListElement_str* ll_element_p = LinkedListElement_str_Constructor(NULL, NULL);

    LinkedList_str_AddElement(ll_p, ll_element_p);

    ASSERT_EQ(ll_p->head_p, ll_element_p);

    LinkedList_str_RemoveElement(ll_p, ll_element_p);

    ASSERT_EQ(ll_p->head_p, nullptr);

    LinkedList_str_Destructor(&ll_p);

    ASSERT_EQ(ll_p, nullptr);
}

TEST(LinkedListTestBasic, AddElementAtHeadCheckData)
{
    char buff[TEST_DATA_SIZE] = {"ASDFGHJ"};

    LinkedList_str* ll_p = LinkedList_str_Constructor(NULL);
    LinkedListElement_str* ll_element_p = LinkedListElement_str_Constructor(NULL, NULL);
    memcpy(ll_element_p->data, buff, TEST_DATA_SIZE);

    LinkedList_str_AddElement(ll_p, ll_element_p);

    ASSERT_EQ(ll_p->head_p, ll_element_p);

    for (int i = 0; i < TEST_DATA_SIZE; i++)
    {
        ASSERT_EQ(buff[i], ll_p->head_p->data[i]);
    }

    LinkedList_str_RemoveElement(ll_p, ll_element_p);

    ASSERT_EQ(ll_p->head_p, nullptr);

    LinkedList_str_Destructor(&ll_p);

    ASSERT_EQ(ll_p, nullptr);
}

TEST(LinkedListTestBasic, AddMultipleElementsClearList)
{
    LinkedList_str* ll_p = LinkedList_str_Constructor(NULL);

    for (int i = 0; i < MAX_NUM_ELEMENTS; i++)
    {
        LinkedListElement_str* ll_element_p = LinkedListElement_str_Constructor(NULL, NULL);
        LinkedList_str_AddElement(ll_p, ll_element_p);
    }

    ASSERT_NE(ll_p->head_p, nullptr);

    for (LinkedListElement_str* lle_p = ll_p->head_p; lle_p != NULL; lle_p = lle_p->next_p)
    {
        ASSERT_NE(lle_p, nullptr);
    }

    LinkedList_str_ClearList(ll_p);

    ASSERT_EQ(ll_p->head_p, nullptr);

    LinkedList_str_Destructor(&ll_p);

    ASSERT_EQ(ll_p, nullptr);
}

TEST(LinkedListTestBasic, AddMultipleElementsCheckData)
{
    char buff[TEST_DATA_SIZE] = {"ASDFGHJ"};

    LinkedList_str* ll_p = LinkedList_str_Constructor(NULL);

    for (int i = 0; i < MAX_NUM_ELEMENTS; i++)
    {
        LinkedListElement_str* ll_element_p = LinkedListElement_str_Constructor(NULL, NULL);
        memcpy(ll_element_p->data, buff, TEST_DATA_SIZE);
        LinkedList_str_AddElement(ll_p, ll_element_p);
    }

    ASSERT_NE(ll_p->head_p, nullptr);

    for (LinkedListElement_str* lle_p = ll_p->head_p; lle_p != NULL; lle_p = lle_p->next_p)
    {
        bool isEqual = strncmp(buff, lle_p->data, TEST_DATA_SIZE);
        ASSERT_NE(lle_p, nullptr);
        ASSERT_TRUE(isEqual);
    }
}

TEST(LinkedListTestBasic, AddMultipleElementsRemoveElementInMiddle)
{
    
}

TEST(LinkedListTestBasic, AddMultipleElementsRemoveElementAtHead)
{
    
}

TEST(LinkedListTestBasic, AddMultipleElementsRemoveElementAtTail)
{
    
}
