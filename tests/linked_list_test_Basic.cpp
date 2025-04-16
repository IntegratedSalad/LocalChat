#include <gtest/gtest.h>
#include "linked_list.h"

TEST(LinkedListTest, InitEmptyDeinitEmpty)
{
    LinkedList_str* ll_p = LinkedList_str_Constructor(NULL);

    ASSERT_NE(ll_p, nullptr);

    LinkedList_str_Destructor(ll_p);

    ASSERT_EQ(ll_p, nullptr);
}