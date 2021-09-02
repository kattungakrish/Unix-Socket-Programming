#include<iostream>
#include<fstream>
#include<string>
#include<vector>
#include<iterator>
#include<typeinfo>
#include<cstdio>
#include<cstdlib>
#include<cstring>
#include<unistd.h>
#include<cerrno>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<netdb.h>
#include<arpa/inet.h>
#include<sys/wait.h>
#include<signal.h>
using namespace std;


#define SERVER_PORT "33362"
#define MAXDATASIZE 1000
#define INET_ADDSTRLEN 16
#define HOSTNAME "127.0.0.1"


int main(){


//Declarations and initializations.
int i,j,k; // Iterators (general)
int status; // Stores the value returned by getaddrinfo()
int socket_fd; //To store the socket file descriptor
struct addrinfo hints;//Stores a struct addr info, so as to initialize its relevant fields, for usage in getaddrinfo().
struct addrinfo *server_info; //pointer to a linked list of struct addrinfos which are the results of getaddrinfo().
struct sockaddr_in client_socket_addr; // To store information about client like: client IP address, port etc.
socklen_t client_socket_addr_len;
int yes=1; // variable used by getsockopt() function.

memset(&hints, 0, sizeof(hints)); //to make sure struct is empty.
hints.ai_family = AF_INET; //Setting address family of struct addrinfo hints to IPv4.
hints.ai_socktype = SOCK_STREAM; // Setting socket type of struct addrinfo hints to TCP socket.

/*The getaddrinfo() function: sets up all the important structs for us */
if((status = getaddrinfo(HOSTNAME, SERVER_PORT, &hints, &server_info))!=0){
  std::cerr << "getaddrinfo error: "<< gai_strerror(status) << '\n';
  return 1;
}

/*NOTE:server_info is initialized to the first addrinfo struct of the linked
list of addrinfos it points to. For my program we only need the first one to create a
socket fd and connect it to the coressponding main server listening on
the respective PORT (33362)*/

//Getting the socket file descriptor- socket()
socket_fd = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);
  if(socket_fd  == -1){
    perror("client:socket");
    close(socket_fd);
    exit(1);
  }
  //Connect client to server listening on its TCP PORT 33362.
  if(connect(socket_fd, server_info->ai_addr, server_info->ai_addrlen)==-1){
    perror("connect");
    cerr<<"Kindly make sure the main server is up and running."<<'\n';
    exit(1);
  }

//If server_info struct is NULL, then no server address to connect to , hence client fails to connect to the server
if(server_info==NULL){
  std::cerr << "Client failed to connect." << '\n';
  return 2; //to exit main().
}

cout<<'\n'<<"Client is up and running."<<'\n';
client_socket_addr_len = sizeof(client_socket_addr);
int get_sock_check = getsockname(socket_fd, (struct sockaddr*)&client_socket_addr, &client_socket_addr_len );
if(get_sock_check==-1){
  perror("getsockname");
  exit(1);
}

//Storing Client's Dynamic TCP port
unsigned short int client_port_no = client_socket_addr.sin_port;

while(1){
  /*Infinite Loop to continuously takes user input for a country name and their own
  User ID to form the User's query to the main server for the user ids of possible friends
  of the  user in the requested country name.*/

  //Declarations and Initializations
  string country_name;
  string user_id;
  cout<<'\n';
  cout<<"Enter country name: ";
  cin>> country_name; //User Input.
  cout<<"Enter user ID: ";
  cin>> user_id; //User Input.
  int country_name_len = country_name.size(); //Store the length of length of the country name string input by the user.
  int user_id_len = user_id.size(); //Store the length of the user id string input by the user.
  char response[MAXDATASIZE]; //respone char buffer, that stores the response received from the main sever.
  string response_str=""; //string to store response of the server
  int bytes_sent; //Store no. of bytes sent by the client (return value of send()).
  int bytes_received;//Store no. of bytes receieved by the client(return value of recv()).
  string possible_friend_user_str="";
  vector<string> recommendedUsers; //Vector to store user ids of possible friends of the user that are recommended by the main server.
  vector<string>:: iterator itr; //Vectore iterator.
  string query_to_main_server; //string to store user query in the format: "<country name>,<user id>".
  int query_to_main_server_len;//Stores the length of user query string.


  query_to_main_server = country_name+","+user_id; //Constructing user query to main server
  query_to_main_server_len = query_to_main_server.size();
  char queryToMain[query_to_main_server_len];

  //Loop to store user query to main server in a char buffer.
  for(i=0; i<query_to_main_server_len;i++){
    queryToMain[i] = query_to_main_server[i];
  }

  /*Send user query: queryToMain to the main server.*/
  bytes_sent = send(socket_fd, queryToMain, query_to_main_server_len, 0);
  //error checking for send()
  if(bytes_sent==-1){
    perror("send");
    exit(1);
  }

  cout<<'\n'<<"Client has sent "<< country_name<<" and User "<<user_id<<" to Main Server using TCP over port "<<client_port_no<<"."<<'\n';

  /*Receive main server 'response' to the user query sent earlier.
  response:char buffer that contains the relevant main server response*/
  bytes_received = recv(socket_fd, response, MAXDATASIZE-1, 0);
  //error checking for recv()
  if(bytes_received == -1){
      perror("recv");
      exit(1);
    }

  /*Conditon when the server closes the connection (is disconnected)
  from its side, we need to close the socket_fd*/
  else if(bytes_received == 0){
      close(socket_fd);
      cout<<"Server disconnected."<<'\n';
      return 3; //To exit main().
    }


  response_str= "";
  //Loop to store main server response char buffer as a string.
  for(i=0; i<bytes_received; i++){
    response_str+=response[i];
  }

  /*If response is 'Country Name: Not found', print that
  the respective country name (that was input) not found*/
  if(response_str == "Country Name: Not found"){
    cout<<'\n'<< country_name<<": Not found.";
  }

  else{

    /*Else if response is "0", means queried user ID was not found in the queried
    country name*/
    if(response_str=="0"){
      cout<<'\n'<<"User "<<user_id<<": Not found.";
    }

    else{
      /*Else response is a string of all possible friends (separated by commas:',')
      of the user recommended by the main server based on user's query and matching interests.*/
      cout<<'\n'<<"User ";
      cout<<response_str<<" is/are possible friend(s) of User "<<user_id<<" in "<<country_name<<"."<<'\n';

      /*For loop to store user IDs recommended by the main server in a vector*/
      for(i=0; i<response_str.size();i++){
       if(response_str[i]==','){
         recommendedUsers.push_back(possible_friend_user_str);
         possible_friend_user_str="";
       }
       else{
         possible_friend_user_str+=response_str[i];
       }
     }
    }
  }
  cout<<'\n'<<"-----Start a new query-----";
  cout<<'\n';
}// Infinite loop while(1) ends.
return 0;
}//int main() ends.
