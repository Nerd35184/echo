#ifndef _LIST_H_
#define _LIST_H_

#include "util.h"
#include <stdint.h>

#define ListNode(T) ListNode_##T
#define alloc_list_node(T) alloc_list_node_##T
#define destruct_list_node(T) destruct_list_node_##T
#define free_list_node(T) free_list_node_##T
#define extern_list_node(T)                                                                                                   \
    typedef struct ListNode(T) ListNode(T);                                                                                   \
    struct ListNode(T)                                                                                                        \
    {                                                                                                                         \
        T data;                                                                                                               \
        ListNode(T) * next;                                                                                                   \
        ListNode(T) * prev;                                                                                                   \
        void (*data_destructor)(T * t_p);                                                                                     \
    };                                                                                                                        \
    ListNode(T) * alloc_list_node(T)(T data, void (*copy_constructor)(T * dst_p, const T *src), void (*destructor)(T * t_p)); \
    void free_list_node(T)(ListNode(T) * *list_node_pp)

#define def_list_node_func(T)                                                                                                  \
                                                                                                                               \
    ListNode(T) * alloc_list_node(T)(T data, void (*copy_constructor)(T * dst_p, const T *src_p), void (*destructor)(T * t_p)) \
    {                                                                                                                          \
        ListNode(T) * list_node_p;                                                                                             \
        MALLOC(list_node_p, 1, sizeof(ListNode(T)));                                                                           \
        list_node_p->data_destructor = destructor;                                                                             \
        list_node_p->data = data;                                                                                              \
        if (copy_constructor != NULL)                                                                                          \
        {                                                                                                                      \
            copy_constructor(&list_node_p->data, &data);                                                                       \
        }                                                                                                                      \
        list_node_p->next = list_node_p;                                                                                       \
        list_node_p->prev = list_node_p;                                                                                       \
        return list_node_p;                                                                                                    \
    }                                                                                                                          \
    void destruct_list_node(T)(ListNode(T) * list_node_p)                                                                      \
    {                                                                                                                          \
        if (list_node_p == NULL)                                                                                               \
        {                                                                                                                      \
            return;                                                                                                            \
        }                                                                                                                      \
        if (list_node_p->data_destructor != NULL)                                                                              \
        {                                                                                                                      \
            list_node_p->data_destructor(&list_node_p->data);                                                                  \
        }                                                                                                                      \
    }                                                                                                                          \
                                                                                                                               \
    void free_list_node(T)(ListNode(T) * *list_node_pp)                                                                        \
    {                                                                                                                          \
        FREE_PP(list_node_pp, destruct_list_node(T));                                                                          \
    }

#define List(T) List##T
#define alloc_list(T) alloc_list_##T
#define destruct_list(T) destruct_list_##T
#define free_list(T) free_list_##T
#define join_list_node2(T) join_list_node_2_##T
#define join_list_node3(T) join_list_node_3_##T
#define push_back_list(T) push_back_list_##T
#define pop_front_list(T) pop_front_##T
#define extern_list(T)                                                                                                                 \
    extern_list_node(T);                                                                                                               \
    typedef struct List(T) List(T);                                                                                                    \
    struct List(T)                                                                                                                     \
    {                                                                                                                                  \
        size_t len;                                                                                                                    \
        ListNode(T) root;                                                                                                              \
    };                                                                                                                                 \
    List(T) * alloc_list(T)();                                                                                                         \
    void free_list(T)(List(T) * *list_pp);                                                                                             \
    int push_back_list(T)(List(T) * list_p, T data, void (*copy_constructor)(T * dst_p, const T *src_p), void (*destructor)(T * t_p)); \
    int pop_front_list(T)(List(T) * list_p, T * dst_p, void (*copy_constructor_or_operator_assignment)(T * dst_p, const T *src))

#define def_list_func(T)                                                                                                              \
    def_list_node_func(T);                                                                                                            \
    List(T) * alloc_list(T)()                                                                                                         \
    {                                                                                                                                 \
        List(T) *list_p = NULL;                                                                                                       \
        MALLOC(list_p, 1, sizeof(List(T)));                                                                                           \
        memset(list_p, 0, sizeof(List(T)));                                                                                           \
        list_p->root.prev = &(list_p->root);                                                                                          \
        list_p->root.next = &(list_p->root);                                                                                          \
        return list_p;                                                                                                                \
    }                                                                                                                                 \
    void destruct_list(T)(List(T) * list_p)                                                                                           \
    {                                                                                                                                 \
        if (list_p == NULL)                                                                                                           \
        {                                                                                                                             \
            return;                                                                                                                   \
        }                                                                                                                             \
        ListNode(T) *root_list_node_p = &(list_p->root);                                                                              \
        ListNode(T) *cur = root_list_node_p->next;                                                                                    \
        ListNode(T) *next = NULL;                                                                                                     \
        /*无法应对循环链表被破坏的情况，可能应该直接报错退出，这里选择忽视*/                          \
        while (cur != root_list_node_p && cur != NULL)                                                                                \
        {                                                                                                                             \
            next = cur->next;                                                                                                         \
            FREE_P(cur, destruct_list_node(T));                                                                                       \
            cur = next;                                                                                                               \
        }                                                                                                                             \
    }                                                                                                                                 \
    int join_list_node3(T)(ListNode(T) * first_p, ListNode(T) * secord_p, ListNode(T) * third_p)                                      \
    {                                                                                                                                 \
        int ret = 0;                                                                                                                  \
        if (first_p == NULL || secord_p == NULL || third_p == NULL)                                                                   \
        {                                                                                                                             \
            ret = -1;                                                                                                                 \
            goto end;                                                                                                                 \
        }                                                                                                                             \
        first_p->next = secord_p;                                                                                                     \
        secord_p->prev = first_p;                                                                                                     \
        secord_p->next = third_p;                                                                                                     \
        third_p->prev = secord_p;                                                                                                     \
    end:                                                                                                                              \
        return ret;                                                                                                                   \
    }                                                                                                                                 \
    int join_list_node2(T)(ListNode(T) * first_p, ListNode(T) * secord_p)                                                             \
    {                                                                                                                                 \
        int ret = 0;                                                                                                                  \
        if (first_p == NULL || secord_p == NULL)                                                                                      \
        {                                                                                                                             \
            ret = -1;                                                                                                                 \
            goto end;                                                                                                                 \
        }                                                                                                                             \
        first_p->next = secord_p;                                                                                                     \
        secord_p->prev = first_p;                                                                                                     \
    end:                                                                                                                              \
        return ret;                                                                                                                   \
    }                                                                                                                                 \
    void free_list(T)(List(T) * *list_pp)                                                                                             \
    {                                                                                                                                 \
        FREE_PP(list_pp, destruct_list(T));                                                                                           \
    }                                                                                                                                 \
    int push_back_list(T)(List(T) * list_p, T data, void (*copy_constructor)(T * dst_p, const T *src_p), void (*destructor)(T * t_p)) \
    {                                                                                                                                 \
        ListNode(T) *root_p = &(list_p->root);                                                                                        \
        ListNode(T) *tail_p = root_p->prev;                                                                                           \
        ListNode(T) *list_node_p = alloc_list_node(T)(data, copy_constructor, destructor);                                            \
        int ret = join_list_node3(T)(tail_p, list_node_p, root_p);                                                                    \
        if (ret != 0)                                                                                                                 \
        {                                                                                                                             \
            goto end;                                                                                                                 \
        }                                                                                                                             \
        ++list_p->len;                                                                                                                \
    end:                                                                                                                              \
        return ret;                                                                                                                   \
    }                                                                                                                                 \
    int pop_front_list(T)(List(T) * list_p, T * dst_p, void (*copy_constructor_or_operator_assignment)(T * dst_p, const T *src))      \
    {                                                                                                                                 \
        int ret = 0;                                                                                                                  \
        if (list_p->len == 0)                                                                                                         \
        {                                                                                                                             \
            ret = -1;                                                                                                                 \
            goto end;                                                                                                                 \
        }                                                                                                                             \
        ListNode(T) *root_p = &(list_p->root);                                                                                        \
        ListNode(T) *front_p = root_p->next;                                                                                          \
        ListNode(T) *front_next_p = front_p->next;                                                                                    \
        ret = join_list_node2(T)(root_p, front_next_p);                                                                               \
        if (ret != 0)                                                                                                                 \
        {                                                                                                                             \
            goto end;                                                                                                                 \
        }                                                                                                                             \
        *dst_p = front_p->data;                                                                                                       \
        if (copy_constructor_or_operator_assignment != NULL)                                                                          \
        {                                                                                                                             \
            copy_constructor_or_operator_assignment(dst_p, &(front_p->data));                                                         \
        }                                                                                                                             \
        free_list_node(T)(&front_p);                                                                                                  \
        --list_p->len;                                                                                                                \
    end:                                                                                                                              \
        return ret;                                                                                                                   \
    }

#endif