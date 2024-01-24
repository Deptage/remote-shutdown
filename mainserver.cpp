#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstdlib>
#include <vector>
#include <memory>
#include <mutex>
#include <arpa/inet.h>
#include <sstream>
#include <algorithm>
#define MAX_MSG_TO_AGENT_SIZE 10
#define MAX_MSG_TO_CLIENT_SIZE 50
#define MAX_CLIENTS 10
using namespace std;

struct Agent{
    //int id;
    string aName;
    //int afd;
    struct sockaddr_in agent_addr;
    bool isOn;
};

struct Client{
    //int id;
    int cfd;
    string name;
    struct sockaddr_in caddr;
    vector<Agent> authorizedAgents;
};

int _write(int cfd, char *buf, int len){
    while (len>0){
        int i = write(cfd, buf, len);
        len -= i;
        buf +=i;
    }
    return 0;
}

int _read(int cfd, char *buf, int bufsize) {
    int bytesRead = 0;
    char *bufPtr = buf;
    char currentChar;
    int readResult;

    while (bytesRead < bufsize - 1) {
        readResult = read(cfd, &currentChar, 1);
        if (readResult <= 0) {
            if (readResult == 0) {
                break;
            } else {
                perror("error reading!!!!");
                break;
            }
        }
        *bufPtr = currentChar;
        bufPtr++;
        bytesRead++;
        if (currentChar == '\n') {
            break;
        }
    }

    *bufPtr = '\0';
    return bytesRead;
}


void* cthread(void* arg){
    Client* c = (struct Client*)arg;
    cout<<"In thread. Client data:"<<endl<<"cfd: "<<c->cfd<<endl;
    printf("[%lu] new connection from: %s:%d\n", (unsigned long int)pthread_self(), inet_ntoa((struct in_addr)c->caddr.sin_addr), ntohs(c->caddr.sin_port));
    char msg_to_c[50];
    //Client should have a menu on its machine. It should be able to ask if the client wants to have a new machine installed.
    //Also, it should be able to ask for statuses. We only send raw data. So we have those types of requests:
    //1. Create a new agent    command: "cn <agentname> <adres IP> <port>"
    //2. Shut down an agent    command: "sd <agentname>"
    //3. Check all agents status  command "st"
    //So let's start with creating a new agent I guess. To create a new agent, we need an IPv4 address and also a port. I think we should also be able to name the machine.

    //BUT FIRST: LET'S WRITE TO THE CLIENT A BANNER OR STH!
    strcpy(msg_to_c, "Zuzanna's remote shutdown!\n");
    _write(c->cfd,msg_to_c,strlen(msg_to_c));
    while(1){
        char msg_from_c[MAX_MSG_TO_CLIENT_SIZE]={};

        _read(c->cfd,msg_from_c,MAX_MSG_TO_CLIENT_SIZE);

        //printf("%s",msg_from_c);
        int cmd=-1;

        string message(msg_from_c);//it has endline at the end
        cout<<message;

        string command=message.substr(0,2);
        if(command=="cn") cmd=0;
        else if(command=="sd") cmd=1;
        else if(command=="st") cmd=2;
        else cout<<"Invalid message"<<endl;

        //CREATE NEW AGENT
        if(cmd==0){
            istringstream iss(message);
            string command, agentName, ip, portStr;

            
            getline(iss, command, ' ');
            getline(iss, agentName, ' ');
            getline(iss, ip, ' ');
            getline(iss, portStr, '\n');
            int port = stoi(portStr);

            cout << "Command: " << command << ", Agent Name: " << agentName << ", IP: " << ip << ", Port: " << port << std::endl;
            Agent a;
            int err = inet_pton(AF_INET, ip.c_str(), &(a.agent_addr.sin_addr));
            if(err <= 0) {
                perror("ip address cast error");
                exit(1);
            }

            a.aName = agentName;
            a.agent_addr.sin_port = htons(port);
            a.agent_addr.sin_family = AF_INET;
            c->authorizedAgents.push_back(a);  // Assuming c->authorizedAgents is a collection of Agent objects
        }
        if(cmd==1){
            cout<<"Message in cmd: "<<message<<endl;
            istringstream iss(message);
            string command, agentName;   
            getline(iss, command, ' ');
            getline(iss, agentName, '\n');
            agentName.erase(remove_if(agentName.begin(), agentName.end(), [](unsigned char c) { return !std::isalnum(c); }), agentName.end());
            cout<<"Command: " << command<<", agentname: "<<agentName<<endl;
            int found=0;
            for(auto& it : c->authorizedAgents) {
                cout<<agentName.size()<<" "<<it.aName.size()<<endl;
                for(int ii = 0; ii<max(agentName.size(),it.aName.size());ii++){
                    cout<<agentName[ii]<<" "<<it.aName[ii]<<endl;
                }
                cout << agentName << endl;
                cout << it.aName << endl;
                if(it.aName == agentName) {
                    found = 1;
                    // agentStatus(it.agent_addr);
                    int afd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
                    connect(afd, (struct sockaddr*)&it.agent_addr, sizeof(it.agent_addr));
                    write(afd, "sd\0\n", 4);
                    cout << "Sent shutdown to the agent!" << endl;
                    break;
                }
            }
            if(found==0){
                //no agent found!
                cout<<"No agent found!"<<endl;
            }
        }
        if(cmd == 2) {
            char st_msg[3];
            for(auto it : c->authorizedAgents) {
                int afd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
                connect(afd, (struct sockaddr*)&it.agent_addr, sizeof(it.agent_addr));
                _write(afd, "st\0\n", 4);
                _read(afd,st_msg,3);
                string response=st_msg;
                cout<<"Agent's response: "<<response<<endl;
            }
        }
    }


    close(c->cfd);
    delete c;
    return nullptr;
}

int main(){
    int sfd=socket(PF_INET,SOCK_STREAM,IPPROTO_TCP);
    if(sfd==-1){
        perror("error creating socket!");
        exit(1);
    }
    struct sockaddr_in saddr;
    saddr.sin_family=AF_INET;
    saddr.sin_addr.s_addr=INADDR_ANY;
    saddr.sin_port=htons(1444);
    int err=bind(sfd,(struct sockaddr*)&saddr,sizeof(saddr));
    if(err==-1){
        perror("error binding!");
        exit(1);
    }
    err=listen(sfd,MAX_CLIENTS);
    if(err==-1){
        perror("error listen!");
        exit(1);
    }
    socklen_t sl;
    while(1){
        struct Client* c = new Client();
        sl=sizeof(c->caddr);
        c->cfd=accept(sfd,(struct sockaddr*)&c->caddr,&sl);
        if(c->cfd==-1){
            perror("error accepting!");
            exit(1);
        }

        pthread_t tid;
        pthread_create(&tid,NULL,cthread,c);
        pthread_detach(tid);
    }
    return 0;
}
