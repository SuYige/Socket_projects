/************************************************************************* 
 * Filename: udpserver.c
 * Description: udp服务器端代码
 * Author: wgg
 * Date: 2019-5-6
 ************************************************************************/
#include<sys/types.h> 
#include<sys/socket.h> 
#include<unistd.h> 
#include<netinet/in.h> 
#include<arpa/inet.h> 
#include<stdio.h> 
#include<stdlib.h> 
#include<errno.h> 
#include<netdb.h> 
#include<stdarg.h> 
#include<string.h> 
  
#define SERVER_PORT 6666 
#define BUFFER_SIZE 1024 
#define FILE_NAME_MAX_SIZE 512 
  
typedef struct
{ 
  int id; 
  int buf_size; 
}PackInfo; 
  
struct SendPack 
{ 
  PackInfo head; 
  char buf[BUFFER_SIZE]; 
} data; 
  
  
int main() 
{ 

  int send_id = 0; 
  
  int receive_id = 0; 
  
  struct sockaddr_in server_addr; 
  bzero(&server_addr, sizeof(server_addr)); 
  server_addr.sin_family = AF_INET; 
  server_addr.sin_addr.s_addr = htonl(INADDR_ANY); 
  server_addr.sin_port = htons(SERVER_PORT); 
  
  /* 创建socket */
  int server_socket_fd = socket(AF_INET, SOCK_DGRAM, 0); 
  if(server_socket_fd == -1) 
  { 
    perror("Create Socket Failed:"); 
    exit(1); 
  } 
  
  /* 绑定套接口 */
  if(-1 == (bind(server_socket_fd,(struct sockaddr*)&server_addr,sizeof(server_addr)))) 
  { 
    perror("Server Bind Failed:"); 
    exit(1); 
  } 
  
  /* 数据传输 */
  while(1) 
  {   
    for(;;) {/* 并行传输 */
        /* 客户端地址 */
        struct sockaddr_in client_addr;
        socklen_t client_addr_length = sizeof(client_addr);

        char buffer[BUFFER_SIZE];
        bzero(buffer, BUFFER_SIZE);
        if (recvfrom(server_socket_fd, buffer, BUFFER_SIZE, 0, (struct sockaddr *) &client_addr, &client_addr_length) ==
            -1) {
            perror("Receive Data Failed:");
            exit(1);
        }

        /* 从buffer中拷贝出file_name */
        char file_name[FILE_NAME_MAX_SIZE + 1];
        bzero(file_name, FILE_NAME_MAX_SIZE + 1);
        strncpy(file_name, buffer, strlen(buffer) > FILE_NAME_MAX_SIZE ? FILE_NAME_MAX_SIZE : strlen(buffer));
        printf("%s\n", file_name);

        FILE *fp = fopen(file_name, "r");
        if (NULL == fp) {
            printf("File:%s Not Found.\n", file_name);
        } else {
            int len = 0;
            /* 每读取一段数据，便将其发给客户端 */
            while (1) {
                PackInfo pack_info;

                if (receive_id == send_id) {
                    ++send_id;
                    if ((len = fread(data.buf, sizeof(char), BUFFER_SIZE, fp)) > 0) {
                        data.head.id = send_id;
                        data.head.buf_size = len; /* 数据长度 */
                        if (sendto(server_socket_fd, (char *) &data, sizeof(data), 0, (struct sockaddr *) &client_addr,
                                   client_addr_length) < 0) {
                            perror("Send File Failed:");
                            break;
                        }
                        /* 接收确认消息 */
                        recvfrom(server_socket_fd, (char *) &pack_info, sizeof(pack_info), 0,
                                 (struct sockaddr *) &client_addr, &client_addr_length);
                        receive_id = pack_info.id;
                    } else {
                        break;
                    }
                } else {
                    /* 如果接收的id和发送的id不相同,重新发送 */
                    if (sendto(server_socket_fd, (char *) &data, sizeof(data), 0, (struct sockaddr *) &client_addr,
                               client_addr_length) < 0) {
                        perror("Send File Failed:");
                        break;
                    }
                    /* 接收确认消息 */
                    recvfrom(server_socket_fd, (char *) &pack_info, sizeof(pack_info), 0,
                             (struct sockaddr *) &client_addr, &client_addr_length);
                    receive_id = pack_info.id;
                }
            }

            fclose(fp);
            printf("File:%s Transfer Successful!\n", file_name);
        }
    }
  } 
  close(server_socket_fd); 
  return 0; 
} 
