 
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
#include<errno.h>
#include<signal.h>
#include<strlib.h>
#include<ctype.h>


#define MEMBER 10
#define MAX 100

struct mypara{
    int sockfd1;
    int sockfd2;
};
void make_server(int socket, int portnum);
void*  recv_and_send(void* arg);
void if_error(char* msg);
int catch_signal(int sig,void(*handler)(int));
void handle_shutdown(int sig); // ctrl+c后，在程序结束前关闭套接字
void* one_fds(void* arg);
void* one_fdf(void* arg);
int find_socket(char * buf);
void* message(void*arg);
void print_menu(int client_id);
void print_history(string history_id,int socket);
void save_history(string history_id,char* buf);

int socket_id;                //服务器socket
int user_socket[MEMBER];      //记录由accept（）函数得到的客户端的socketfd,10个元素
char* user_name[MEMBER];      //记录由客服端返传回来的用户名
int flag[MEMBER];             //记录客户端是否进入点对点聊天，值为1,返回忙碌；值为0,返回空闲
char* IP = "127.0.0.1";
string history_id[MEMBER]={"0.txt","1.txt","2.txt","3.txt","4.txt","5.txt","6.txt","7.txt","8.txt","9.txt"};
string dir="/root/socket/history";
//char* IP ="192.168.31.212";
//char user[20] = {"no use\0"};
//char* IP = "192.168.43.184";

 
int main()
{
     if(catch_signal(SIGINT,handle_shutdown) == -1) {
        if_error("Can't set the interrupt handle");
     }
     
     /*登录成员信息记录部分初始化*/
     for(int i; i<MEMBER; i++) {
         user_name[i] =NULL;
         user_socket[i] = 0;
         flag[i] = 0;
    }
     
     int portnum = 10000;
     socket_id = socket(PF_INET,SOCK_STREAM,0);
     make_server(socket_id, portnum);
     printf("Waitting for connection......\n");
     printf("--------------------------------------- \n");

     while(1){
        struct sockaddr_storage client_addr;
        unsigned int address_size = sizeof(client_addr);
        int client_id = accept(socket_id,(struct sockaddr*)&client_addr,&address_size);
        if (client_id == -1){
            printf("客户端连接失败\n");
            continue;
        }
        int i;
        for (i = 0;i < MEMBER;i++){
            if (user_socket[i] == 0 ){
                user_socket[i] = client_id;
                printf("client_id = %d\n",client_id);
                /*进入多人聊天室*/
                pthread_t tid;
                pthread_create(&tid,0,recv_and_send,&client_id);
                break;
            }
        if (MEMBER == i){
            char* str = "对不起，聊天室已经满了!";
            send(client_id,str,strlen(str),0);
            close(client_id);
        }
       }
        }
}

void make_server(int socket, int portnum)
{
    if (socket_id == -1){
        if_error("创建socket失败");

    }
    struct sockaddr_in name;
    name.sin_family = AF_INET;
    name.sin_port = htons(portnum);
    name.sin_addr.s_addr = inet_addr(IP);
    if (bind(socket_id,(struct sockaddr*)&name,sizeof(name)) == -1){
        if_error("无法设置套接字");
    }//把套接字绑定在30000端口
    if (listen(socket_id,10) == -1){
        if_error("Can't listen");
    }//把监听队列对长设置为10
}

void print_menu(int client_id)
{
    char msg_f[50]={"用户名 \t套接字 \tflag\r\t"};
    send(client_id,msg_f,sizeof(msg_f),0);
    for(int i = 0; i < MEMBER;i++) {
        char msg[50] = {'\0'};
        sprintf(msg,"%s \t%d \t%d\r\n",user_name[i],user_socket[i],flag[i]);
        send(client_id,msg,strlen(msg),0);
    }
}
void* one_fdf(void* arg)                                //处理sockfd1对应的套接字符
{
    struct mypara* L;
    L = (struct mypara*)arg;
    char buf1[MAX];
    int len1 = sizeof(buf1);
    while(1){
       recv(L->sockfd1,buf1,len1,0);
       if(FindString("history",buf1,0)!=-1){
          char a[50]={"---------------------------- \n"};
          send(L->sockfd1 ,a,strlen(a),0);
          print_history(history_id[L->sockfd1-4],L->sockfd1);
        }else if(FindString("see you",buf1,0) != -1){
           send(L->sockfd2,buf1,strlen(buf1),0);
           return NULL;
       }else{
           send(L->sockfd1,buf1,strlen(buf1),0);
           send(L->sockfd2,buf1,strlen(buf1),0);
       }
       
       /*打印登录成员信息*/
       if(FindString("user menu",buf1,0) != -1  ){
             print_menu(L->sockfd1);
       }
       
       /*保存历史记录*/
       save_history(history_id[L->sockfd1-4],buf1);
       
      memset(buf1,0,sizeof(buf1));
    }
}


void* one_fds(void*arg)                                 //处理sockfd2对应的套接字符
{
    struct mypara* L;
    L = (struct mypara*)arg;
    char buf2[MAX];
    int len2 = sizeof(buf2);
    while(1){
       recv(L->sockfd2,buf2,len2,0);
       if(FindString("history",buf2,0)!=-1){
           char a[50]={"---------------------------- \n"};
           send(L->sockfd2,a,strlen(a),0);
           print_history(history_id[L->sockfd1-4],L->sockfd2);
        }else if(FindString("see you",buf2,0) != -1){
            send(L->sockfd1,buf2,strlen(buf2),0);
            return NULL;
       }else{
           send(L->sockfd1,buf2,strlen(buf2),0);
           send(L->sockfd2,buf2,strlen(buf2),0);
       }
      
      /*打印登录成员信息*/
      if(FindString("user menu",buf2,0) != -1  ){
             print_menu(L->sockfd2);
            } 
       /*保存历史记录*/     
       save_history(history_id[L->sockfd1-4],buf2);  
       memset(buf2,0,sizeof(buf2));
    }
}


int find_socket(char * buf)                             //提取“connect socket”中的socket值
{       
        int id;
        char* myargv[100];
        char*start = buf;
        int index = 1;
        while((*start==' ')){
            start++;
        }
        myargv[0] = start;
        while(*start){
            if(isspace(*start)){
               *start = '\0';
               start++;
               while((*start==' '&&*start)){
                    start++;
               }
               myargv[index] = start;
               index++;
            }else {
               start++;
            }
        }
        myargv[index] = NULL;
        id = atoi(myargv[8]);                           //将char型转化为int型
        return id;
}


pthread_mutex_t selection_lock = PTHREAD_MUTEX_INITIALIZER;
void*  recv_and_send(void* arg)
{
     int client_id = *(int*)arg;
     pthread_mutex_lock(&selection_lock);
     char buf1[10];
     memset(buf1,0,sizeof(buf1));
     
     /*记录成员登录名*/
     recv(client_id,buf1,sizeof(buf1),0);
     for(int i =0;i < MEMBER; i++){
        if((user_socket[i] == client_id) && (user_name[i] == NULL)){
            user_name[i] = buf1;
         }
      }
     pthread_mutex_unlock(&selection_lock);
     
     char buf[MAX];
     int len = sizeof(buf);
     while(1){
         FILE *fd;
         if((fd=fopen("/root/socket/history/history.txt","a"))==NULL)
         perror("stream open() failed");
         int c = recv(client_id,buf,len,0);
         if(c<0){                                         //读取错误
             for (int i = 0;i < MEMBER;i++){
                if (client_id == user_socket[i]){
                    user_socket[i] = 0;
                    user_name[i] = '\0';
                    return NULL;
                }
             }
         }else if(c == 0){
             buf[0] = '\0';                               // '\0'起\r的作用
         }else{ 
             printf("%s",buf);
         }
         fputs(buf,fd);  //写入历史信息
         /*打印登录成员信息*/
         if(FindString("user menu",buf,0) != -1  ){
            print_menu(client_id);
         }
         /*查看历史信息*/
         if(FindString("history",buf,0)!=-1){
             FILE *fd;
             size_t len=0;
             ssize_t read;
             char* line=NULL;
             if((fd=fopen("/root/socket/history/history.txt","r"))==NULL)
             perror("fd open() failed");
             char a[50]={"---------------------------- \n"};
             send(client_id,a,strlen(a),0);
             while((read=getline(&line,&len,fd))!=-1){
                send(client_id,line,strlen(line),0);
                }
            fclose(fd);
          }
         if(FindString("connect",buf,0) != -1){
            int id = find_socket(buf);                        //将char型转化为int型
            for(int i; i < MEMBER; i++){
                if(id == user_socket[i] && flag[i] == 1){
                    char buf4[100] ={"忙碌，该客户正在和其他客户私聊!\n"};
                    send(client_id,buf4,strlen(buf4),0);
                    break;
                }
                if(id == user_socket[i] && flag[i] == 0){
                    flag[i] = 1;                               //id记录为忙碌
                    for(int i; i < MEMBER; i++){
                       if(client_id == user_socket[i]){
                          flag[i] = 1;
                       }  
                    }
                     struct mypara p;
                    p.sockfd1 = client_id;
                    p.sockfd2 = id;
                    void* result1;
                    void* result2;
                    pthread_t tid1;
                    pthread_t tid2;
                    pthread_create(&tid1,NULL,one_fdf,&(p));
                    pthread_create(&tid2,NULL,one_fds,&(p));
                    if(pthread_join(tid1,&result1) == 0 || pthread_join(tid2,&result2) == 0){ //点对点聊天结束
                       for(int i; i < MEMBER; i++){
                            if(p.sockfd1 ==user_socket[i]){
                                flag[i] = 0;
                            }
                            if(p.sockfd2 == user_socket[i]){
                                flag[i] = 0;
                            }
                       }
                    }
                    break;
                }
            }
         }
         
        if(FindString("bye",buf,0) != -1){
               return NULL;
        }else{
            for (int i = 0;i < MEMBER;i++){
               if (user_socket[i] != 0 && flag[i] == 0){
                 send(user_socket[i],buf,strlen(buf),0);
               }
           }
           memset(buf,0,sizeof(buf));
           fclose(fd);
        }
        

    }

   
}


void if_error(char* msg)                               //报错函数
{
    fprintf(stderr, "%s: %s\n",msg,strerror(errno));   //保存服务器的主监听套接字
    exit(1);
}
void handle_shutdown(int sig)                          // ctrl+c后，在程序结束前关闭套接字
{
    if(socket_id){
        close(socket_id);
    }
    fprintf(stderr,"Bye!\n");
    exit(0);
}
int catch_signal(int sig,void(*handler)(int))
{
    struct sigaction action;
    action.sa_handler = handler;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    return sigaction(sig, &action, NULL);
}
void save_history(string history_id,char* buf)
{
    FILE *stream;
    if((stream=fopen(history_id,"a"))==NULL)
        perror("stream open() failed");
    fputs(buf,stream);
    fclose(stream);
}

void print_history(string history_id,int socket)
{
    FILE *fd;
    size_t len=0;
    ssize_t read;
    char* line=NULL;
    if((fd=fopen("history_id","r"))==NULL)
        perror("fd open() failed");
        while((read=getline(&line,&len,fd))!=-1){
            send(socket,line,strlen(line),0);
        }
    fclose(fd);
}
