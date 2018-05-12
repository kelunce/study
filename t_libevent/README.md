```
在xp vs2010中测试
portMapper  服务端
client 客户端
lib及include目录是libevent库及其头文件 其中lib库通过微软nmake生成:
	新建一个include目录，这个文件包含F:\libevent\libevent-2.0.21-stable\include下的文件和\WIN32-Code下的文件
	编译lib库放到lib目录
	cd/d F:\libevent\libevent-2.0.21-stable
	nmake /f Makefile.nmake
项目属性设置：
VC++目录：
包含目录，添加：F:\Projects\LibeventTest\LibeventTest\Include;
库目录，添加：F:\Projects\LibeventTest\LibeventTest\Lib;
C/C++：
代码生成-->运行库：多线程调试 (/MTd)（Debug下），多线程 (/MT)（Release下）
连接器：
输入：ws2_32.lib;wsock32.lib;libevent.lib;libevent_core.lib;libevent_extras.lib;
ws2_32.lib;wsock32.lib;是用来编译Windows网络相关的程序库。
```
