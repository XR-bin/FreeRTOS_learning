# FreeRTOS系统配置文件详解

------



### 1、FreeRTOSConfig.h配置文件作用

答：对FreeRTOS进行功能配置和裁剪，以及API函数的使能。



### 2、学习途径

1. 官方的在线文档中有详细说的说明：https://www.freertos.org/a00110.html 。
2. 正点原子《FreeRTOS开发指南》第三章的内容 --- FreeRTOS系统配置。



### 3、配置文件中相关宏的分类

答：相关宏大致可以分为三类。

- **‘INCLUDE’开头**  ---  配置FreeRTOS中可选的API函数。
- **’config‘开头**  ---  完成FreeRTOS的功能配置和裁剪(如调度方式、使能信号量功能等)。
- **其他配置**  ---  PendSV宏定义、SVC宏定义。

------

