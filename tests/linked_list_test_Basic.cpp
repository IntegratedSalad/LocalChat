#include <gtest/gtest.h>
#include "linked_list.h"

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
    head_p of the new linked list will point to the newly added element
*/
TEST(LinkedListTestBasic, AddElementRemoveElement)
{
    LinkedList_str* ll_p = LinkedList_str_Constructor(NULL);
    LinkedListElement_str* ll_element_p = LinkedListElement_str_Constructor(NULL, NULL);

    LinkedList_str_AddElement(ll_p, ll_element_p);

    ASSERT_EQ(ll_p->head_p, ll_element_p);

    LinkedList_str_RemoveElement(ll_p, ll_element_p);

    ASSERT_EQ(ll_p->head_p, nullptr);
}

TEST(LinkedListTestBasic, AddElementCheckData)
{

}

TEST(LinkedListTestBasic, AddMultipleElementsClearList)
{

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