#include<iostream>
#include<fstream>
#include<map>
#include<vector>
#include<string>
#include<iterator>
#include<typeinfo>
#include<map>
#include<cstdio>
#include<cstdlib>
#include<cstring>
#include<unistd.h>
#include<errno.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<netdb.h>
#include<arpa/inet.h>
#include<sys/wait.h>
#include<signal.h>
using namespace std;

#define PORT_TCP "33362"
#define PORT_UDP "32362"
#define BACKLOG 10
#define INET_ADDSTRLEN 16
#define MAXDATASIZE 100
#define HOSTNAME "127.0.0.1"

/*Function to handle zombie processes.*/
/*I have directly adapted this function code segment from
beej's guide*/
void sigchld_handler(int s)
{
   // waitpid() might overwrite errno, so we save and restore it:
int saved_errno = errno;

 while(waitpid(-1, NULL, WNOHANG) > 0);
 errno = saved_errno;
}

int main(){
//MAIN SERVER BOOTUP

//Declarations and Initializations.
int duplicate_count; //Duplicate counter used handling removal of duplicate elements in a vector.
int i,j,k,f; // Iterators (general)

/* Iterators of type vector string to iterate through elements of a vector.*/
vector<string>:: iterator itr;
vector<string>:: iterator itr2;

/* Iterator of type map to iterate through elements of a map.*/
map<string, vector<string> >:: iterator itr_map; // Iterator of type map<string, vector<string>> to iterate through elements of a map.

//MAIN SERVER TCP SOCKET SETUP
int status; // Stores the value returned by getaddrinfo()
int socket_fd; //TCP socket file descriptor: To store the socket file descriptor of the parent process (the listener).
int new_socket_fd; //To store the socket file descriptor of child processes.
struct sigaction sa; // Used by code segment handling reap of zombie processes.
struct addrinfo hints_tcp; //Stores a struct addr info, so as to initilize its relevant fields, for usage in getaddrinfo().
struct addrinfo *server_info; //pointer to a linked list of struct addrinfos which are the results of getaddrinfo().
struct sockaddr_storage client_addr; // To store information about remotely connecting client like: client IPv4 address, port etc.
socklen_t addr_size;// to store the size of client address.
int yes=1; // variable used by getsockopt() function.
char server_ip[INET_ADDSTRLEN]; // To store ip address of server.

memset(&hints_tcp, 0, sizeof(hints_tcp)); //to make sure struct is empty.
hints_tcp.ai_family = AF_INET; //Setting address family of struct addrinfo hints_tcp to IPv4.
hints_tcp.ai_socktype = SOCK_STREAM; // Setting socket type of struct addrinfo hints_tcp to TCP socket.
hints_tcp.ai_flags = AI_PASSIVE; // Fill in the IP automatically.

/*The getaddrinfo() function: sets up all the important structs for us */
if((status = getaddrinfo(HOSTNAME, PORT_TCP, &hints_tcp, &server_info))!=0){
  std::cerr << "getaddrinfo error: "<< gai_strerror(status) << '\n';
  return 1; //exit main if error.
}

/*NOTE:server_info is initialized to the first addrinfo struct of the linked
list of addrinfos it points to. For my program we only need the first one to create a
socket fd and bind it to the main server PORT_TCP (33362)*/

//Getting the file descriptor- socket()
  socket_fd = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);
  if(socket_fd == -1){
    perror("server:socket");
    exit(1);
  }
  // setsockopt(): to allow reuse of the port.
  if(setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes))== -1){
    perror("setsockopt");
    exit(1);
  }
  //Binding TCP port to the socket (socket file descriptor)- bind().
  if(bind(socket_fd, server_info->ai_addr, server_info-> ai_addrlen)== -1){
    //close(socket_fd);
    perror("server:bind");
    exit(1);
  }
//If server_info struct is NULL, then no server address to bind to, hence server fails to bind.
if(server_info==NULL){
  std::cerr << "server: failed to bind."<< '\n';
  exit(1);
}
//listen(): server set to listen on the PORT 33362.socket_fd was earlier bound to the PORT_TCP 33362.
if(listen(socket_fd,BACKLOG) == -1){
  perror("listen");
  exit(1);
}


//MAIN SERVER UDP SOCKET SETUP
int socket_udp_fd; //To store the socket file descriptor for the main server.
struct addrinfo hints_udp; //Stores a struct addr info, so as to initilize its relevant fields, for usage in getaddrinfo().
struct addrinfo *server_main_info; //pointer to a linked list of struct addrinfos which are the results of getaddrinfo().
struct sockaddr_in server_a_addr;// To store information about the backend server A.
struct sockaddr_in server_b_addr;// To store information about backend server B.
yes=1;// used in getsockopt().

memset(&hints_udp, 0, sizeof(hints_udp)); //to make sure hints_udp is empty.
hints_udp.ai_family = AF_INET; //Setting address family of struct addrinfo hints_udp to IPv4.
hints_udp.ai_socktype = SOCK_DGRAM; // Setting socket type of struct addrinfo hints_udp to UDP socket.
hints_udp.ai_protocol= IPPROTO_UDP;//Transport Layer Protocol - UDP
hints_udp.ai_flags = AI_PASSIVE; // Fill in the IP as local host IP address.

/*The getaddrinfo() function: sets up all the important structs for us */
if((status = getaddrinfo(HOSTNAME, PORT_UDP, &hints_udp, &server_main_info))!=0){
  std::cerr << "getaddrinfo error: "<< gai_strerror(status) << '\n';
  return 1; //exit main if error.
}

/*NOTE:server_main_info is initialized to the first addrinfo struct of the linked
list of addrinfos it points to. For my program we only need the first one to create a
socket fd and bind it to the main server PORT_UDP (32362)*/

//Getting the file descriptor- socket()
socket_udp_fd = socket(server_main_info->ai_family, server_main_info->ai_socktype, server_main_info->ai_protocol);
if(socket_udp_fd == -1){
  perror("server:socket");
  exit(1);
  }

// setsockopt(): to allow reuse of the port.
  if(setsockopt(socket_udp_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes))== -1){
    perror("setsockopt");
    exit(1);
  }

//Binding UDP port to the socket (socket file descriptor)- bind().
if(bind(socket_udp_fd, server_main_info->ai_addr, server_main_info-> ai_addrlen)== -1){
    perror("server:bind");
    exit(1);
  }
//If server_main_info struct is NULL, then no server address to bind to, hence server fails to bind.
if(server_main_info==NULL){
  std::cerr << "server: failed to bind."<< '\n';
  exit(1);
}

/*Code segment to handle and reap all zombie processes.*/
sa.sa_handler = sigchld_handler;
sa.sa_flags = SA_RESTART;
if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
      }

cout<<'\n'<<"Main Server is up and running."<<'\n';

//Further Declarations and Initializations.
vector<string> country_list_A; //Vector to store list of country names handled by backend server A.
vector<string> country_list_B; //Vector to store list of country names handled by backend server B.
int bytes_sent; // no. of bytes sent by the main server.
int bytes_received; // no. of bytes received by the main server.
string server_main_status;

//Store information about Backend Server A.
memset(&server_a_addr,0, sizeof(server_a_addr));
server_a_addr.sin_family = AF_INET; //Server A address family= IPv4.
server_a_addr.sin_addr.s_addr = inet_addr(HOSTNAME); // Server A IP address= local host : 127.0.0.1
server_a_addr.sin_port = htons(30362); // Server A UDP Port= 30362.
socklen_t server_a_addr_len = sizeof(server_a_addr);

//Store information about Backend Server B.
memset(&server_b_addr,0, sizeof(server_b_addr));
server_b_addr.sin_family = AF_INET; // Server B address family: IPv4
server_b_addr.sin_addr.s_addr = inet_addr(HOSTNAME);// Server B IP address = local host: 127.0.0.1
server_b_addr.sin_port = htons(31362);// Server B UDP Port= 31362
socklen_t server_b_addr_len = sizeof(server_b_addr);

map<string, vector<string> > country_log;//Map to book-keep the countries handled by the respective backend servers.
server_main_status = "ON"; // Main server status message once it is up and running.
int server_main_status_len = server_main_status.size();
char serv_main_status[server_main_status_len]; //Char buffer to store main server status.
string backend_server_response; // To store backend server response (A or B) whenever needed.

//Stores server main status message into the char buffer.
for(i=0; i<server_main_status_len;i++){
  serv_main_status[i] = server_main_status[i];
}

//Sending an intimation to the backend server A that the main server is now ON.
bytes_sent = sendto(socket_udp_fd, serv_main_status, server_main_status_len, 0, (struct sockaddr*)&server_a_addr, server_a_addr_len);
if(bytes_sent == -1){
  std::cerr << "Couldn't Send." << '\n';
}

/*Main server receives country list sent by Backend server A.*/
string no_of_countries_A="";
char num_countries_A[MAXDATASIZE];
int num_of_countries_A;
//First, main server receives number of countries the Backend server A is handling.
bytes_received = recvfrom(socket_udp_fd, num_countries_A, MAXDATASIZE-1, 0, (struct sockaddr*)&server_a_addr, &server_a_addr_len);
if(bytes_received==-1){
  std::cerr << "Couldn't Receive." << '\n';
}
for(i=0;i<bytes_received;i++){
  no_of_countries_A+= num_countries_A[i];
}
num_of_countries_A = stoi(no_of_countries_A);
char backend_serv_response[MAXDATASIZE];

//For Loop to receive the country list sent by Backend server A.
for(j=0; j<num_of_countries_A;j++){
  bytes_received = recvfrom(socket_udp_fd, backend_serv_response, MAXDATASIZE-1, 0, (struct sockaddr*)&server_a_addr, &server_a_addr_len);
  if(bytes_received==-1){
    std::cerr << "Couldn't Receive." << '\n';
  }
  backend_server_response="";
  for(i=0;i<bytes_received;i++){
    backend_server_response+=backend_serv_response[i];
  }
  country_list_A.push_back(backend_server_response);
}

cout<<"Main server has received the country list from server A using UDP over port "<<PORT_UDP<<"."<<'\n';

//Sending an intimation to the backend server B that the main server is now ON.
bytes_sent = sendto(socket_udp_fd, serv_main_status, server_main_status_len, 0, (struct sockaddr*)&server_b_addr, server_b_addr_len);
if(bytes_sent == -1){
  std::cerr << "Couldn't Send." << '\n';
}

//Main server receives the list of country names handled by Backend server B.
string no_of_countries_B="";
char num_countries_B[MAXDATASIZE];
int num_of_countries_B;
//Main server receives number of countries the backend server B is handling.
bytes_received = recvfrom(socket_udp_fd, num_countries_B, MAXDATASIZE-1, 0, (struct sockaddr*)&server_b_addr, &server_b_addr_len);
if(bytes_received==-1){
  std::cerr << "Couldn't Receive." << '\n';
}

//For Loop to receive the country list sent by Backend server B.
for(i=0;i<bytes_received;i++){
  no_of_countries_B+= num_countries_B[i];
}
num_of_countries_B = stoi(no_of_countries_B);

for(j=0; j<num_of_countries_B;j++){
  bytes_received = recvfrom(socket_udp_fd, backend_serv_response, MAXDATASIZE-1, 0, (struct sockaddr*)&server_b_addr, &server_b_addr_len);
  if(bytes_received==-1){
    std::cerr << "Couldn't Receive." << '\n';
  }
  backend_server_response="";
  for(i=0;i<bytes_received;i++){
    backend_server_response+=backend_serv_response[i];
  }
  country_list_B.push_back(backend_server_response);
}

cout<<"Main server has received the country list from server B using UDP over port "<<PORT_UDP<<"."<<'\n';
cout<<"---------------------------"<<'\n';

//Printing out the list of countries handled by backend servers A and B respectively.
cout<<"Server A"<<'\n';
for(itr = country_list_A.begin(); itr!= country_list_A.end(); itr++){
  cout<<*itr<<'\n';
}
cout<<"---------------------------"<<'\n';
cout<<"Server B"<<'\n';
for(itr = country_list_B.begin(); itr!= country_list_B.end(); itr++){
  cout<<*itr<<'\n';
}

//Inserting elements into the country_log Map.
country_log.insert(pair<string, vector<string> >("A", country_list_A));
country_log.insert(pair<string, vector<string> >("B", country_list_B));
cout<<"---------------------------"<<'\n';
//MAIN SERVER BOOTUP COMPLETE.

/*At this point, Main Server has booted up and is listening for incoming connections
at PORT_TCP 33362. The socket fd: socket_fd is a TCP socket fd of the parent process that is
continuously listening on PORT_TCP 33362 for incoming connections.*/

cout<<"Server waiting for connections..."<<'\n';
int client_id= 0;

while(1){
  /*Infinite loop to continuously handle client requests from any number of
  clients until program terminated manually (CTRL+C).*/

  addr_size = sizeof(client_addr);
  vector<string> server_a_resp_userids; //vector to store user ID's sent back by backend server A as a response to the main server request.
  vector<string> server_b_resp_userids;
  /*Once a client tries to connect (via connect()) to the server listening
  on PORT_TCP 33362. Server accepts the connection and creates a new socket fd for
  the child process that will handle the clients request. The parent process socket_fd
  keep listening on the PORT_TCP for further incoming connections. */
  new_socket_fd = accept(socket_fd, (struct sockaddr*)&client_addr, &addr_size);
  if(new_socket_fd==-1){
    perror("accept");
    exit(1);
  }
  else if(new_socket_fd!=-1){
    // Once a new socket fd is created, a new client is connected to the server.
      client_id+=1;
  }

cout<<'\n'<<"Server got connection from client "<< client_id<<'\n';
while(!fork()){
  /*Infinite loop to handle any number of requests from a particular client
  until program is terminated manually (CTRL+C).*/

  //int bytes_received;//Store no. of bytes receieved by the client(return value of recv()).
  int country_name_len;
  string country_name = "";  // To store client request message recieved by the server as string.
  string user_id="";
  int user_id_len;
  char clientRequest[MAXDATASIZE]; //char buffer to store the client request.
  string request_client; //String to store the client request.
  int request_client_len;
  int comma; //Identifier for comma:","


  /*Receive client request*/
  bytes_received =recv(new_socket_fd, clientRequest, MAXDATASIZE-1, 0);
  if(bytes_received==-1){
    perror("recv");
    exit(1);
  }

  /*Conditon when the client is disconnected
  from its side, we need to close the respective TCP socket: new_socket_fd*/
  else if(bytes_received==0){
    close(new_socket_fd);
    cout<<"----------------------"<<'\n';
    cout<<"Connection from client "<< client_id<<" has been closed."<<'\n';
    return 3;
  }

/* This for loop segment extracts the country name and user ID
sent by the client as a part of its query in separate variables.
Note: The client query/request is a string of format: "<country name>,<user id>"*/
comma = 0;
for(i=0; i<bytes_received; i++){
  if(clientRequest[i]==','){
    comma=1;
  }
  if(comma==0){
    country_name+= clientRequest[i];
  }
  else if(comma == 1 && clientRequest[i]!= ',') {
    user_id+= clientRequest[i];
  }
}
country_name_len = country_name.size();
user_id_len = user_id.size();
cout<<"----------------------"<<'\n';
cout<<"Main server has received the request on User "<< user_id<<'\n'<<"in "<<country_name<<" from client "<<client_id<<" using TCP over port "<< PORT_TCP<<"."<<'\n';


/*Finding the respective Backend server that handles the user queried country name*/
string host_server; // To store the respective backend server ID hosting the user queried country name.
int match=0; //Match identifier
for(itr_map= country_log.begin(); itr_map!= country_log.end(); itr_map++){
  for(itr= (itr_map->second).begin(); itr!=(itr_map->second).end(); itr++){
    if(*itr == country_name){
      /*If a match for user entered country is found to be handled by either of
      the backend servers(A or B), then its a match--> match =1*/
      host_server = itr_map->first;
      match = 1;
    }
  }
}
if(match==0){
  //If no match was found, none of the servers hosts the user queried country name.
  host_server="none";
}
if(host_server=="A"){
  //If a match was found in backend server A.
  cout<<'\n'<<country_name<<" shows up in server "<<host_server<<"."<<'\n';

  /*Send the user queried county name and user ID to backend server A, requesting for the
  possible userids in the respective country handled that match with the user's interests.*/
  char req_country[country_name_len];
  for(i=0; i<country_name_len; i++){
    req_country[i] = country_name[i];
  }
  bytes_sent = sendto(socket_udp_fd, req_country, country_name_len, 0, (struct sockaddr*)&server_a_addr, server_a_addr_len);
  if(bytes_sent==-1){
    std::cerr << "Couldn't send." << '\n';
    exit(1);
  }
  char req_userID[user_id_len];
  for(i=0; i<user_id_len;i++){
    req_userID[i] = user_id[i];
  }
  bytes_sent = sendto(socket_udp_fd, req_userID, user_id_len,0, (struct sockaddr*)&server_a_addr, server_a_addr_len);
  if(bytes_sent==-1){
    std::cerr << "Couldn't send." << '\n';
    exit(1);
  }
  cout<<'\n'<<"The Main Server has sent request for "<<user_id<< " to server "<<host_server<<" using UDP over port "<< PORT_UDP<<"."<<'\n';

  /*Main server receives backend server A's response of the no. of user ids
  and then the user ids catering the user's request.*/
  string no_of_userids="";
  int no_of_users;
  char num_userids_recv[MAXDATASIZE];
  string server_a_response;
  string server_main_response="";
  int server_main_response_len;
  bytes_received =recvfrom(socket_udp_fd, num_userids_recv, MAXDATASIZE-1, 0, (struct sockaddr*)&server_a_addr, &server_a_addr_len);
  if(bytes_received==-1){
    std::cerr << "Couldn't receive." << '\n';
  }
  for(i=0;i<bytes_received;i++){
    no_of_userids+=num_userids_recv[i];
  }
  no_of_users = stoi(no_of_userids);
  char num_userids[no_of_userids.size()];
  for(i=0; i<no_of_userids.size(); i++){
    num_userids[i]= no_of_userids[i];
  }

  if(no_of_users == 0){
    /*If no user queried user id was not found in the respective country handled by backend server A*/
    cout<<'\n'<<"Main server has received 'User "<<user_id<<": Not found' from server "<<host_server<<"."<<'\n';
    cout<<"Main Server has sent message to client "<<client_id<<" using TCP over "<<PORT_TCP<<"."<<'\n';
    bytes_sent = send(new_socket_fd, num_userids, no_of_userids.size(), 0);
    if(bytes_sent==-1){
      perror("send");
      exit(1);
    }
  }

  else{
    //Backend Server A has found user ids of possible friends for the user queried user id.
    server_main_response= "";
    for(j=0;j<no_of_users;j++){
      /*For loop to repeatedly receive user ids being sent by backend server A.*/
      char serv_a_response[MAXDATASIZE];
      server_a_response="";
      bytes_received = recvfrom(socket_udp_fd, serv_a_response, MAXDATASIZE-1, 0, (struct sockaddr*)&server_a_addr, &server_a_addr_len);
      if(bytes_received==-1){
        std::cerr << "Couldn't receive." << '\n';
      }
      for(k=0; k<bytes_received;k++){
        server_a_response+= serv_a_response[k];
      }
      server_a_resp_userids.push_back(server_a_response);
    }// for loop ends.

    //Constructing a main server response to the client query.
    /*NOTE: The main server response here is a single string consisting of all the
    user ids received from server A separated by a comma*/
    for(itr= server_a_resp_userids.begin(); itr!= server_a_resp_userids.end(); itr++){
      server_main_response += (*itr+",");
    }

    /*Send Main server response to client*/
    server_main_response_len = server_main_response.size();
    char serv_main_response[server_main_response_len];
    for(i=0; i<server_main_response_len;i++){
        serv_main_response[i] = server_main_response[i];
      }
      bytes_sent = send(new_socket_fd, serv_main_response, server_main_response_len, 0);
      if(bytes_sent==-1){
        perror("send");
        exit(1);
      }
    cout<<'\n';
    cout<<"The Main Server has received searching result of User "<<user_id<<" from server "<<host_server<<"."<<'\n';
    cout<<"Main server has sent searching result(s) to client "<<client_id<<" using TCP over port "<<PORT_TCP<<"."<<'\n';
  }

  cout<<'\n';
  server_a_resp_userids.clear();
}

else if(host_server=="B"){
  //If a match was found in backend server B.
  cout<<'\n'<<country_name<<" shows up in server "<<host_server<<"."<<'\n';

  /*Send the user queried county name and user ID to backend server B, requesting for the
  possible userids in the respective country handled that match with the user's interests.*/

  char req_country[country_name_len];
  for(i=0; i<country_name_len; i++){
    req_country[i] = country_name[i];
  }
  bytes_sent = sendto(socket_udp_fd, req_country, country_name_len, 0, (struct sockaddr*)&server_b_addr, server_b_addr_len);
  if(bytes_sent==-1){
    std::cerr << "Couldn't send." << '\n';
    exit(1);
  }
  char req_userID[user_id_len];
  for(i=0; i<user_id_len;i++){
    req_userID[i] = user_id[i];
  }
  bytes_sent = sendto(socket_udp_fd, req_userID, user_id_len,0, (struct sockaddr*)&server_b_addr, server_b_addr_len);
  if(bytes_sent==-1){
    std::cerr << "Couldn't send." << '\n';
    exit(1);
  }
  cout<<'\n'<<"The Main Server has sent request for "<<user_id<< " to server "<<host_server<<" using UDP over port "<< PORT_UDP<<"."<<'\n';

  /*Main server receives backend server B's response of the no. of user ids
  and then the user ids catering the user's request.*/
  string no_of_userids="";
  int no_of_users;
  char num_userids_recv[MAXDATASIZE];
  string server_b_response;
  string server_main_response="";
  int server_main_response_len;
  bytes_received =recvfrom(socket_udp_fd, num_userids_recv, MAXDATASIZE-1, 0, (struct sockaddr*)&server_b_addr, &server_b_addr_len);
  if(bytes_received==-1){
    std::cerr << "Couldn't receive." << '\n';
  }
  for(i=0;i<bytes_received;i++){
    no_of_userids+=num_userids_recv[i];
  }
  //cout<<no_of_userids<<'\n';
  no_of_users = stoi(no_of_userids);
  //vector<string> possible_friends_of_user;
  char num_userids[no_of_userids.size()];
  for(i=0; i<no_of_userids.size(); i++){
    num_userids[i]= no_of_userids[i];
  }
  if(no_of_users == 0){
    /*If no user queried user id was not found in the respective country handled by backend server B*/
    cout<<'\n'<<"Main server has received 'User "<<user_id<<": Not found' from server "<<host_server<<"."<<'\n';
    cout<<"Main Server has sent message to client "<<client_id<<" using TCP over "<<PORT_TCP<<"."<<'\n';
    bytes_sent = send(new_socket_fd, num_userids, no_of_userids.size(), 0);
    if(bytes_sent==-1){
      perror("send");
      exit(1);
    }
  }

  else{
    //Backend Server B has found user ids of possible friends for the user queried user id.
    server_main_response="";
    for(j=0;j<no_of_users;j++){
      /*For loop to repeatedly receive user ids being sent by backend server B.*/
      char serv_b_response[MAXDATASIZE];
      server_b_response="";
      bytes_received = recvfrom(socket_udp_fd, serv_b_response, MAXDATASIZE-1, 0, (struct sockaddr*)&server_b_addr, &server_b_addr_len);
      if(bytes_received==-1){
        std::cerr << "Couldn't receive." << '\n';
      }
      for(k=0; k<bytes_received;k++){
        server_b_response+= serv_b_response[k];
      }
      server_b_resp_userids.push_back(server_b_response);
    }// for loop ends.

    //Constructing a main server response to the client query.
    /*NOTE: The main server response here is a single string consisting of all the
    user ids received from server A separated by a comma*/
    for(itr= server_b_resp_userids.begin(); itr!= server_b_resp_userids.end(); itr++){
      server_main_response += (*itr+",");
    }

    /*Send Main server response to client*/
    server_main_response_len = server_main_response.size();
    char serv_main_response[server_main_response_len];
      for(i=0; i<server_main_response_len;i++){
        serv_main_response[i] = server_main_response[i];
      }
    bytes_sent = send(new_socket_fd, serv_main_response, server_main_response_len, 0);
    if(bytes_sent==-1){
        perror("send");
        exit(1);
      }
    cout<<'\n';
    cout<<"The Main Server has received searching result of User "<<user_id<<" from server "<<host_server<<"."<<'\n';
    cout<<"Main server has sent searching result(s) to client "<<client_id<<" using TCP over port "<<PORT_TCP<<"."<<'\n';
}
cout<<'\n';
server_b_resp_userids.clear();
}

else if(host_server=="none"){
  //If no match for user queried country name was found
  cout<<'\n'<<country_name<<" does not show up in server A & B."<<'\n';
  cout<<'\n'<<"Main Server has sent '"<<country_name<<" : Not found' to client "<<client_id<<'\n'<<"using TCP over port "<<PORT_TCP<<"."<<'\n';
  string response_str = "Country Name: Not found"; //Relevant main server response to the client.
  int response_len = response_str.size();
  char response[response_len];
  for(i=0;i<response_len;i++){
    response[i] = response_str[i];
  }
/*Sending main server response to the client*/
  bytes_sent = send(new_socket_fd, response, response_len, 0);
  if(bytes_sent == -1){
    perror("send");
  }
}
cout<<'\n';
}// End of while(!fork()) loop.
}//End of while(1) Loop.
return 0;
}// int main() ends.
