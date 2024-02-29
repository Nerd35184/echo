#include"pair.h"


typedef struct Test1
{
    int i;
} Test1_t;

void test1_copy_construct(Test1_t *this, const Test1_t *src)
{
    printf("test_copy_construct src:%d\n", src->i);
    this->i = src->i;
}

void test1_destructor(Test1_t *this)
{
    printf("test_destructor %d\n", this->i);
}


typedef struct Test2
{
    int i;
} Test2_t;


void test2_copy_construct(Test2_t *this, const Test2_t *src)
{
    printf("test2_copy_construct src:%d\n", src->i);
    this->i = src->i;
}

void test2_destructor(Test2_t *this)
{
    printf("test2_destructor %d\n", this->i);
}


extern_pair(Test1_t,Test2_t);
def_pair_func(Test1_t,Test2_t);


int main(){
    Test1_t t1 = {i:10};
    Test2_t t2 = {i:20};
    Pair(Test1_t, Test2_t)* pair_p =  alloc_pair(Test1_t,Test2_t)(t1,test1_destructor,test1_copy_construct,t2,test2_destructor,test2_copy_construct);
    free_pair(Test1_t,Test2_t)(&pair_p);
    return 0;
}