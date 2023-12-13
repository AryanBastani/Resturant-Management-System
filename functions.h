#ifndef FUNCTIONS_H_INCLUDE
#define FUNCTIONS_H_INCLUDE

#include"defines.h"

int connect_server(int port);
int accept_client(int server_fd);
void long_to_short(int begin, char* long_str, char* short_str, int len);
void short_to_long(int begin, char* short_str, char* long_str, int len);
void erase_element(char* string, char* erased, int which_one);
int port_tcp_gener();
void my_substr(char* string, char* substr, int which_one);
int make_sock(int conn_type);
struct sockaddr_in make_addr(int port, int connec_type);
void send_massg(int sock, struct sockaddr_in destination);
int find_action(int max_sd, fd_set working_set);
void read_cmd(char* container, char* command);
int same_cmd(char* input, char* the_cmd);
void notice(int sock, char* notice_massg, struct sockaddr_in
    destination, char* username);
int int_max(int number);
void int_to_str(int number, char* string);

#endif