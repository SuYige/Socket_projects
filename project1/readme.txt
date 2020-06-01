支持多线程的文件传输

操作步骤：
1 gcc server.c -o server -lpthread
2 ./server
3 gcc client.c -o client
4 ./client
...
（根据需求运行多个client）
5 选择lookfile/downfile
6 lookfile 查看server下所有文件
7 downfile 下载文件到client
