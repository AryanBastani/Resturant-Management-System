#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include "cJSON-master/cJSON.c"

#define USERNAME_LEN 32
#define CMD_MAX 64
#define MAX_INGREDIENTS 20
#define MAX_FOODS 20

#define NOT_SENT 0
#define IS_SENT 1

#define STDIN 0
#define UDP_CONNEC 3
#define TCP_CONNEC 4

#define GET_USERNAME_MASS "Please enter username: "
#define USERNAME_MASS_LEN 23

#define RESTUREN_USER_NOTICE " as resturan entered"
#define SUPPLIER_USER_NOTICE " as supplier entered"
#define CUSTOMER_USER_NOTICE " as customer entered"
#define CLOSED_NOTICE " Resturant closed!"

#define OPENED_NOTICE " Resturant opened!"
#define OPENED 1

#define CLOSE_CMD "break"
#define SHOW_SUPPS "show suppliers"
#define SHOW_SUPPS_S "show suppliers "
#define SHOW_RECIPES "show recipes"
#define REQ_INGR "request ingredient"
#define NEW_INGR_REQ "new request ingredient!"
#define NEW_FOOD_REQ "new request food!"
#define IM_BUSY_SUPP "!!!\n"
#define IM_BUSY_REST "!!!"
#define END_REQ_REST "END!\n"
#define END_REQ_SUPP "END!"
#define ANSWER_REQ "answer request"
#define WITHOUT_REQ "you dont have request!\n"
#define PLEASE_ANSWER "your answer (yes/no): "
#define YES_ANSWER "yes"
#define NO_ANSWER "no"
#define ACCEPTED_SUPP "accepted!\n"
#define ACCEPTED_REST "accepted!"
#define REJECTED_SUPP "rejected!\n"
#define REJECTED_REST "rejected!"
#define INVALID_INPUT "invalid input!\n"
#define ACCEPTED_MASSG "your request accepted!\n"
#define REJECTED_MASSG "your request rejected!\n"
#define SUPP_IS_BUSY "supplier is busy!\n"
#define INVALID_INGR_MASSG "invalid name! try again: "
#define INVALID_PORT "invalid port! try again: "
#define SHOW_INGRS "show ingredients"
#define NO_INGRS_MASSG "you dont have any ingredients!\n"
#define SHOW_FOODS "show menu"
#define SHOW_RESTS "show resturants"
#define SHOW_RESTS_REST "show resturants "
#define ORDER_FOOD "order food"
#define IM_CLOSED "im closed!"
#define REST_IS_CLOSED "resturant is closed!"
#define FOOD_ACCEPTED "food accepted!"
#define FOOD_ACCEPT_MASSG " Resturant accepted and your food is ready!\n"
#define FOOD_REJECTED "food rejected!"
#define FOOD_REJECT_MASSG " Resturant rejected and cry about it!\n"
#define SHOW_REQS "show requests list"
#define CHOOSE_REQ_PORT "port of request: "
#define NOT_ENOUGH_MASSG "you dont have enough ingredients! so i reject the request.\n"
#define REJECTED_FOOD_REST "food rejected!\n"
#define ACCEPTED_FOOD_REST "food accepted!\n"
#define STILL_HAVE_REQS "you still have some requests waiting! so the resturant must be opened.\n"
#define SHOULD_PRINT_SIGN "<!>"
#define UNIQUITY_SIGN "<#>"
#define DONE_MASSG "done!\n"
#define PRINT_HISTORY "show sales history"
#define ACCEPTED_HIST_MASSG "accepted"
#define REJECTED_HIST_MASSG "denied"
#define NOT_UNIQUE "your username is not unique! try again: "

#define NOT_OPENED 0
#define NOT_FOUND -1
#define NOT_ENOUGH_INGRS -1
#define RD_WR 0666
#define ENOUGH_INGRS 1

#define SAME 1
#define NOT_SAME 0

#define GLOBAL_IP "255.255.255.255"

#define SHOWING 1
#define NOT_SHOWING 0

#define NUM_OF_CONNCS 2
#define UDP 0
#define TCP 1

#define BUFFER_NAME        32
#define RECIPE_FILEPATH  "recipes.json"

#define GET_PORT_OF_SUPP "port of suplier: "
#define GET_NAME_OF_INGR "name of ingredient: "
#define GET_NUM_OF_INGR "number of ingredient: "
#define GET_PORT_OF_REST "port of resturant: "
#define GET_NAME_OF_FOOD "name of food: "

#define NO 0
#define YES 1

#define VALID_INGR 1
#define INVALID_INGR 0
#define VALID_FOOD 1
#define INVALID_FOOD 0 
#define REJECTED_STATE -1
#define WAITING_STATE 0
#define ACCEPTED_STATE 1
#define EXISTS 1
#define NOT_EXISTS 0
#define UNIQUE_NUMBER -1

#define LOG_NAME "All_users"

#define SHOW_SUPPS_LOG "is getting suppliers info"
#define SHOW_RECIPES_LOG "is printing recipes"
#define SHOW_INGRS_LOG "is printing ingredients"
#define REQUEST_INGR_LOG "is requesting for some ingredients"
#define ANSWER_ORDER_LOG "is answering to the food order"
#define SHOW_REQS_LOG "is printing the food requests list"
#define SHOW_HISTORY_LOG "is printing the sales history"
#define SEND_UDP_LOG "is sending a massage in udp connection"
#define ANSWER_INGR_LOG "is answering to the ingredient request"
#define SHOW_MENU_LOG "is printing the menu"
#define SHOW_RESTS_LOG "is printing available resturatns"
#define ORDERING_LOG "is ordering food"


struct Ingredient
{
    char* name;
    int amount;
};

struct Request
{
    char* username;
    int port;
    char* food_name;
    int socket;
    int state;
};

struct Recipe
{
    char name[USERNAME_LEN];
    struct Ingredient ingredients[MAX_INGREDIENTS];
    int num_of_ingrs;
};

struct Resturant
{
    char username[USERNAME_LEN];
    int port;
    int opened;
    struct Ingredient ingredients[MAX_INGREDIENTS];
    struct Recipe foods[MAX_FOODS];
    int num_of_foods;
    int num_of_ingrs;
    struct Request requests_list[CMD_MAX];
    int num_of_reqs;
    int current_req_indx;
};

struct Supplier
{
    char username[USERNAME_LEN];
    int port;
    int have_req;
    int req_sock;
};

struct Customer
{
    char username[USERNAME_LEN];
    int port;
    char restu_name[USERNAME_LEN];
};

int connect_server(int port) 
{

    int fd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET; 
    server_address.sin_port = htons(port); 
    server_address.sin_addr.s_addr = INADDR_ANY;

    if (connect(fd, (struct sockaddr *)&server_address,
        sizeof(server_address)) < 0) { // checking for errors
        return(-1);
    }

    return fd;
}

int accept_client(int server_fd) 
{
    int client_fd;
    struct sockaddr_in client_address;
    int address_len = sizeof(client_address);
    
    client_fd = accept(server_fd, (struct sockaddr *)
        &client_address, (socklen_t*) &address_len);

    return client_fd;
}

void long_to_short(int begin, char* long_str, char* short_str, int len)
{
    for(int i = 0; i < len; i++)
        if(long_str[i + begin] != '\n')
            short_str[i] = long_str[i + begin];
}

void short_to_long(int begin, char* short_str, char* long_str, int len)
{
    for(int i = 0; i < len; i++)
        long_str[i + begin] = short_str[i];
}

void erase_element(char* string, char* erased, int which_one)
{
    int counter = 1, distance = 0, j;
    for(int i = 0; i < strlen(string); i++)
    {
        if(counter == which_one)
        {
            for(j = i; j < strlen(string); j++)
            {
                if(string[j] == ' ')
                    break;
                distance++;
            }
            if(j == strlen(string))
                return;
            counter++;
            i = j;
        }
        if(string[i] == ' ')
            counter++;
        erased[i - distance] = string[i];
    }
}
	
int port_tcp_gener()
{
    int low_thresh = 49152;
    int upp_thresh = 65535;
    int random;

    srand(time(0));
    random = (rand() % (upp_thresh - low_thresh + 1)) + low_thresh;

    return random;
}

void my_substr(char* string, char* substr, int which_one)
{
    int counter = 1, j;
    for(int i = 0; i < strlen(string); i++)
    {
        if(counter == which_one)
        {
            for(j = i; j <= strlen(string); j++)
            {
                if(string[j] == ' ' || j == strlen(string))
                    return;
                substr[j - i] = string[j];
            }
        }
        if(string[i] == ' ')
            counter++;
    }   
}

int make_sock(int conn_type)
{
    int sock, broadcast = 1, opt = 1;

    if(conn_type == UDP_CONNEC)
    {
        sock = socket(AF_INET, SOCK_DGRAM, 0);
        setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast));
        setsockopt(sock, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));
    }
    else
    {
        sock = socket(AF_INET, SOCK_STREAM, 0);
        setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    }
    return(sock);
}

struct sockaddr_in make_addr(int port, int connec_type)
{
    struct sockaddr_in bc_address;
    bc_address.sin_family = AF_INET; 
    bc_address.sin_port = htons(port);
    if(connec_type == TCP_CONNEC)
        bc_address.sin_addr.s_addr = INADDR_ANY;
    else
        bc_address.sin_addr.s_addr = inet_addr(GLOBAL_IP);

    return(bc_address);
}

void send_massg(int sock, struct sockaddr_in destination)
{
    char buffer[1024] = {0};
    memset(buffer, 0, 1024);
    read(0, buffer, 1024);
    int a = sendto(sock, buffer, strlen(buffer), 0,
        (struct sockaddr *)&destination, sizeof(destination));
}

int find_action(int max_sd, fd_set working_set)
{
    select(max_sd + 1, &working_set, NULL, NULL, NULL);
    for (int i = 0; i <= max_sd; i++)
    {
        if(FD_ISSET(i, &working_set))
                return(i);
    }
}

void read_cmd(char* container, char* command)
{
    for(int i = 0; i < 1024; i++)
    {
        if(container[i] == '\n')
            break;
        command[i] = container[i];
    }
}

int same_cmd(char* input, char* the_cmd)
{
    for(int i = 0; i < strlen(the_cmd); i++)
    {
        if(input[i] != the_cmd[i])
            return(NOT_SAME);
    }
    return(SAME);
}

void add_name_to_massg(char* name, char* new_massg, char* text)
{

    short_to_long(0, name, new_massg, strlen(name));
    new_massg[strlen(new_massg)] = ' ';

    short_to_long(strlen(new_massg), text, new_massg, strlen(text));
}

void print_consele(char * buffer)
{
    char line[1024] = {0};
    memset(line, 0, 1024);
    int first_len = strlen(SHOULD_PRINT_SIGN) + 1; 

    for(int i = first_len; i < strlen(buffer); i++)
    {
        line[i - first_len] = buffer[i];
    }

    write(1, line, strlen(line));
}

void write_log(char* msg)
{
    char path[1024];
    sprintf(path, "%s.log", LOG_NAME);
    int fd = open(path, O_WRONLY | O_APPEND);
    char buf[1024];
    sprintf(buf, "%s\n", msg);
    write(fd, buf, strlen(buf));
    close(fd);
}

void notice(int sock, char* notice_massg, struct sockaddr_in
    destination, char* username)
{
    char buffer[1024] = {0};
    memset(buffer, 0, 1024);
    int notice_len = strlen(notice_massg);
    int massage_len = strlen(username) + notice_len;
    char massage[CMD_MAX] = {0};
    memset(massage, 0, CMD_MAX);

    short_to_long(0, username, massage, notice_len);
    short_to_long(strlen(username),
        notice_massg, massage, notice_len);

    short_to_long(0, SHOULD_PRINT_SIGN,
        buffer, strlen(SHOULD_PRINT_SIGN));
    buffer[strlen(buffer)] = ' ';

    short_to_long(strlen(buffer), massage, buffer, massage_len);
    buffer[strlen(buffer)] = '\n';

    int a = sendto(sock, buffer, strlen(buffer), 0,
        (struct sockaddr *)&destination, sizeof(destination));
    write_log(massage);
}

int int_max(int number)
{
    int size = 1;
    int helper = 10;
    while(1)
    {
        if(number < helper)
            return((helper / 10));
        size++;
        helper *= 10;
    }
}

void int_to_str(int number, char* string)
{
    int max = int_max(number);
    max *= 10;
    int i = 0;
    do
    {
        number %= max;
        max /= 10;
        int a = number / max;
        string[i] = '0' + a;
        i++;
    } while (max != 1); 
}

char* read_file(const char* filename) 
{
    int fd = open(filename, O_RDONLY);

    struct stat st;
    fstat(fd, &st);

    char* buffer = malloc(st.st_size + 1);
    ssize_t bytes_read = read(fd, buffer, st.st_size);
    buffer[st.st_size] = '\0';
    close(fd);
    return buffer;
}

cJSON* load_json() 
{
    char* jsonAddress = RECIPE_FILEPATH;
    char* json = read_file(jsonAddress);
    cJSON* root = cJSON_Parse(json);
    return root;
}

void make_show_massg(char* buffer, int port, char* sub_massg)
{
    char port_str[USERNAME_LEN];
    int_to_str(port, port_str);
    short_to_long(0, sub_massg, buffer, strlen(sub_massg));
    buffer[strlen(sub_massg)] = ' ';
    short_to_long(strlen(sub_massg) + 1, port_str, buffer, strlen(port_str));
    buffer[strlen(buffer)] = '\n';
}

int is_show_req(char* buffer, char* show_massg)
{
    char erased[CMD_MAX] = {0};
    memset(erased, 0, CMD_MAX);
    erase_element(buffer, erased, 3);
    return(same_cmd(erased, show_massg));
}

void timeout_handler(int sig)
{
    write(1, "Timeout!\n", 9);
}

void try_again(char* buffer)
{
    write(1, INVALID_INPUT, strlen(INVALID_INPUT));
    memset(buffer, 0, 1024);
    read(0, buffer, 1024);
}

void compare_names(char* name, char* buffer)
{
    int new_sock;
    char fixed_name[USERNAME_LEN] = {0}; 
    char new_name[USERNAME_LEN] = {0};
    char tcp_str[USERNAME_LEN] = {0};
    memset(new_name, 0, USERNAME_LEN);
    memset(tcp_str, 0, USERNAME_LEN);
    memset(fixed_name, 0, USERNAME_LEN);

    my_substr(buffer, new_name, 2);
    my_substr(buffer, tcp_str, 3);

    short_to_long(0, name, fixed_name, strlen(name));
    fixed_name[strlen(fixed_name)] = '\n';

    if(strlen(new_name) == strlen(fixed_name) &&
        same_cmd(new_name, fixed_name))
        new_sock = connect_server(atoi(tcp_str));
}

void make_uniquity_massg(char* buffer, char* check_str, int tcp_port)
{
    char port_str[USERNAME_LEN] = {0};
    memset(port_str, 0, USERNAME_LEN);

    short_to_long(0, UNIQUITY_SIGN, check_str, strlen(UNIQUITY_SIGN));
    check_str[strlen(check_str)] = ' ';

    short_to_long(strlen(check_str), buffer, check_str, strlen(buffer));
    check_str[strlen(check_str)] = ' ';

    int_to_str(tcp_port, port_str);
    short_to_long(strlen(check_str), port_str, check_str, strlen(port_str));
    check_str[strlen(check_str)] = '\n';
}

void do_nothing(int sig)
{

}

int check_uniquity(int connections[NUM_OF_CONNCS],
    struct sockaddr_in udp_addr, char* buffer, int tcp_port)
{ 
    int is_unique;
    char check_str[1024] = {0};
    memset(check_str, 0, 1024);

    make_uniquity_massg(buffer, check_str, tcp_port);

    int a = sendto(connections[UDP], check_str, strlen(check_str), 0,
        (struct sockaddr *)&udp_addr, sizeof(udp_addr));
    signal(SIGALRM, do_nothing);
    siginterrupt(SIGALRM, 1);

    alarm(1);
        is_unique = accept_client(connections[TCP]);
    alarm(0);
    return(is_unique);
}