#include<iostream>
#include<fstream>
#include<map>
#include<vector>
#include<string>
#include<iterator>
#include<typeinfo>
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

#define MYPORT "30362"
#define INET_ADDSTRLEN 16
#define MAXDATASIZE 100
#define HOSTNAME "127.0.0.1"


/*Function getString(char x): This function takes a single char input and returns it as a string object.
This function uses the string class constructor string s(size_s,x), that creates
a string s of size size_s and fills it with the input char x.*/
string getString(char x){
  string s(1,x);
  return s;
}

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
  //BACKEND SERVER A BOOTUP

  //Code Segment to read and store data from the file input.
  /*NOTE: This segment take input from the specific file dataA.txt.
  Incase of using a different file name change the name accordingly
  at the field indicated below.*/
  string line;
  ifstream datafile_test_cases("dataA.txt"); //**Kindly change the filename here if taking input from a different file name.
  int count = 0;
  int duplicate_count;
  vector <string> file_input_vector ; // vector to store each line of the file input as its element.
  if(datafile_test_cases.is_open())
  {
    while ( getline (datafile_test_cases,line)){
      file_input_vector.push_back(line);
    }

    datafile_test_cases.close();
  }

  else{
    cout << "Unable to open file.";
  }

  //Declarations and Initializations

  // Vector Iterators
  vector<string>:: iterator itr;
  vector<string>:: iterator itr2;

  int i,j,k,f;//Integers Iterators.
  //Map Iterators
  map<string,vector<string> >:: iterator itr_map2;//1D Map Iterator
  map<string, map<string, vector<string> > >:: iterator itr_map; //2D Map Iterator

  string country_name=" ";//String to store when a country name is read.
  string user_id = " "; //String to store when a user id for a country is read.
  vector<string> userID; //Vector to store all user IDs for a particular country.
  vector<string> countries; //Vector to store a list of all countries handled by this backend server.
  int group_no;
  string group_num;

  map<string, vector<string> > group;
  /* 1D Map: Stores group no. (1,2,3...) of a respective country as the keys and
  the respective user IDs part of this group no. as the coressponding elements of the keys.
  Map format: {key= Group No.: value= vector(all coressponsing user IDs)}
  Note: User ID's of the same group have matching interests.*/

  map<string, map<string, vector<string> > > data_set_a;
  /* Data set of A (2D Map): Stores country name as the keys and
  its respective group map as the coressponding elements of the keys.
  Map format: {key= Country Name : value= 1D MAP(Group no : vector(all corressponding user ID's))}
  */


//Reading the file vector to store and build the backend server dataset : data_set_a (Data set of A)
  for(i=0; i<file_input_vector.size(); i++){
    if(int(file_input_vector[i][0])>=48 && int(file_input_vector[i][0])<=57){//Main if
      /*If the first character of a line is a number. This line consists of all user IDs
      of a particular interest group(separated by spaces) handled by a particular country.*/

      for(j=0; j<file_input_vector[i].size(); j++){
        if(file_input_vector[i][j]!= ' '){
          user_id+= getString(file_input_vector[i][j]);
        }
        else if( file_input_vector[i][j] == ' '){
          //When a space is encountered, it means a single user id has been read.
          duplicate_count=0;
          for(itr=userID.begin(); itr!=userID.end();itr++){
            //To check if the just read user ID is repeating or not.
            //Do not store if it is repeating (meaning it was stored earlier.)
            if(user_id == *itr){
              //If user id already read before
              duplicate_count+=1;
            }
          }
          if(duplicate_count==0){
          //If user id was not read before (not a duplicate) then store it in the user ids vector.
          userID.push_back(user_id);
          }
          user_id = "";
        }

      }
      //To store the last user ID in the line, if not read before.
      duplicate_count=0;
      for(itr=userID.begin(); itr!=userID.end();itr++){
        if(user_id == *itr){
          duplicate_count+=1;
        }
      }
      if(duplicate_count==0){
        userID.push_back(user_id);
      }
      group_no+=1;
      group_num = to_string(group_no);
      /*Storing an interest group and its corressponding user ids of a
      particular country in the group map*/
      group.insert(pair<string, vector<string> >(group_num, userID));
      userID.clear();
      user_id="";
    }// Main if ends.

    else{//Main else
      //Else the line consists of a country name.
      string b;
      if(country_name!=" "){
      /*Storing {country name: map of its coressponding interest groups} as an
      element of the 2D map data_set_a.*/
      data_set_a.insert(pair<string,map<string, vector<string> > >(country_name, group));
      //Storing country name in the countires list vector.
      countries.push_back(country_name);
      user_id = "";
      userID.clear();
      group.clear();
      country_name=" ";
      }
      if(country_name==" "){
        //Read the country name into a string variable.
        group_no=0;
        country_name = file_input_vector[i];
      }
    }//Main else ends.
  }

//Inserting the last country name and its coressponding interest groups map.
data_set_a.insert(pair<string, map<string, vector<string> > > (country_name, group));
//Inserting the last country name into the country names vector.
countries.push_back(country_name);

//BACKEND SERVER A UDP SOCKET SETUP
int countries_num = countries.size();//Size of the country names vector = No. of countries handled by this server.
int status; // Stores the value returned by getaddrinfo() call.
int socket_fd; //To store the socket file descriptor for this backend server.
struct sigaction sa; // Used by code segment handling reap of zombie processes.
struct addrinfo hints; //Stores a struct addr info, so as to initilize its relevant fields, for usage in getaddrinfo().
struct addrinfo *server_a_info; //pointer to a linked list of struct addrinfos which are the results of getaddrinfo().
struct sockaddr_in server_main_addr;  // To store information about the main server: IPv4 address, port etc.
socklen_t server_main_addr_len;// to store the size of main server address.
int yes=1; // variable used by getsockopt() function.

memset(&hints, 0, sizeof(hints)); //to make sure struct is empty.
hints.ai_family = AF_INET; //Setting address family of struct addrinfo hints to IPv4.
hints.ai_socktype = SOCK_DGRAM; // Setting socket type of struct addrinfo hints to UDP socket.
hints.ai_protocol=IPPROTO_UDP;//Transport Layer Protocol used: UDP.
hints.ai_flags = AI_PASSIVE; // Fill in the IP automatically (as local host IP address: 127.0.0.1).

/*The getaddrinfo() function: sets up all the important structs for us */
if((status = getaddrinfo(NULL, MYPORT, &hints, &server_a_info))!=0){
  std::cerr << "getaddrinfo error: "<< gai_strerror(status) << '\n';
  return 1; //exit main if error.
}

/*NOTE:server_a_info is initialized to the first addrinfo struct of the linked
list of addrinfos it points to. For my program we only need the first one to create a
socket fd and bind it to the server A PORT (30362)*/

//Getting the file descriptor- socket()
socket_fd = socket(server_a_info->ai_family, server_a_info->ai_socktype, server_a_info->ai_protocol);
if(socket_fd == -1){
  perror("server:socket");
  exit(1);
  }
  //cout<<"Socket created successfully"<<'\n';

  // setsockopt(): to allow reuse of the port.
  if(setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes))== -1){
    perror("setsockopt");
    exit(1);
  }
  //Binding UDP port to the socket (socket file descriptor)- bind().
  if(bind(socket_fd, server_a_info->ai_addr, server_a_info-> ai_addrlen)== -1){
    close(socket_fd);
    perror("server:bind");
    exit(1);
  }
//If server_a_info struct is NULL, then no server address to bind to, hence server fails to bind.
if(server_a_info==NULL){
  std::cerr << "server: failed to bind"<< '\n';
  exit(1);
}
//SERVER A BOOTUP COMPLETE.
cout<<'\n'<<"Server A is up and running using UDP on port "<<MYPORT<<"."<<'\n';


/*Code segment to handle and reap all zombie processes.*/
sa.sa_handler = sigchld_handler;
sa.sa_flags = SA_RESTART;
if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
}

/*At this point, Server has booted up and is listening for incoming datagrams
at PORT 30362.*/
int bytes_sent; // no. of bytes sent by the backend server A.
int bytes_received; // no. of bytes received  by the backend server A.
server_main_addr_len = sizeof(server_main_addr);
string country_str;
int country_len;
string server_request_country;
string server_request_userid;
string server_request="";
char serv_request[MAXDATASIZE];
char serv_message[MAXDATASIZE];

/*Receiving status message from the main server.
This message intimates the backend server that the main server is ON.*/
bytes_received = recvfrom(socket_fd, serv_request, MAXDATASIZE-1, 0, (struct sockaddr*)&server_main_addr, &server_main_addr_len);
if(bytes_received==-1){
  std::cerr << "Couldn't receive." << '\n';
}

/* Sending List of countries handled by this backend server to the main server */
//Sending the no. of countries handled by this backend server (A).
string no_of_countries = to_string(countries.size());
char num_countries[no_of_countries.size()];
for(i=0; i<no_of_countries.size();i++){
num_countries[i]= no_of_countries[i];
}
bytes_sent = sendto(socket_fd, num_countries, no_of_countries.size(), 0, (struct sockaddr*)&server_main_addr, server_main_addr_len);
if(bytes_sent==-1){
  std::cerr << "Couldn't send." << '\n';
  exit(1);
}

//Sending all the country names handled by this backend server (A) one by one.
for(itr= countries.begin(); itr!= countries.end(); itr++){
  country_len = (*itr).size();
  country_str = *itr;
  char country[country_len];
  for(j=0;j<country_len;j++){
    country[j]= country_str[j];
  }
  bytes_sent =  sendto(socket_fd, country, country_len, 0,(struct sockaddr *)&server_main_addr, server_main_addr_len);
  if(bytes_sent==-1){
    std::cerr << "Couldn't send." << '\n';
    exit(1);
  }
}
cout<<"Server A has sent a country list to Main Server."<<'\n';

while(1){
  /*Infinite loop to continuously handle any number of main server requests until
  program is manually terminated (CTRL+C).*/
  cout<<"---------------------------"<<'\n';

  server_request_country="";
  int server_request_country_len;
  char serv_request_country[MAXDATASIZE];

  //Receiving country name request from the main server.
  bytes_received = recvfrom(socket_fd, serv_request_country, MAXDATASIZE-1, 0 ,(struct sockaddr*)&server_main_addr, &server_main_addr_len);
  if(bytes_received==-1){
    std::cerr << "Couldn't receive"<< '\n';
    return 3;
  }

  for(k=0; k<bytes_received;k++){
    server_request_country+= serv_request_country[k];
  }

  server_request_userid = "";
  int server_request_userid_len;
  char serv_request_userid[MAXDATASIZE];
  int user_id_len;

  //Receiving User ID request from the main server.
  bytes_received = recvfrom(socket_fd, serv_request_userid, MAXDATASIZE-1, 0,(struct sockaddr*)&server_main_addr, &server_main_addr_len);
  if(bytes_received==-1){
    std::cerr << "Couldn't receive." << '\n';
    return 3;
  }

  for(k=0; k<bytes_received; k++){
    server_request_userid+= serv_request_userid[k];
  }

  cout<<"Server A has received a request for finding possible friends of User "<< server_request_userid<<" in "<<server_request_country<<"."<<'\n';

  //Server A reply.
  server_request_userid_len = server_request_userid.size();
  vector<string> match_group_nos; //To store list of interest groups of the queried user ID in the respective country.
  string match_group_no;
  string match_country;

  //Match identifier variables
  int match;
  int match_count=0;

  string no_of_user_friends;
  vector<string> match_users;
  /*Vector consisting of all user ID's among various interest groups of the
  queried country name that match with the interests of the queried user ID.*/
  vector<string> possible_friends;
  /*Same as match_users just with all repeatitions of user IDs filtered out */


  /*for Loop to begin searching for user IDs with same interests as the queried user ID among the
  interest groups of the queried country name*/
  for(itr_map2= data_set_a[server_request_country].begin(); itr_map2!= data_set_a[server_request_country].end(); itr_map2++){
    match =0;
    for(itr= (itr_map2->second).begin(); itr!= (itr_map2->second).end(); itr++){
      if(*itr == server_request_userid){
        /*If queried user id found in a particular interest group of the
        queried country*/
        match_group_no = itr_map2->first;
        match_group_nos.push_back(itr_map2->first);
        match =1;
        break;
      }
    }
  if(match==1){
    match_count+=1;
    for(itr= (itr_map2->second).begin(); itr!= (itr_map2->second).end(); itr++) {
      if(*itr!= server_request_userid){
        /*Insert all the user ids coressponding to the interest groups in the match_group_nos
        list except the queried user id itself.*/
      match_users.push_back(*itr);
      }
    }
  }
  }

int no_of_user_str_len;
int serv_a_response_len;
string server_a_response;
  if(match_count!=0){
    /*Match for queried user ID was found in one or more of the interest groups
    of the queried country name.*/

    /* Code Segment to handle any repeatitions of user ids in the match_user vector
    by constructing a new vector possible_friends with user ids entered only once (Not
    repeated)*/
    possible_friends.push_back(match_users[0]);
    for(itr = match_users.begin(); itr!= match_users.end(); itr++){
      duplicate_count=0;
      for(i = 0; i<possible_friends.size(); i++){
        if((*itr) == possible_friends[i]){
          duplicate_count+=1;
        }
      }
      if(duplicate_count==0){
        possible_friends.push_back(*itr);
      }
    }
    cout<<'\n'<<"Server A found the following possible friends for User "<<server_request_userid<<" in "<<server_request_country<<": ";

    //Serially sending user ids of all possible friends of the queried user id in the queried country name one by one.
    no_of_user_friends = to_string(possible_friends.size());
    no_of_user_str_len = no_of_user_friends.size();
    char num_users[no_of_user_str_len];
    for(k=0; k< no_of_user_str_len; k++){
      num_users[k] = no_of_user_friends[k];
    }

    bytes_sent = sendto(socket_fd, num_users, no_of_user_str_len, 0, (struct sockaddr*)&server_main_addr, server_main_addr_len);
    if(bytes_sent==-1){
      std::cerr << "Couldn't send." << '\n';
    }

    for(itr = possible_friends.begin(); itr!= possible_friends.end(); itr++){
      server_a_response = *itr;
      cout<< *itr<<",";
      serv_a_response_len = server_a_response.size();
      char serv_a_response[serv_a_response_len];
      for(k=0; k<serv_a_response_len; k++){
        serv_a_response[k] = server_a_response[k];
      }

      bytes_sent = sendto(socket_fd,serv_a_response, serv_a_response_len, 0, (struct sockaddr*)&server_main_addr, server_main_addr_len);
      if(bytes_sent==-1){
        std::cerr << "Couldn't send." << '\n';
      }
    }
    cout<<"\n";
    cout<<'\n'<<"Server A has sent the result(s) to Main Server."<<'\n';
  }

  else if (match_count==0){
    /*No Match for queried user ID was found in any of the interest groups of
    the queried country name.*/
    cout<<'\n'<<"User "<< server_request_userid<<" does not show up in "<<server_request_country<<"."<<'\n';
    cout<<"Server A has sent 'User "<<server_request_userid<<" not found' to Main Server."<<'\n';
    no_of_user_friends ="0";
    no_of_user_str_len = no_of_user_friends.size();
    char num_users[no_of_user_str_len];
    for(k=0; k< no_of_user_str_len; k++){
      num_users[k] = no_of_user_friends[k];
    }
    //Sending the relevant response to let the main server know that queried user id was not found.
    bytes_sent = sendto(socket_fd, num_users, no_of_user_str_len, 0, (struct sockaddr*)&server_main_addr, server_main_addr_len);
    if(bytes_sent==-1){
      std::cerr << "Couldn't send." << '\n';
    }
  }
}//Infinite while loop ends.
close(socket_fd);
return 0;
}// int main() ends.
