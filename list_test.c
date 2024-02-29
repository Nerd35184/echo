#include "list.h"
#include <stdio.h>

typedef struct Test
{
    int i;
} Test_t;

void test_copy_construct(Test_t *this, const Test_t *src)
{
    printf("test_copy_construct src:%d\n", src->i);
    this->i = src->i;
}

void test_destructor(Test_t *this)
{
    printf("test_destructor %d\n", this->i);
}

extern_list(Test_t);
def_list_func(Test_t);

void print_test_list(List(Test_t) * list_p)
{
    ListNode(Test_t) *root_p = &(list_p->root);
    ListNode(Test_t) *cur = root_p->next;
    ListNode(Test_t) *next = NULL;
    printf("{\n len:%I64d\n", list_p->len);
    while (cur != root_p)
    {
        printf("    %d\n", cur->data.i);
        next = cur->next;
        cur = next;
    }
    printf("}\n");
}

int main()
{
    int ret = 0;
    List(Test_t) *list_p = alloc_list(Test_t)();
    print_test_list(list_p);
    for (int i = 0; i < 5; ++i)
    {
        Test_t t1 = {i : i};
        ret = push_back_list(Test_t)(list_p, t1, test_copy_construct, test_destructor);
        if (ret != 0)
        {
            goto end;
        }
        print_test_list(list_p);
    }
    printf("---------------------------------\n");
    for (int i = 0; i < 5; ++i)
    {
        Test_t t1 = {i : -1};
        ret = pop_front_list(Test_t)(list_p, &t1, test_copy_construct);
        if (ret != 0)
        {
            goto end;
        }
        print_test_list(list_p);
        printf("%d\n", t1.i);
    }
    printf("---------------------------------\n");
    for (int i = 0; i < 5; ++i)
    {
        Test_t t1 = {i : i};
        ret = push_back_list(Test_t)(list_p, t1, test_copy_construct, test_destructor);
        if (ret != 0)
        {
            goto end;
        }
        print_test_list(list_p);
    }
    printf("done\n");
end:
    free_list(Test_t)(&list_p);
    return 0;
}