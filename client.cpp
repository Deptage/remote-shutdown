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

int _write(int cfd, char *buf, int len);
int _read(int cfd, char *buf, int bufsize);

int main(int argc, char* argv[]) {
    if (argc != 3) {
        cerr << "Usage: " << argv[0] << " <Server IP> <Port>" << endl;
        return 1;
    }

    string serverIP = argv[1];
    int port = stoi(argv[2]);
    struct sockaddr_in server_addr;
    server_addr.sin_family=AF_INET;
    server_addr.sin_port=htons(port);
    int err = inet_pton(AF_INET, serverIP.c_str(), &(server_addr.sin_addr));
    if(err <= 0) {
        perror("ip address cast error");
        exit(1);
    }
    displaymenu();
    string input;
    int cfd=socket(PF_INET,SOCK_STREAM,IPPROTO_TCP);
    err=connect(cfd,(struct sockaddr*)&server_addr,sizeof(server_addr));
    if(err==-1){
        perror("error connecting!");
        exit(1);
    }
    cout<<endl<<"Polaczono z serwerem..."<<endl;
    while(true){
        cout<<">";
        getline(cin, input);
        //cout<<input;
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
        input+='\n';
        if(command!="cn" && command !="st" && command != "sd"){
            cout<<"Nie ma takiej komendy!"<<endl;
            continue;
        }
        if(command=="cn"&&args.size()==4){
            cout<<"======TWORZENIE AGENTA======"<<endl;
            _write(cfd,(char*)input.c_str(),input.size());
            cout<<"Wysylanie komendy.."<<endl;
            sleep(1);
            char response[3];
            _read(cfd,response,3);
            if(response[0]=='a') cout<<"Dodano agenta!"<<endl;
        }
        //status
        else if(command=="st"){
            cout<<"======PINGOWANIE AGENTOW======"<<endl;
            _write(cfd,(char*)input.c_str(),input.size());
            cout<<"Wysylanie komendy.."<<endl;
            sleep(3);
            char response[200];
            _read(cfd,response,100);
            string res = response;
            cout<<response;
        }
        //shutdown
        else if(command=="sd"){
            cout<<"======WYLACZANIE AGENTA======"<<endl;
            _write(cfd,(char*)input.c_str(),input.size());
            cout<<"Wysylanie komendy.."<<endl;
            sleep(3);
            char response[3];
            _read(cfd,response,3);
            if(response[0]=='a') cout<<"Wylaczono agenta"<<endl;
            if(response[0]=='f') cout<<"Agenta nie znaleziono"<<endl;
        }
        else cout<<"Nie ma takiej komendy."<<endl;
    }
    close(cfd);
    cout << "Wyjscie." << endl;
    return 0;
}

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
