#ifndef __PAIR_H__
#define __PAIR_H__

#include "util.h"

#define Pair(T1, T2) pair_##T1_##T2
#define alloc_pair(T1, T2) alloc_pair_##T1_##T2
#define free_pair(T1, T2) free_pair_##T1_##T2
#define destruct_pair(T1, T2) destruct_pair_##T1_##T2
#define extern_pair(T1, T2)                                                                                                     \
    typedef struct Pair(T1, T2)                                                                                                 \
    {                                                                                                                           \
        T1 first;                                                                                                               \
        T2 second;                                                                                                              \
        void (*t1_destructor)(T1 * first_p);                                                                                    \
        void (*t2_destructor)(T2 * second_p);                                                                                   \
    }                                                                                                                           \
    Pair(T1, T2);                                                                                                               \
    Pair(T1, T2) * alloc_pair(T1, T2)(T1 t1, void (*t1_destructor)(T1 * first_p), void (*init_t1)(T1 * dst_p, const T1 *src),   \
                                      T2 t2, void (*t2_destructor)(T2 * second_p), void (*init_t2)(T2 * dst_p, const T2 *src)); \
    void free_pair(T1, T2)(Pair(T1, T2) * *pair_pp);
#define def_pair_func(T1, T2)                                                                                                  \
    Pair(T1, T2) * alloc_pair(T1, T2)(T1 t1, void (*t1_destructor)(T1 * first_p), void (*init_t1)(T1 * dst_p, const T1 *src),  \
                                      T2 t2, void (*t2_destructor)(T2 * second_p), void (*init_t2)(T2 * dst_p, const T2 *src)) \
    {                                                                                                                          \
        Pair(T1, T2) * pair_p;                                                                                                 \
        MALLOC(pair_p, 1, sizeof(Pair(T1, T2)));                                                                               \
        pair_p->first = t1;                                                                                                    \
        pair_p->second = t2;                                                                                                   \
        pair_p->t1_destructor = t1_destructor;                                                                                 \
        pair_p->t2_destructor = t2_destructor;                                                                                 \
        init_t1(&(pair_p->first), &t1);                                                                                        \
        init_t2(&(pair_p->second), &t2);                                                                                       \
        return pair_p;                                                                                                         \
    }                                                                                                                          \
    void destruct_pair(T1, T2)(Pair(T1, T2) * pair_p)                                                                          \
    {                                                                                                                          \
        if (pair_p == NULL)                                                                                                    \
        {                                                                                                                      \
            goto end;                                                                                                          \
        }                                                                                                                      \
        if (pair_p->t1_destructor != NULL)                                                                                     \
        {                                                                                                                      \
            pair_p->t1_destructor(&(pair_p->first));                                                                           \
        }                                                                                                                      \
        if (pair_p->t2_destructor != NULL)                                                                                     \
        {                                                                                                                      \
            pair_p->t2_destructor(&(pair_p->second));                                                                          \
        }                                                                                                                      \
                                                                                                                               \
    end:                                                                                                                       \
        return;                                                                                                                \
    }                                                                                                                          \
    void free_pair(T1, T2)(Pair(T1, T2) * *pair_pp)                                                                            \
    {                                                                                                                          \
        FREE_PP(pair_pp, destruct_pair(T1, T2));                                                                               \
    }

#endif