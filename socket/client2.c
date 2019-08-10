#include<stdio.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<string.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<netdb.h>
#include<sys/types.h>
#include<errno.h>
#include<stdlib.h>
#include<signal.h>
#include<pthread.h>
#include<strlib.h>

#define MAX 100
#define MEMBER 10


int make_client(int socket,int portnum);
void* recv_from_server(void*arg);
void if_error(char* msg);

int socket_id; 
char name[MEMBER];
char* IP = "127.0.0.1";
//char* IP = "192.168.43.184";
//char* IP = "192.168.31.233";
int main()
{  
    int portnum = 10000;
    socket_id = socket(PF_INET,SOCK_STREAM,0);
    make_client(socket_id,portnum);
    printf("欢迎登录！\n");
    pthread_t pt;                                                                
    pthread_create(&pt,0,recv_from_server,0);
    char buf1[MAX];
    printf("你的登录名： \n");
    scanf("%s",buf1);
    send(socket_id,buf1,strlen(buf1),0); //向服务器传入客户名
    while(1){
        char buf2[MAX];
        /*从键盘中读取数据*/
        ssize_t _s = read(0, buf2, sizeof(buf2)-1);
		if(_s >0)
		{
			buf2[_s-1] ='\0';
		}
		else
		{
			printf("read!\n");
			return 1;
		}
		
		/*时间部分*/
        struct tm *timeptr; 
        time_t timeval;
        char tm[50];
        (void)time(&timeval);
        
        char msg[MAX];
        sprintf(msg,"\033[31m%s   \033[32m%s \n\033[33m(#_#):%s\n",buf1,ctime(&timeval),buf2);
        send(socket_id,msg,strlen(msg),0);
        if (strcmp(buf2,"bye") == 0){
            memset(buf2,0,sizeof(buf2));
            sprintf(buf2,"%s:bye\n退出了聊天室",buf1);
            send(socket_id,buf2,strlen(buf2),0);  
            break;
        }
            
        if(strcmp(buf2,"see you") == 0){
            memset(buf2,0,sizeof(buf2));
            sprintf(buf2,"see you.对方退出了点对点聊天，空闲,",buf1); //自己退出点对点聊天
            printf("%s",msg);
            send(socket_id,buf2,strlen(buf2),0);  
        }
    memset(msg,0,sizeof(msg));
    }

    return 0;
}

int make_client(int socket,int portnum)
{
    if (socket == -1){
        if_error("创建socket失败");
    }
    struct sockaddr_in name;
    memset(&name,0,sizeof(name));
    name.sin_family = AF_INET;
    name.sin_port = htons(portnum);  //把机器整型数转变为网络字节顺序
    name.sin_addr.s_addr = inet_addr(IP);
    if(connect(socket,(struct sockaddr*)&name, sizeof (name)) == -1){
        if_error("登录失败!\n");
    } //把套接字绑定在30000端口
    printf("登录成功!\n");
}

void* recv_from_server(void*arg)
{   
      while(1){
        char buf[50] = {};
        int c = recv(socket_id,buf,sizeof(buf),0);
        if(c <= 0){ //从TPC端接受数据
            return NULL;
        }
        printf("%s\n",buf);
        }
}

void if_error(char* msg) //报错函数
{
    fprintf(stderr, "%s: %s\n",msg,strerror(errno)); //保存服务器的主监听套接字
    exit(1);
}
