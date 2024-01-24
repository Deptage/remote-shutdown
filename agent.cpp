#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstdlib>
#include <cstring>
#define MAX_MSG_SIZE 10

int _read(int cfd, char *buf, int bufsize);

int main(int argc, char* argv[]) {
    int port = 1697;
    if(argc>1){
        port=std::stoi(argv[1]);
    }
    socklen_t sl;
    struct sockaddr_in saddr, caddr;
    memset(&saddr,0,sizeof(saddr));
    saddr.sin_family=AF_INET;
    saddr.sin_addr.s_addr=INADDR_ANY;
    saddr.sin_port=htons(port);

    int sfd=socket(PF_INET,SOCK_STREAM,IPPROTO_TCP);
    if(sfd==-1){
        perror("socket creating error!");
        exit(1);
    }
    int err=bind(sfd,(struct sockaddr*)&saddr,sizeof(saddr));
    if(err==-1){
        perror("bind error!");
        exit(1);
    }
    err=listen(sfd,1);
    if(err==-1){
        perror("listen error!");
        exit(1);
    }
    std::cout<<"Agent is listening on a port "<<port<<"..."<<std::endl;
    while(1){
        sl=sizeof(caddr);
        int cfd=accept(sfd, (struct sockaddr*)&caddr, &sl);
        if(cfd==-1){
            perror("accept error!");
            exit(1);
        }
        char buf[MAX_MSG_SIZE]={};
        //int bytes_read=0;
        //int res;
        _read(cfd,buf,MAX_MSG_SIZE);
        //std::cout<<"Przeczytane bajty: "<<bytes_read<<std::endl;
        std::string msg=buf;
        //std::cout<<msg<<std::endl;
        //buf[8]='\0';
        //char command[9]="shutdown";
        //char status[7]="status";
        std::cout<<"Odczytano wiadomosc od glownego serwera: "<<msg<<std::endl;
        //for(int ii=0; ii<msg.size();ii++){
        //    std::cout<<msg[ii]<<std::endl;
        //}
        //std::cout<<strcmp(buf,command)<<std::endl;
        //if(strcmp(buf,command)==0){
        if(msg.substr(0,2)=="sd"){
            std::cout<<"Otrzymano rozkaz shutdown!"<<std::endl;
            write(cfd,"a\n",2);
            close(cfd);
            close(sfd);
            //system("shutdown -h now");
            exit(1);
        }else if(msg.substr(0,2)=="st"){
            std::cout<<"Otrzymano rozkaz status!"<<std::endl;
            write(cfd,"a\n",2);
        }
        close(cfd);
    }
    close(sfd);
    return 0;
}

int _read(int cfd, char *buf, int bufsize) {
    int bytesRead = 0;
    char *bufPtr = buf;
    char currentChar;
    int readResult;

    while (bytesRead<bufsize-1) {
        readResult = read(cfd,&currentChar,1);
        if (readResult<=0) {
            if (readResult==0) {
                break;
            }else{
                perror("error reading!!!!");
                break;
            }
        }
        *bufPtr=currentChar;
        bufPtr++;
        bytesRead++;
        if(currentChar=='\n') {
            break;
        }
    }

    *bufPtr = '\0';
    return bytesRead;
}
