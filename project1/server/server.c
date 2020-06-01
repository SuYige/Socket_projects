#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <dirent.h>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define MAX_LINE 1024
#define BUFFER_SIZE 1024
#define PORT 8001
#define FILE_NAME 20
#define Path_SIZE 60
#define MAX_NAME 50

struct regis    //用户注册信息
{
	char flag[10];  			//客户端发来的服务器操作信号
	char filename[FILE_NAME];	//客户端发来的上传文件名字
}RecvReg;

struct DIR_FILE     //客户端下载文件时，返回当前文件夹下的所有文件
{
	char Dir_File[NAME_MAX];
}D_File;

void *recvmation(void *argv);	//接收客户端发送过来的信息，线程
//void recvfile(int sfd, char Send_Filename[]);   //接受客户端发来的文件
void sendfile(int sfd, char Down_Filename[]);   //向客户端发送文件

int main()
{
	struct sockaddr_in sin;
	struct sockaddr_in cin;
	int l_fd;
	int c_fd;
	socklen_t len;
	char addr_p[INET_ADDRSTRLEN];
	//int port = 6666;
	int n;
	
	bzero(&sin, sizeof(sin));
	
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = INADDR_ANY;
	sin.sin_port = htons(PORT);
	pthread_t pid;    //线程标识符
	
	if((l_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		perror("fail to create socket!\n");
		exit(1);
	}
	
	if(bind(l_fd, (struct sockaddr *) &sin, sizeof(sin)) == -1)
	{
		perror("fail to bind\n");
		exit(1);
	}
	
	if(listen(l_fd, 10) == -1)
	{
		perror("fail to listen\n");
		exit(1);
	}
	
	printf("waiting.......\n");
	while(1)
	{	
		if((c_fd = accept(l_fd, (struct sockaddr *) &cin, &len)) == -1)
		{
			perror("fail to accept\n");
			continue;
		}
		else
		{
			printf("c_fd = %d\n", c_fd);

			inet_ntop(AF_INET, &cin.sin_addr, addr_p, sizeof(addr_p));
		    	printf("client IP is %s, port is %d\n", addr_p, ntohs(sin.sin_port));
		    
			pthread_create(&pid, NULL, recvmation, (void *)&c_fd);
			pthread_detach(pid);   //设置线程为分离状态
			continue;				//退出本次循环
		}		
	}	
	if(close(c_fd) == -1)
	{
		perror("fail to close\n");
		exit(1);
	}
	
	return 0;
}

void *recvmation(void *arg)	//接收客户端发送过来的信息，线程
{
	int my_fd = *((int *)arg);
	int n;
		
	while(1)
	{	
		n = recv(my_fd, (struct regis *)&RecvReg, sizeof(RecvReg), 0);
		if(n == -1)
		{
			perror("fail1 to receive!\n");
			//exit(1);
		}
		else if(n == 0)
		{
			continue;   //如果客户端断开连接，服务器继续监听接收
		}

		printf("filename:%s\n", RecvReg.filename);
		printf("flag = %s\n", RecvReg.flag);
		printf("my_fd = %d\n", my_fd);

		if(strncmp(RecvReg.flag, "lookfile", 8) == 0)   //客户端从服务器下载文件
		{
			char Down_Filename[FILE_NAME];
			DIR *dp = NULL;
			struct dirent *dirp;
			
			if((dp = opendir("test")) == NULL)    //
				printf("cannot open\n");
			while((dirp = readdir(dp)) != NULL)
			{
				printf("%s\n",dirp->d_name);
				strcpy(D_File.Dir_File, dirp->d_name);
				if(send(my_fd, (struct DIR_FILE *)&D_File, sizeof(D_File), 0) < 0)
					break;
				usleep(1000);		//睡眠函数延迟，这个必须有，因为是非阻塞
				bzero((struct DIR_FILE *)&D_File, sizeof(D_File));			//在这里注意使用清空函数，否则会发生意想不到的错误   
			}
			closedir(dp);
			send(my_fd, "OK", 2, 0);
		}
		if(strncmp(RecvReg.flag, "downfile", 8) == 0)   //客户端从服务器下载文件
		{
			char Down_Filename[FILE_NAME];
			strcpy(Down_Filename, RecvReg.filename);
			sendfile(my_fd, Down_Filename);   //客户端从服务器下载文件
		}
	}
	
	close(my_fd);			//完成所有操作后，关闭该文件描述符
}

void sendfile(int sfd, char Down_Filename[])   //向客户端发送文件
{
	char file_name[FILE_NAME];
	char buffer[BUFFER_SIZE];      
	char Path[Path_SIZE];
	
	strcpy(file_name, Down_Filename);
   
	printf("%s\n", file_name);  
	sprintf(Path, "test/%s", file_name);       //此路径可以根据自己需要修改
	// 打开文件并读取文件数据      
	FILE *fp = fopen(Path, "r");   

	if(NULL == fp)      
	{        
		printf("File:%s Not Found\n", file_name);      
	}      
	else     
	{        
		bzero(buffer, BUFFER_SIZE);        
		int length = 0;        // 每读取一段数据，便将其发送给客户端，循环直到文件读完为止 

		while((length = fread(buffer, sizeof(char), BUFFER_SIZE, fp)) > 0)        
		{          
			printf("length = %d\n", length);
			if(send(sfd, buffer, length, 0) < 0)          
			{            
				printf("Send File:%s Failed.\n", file_name);            
				break;          
			}          
			bzero(buffer, BUFFER_SIZE);        //在这里注意使用清空函数，否则会发生意想不到的错误   
		}           // 关闭文件        
		fclose(fp);   
		sleep(1);//延迟  //睡眠函数延迟，这个必须有，因为是非阻塞
		send(sfd, "OK", 2, 0);   
		printf("File:%s Transfer Successful!\n", file_name);      
	}      // 关闭与客户端的连接      
}

