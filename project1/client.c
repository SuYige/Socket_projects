#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <netinet/in.h> 
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define MAX_LINE 100
#define BUFFER_SIZE 1024   
#define FILE_NAME 20
#define MAX_NAME 50

struct regis    //用户注册信息
{
	char flag[10];
	char file_name[FILE_NAME];
}SendReg;

struct DIR_FILE    //服务器当前文件夹下所有文件信息
{
	char Dir_File[MAX_NAME];
}D_File;

void recvfile(int client_socket_fd, char Down_Filename[])		//客户端从服务器下载文件
{    
	char file_name[FILE_NAME];
	char buffer[BUFFER_SIZE];
	        
	strcpy(file_name, Down_Filename);         
	// 向服务器发送buffer中的数据    

	FILE *fp = fopen(file_name, "w");    
	if(NULL == fp)    
	{      
		printf("File:\t%s Can Not Open To Write\n", file_name);      
		exit(1);    
	}       
	// 从服务器接收数据到buffer中    
	// 每接收一段数据，便将其写入文件中，循环直到文件接收完并写完为止    
	bzero(buffer, BUFFER_SIZE);    
	int length = 0;  
	
	while((length = recv(client_socket_fd, buffer, BUFFER_SIZE, 0)) > 0)    
	{      
		printf("length = %d\n", length);
		if(strcmp(buffer, "OK") == 0)
		{
			break;
		}
		if(fwrite(buffer, sizeof(char), length, fp) < length)      
		{        
			printf("File:\t%s Write Failed\n", file_name);        
			break;      
		}      
		bzero(buffer, BUFFER_SIZE);    
	}       // 接收成功后，关闭文件，关闭socket    
	printf("Receive File:\t%s From Server IP Successful!\n", file_name);    
	fclose(fp);  
	printf("client_socket_fd = %d\n", client_socket_fd);  
}

void Login_Menu()  //登录成功后选择界面
{
	printf("\n\n");
	printf("\t----lookfile--查看----\n");
	printf("\t----downfile--下载----\n");
	
	printf("\n请选择\n");
}

int main(int argc, char *argv[])
{
	struct sockaddr_in sin;
	char buf[MAX_LINE];
	int s_fd;
	int port = 8001;
	int n;
	
	bzero(&sin, sizeof(sin));
	sin.sin_family = AF_INET;
	inet_pton(AF_INET, "127.0.0.1", &sin.sin_addr);
	sin.sin_port = htons(port);

	if((s_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)  //socket
	{
		perror("fail to creat socket\n");
		exit(1);
	}
	
	if(connect(s_fd, (struct sockaddr *) &sin, sizeof(sin)) == -1)//connect
	{
		perror("fail to connect\n");
		exit(1);
	}
	while(1)
	{			
		char Filename[10];
		//char Send_Filename[BUFFER_SIZE];
		char Down_Filename[BUFFER_SIZE];
		
		Login_Menu();  //登录成功后选择界面
		
		scanf("%s", Filename);
		printf("\n");

		if(strncmp(Filename, "lookfile", 8) == 0)   //查看文件从服务器
		{
			strcpy(SendReg.flag, "lookfile");
			n = send(s_fd, (struct regis *)&SendReg, sizeof(SendReg), 0);
			if(n == -1) 
			{  
				perror("fail to send\n"); 
				exit(1); 
			}
			int length = 0;
			int n = 0;
			printf("当前文件夹下的所有文件如下：\n\n");
			while((length = recv(s_fd, (struct DIR_FILE *)&D_File, sizeof(D_File), 0)) > 0)
			{
				++n;
				if(strcmp(D_File.Dir_File, "OK") == 0)
					break;
				printf("[ %s ]", D_File.Dir_File);
				if(n % 5 == 0)					
					printf("\n");
			}
		}
		if(strncmp(Filename, "downfile", 8) == 0)   //下载文件从服务器
		{
			strcpy(SendReg.flag, "downfile");
			printf("Please Input File Name Downfile On Server:\t");    
			scanf("%s", SendReg.file_name); 
			n = send(s_fd, (struct regis*)&SendReg, sizeof(SendReg), 0);
			if(n == -1) 
			{  
				perror("fail to send\n"); 
				exit(1); 
			}
			strcpy(Down_Filename, SendReg.file_name);
			recvfile(s_fd, Down_Filename);	
		}						
	}
	if(close(s_fd) == -1)
	{
		perror("fail to close\n");
		exit(1);
	}
	
	return 0;
}


