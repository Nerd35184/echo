# echo

## 总体介绍
学习网络编程相关过程中，采用纯c语言实现的echo服务器，包好一个简单的 echo server 以及 client。采用经典的 `one loop per thread + thread pool` 模式。

## 文件介绍

| file      | Description |
| ----------- | ----------- |
| client.c      | 一个简单的客户端       |
| server.c   | 服务端入口文件，一个服务器控制多个worker，并在worker之间调度        |
| util.c/h   | 一些工具函数的封装，以及一些共用的常量，比如free malloc        |
| makefile   | makefile        |
| worker.c/h   |  工作线程，实现epoll多路io复用        |
| worker_test.c   |  单元测试        |
| task.c/h   |  工作类，worker持有一个task列表，按顺序执行task        |
| list.h   |  泛型双向环状链表        |
| list_test.h   |    单元测试      |
| pair.h   |    泛型键值对类，本来想要在map中使用，后因精力有限，未排上用场      |
| pair_test.h   |    单元测试      |

