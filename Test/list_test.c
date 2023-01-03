/**
 * @file list_test.c
 * @author your name (you@domain.com)
 * @brief FreeRTOS 提供的链表测试
 * @version 0.1
 * @date 2023-01-02
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#include "list_test.h"
#include "FreeRTOS.h"
#include "list.h"

List_t testList;

ListItem_t listItem1;
ListItem_t listItem2;
ListItem_t listItem3;

void list_test()
{
    vListInitialise(&testList);

    vListInitialiseItem(&listItem1);
    listItem1.xItemValue = 1;

    vListInitialiseItem(&listItem2);
    listItem2.xItemValue = 2;
    
    vListInitialiseItem(&listItem3);
    listItem3.xItemValue = 3;

    vListInsert(&testList, &listItem2);
    vListInsert(&testList, &listItem1);
    vListInsert(&testList, &listItem3);
    printf("list test complete\r\n");
    return;
}
