#include <bits/stdc++.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <openssl/sha.h>
#include <arpa/inet.h> 

#define ll long long
using namespace std;
class peerinfo{
    public:
    string peerIpasServer;
    string nameofFile;
    long long filesize;
} ;

class chunkinfox{
    public:
    string peerIpasServer;
    string nameofFile;
    long long chunkindex; 
    string destination;
} ;

string ipOfPeer;
string  logFile;
unordered_map<string, string> ftof;
unordered_map<string, vector<int>> ChunkInfo;
 int portOfPeer;
int t1_port,t2_port, PRET_Port;
string t1_ip,t2_ip,  PRET_IP;
bool loggedIn=false;
unordered_map<string, unordered_map<string, bool>> Uploaded;
unordered_map<string, string> downloadedFiles;

bool isdamage;
vector<string> curFilePiecewiseHash;
vector<vector<string>> download_chunk;
void stringhash(string , string& );
int writing(int, ll , char*);
void chunk_send(char* , int , int );
void error(const char *msg)
 {
    perror(msg);
    exit(1);
 }
 
long long file_size(char *p){
    FILE *fpointer = fopen(p, "rb"); 

    long x=-1;
    if(!fpointer){
         printf("File not found.\n");
         return x;
    }
   fseek (fpointer, 0, SEEK_END);
    x= ftell(fpointer)+1;
    fclose(fpointer);
    return x;
}

void client_handeling(int connected_socket){
    cout<<"hiiiiiiiiiii"<<"\n";

    char inputline[1024]; 
    bzero(inputline,sizeof(inputline));
    int y=read(connected_socket , inputline, 1024);
    if(y<=0){
        close(connected_socket);
        return;
    }
    
  
     vector<string>  inputcom;
    string tempx="";
    string fd=string(inputline);
    int nlen=fd.size();
    for(int i=0;i<nlen;i++)
    {
        if((fd[i])=='#')
        {
        inputcom.push_back(tempx);
        tempx="";
        }
        else
        tempx+=fd[i];
    }
    inputcom.push_back(tempx);

    if(inputcom[0] == "getVector"){
        string tmp = "";
        int nsize=ChunkInfo[inputcom[1]].size();
       cout<<inputcom[1]<<"check"<<nsize<<"\n";
        for(int i=0;i<nsize;i++)
        {
         tmp += to_string(ChunkInfo[inputcom[1]][i]);
        }
        write(connected_socket, &tmp[0], tmp.size());
        close(connected_socket);
        return;
    }
    if(inputcom[0] == "getChunk"){
         long long chunkindex = stoll(inputcom[2]);
         cout<<":::::::"<<inputcom[1]<<"\n";
        chunk_send(&ftof[inputcom[1]][0], chunkindex, connected_socket);
        close(connected_socket);
        return;
        
    }
    if(inputcom[0] == "getPath"){
       
        write(connected_socket, &ftof[inputcom[1]][0], ftof[inputcom[1]].size());
        close(connected_socket);
       return;
    }
}
string connectToPeer(char* peerIpasServer, char* serverPortIP, string present_command){
    int localsock = 0;

    int socknew=(localsock = socket(AF_INET, SOCK_STREAM, 0));
    if (socknew< 0) {  
        printf("\n Socket creation error \n"); 
        return "error"; 
        } 
    struct sockaddr_in psv; 
    psv.sin_family = AF_INET; 
    uint16_t pPort = stoi(string(serverPortIP));
    psv.sin_port = htons(pPort); 
     string temp="";
    string fdx(present_command);
    int ninl=fdx.size();
    for(int i=0;i<ninl;i++)
    {
        if((fdx[i])=='#')
        {
         break;
        }
        else
        temp+=fdx[i];
    }
     string ccmd=temp;
     cout<<ccmd<<"-"<<string(peerIpasServer)<<" "<<string(serverPortIP)<<" "<<present_command<<"\n";

   int inetx=inet_pton(AF_INET, peerIpasServer, &psv.sin_addr);
    if(inetx < 0){ 
        perror("Connection-Error");
    } 
    int connection =connect(localsock, (struct sockaddr *)&psv, sizeof(psv)) ;
    if (connection < 0) { 
        perror("Connection-Error");
    } 
   

    if(ccmd == "getVector"){
         char reply[10240];
         cout<<"getvector--"<<string(present_command)<<" "<<string(peerIpasServer)<<" "<<string(serverPortIP)<<"\n";
        int sentrep=send(localsock , &present_command[0] , strlen(&present_command[0]) , MSG_NOSIGNAL ) ;
        if(sentrep == -1){
           cout<<"error"<<strerror(errno)<<"\n";
            return "error"; 
        }
        bzero(reply ,sizeof(reply));
        int read_ack=read(localsock, reply, 10240);
        if(read_ack < 0){
            perror("err");
            return "error";
        }
        close(localsock);
        cout<<"sh"<<string(reply)<<"99"<<"\n";
        return string(reply);
    }
     if(ccmd == "getPath"){
        int sentrep=send(localsock , &present_command[0] , strlen(&present_command[0]) , MSG_NOSIGNAL );
        if(sentrep== -1){
            printf("Error--%s\n",strerror(errno));
            return "error"; 
        }
        char reply[10240] ;
        bzero(reply,sizeof(reply));
        int readack=read(localsock, reply, 10240) ;
        if(readack< 0){
            perror("err--");
            return "error";
        }
        
        string tempx="";
        string fd1x(present_command);
        int ninl1=fd1x.size();
        for(int i=0;i<ninl1;i++)
        {
            if((fd1x[i])=='#')
            {
            tempx="";
            }
            else
            tempx+=fd1x[i];
        }

        ftof[tempx] = string(reply);
        close(localsock);
        return "aa";
    }

    if(ccmd == "getChunk"){
        if(send(localsock , &present_command[0] , strlen(&present_command[0]) , MSG_NOSIGNAL ) == -1){
            printf("Error: %s\n",strerror(errno));
            return "error"; 
        }
        vector<string>  tokens;
        string tempx="";
        string fd1x(present_command);
        int ninl1=fd1x.size();
        for(int i=0;i<ninl1;i++)
        {
            if((fd1x[i])=='#')
            {
            tokens.push_back(tempx);
            tempx="";
            }
            else
            tempx+=fd1x[i];
        }
        tokens.push_back(tempx);
         long long index = stoll(tokens[2]);

        string despath = tokens[3];

        writing(localsock, index, &despath[0]);

        return "ss";
    }
     close(localsock);
    return "aa";
}
void* pseudoserver(void* arg){
    int opt = 1; 

     struct sockaddr_in add; 
   
    int lenadd = sizeof(add); 
    int sockid=socket(AF_INET, SOCK_STREAM, 0);
    if (sockid  == 0) { 
        perror("failed"); 
        exit(EXIT_FAILURE); 
    } 
    add.sin_port = htons(portOfPeer); 
    add.sin_family = AF_INET; 
    if (setsockopt(sockid, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) { 
        perror("set-error"); 
        exit(EXIT_FAILURE); 
    } 
   
    int inetx=inet_pton(AF_INET, &ipOfPeer[0], &add.sin_addr);
    if(inetx<=0)  { 
        printf("wrong address \n"); 
        return NULL; 
    } 
     int bond=bind(sockid, (struct sockaddr *)&add,  sizeof(add));
    if (bond<0) { 
        perror("failing in binding"); 
        exit(EXIT_FAILURE); 
    } 
    int listener=listen(sockid, 5);
    if (listener< 0) { 
        perror("listening....eroor"); 
        exit(EXIT_FAILURE); 
    } 

    vector<thread> trex;
    while(true){

        int tsocket;
        tsocket = accept(sockid, (struct sockaddr *)&add, (socklen_t *)&lenadd);

        if( tsocket< 0){
            perror("error");
        }

        trex.push_back(thread(client_handeling, tsocket));
    }
    for(auto it=trex.begin(); it!=trex.end();it++){
        if(it->joinable()) it->join();
    }
    close(sockid);
}



void Chunkset(string nameofFile, long long int left, long long int right, bool boolianUpload){
    if(!boolianUpload){
        ChunkInfo[nameofFile][left] = 1;
        return;
    }
     vector<int> tmp(right-left+1, 1);
       ChunkInfo[nameofFile] = tmp;
}



void chunk_send(char* filepath, int chunkindex, int connectedsocket){

   int varx = 0;

    std::ifstream fp1(filepath, std::ios::in|std::ios::binary);
    fp1.seekg(chunkindex*524288, fp1.beg);

    char buffer[524288] ; 
    bzero(buffer,sizeof(buffer));
    fp1.read(buffer, strlen(buffer));
    int noofg = fp1.gcount();
    varx = send(connectedsocket, buffer, noofg, 0);

    if (varx != -1) 
    {
        fp1.close();
        return;
    }
        error("Error during the time of sending file.");
    fp1.close();
} 
int writing(int sock, long long chunkindex, char* file_path){  
    
    int n=0;
    char buffer[524288];
    cout<<chunkindex<<" "<<string(file_path)<<"\n";
    string content = "";  string hash = "";
    int  ind=0;
     cout<<"hi1"<<"\n";
      for(;ind< 52428; ind = ind+n){
        
         bzero(buffer, 524288);
         cout<<"hi1"<<"\n";
        n = read(sock, buffer, strlen(buffer));
        if (n > 0){
       cout<<"hi2"<<"\n";
        fstream outx(file_path, std::fstream::in | std::fstream::out | std::fstream::binary);
        outx.seekp(chunkindex*524288+ind, ios::beg);
         buffer[n] = 0;
        outx.write(buffer, n);
        outx.close();

       
        content += buffer;
        cout<<"content "<<content<<"\n";
       
        }
        else break;
    }
    
    stringhash(content, hash);


    for(int ind=0;ind<2;ind++)
    hash.pop_back();
    if(hash != curFilePiecewiseHash[chunkindex]){
        isdamage = true;
    } 
    
    string tempx="";
    string fd=string(file_path);
    int nlen=fd.size();
    for(int i=0;i<nlen;i++)
    {
        if((fd[i])=='/')
        {
        tempx="";
        }
        else
        tempx+=fd[i];
    }

    Chunkset(tempx, chunkindex, chunkindex, false);

    return 0;
}
void chunkInformation(peerinfo* pf){
    
     vector<string>  PeerAddress;
    string tempx="";
    string fd=pf->peerIpasServer;
    int nlen=fd.size();
    for(int i=0;i<nlen;i++)
    {
        if((fd[i])==':')
        {
        PeerAddress.push_back(tempx);
        tempx="";
        }
        else
        tempx+=fd[i];
    }
    PeerAddress.push_back(tempx);
    string command = "getVector#" ;
    command+= string(pf->nameofFile);
    string sx=PeerAddress[0];
    string sy=PeerAddress[1];
    string response = connectToPeer(&sx[0], &sy[0], command);
    int i=0;
    int dsz=download_chunk.size();
    while(i<dsz){
        if(response[i] == '1'){
            download_chunk[i].push_back(string(pf->peerIpasServer));
            i++;
        }
        else
        i++;
    }

    delete pf;
}

void Chunkfind(chunkinfox* chunkx){

  
    string nameofFile = chunkx->nameofFile;

     vector<string> peerIpasServer;
        string tempx="";
          long long chunkindex = chunkx->chunkindex;
        string fd=chunkx->peerIpasServer;
        int nlen=fd.size();
        for(int i=0;i<nlen;i++)
        {
            if((fd[i])==':')
            {
            peerIpasServer.push_back(tempx);
            tempx="";
            }
            else
            tempx+=fd[i];
        }
        peerIpasServer.push_back(tempx);
  
   
    string command = "getChunk#";
    string delm="#";
    command  += nameofFile + delm;
    command += to_string(chunkindex) + delm;
     string destadds = chunkx->destination;
    command += destadds;
    connectToPeer(&peerIpasServer[0][0], &peerIpasServer[1][0], command);
    
    delete chunkx;
    return;
}
 

void piecewisesending(vector<string> inputvec, vector<string> peers){
    download_chunk.clear();
    string peerback;
    int peersize=peers.size();
    peerback=peers[peersize-1];
    long long filesize = stoll(peerback);
    peers.pop_back();
    long long noofseg = filesize/524288+1;
    download_chunk.resize(noofseg);

    
    vector<thread> threads, threads2;
    int i=0;
    while(i<peersize-1){
        peerinfo* pf = new peerinfo;
        pf->nameofFile = inputvec[2];
        pf->peerIpasServer = peers[i];
        i++;
        pf->filesize = noofseg;
        threads.push_back(thread(chunkInformation, pf));
    }
    int d_size=download_chunk.size();
    for(auto it=threads.begin(); it!=threads.end();it++){
        if(it->joinable()) it->join();
    }
     threads.clear();

    
    i=0;
    long long  received_segment = 0;
    while(i<d_size){
        if(download_chunk[i].size() > 0)
        {
            i++;
        }
        else
        {
            cout << "All parts of the file are not available." << endl;
            return;
        }
    }
     srand((unsigned) time(0));
 
   ChunkInfo[inputvec[2]].resize(noofseg,0);
    isdamage = false;

    string des_path = inputvec[3] ;
    des_path+= "/" + inputvec[2];
    FILE* fp = fopen(&des_path[0], "r+");
	if(fp != 0){
		printf("The is existing file.\n") ;
        fclose(fp);
        return;
	}
    string ss(filesize, '\0');
    fstream in(&des_path[0],ios::out|ios::binary);
    in.write(&ss[0],ss.size());  
    in.close();

    
    vector<int> tmp(noofseg);
    for(int i=0;i<noofseg;i++)
    {
        tmp[i]=0;
        ChunkInfo[inputvec[2]][i] = tmp[i];
    }
    
    string peerpath;
    for(;received_segment <noofseg;  received_segment++){
        long long randompiece;
       for(;;){
            randompiece = rand()%noofseg;
            if(ChunkInfo[inputvec[2]][randompiece] != 0) continue;
            else
            break;
        }
        int u=rand()%(download_chunk[randompiece].size());
        string randompeer = download_chunk[randompiece][u];
        string dfml="/";
        chunkinfox* req = new chunkinfox;
        req->nameofFile = inputvec[2];
        req->destination = inputvec[3];
        req->destination += dfml + inputvec[2];
        req->peerIpasServer = randompeer;
        req->chunkindex = randompiece;

        ChunkInfo[inputvec[2]][randompiece] = 1;

        threads2.push_back(thread(Chunkfind, req));
       peerpath = randompeer;
    }    
    for(auto it=threads2.begin(); it!=threads2.end();it++){
        if(it->joinable()) it->join();
    } 
    if(isdamage==0){
       
         cout << "Download completed successfully" << endl;
    }
    if(isdamage==1){
         cout << "Downloaded file may be corrupted." << endl;
    }
    downloadedFiles.insert({inputvec[2], inputvec[1]});


      vector<string> addofserver;
        string tempx="";
        string fd=peerpath;
        int nlen=fd.size();
        for(int i=0;i<nlen;i++)
        {
            if((fd[i])==':')
            {
            addofserver.push_back(tempx);
            tempx="";
            }
            else
            tempx+=fd[i];
        }
        addofserver.push_back(tempx);

    connectToPeer(&addofserver[0][0], &addofserver[1][0], "getPath#" + inputvec[2]);
    return;
}






int download(string filemeta,vector<string> inpt, int sockets){
    
    if(inpt.size() != 4){
        return 0;
    }
   
    
    int x=send(sockets , &filemeta[0] , strlen(&filemeta[0]) , MSG_NOSIGNAL );
    if(x == -1){
        printf("Error: %s\n",strerror(errno));
        return -1;
    }

    char reply[524288] ; 
    bzero(reply,sizeof(reply));
    read(sockets , reply, 524288); 

    if(string(reply) == "File disappear"){
        cout << reply << endl;
        return 0;
    }


     vector<string>  peersWithFile;
    string temp="";
    string fdx(reply);
    int ninl=fdx.size();
    for(int i=0;i<ninl;i++)
    {
        if((fdx[i])=='#')
        {
         peersWithFile.push_back(temp);
        temp="";
        }
        else
        temp+=fdx[i];
    }
     peersWithFile.push_back(temp);
    
    string dum1="test";
    write(sockets, &dum1[0], dum1.size());

    bzero(reply, 524288);
    read(sockets , reply, 524288); 


    vector<string>  tmp;
    string temp1="";
    string fdy(reply);
    int ninl1=fdy.size();
    for(int i=0;i<ninl1;i++)
    {
        if((fdy[i])=='#')
        {
         tmp.push_back(temp1);
        temp1="";
        }
        else
        temp1+=fdy[i];
    }
     tmp.push_back(temp1);

    curFilePiecewiseHash = tmp;
   
    piecewisesending(inpt, peersWithFile);

    return 0;
}

void stringhash(string segment, string& hashVal){
    unsigned char md[20];
    if(!SHA1(reinterpret_cast<const unsigned char *>(&segment[0]), segment.length(), md)){
        printf("incorrect hash\n");
         hashVal += "#";
         return;
    }
   int i=0;
    while(i<20)
    {
        char buffer[3];
        sprintf(buffer, "%02x", md[i]&0xff);
        i++;
         hashVal += string(buffer);
    }
   hashVal += "#";
}

string hashing(char* path){
    
   char Line[32769];
    string hashval = "";
    int  i, acc;
    FILE *fp1;

    long long sizeoffile = file_size(path);
    if(sizeoffile <0){
        return "#";
    }

    fp1 = fopen(path, "r");

    if(fp1){ 
        int i=0;
        while(i<=sizeoffile/524288 )
        {
             acc = 0;
            string strsegment;

            int readc;
            for( ;acc <524288 && (readc = fread(Line, 1, min(32768-1, 524288-acc), fp1)); ){
                Line[readc] = '\0';
                
                strsegment = strsegment+Line;
                acc =  acc+strlen(Line);
                memset(Line, 0, sizeof(Line));
            }

            stringhash(strsegment, hashval);
            i++;
        }
        
        
        fclose(fp1);
    }
    else{
        printf("File not found.\n");
    }
    for(int ind=0;ind<2;ind++)
    hashval.pop_back();
    
    return hashval;
}

string filehashing(char* path){

    ostringstream buffer; 
    ifstream inputstr (path); 
     string  hashVal;
    buffer << inputstr.rdbuf(); 
    string contents =  buffer.str();

    unsigned char md[SHA256_DIGEST_LENGTH];
    if(!SHA256(reinterpret_cast<const unsigned char *>(&contents[0]), contents.length(), md)){
        printf("Inappropiate hashing\n");
         return hashVal;
    }
    int i=0;
    while(i<SHA256_DIGEST_LENGTH)
    {
         char buffer[3];
        sprintf(buffer, "%02x", md[i]&0xff);
        i++;
        hashVal += string(buffer);
    }
    
    return hashVal;
}


 int processCMD(vector<string> inputvec, int sock){
    char reply[10240]={0}; 
  
    read(sock , reply, 10240); 

 
    if(string(reply) == "Invalid argument count") 
    {
         cout<< reply << endl;
        return 0;
    }

    
    string peerAddress = ipOfPeer + ":" + to_string(portOfPeer);
    if(inputvec[0] == "login"){
        cout << reply << endl;
        if(string(reply) == "Login-Successful"){
            write(sock, &peerAddress[0], peerAddress.length());
             loggedIn = true;
        }
    }
    else if(inputvec[0]=="create_user"||inputvec[0]=="create_group")
    {
     cout<< reply << endl;
    }
    else if(inputvec[0] == "logout"){
        loggedIn = false;
        cout << reply << endl;
    }
    else if(inputvec[0] =="join_group")
    {
        cout<<reply<<endl<<"\n";
    }
    else if(inputvec[0] =="leave_group")
    {
        cout<<reply<<endl<<"\n";
    }
    else if(inputvec[0] =="accept_request")
    {
        cout<<reply<<endl<<"\n";
    }
    else if(inputvec[0] == "list_groups"){
       
         vector<string> result_group;
        string temp="";
        string replystr(reply);
        int ninl=replystr.size();
        
        for(int i=0;i<ninl;i++)
        {
            if((replystr[i])=='/')
            {
            result_group.push_back(temp);
            temp="";
            }
            else
            temp+=replystr[i];
        }
        result_group.push_back(temp);
        cout<<"All requests are from:"<<"\n";
        for(auto i:result_group){
            cout << i << endl;
        }
    }
     else if(inputvec[0] == "list_requests")
     {
        
        if(string(reply) == "**err**") return -1;
        if(string(reply) == "**er2**") return 1;
         vector<string> result_group;
        string temp="";
        string replystr(reply);
        int ninl=replystr.size();
        
        for(int i=0;i<ninl;i++)
        {
            if((replystr[i])=='/')
            {
            result_group.push_back(temp);
            temp="";
            }
            else
            temp+=replystr[i];
        }
        result_group.push_back(temp);
        cout<<"All groups are:"<<"\n";
        for(auto i:result_group){
            cout << i << endl;
        }
     }
     else if(inputvec[0]=="upload_file")
     {
        if(string(reply) == "Error-1"){
            cout << "Group even not exist" << endl;
            return 0;
        }
        else  if(string(reply) == "Error-2"){
            cout << "permission denied" << endl;
            return 0;
        }
        else  if(string(reply) == "Error-3"){
            cout << "this File not exist" << endl;
            return 0;
        }
        else 
        {
            cout<<reply<<"\n";
            if(inputvec.size() != 3)
            {
                 return 0;
            }
            char* fileDetails = &inputvec[1][0];
                string temp="";
                string fd(fileDetails);
                int ninl=fd.size();
                for(int i=0;i<ninl;i++)
                {
                    if(int(fd[i])=='/')
                    {
                  
                         temp="";
                    }
                    else
                         temp+=fd[i];
                }
               string file_name =temp;


            if(Uploaded[inputvec[2]].find(file_name) != Uploaded[inputvec[2]].end())
            {
                cout << "File already uploaded" << endl;
                if(send(sock , "error" , strlen("error") , MSG_NOSIGNAL ) != -1){
                     return 0;
                }
                error("Error ocured");
                    return -1;
            }

             string piecewiseHash = hashing(fileDetails);

            if(piecewiseHash == "#") return 0;
            string filehash = filehashing(fileDetails);
            string filesize = to_string(file_size(fileDetails));
            string total_file="";
            string delm1="#";
             total_file += string(fileDetails) + delm1;
           total_file += string(ipOfPeer) + ":" + to_string(portOfPeer) + delm1;
            total_file += filesize +delm1;
            total_file += filehash + delm1;
            total_file += piecewiseHash;

            Uploaded[inputvec[2]][file_name] = true;
                ftof[file_name] = string(fileDetails);
                
            if(send(sock , &total_file[0] , strlen(&total_file[0]) , MSG_NOSIGNAL ) == -1){
                printf("Error: %s\n",strerror(errno));
                return -1;
            }
            
            char reply[10240]; 
            bzero(reply,sizeof(reply));
            read(sock , reply, 10240); 
            cout << reply << endl;
           Chunkset(file_name, 0, stoll(filesize)/524288 + 1, true);
           
        }
     }
     else if(inputvec[0] == "download_file"){
        cout<<reply<<"\n";

        if(string(reply) == "Error-1:"){
            cout << "Group doesn't exist" << endl;
            return 0;
        }
        if(string(reply) == "Error-2:"){
            cout << "permision denied" << endl;
            return 0;
        }
        if(string(reply) == "Error-3:"){
            cout << "Directory not exist" << endl;
            return 0;
        }
        if(downloadedFiles.find(inputvec[2])!= downloadedFiles.end()){
            cout << "File already present downloaded" << endl;
            return 0;
        }
         
          string fd = "";
            fd += inputvec[2] + "#";
            fd += inputvec[3] + "#";
            fd += inputvec[1];
        return download(fd,inputvec, sock);
    }

   
      return 0;
 }
 
 signed main(int argc, char *argv[])
 {
    if(argc!=3)
    {
        fprintf(stderr, "maintain the formats: argument structure:<peer IP:port> and <tracker info file name>\n");
        exit(1);
    }
    string infoOfPeer = argv[1];
    int sn=sizeof(infoOfPeer);
    string tx1="";
    string tx2="";
    int f=0;
    for(int i=0;i<sn;i++)
    {
        if(infoOfPeer[i]==':')
        {
            f=1;
            continue;
        }
       if(f==0)
       tx1+=infoOfPeer[i];
       else
       tx2+=infoOfPeer[i];
    }
    ipOfPeer=tx1;
     portOfPeer=stoi(tx2);

    char cureentDir[128];
    getcwd(cureentDir, 128);
    string cdir(cureentDir);
     vector<string> tempv;
     char* ifilestr=argv[2];
     string inpstr(ifilestr);
     string delim="/";
     inpstr=cureentDir+delim+inpstr;
     ifstream input_file(inpstr);


    if (!input_file.is_open()) {
        cerr << "Could not open the file - '" << "'" << endl;
        return EXIT_FAILURE;
    }
    string line;
    while (getline(input_file, line)){
        tempv.push_back(line);

    }

   logFile = infoOfPeer + "_log.txt";
    ofstream out;
    out.open(logFile);
    out.clear();
    out.close();


    t1_ip = tempv[0];
    t1_port = stoi(tempv[1]);
    t2_ip = tempv[2];
    t2_port = stoi(tempv[3]);
   

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
       pthread_t  multi_thread_var;
      

    if(pthread_create(&multi_thread_var, NULL, pseudoserver, NULL) == -1){
        perror("pthread"); 
        exit(EXIT_FAILURE); 
    }


    PRET_IP = t1_ip; 
    PRET_Port = t1_port;

    serv_addr.sin_family = AF_INET; 
    serv_addr.sin_port = htons(PRET_Port); 
     if(inet_pton(AF_INET, &PRET_IP[0], &serv_addr.sin_addr)<=0)  { 
        cout<<"error"<<"\n";
    } 
    if(connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr))<0 )
    {
        PRET_IP = t2_ip; 
        PRET_Port = t2_port;
       
        serv_addr.sin_family = AF_INET; 
        serv_addr.sin_port = htons(PRET_Port); 
        
        if(connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr))<0)
        error("tracker is not available\n");
       
    }
   

      while(true){ 
        cout << ":::->";
        string input_line, s;
        getline(cin, input_line);
        
        if(input_line.length() < 1) continue;
        
        vector<string> input_com;
        string temp="";
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
        
        if(input_com[0] == "login" && loggedIn){
            cout << "You already are present in one active session" << endl;
            continue;
        }
       
        if( input_com[0] !="login"&& input_com[0]!= "create_user"&&!loggedIn){
             cout << "Either you don't have any account or your login session is over"<<input_com[0] << endl;
                continue;
        }
        int sentvar=send(sockfd , &input_line[0] , strlen(&input_line[0]) , MSG_NOSIGNAL );
        if(sentvar == -1){
            printf("Error-%s\n",strerror(errno));
            return -1;
        }
        processCMD(input_com, sockfd);
    }
    close(sockfd);


 }