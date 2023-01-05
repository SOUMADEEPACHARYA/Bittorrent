#include <bits/stdc++.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fstream>
#include <sys/stat.h>
#include <pthread.h>
using namespace std;
using std::cout; using std::cerr;
using std::endl; using std::string;
using std::ifstream; using std::vector;
string  logFile;
unordered_map<string, set<string> > members_group;
int t1_port,t2_port, PRET_Port;
string t1_ip,t2_ip,  PRET_IP;
unordered_map<string, string> Hash_piece; 
unordered_map<string, unordered_map<string, set<string>>> Seeder;
unordered_map<string, bool> loginvar;
unordered_map<string, string> detoflogin,admin_group;
unordered_map<string, string> userport;
unordered_map<string, string> filesize;
vector<string> groups_list;
 unordered_map<string, set<string> > pendngRequests;
 
void* funcheck(void* arg){
    while(1){
        string inputline;
        getline(cin, inputline);
        if(inputline != "quit"){
           continue;
        }
        else
        exit(0);
    }
}
vector<string> splitString(string str, string delim){
    vector<string> res;

    size_t pos = 0;
    while ((pos = str.find(delim)) != string::npos) {
        string t = str.substr(0, pos);
        res.push_back(t);
        str.erase(0, pos + delim.length());
    }
    res.push_back(str);

    return res;
}
 void error(const char *msg)
 {
    perror(msg);
    exit(1);
 }
void writeLog(const string &text ){
    ofstream log_file(logFile, ios_base::out | ios_base::app );
    log_file << text << endl;
}
int createUser(string u_id,string password){

    if(detoflogin.find(u_id) != detoflogin.end()){

        return -1;
    }
    detoflogin.insert({u_id, password});
    return 0;
}
int validateLogin(string user_id,string passwd){
    if(detoflogin.find(user_id) == detoflogin.end()){
        return -1;
    }
    if(detoflogin[user_id] != passwd)
    return -1;

    if(loginvar.find(user_id) != loginvar.end()&&loginvar[user_id]){
         return 1;
    }
     if(loginvar.find(user_id) != loginvar.end())
      loginvar[user_id] = true;
    else{
          loginvar.insert({user_id, true});

    }
   
    return 0;
}
int groupCreate(string t1,string t2, int client_socket, string client_uid){

    int n=groups_list.size();
    for(int i=0;i<n;i++){
        if(groups_list[i]== t2) 
        return -1;
    }
    groups_list.push_back(t2);
    admin_group.insert({t2, client_uid});
    members_group[t2].insert(client_uid);
    return 0;
}

void handle_connection(int connected_socket){
    string c_uid = "",c_gid = "";
   
    writeLog("***********pthread  for client socket number " + to_string(connected_socket));

    while(true){
        char inputLine[1024] ;
         bzero(inputLine, 1024);

        if(read(connected_socket , inputLine, 1024) <=0){
            loginvar[c_uid] = false;
            close(connected_socket );
            break;
        }

        writeLog("client request:" + string(inputLine));

       
        vector<string>input_com;

        string temp="";
        string input_line(inputLine);
        int ninl=input_line.size();
        for(int i=0;i<ninl;i++)
        {
            if(int(input_line[i])==' ')
            {
            input_com.push_back(temp);
            temp="";
            }
            else
            temp+=input_line[i];
        }
        input_com.push_back(temp);

       
    if(input_com[0] == "create_user"){
            if(input_com.size() != 3){
                write(connected_socket , "Invalid count no", strlen("Invalid count no"));
            }
            else{
                string u_id = input_com[1];
                string password = input_com[2];

                if(createUser(u_id,password) < 0){
                    write(connected_socket , "User exists already", strlen("User exists already"));
                }
                else{
                    write(connected_socket , "Account has been created", sizeof("Account has been created"));
                }
            }
        }
         else if(input_com[0] == "login"){
            if(input_com.size() != 3){
                write(connected_socket , "Invalid argument count", strlen("Invalid argument count"));
            }
            else{
                  string u_id = input_com[1];
                string password = input_com[2];
                int r;
                if((r = validateLogin(u_id,password)) < 0){
                    write(connected_socket , "incorrect credentials check your Username and password", strlen( "incorrect credentials check your Username and password"));
                }
                if(r==0){
                   
                    write(connected_socket , "Login-Successful", strlen("Login-Successful"));
                    c_uid = input_com[1];
                    char buf[96];
                    read(connected_socket , buf, 96);
                    string peerAddress = string(buf);
                    userport[c_uid] = peerAddress;
                }
                if(r > 0){
                    write(connected_socket , "You already have one active session", strlen("You already have one active session"));
                }
              
            }            
        }
         else if(input_com[0] == "logout"){
            
            write(connected_socket, "Logout Successful", sizeof("Logout Successful"));
            
            writeLog("logout sucess\n");
             loginvar[c_uid] = false;
    }
    else if(input_com[0] == "create_group"){
        if(input_com.size() != 2){
        write(connected_socket, "Invalid argument count", strlen("Invalid argument count"));
       }
        string t1=input_com[0];
        string t2=input_com[1];
        int v=groupCreate(t1,t2, connected_socket, c_uid);
            if(v >=0){
                write(connected_socket, "Group created",strlen( "Group created"));
                 c_gid = input_com[1];
            }
            else{
                write(connected_socket, "Group exists", strlen("Group exists"));
            }
        }
        else if(input_com[0]=="list_groups")
        {
              if(input_com.size() != 1){
                    write(connected_socket, "Invalid number of argument", strlen("Invalid number of argument"));
                   continue;
              }
                
                if(groups_list.size() == 0){
                    write(connected_socket, "No groups present", strlen("No groups present"));
                    continue;
                }

                string reply = "";
                string delm="/";
                for(auto i:groups_list){
                    reply += i+ delm;
                }
               
                write(connected_socket, &reply[0], reply.length());
         }
         else if(input_com[0]=="join_group")
         {
              if(input_com.size() != 2){

                write(connected_socket, "Invalid argument count", strlen( "Invalid argument count"));
                 continue;
             }
             string s;

            if(admin_group.find(input_com[1]) == admin_group.end()){
                s= "Group ID. is not valid one";
            }
            else if(members_group[input_com[1]].find(c_uid) != members_group[input_com[1]].end()){
                s="You are present in this group already";
            }
            else{
                pendngRequests[input_com[1]].insert(c_uid);
                s= "Group request has been sent"; 
            }
           write(connected_socket, &s[0], s.size());
             writeLog("this is join request");
    
         }
         else if(input_com[0]=="accept_request")
         {
                if(input_com.size() != 3){
                    write(connected_socket, "Invalid argument count", strlen("Invalid argument count"));
                     continue;
            }
           string s;
            if(admin_group.find(input_com[1]) == admin_group.end()){
                writeLog("inside accept_request if");
                s="Invalid group ID.";
            }
            else if(admin_group[(input_com[1])] != c_uid){
                  writeLog("inside accept_request else");
                  s="You are not allowed for this";
         
            }
            else{
                writeLog("inside accept_request else if with pending list:");
                s="Request approved.";
                for(auto i: pendngRequests[input_com[1]]){
                    writeLog(i);
                }
                
              }
            
              if(s=="Request approved.")
              {
                  members_group[input_com[1]].insert(input_com[2]);
                    pendngRequests[input_com[1]].erase(input_com[2]);
              }
                write(connected_socket, &s[0], s.size());

         }
         else if(input_com[0] == "list_requests"){
             if(input_com.size() != 2){
                write(connected_socket, "Invalid argument count",strlen("Invalid argument count"));
                continue;
            }
            
            string s="";
            string delm="/";
            if(admin_group.find(input_com[1])==admin_group.end() ){
                writeLog("iffff");
                s="error1";
               
            }
            else if(admin_group[input_com[1]] != c_uid)
            {
                writeLog("iffff");
                s="error1";
               
            }
            else if( pendngRequests[input_com[1]].size() == 0){
                
                s="error2";
            }
            else {
                
                writeLog("pending request size: "+  to_string( pendngRequests[input_com[1]].size()));
                for(auto i : pendngRequests[input_com[1]]){
                    s+=i+delm;
                }
                writeLog("reply :" + s);
            }
            write(connected_socket, &s[0], s.size());
        }
        else if(input_com[0] == "leave_group")
        {
             if(input_com.size() != 2){
                write(connected_socket, "Invalid argument count", strlen( "Invalid argument count"));
                continue;
                 }
                 string s;
                if(admin_group.find(input_com[1]) == admin_group.end()){
                    s="This is Invalid group ID.";
                }
                else if( members_group[input_com[1]].find(c_uid) ==  members_group[input_com[1]].end()){
                         s= "You are not permitted";
                    }
                 else  if(admin_group[input_com[1]] == c_uid){
                          s= "Admin cant leave!";
                    }
                else{
                    members_group[input_com[1]].erase(c_uid);
                    s= "You left succesfully";
                    
                }
                  
                   write(connected_socket, &s[0], s.size());
        }
        else if(input_com[0]=="upload_file")
        {
            string s=input_com[1];
             struct stat buffer;
             bool isFileExist= (stat (s.c_str(), &buffer) == 0);
             string sx;

             if(input_com.size() != 3)
             {
                sx="Invalid argument count";
             }
          else if(!isFileExist){
                sx="Error-3";
                 write(connected_socket, &sx[0], sx.size());
                 continue;
            }
            else if(members_group.find(input_com[2]) == members_group.end()){
                sx="Error-1";
            }
            else if(members_group[input_com[2]].find(c_uid) == members_group[input_com[2]].end()){
                sx="Error-2";
            }
            else
            {
                char fileDetails[524288];
                bzero(fileDetails,sizeof(fileDetails));
                write(connected_socket, "UPLOADING........",strlen( "UPLOADING........"));
               

                if(read(connected_socket , fileDetails, 524288))
                {
                    if(string(fileDetails) == "error") continue;
                    cout<<string(fileDetails)<<"\n";

                     vector<string> detailFiles;
                    string temp="";
                    string fd(fileDetails);
                    int ninl=fd.size();
                    for(int i=0;i<ninl;i++)
                    {
                        if(int(fd[i])=='#')
                        {
                        detailFiles.push_back(temp);
                        temp="";
                        }
                        else
                        temp+=fd[i];
                    }
                    detailFiles.push_back(temp);


    
                    string temp1="";
                    string fd1(detailFiles[0]);
                    int ninl1=fd1.size();
                    for(int i=0;i<ninl1;i++)
                    {
                        if(int(fd[i])=='/')
                        {
                        temp1="";
                        }
                        else
                        temp1+=fd[i];
                    }
                    string filename=temp1;
            

                    string Pieces = "";
                    string delm1="#";
                    for(int i=4; i<detailFiles.size(); i++){
                        if(i == detailFiles.size()-1) 
                        {
                             Pieces += detailFiles[i];
                        }
                        else
                        {
                            Pieces =Pieces+ detailFiles[i]+delm1;
                        }
                    }
            
                    Hash_piece[filename] = Pieces;
                    
                    if(Seeder[input_com[2]].find(filename) == Seeder[input_com[2]].end()){
                         Seeder[input_com[2]].insert({filename, {c_uid}});
                         filesize[filename]=detailFiles[2];
                         sx="Upload completed";
                    }
                   else
                   {
                        Seeder[input_com[2]][filename].insert(c_uid);
                        filesize[filename]=detailFiles[2];
                        sx="Upload completed";
                   }
                }
            }
            write(connected_socket, &sx[0], sx.size());
        }
        else if(input_com[0]=="download_file")
        {
            string s=input_com[3];
             struct stat buffer;
             bool isFileExist= (stat (s.c_str(), &buffer) == 0);
             string sx;
            if(input_com.size() != 4){
                  write(connected_socket, "Invalid argument count", strlen("Invalid argument count"));
                  continue;
            }
            if(members_group.find(input_com[1]) == members_group.end()){
                sx="Error-1:";
                write(connected_socket, &sx[0], sx.size());
                 continue;
            }
            if(members_group[input_com[1]].find(c_uid) == members_group[input_com[1]].end()){
                sx= "Error-2:";
                write(connected_socket, &sx[0], sx.size());
                 continue;
            }
            if(!isFileExist){
                sx="Error-3:";
                 write(connected_socket, &sx[0], sx.size());
                 continue;
            }
           
                char fileDetails[524288] ;
                bzero(fileDetails,sizeof(fileDetails));
                write(connected_socket, "DOWNLOADING.....", strlen("DOWNLOADING....."));
                 if(read(connected_socket , fileDetails, 524288)){

                    if(string(fileDetails) == "error") continue;

                     vector<string> detailFiles;
                    string temp2="";
                    string fd(fileDetails);
                    int ninl2=fd.size();
                    for(int i=0;i<ninl2;i++)
                    {
                        if(int(fd[i])=='#')
                        {
                        detailFiles.push_back(temp2);
                        temp2="";
                        }
                        else
                        temp2+=fd[i];
                    }
                    detailFiles.push_back(temp2);



                string reply = "";
                if(Seeder[input_com[1]].find(detailFiles[0]) != Seeder[input_com[1]].end()){
                    for(auto i: Seeder[input_com[1]][detailFiles[0]]){
                        if(!loginvar[i]){
                            continue;
                        }
                        else
                        reply += userport[i] + "#";
                    }
                    reply += filesize[detailFiles[0]];
                    writeLog("seeder list: "+ reply);
                    write(connected_socket, &reply[0], reply.length());

                    char dummy[6];
                    read(connected_socket, dummy, 6);
                     Seeder[input_com[1]][input_com[2]].insert(c_uid);
                    write(connected_socket, &Hash_piece[detailFiles[0]][0], Hash_piece[detailFiles[0]].length());
                    continue;
                }
                 write(connected_socket, "File disappear", strlen("File disappear"));
                 continue;
        
    }
                
               

       }
       else{
            write(connected_socket, "Invalid command", 16);
        }
     }
close(connected_socket);

}

 signed main(int argc, char *argv[])
 {
    if(argc<2)
    {
        fprintf(stderr, "maintain the formats: argument structure <tracker info file name> and <tracker_number>\n");
        exit(1);
    }
   
      char cureentDir[128];
    getcwd(cureentDir, 128);
    string cdir(cureentDir);
     vector<string> tempv;
     char* ifilestr=argv[1];
     string inpstr(ifilestr);
     string path="";
     string delm="/";
     
     inpstr=cureentDir+delm+inpstr;
     ifstream input_file(inpstr);

    if (!input_file.is_open()) {
        
         error("error in openning");

    }
    string line;
    while (getline(input_file, line)){
       
        tempv.push_back(line);

    }
    logFile ="tlog" + string(argv[2]) + ".txt";
    ofstream out;
    out.open(logFile);
    out.clear();
    out.close();



    if(string(argv[2]) == "1")
    {
        t1_ip = tempv[0];
        t1_port = stoi(tempv[1]);
        PRET_IP =t1_ip;
        PRET_Port =t1_port;
    }
    else
    {
      
        t2_ip = tempv[2];
        t2_port = stoi(tempv[3]);
        PRET_IP = t1_ip;
        PRET_Port = t1_port;
    }
    writeLog("Adress of Tracker 1: " + string(t1_ip)+ ":" +to_string(t1_port));
    writeLog("Address ofTracker 2:" + string(t2_ip)+ ":" +to_string(t2_port));
    writeLog("name of Log file : " + string(logFile) + "\n");
     pthread_t  multi_thread_var;

     int sockfd,newsockfd,portno,n;
    char buffer[255];
    struct sockaddr_in serv_addr , cli_addr;
    socklen_t clilen;

    int opt=1;
    sockfd= socket(AF_INET ,SOCK_STREAM , 0);
    if(sockfd<0)
    {
        error("error in openning socket");
    }
   if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) { 
        perror("setsockopt"); 
        exit(EXIT_FAILURE); 
    } 
     bzero((char*)&serv_addr,sizeof(serv_addr));
    serv_addr.sin_family=AF_INET;
    serv_addr.sin_addr.s_addr=INADDR_ANY;
    serv_addr.sin_port=htons(PRET_Port);
    if(bind(sockfd,(struct sockaddr *)&serv_addr, sizeof(serv_addr))<0)
    {
        error("binding failed");
    };
    writeLog("Binding completed.");
     if (listen(sockfd, 5) < 0) { 
        error("listen"); 
    } 
     writeLog("Listening...");
      vector<thread> multiThread_v;
      int thret=pthread_create(&multi_thread_var, NULL, funcheck, NULL);
      if(thret==-1)
      {
        error("pthread error ");
      }


    while(true){
    int sockidClient;
    sockidClient=accept(sockfd ,(struct sockaddr *)&cli_addr, &clilen);
    if(sockidClient < 0){
        perror("error in getting Acceptance");
        writeLog("Error in case of accept"); 
    }
    writeLog("Connection Accepted");

    multiThread_v.push_back(thread(handle_connection, sockidClient));
    }
    for(auto i=multiThread_v.begin(); i!=multiThread_v.end(); i++){
        if(i->joinable()) i->join();
    }

    writeLog("EXITING.");



 }