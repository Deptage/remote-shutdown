#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <sstream>
#include <arpa/inet.h>
using namespace std;

void displaymenu(){
    cout<<"ZDALNE WYLACZANIE SYSTEMOW OPERACYJNYCH"<<endl;
    cout<<"Wpisz: 'cn <nazwa agenta alfanumeryczna> <adres IP> <port>' by dodac agenta!"<<endl;
    cout<<"Wpisz: 'st' by wyswietlic statusy wszystkich twoich agentow!"<<endl;
    cout<<"Wpisz: 'sd <nazwa agenta>' by wylaczyc agenta!"<<endl;
    cout<<"Wpisz 'exit' by wyjsc!"<<endl;
}

int _write(int cfd, char *buf, int len){
    while (len>0){
        int i = write(cfd, buf, len);
        len -= i;
        buf +=i;
    }
    return 0;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        cerr << "Usage: " << argv[0] << " <Server IP> <Port>" << endl;
        return 1;
    }
    int cfd=socket(PF_INET,SOCK_STREAM,IPPROTO_TCP);
    string serverIP = argv[1];
    int port = stoi(argv[2]);
    struct sockaddr_in server_addr;
    server_addr.sin_family=AF_INET;
    int err = inet_pton(AF_INET, serverIP.c_str(), &(server_addr.sin_addr));
    if(err <= 0) {
        perror("ip address cast error");
        exit(1);
    }
    server_addr.sin_port=port;
    displaymenu();
    string input;
    while(true){
        cout<<">";
        getline(cin, input);
        cout<<input;
        if (input == "exit") {
            break;
        }
        stringstream ss(input);
        vector<string> args;
        string arg;
        while (ss >> arg) {
            args.push_back(arg);
        }
        string command=args[0];
        if(command!="cn" && command !="st" && command!="sd"){
            cout<<"Nie ma takiej komendy!"<<endl;
            continue;
        }else{
            err=connect(cfd,(struct sockaddr*)&server_addr,sizeof(server_addr));
            if(err==-1){
                perror("error connecting!");
                exit(1);
            }
            cout<<"Polaczono z serwerem..."<<endl;
            input+='\n';
            _write(cfd,(char*)input.c_str(),input.size()+1);
            cout<<"Wysylanie komendy.."<<endl;
            close(cfd);
        }
        //create new agent
        if(command=="cn"&&args.size()==4){
            
        }
        //status
        else if(command=="st"){
            
        }
        //shutdown
        else if(command=="sd"){
            
        }
        else cout<<"Nie ma takiej komendy."<<endl;
    }

    cout << "Wyjscie." << endl;
    return 0;
}