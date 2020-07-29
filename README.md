# CH552E_IR_USB
CH552E芯片 红外接收 PPT翻页器

有网友提了一些建议：采纳一下  

关于printf输出，可以参考这个帖子  
http://news.eeworld.com.cn/mcu/article_2017011933438.html  
http://www.keil.com/support/man/docs/c51/c51_printf.htm  

KEIL C51 printf格式化输出特殊用法  

KEIL里扩展出了b,h,l来对输入字节宽的设置：  
（1）b八位  
（2）h十六位(默认)  
（3）l三十二位  

在Keil C51中用printf输出一个单字节变量时要使用%bd,如  
unsigned char counter;  
printf(“Current count: %bd\n”, counter);//输出8位”十进制有符号整数”  
printf(“Current count: %bx\n”, counter);//输出8位”无符号以十六进制表示的整数”  
