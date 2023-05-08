# some-os

事操作系统，名字还没想好。  
目标是在真机上跑，还能读磁盘文件系统的那种。  
我把默认的limine bootloader的readme删啦  

这里就简单记录一下历史吧  
> `23-05-05 20:27   printf   319164c0311ac833e6f5ef4d7cd74754533c3dd9`  

+ 一个简单的printf，输出就写到这里吧。  


> `23-05-05 11:41    unicode字符串输出   48933d33aea70193ac997e048849faeed9a0c91` 

+ ​kernel/font/a.py用于把bdf字体弄到数组里。
+ output.c实现了putchar和puts。
+ utf-8编码。