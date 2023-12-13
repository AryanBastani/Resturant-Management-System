#include "functions.c"

void recv_udp_supp(int sock, struct Supplier* supplier)
{
    char buffer[1024] = {0};
    char cmd[CMD_MAX] = {0};
    memset(buffer, 0, 1024);
    memset(cmd, 0, CMD_MAX);

    recv(sock, buffer, 1024, 0);
    read_cmd(buffer, cmd);

    if(is_show_req(buffer, SHOW_SUPPS_S))
    {
        char port_str[USERNAME_LEN], massg[CMD_MAX] = {0};
        memset(massg, 0, CMD_MAX);
        my_substr(buffer, port_str, 3);
        int server_port = atoi(port_str); 
        int new_sock = connect_server(server_port);
        make_show_massg(massg, supplier->port, supplier->username);
        memset(buffer, 0, 1024);

        short_to_long(0, SHOULD_PRINT_SIGN, buffer, strlen(SHOULD_PRINT_SIGN));
        buffer[strlen(buffer)] = ' ';

        short_to_long(strlen(buffer), massg, buffer, strlen(massg));
        send(new_sock, buffer, strlen(buffer), MSG_NOSIGNAL);
    }
    else if(same_cmd(cmd, UNIQUITY_SIGN))
        compare_names(supplier->username, buffer);
    else if(same_cmd(cmd, SHOULD_PRINT_SIGN))
        print_consele(buffer);
}

int handle_answer(char* buffer, struct Supplier* supplier)
{
    char cmd[CMD_MAX] = {0};
    memset(cmd, 0, CMD_MAX);
    int socket = supplier->req_sock;
    int right_answer = 0;

    read_cmd(buffer, cmd);
    if(same_cmd(cmd, YES_ANSWER))
    {
        supplier->have_req = NO;
        write(1, DONE_MASSG, strlen(DONE_MASSG));
        send(socket, ACCEPTED_SUPP, strlen(ACCEPTED_SUPP), MSG_NOSIGNAL);
        right_answer = 1;
    }
    else if(same_cmd(cmd, NO_ANSWER))
    {
        supplier->have_req = NO;
        write(1, DONE_MASSG, strlen(DONE_MASSG));
        send(socket, REJECTED_SUPP, strlen(REJECTED_SUPP), MSG_NOSIGNAL);
        right_answer = 1; 
    }
    return(right_answer);
}

void handle_req(struct Supplier* supplier)
{
    char buffer[1024] = {0};
    memset(buffer, 0, 1024);
    if(!supplier->have_req)
        write(1, WITHOUT_REQ, strlen(WITHOUT_REQ));
    else
    {
        write(1, PLEASE_ANSWER, strlen(PLEASE_ANSWER));
        read(0, buffer, 1024);
        while(!handle_answer(buffer, supplier))
            try_again(buffer);  
    }

    char massg[CMD_MAX] = {0};
    memset(massg, 0, CMD_MAX);
    add_name_to_massg(supplier->username, massg, ANSWER_INGR_LOG);
    write_log(massg);
}

void handle_input_supp(int connections[NUM_OF_CONNCS], struct
    sockaddr_in addresses[NUM_OF_CONNCS], struct Supplier* supplier)
{
    char buffer[1024] = {0};
    char command[CMD_MAX] = {0}; 
    int a;

    memset(buffer, 0, 1024);
    read(0, buffer, 1024);
    memset(command, 0, CMD_MAX);
    read_cmd(buffer, command);
    if(same_cmd(command, ANSWER_REQ))
        handle_req(supplier);
    else
    {
        a = sendto(connections[UDP], buffer, strlen(buffer), 0,
            (struct sockaddr *)&addresses[UDP], sizeof(addresses[UDP]));
        recv(connections[UDP], buffer, 1024, 0);
        memset(buffer, 0, 1024);

        char massg[CMD_MAX] = {0};
        memset(massg, 0, CMD_MAX);
        add_name_to_massg(supplier->username, massg, SEND_UDP_LOG);
        write_log(massg);
    }
}

void recv_tcp_supp(int socket, struct Supplier* supplier)
{
    char buffer[1024] = {0};
    char cmd[CMD_MAX] = {0};
    memset(buffer, 0, 1024);
    memset(cmd, 0, CMD_MAX);
    recv(socket , buffer, 1024, 0);
    read_cmd(buffer, cmd);
    if(same_cmd(cmd, NEW_INGR_REQ))
    {
        if(supplier->have_req)
            send(socket, IM_BUSY_SUPP, strlen(IM_BUSY_SUPP), MSG_NOSIGNAL);
        else
        {
            supplier->have_req = YES;
            supplier->req_sock = socket;
        }
    }
    else if(same_cmd(cmd, END_REQ_SUPP))
        supplier->have_req = NO;
    else if(same_cmd(cmd, SHOULD_PRINT_SIGN))
        print_consele(buffer);
}

void start_supp(int max_sd, int connections[NUM_OF_CONNCS], fd_set master_set,
    struct sockaddr_in addresses[NUM_OF_CONNCS], struct Supplier supplier)
{

    while (1) 
    {
        int action_id = find_action(max_sd, master_set);
        int new_sock;

        switch (action_id)
        {
            case STDIN:
                handle_input_supp(connections, addresses, &supplier);
                break;
            case UDP_CONNEC:
                recv_udp_supp(connections[UDP], &supplier);
                break;
            case TCP_CONNEC:
                new_sock = accept_client(connections[TCP]);
                FD_SET(new_sock, &master_set);
                if (new_sock > max_sd)
                    max_sd = new_sock;
                break;
            default: //clients sending message
                recv_tcp_supp(action_id, &supplier);
                break;
        }
    } 
}

struct Supplier signup_supp(int port, int connections[
    NUM_OF_CONNCS], struct sockaddr_in udp_addr)
{
    struct Supplier supplier;
    write(1, GET_USERNAME_MASS, USERNAME_MASS_LEN);

    char buffer[1024] = {0};
    memset(buffer, 0, 1024);
    read(0, buffer, 1024);
    int unique_name = check_uniquity(connections, udp_addr, buffer, port);
    while(unique_name != UNIQUE_NUMBER)
    {
        write(1, NOT_UNIQUE, strlen(NOT_UNIQUE));

        memset(buffer, 0, 1024);
        read(0, buffer, 1024);
        unique_name = check_uniquity(connections, udp_addr, buffer, port);
    }

    memset(supplier.username, 0, USERNAME_LEN);
    long_to_short(0, buffer, supplier.username, 32);

    supplier.port = port;
    supplier.have_req = NO;

    return(supplier);
}

void run_supp(int udp_port)
{
    int connections[NUM_OF_CONNCS];
    int max_sd, tcp_port = port_tcp_gener();
    struct sockaddr_in addresses[NUM_OF_CONNCS];
    fd_set master_set;

    addresses[UDP] = make_addr(udp_port, UDP_CONNEC);
    addresses[TCP] = make_addr(tcp_port, TCP_CONNEC);;
    connections[UDP] = make_sock(UDP_CONNEC);
    connections[TCP] = make_sock(TCP_CONNEC);
    bind(connections[UDP], (struct sockaddr *)&addresses[UDP], sizeof(addresses[UDP]));
    while(bind(connections[TCP], (struct sockaddr *)&addresses[TCP], sizeof(addresses[TCP]))==-1)
    {
        tcp_port = port_tcp_gener();
        addresses[TCP] = make_addr(tcp_port, TCP_CONNEC);
    }
    listen(connections[TCP], 4);

    FD_ZERO(&master_set);
    max_sd = connections[TCP];
    FD_SET(0, &master_set);
    FD_SET(connections[UDP], &master_set);
    FD_SET(connections[TCP], &master_set);

    struct Supplier supplier = signup_supp(tcp_port, connections, addresses[UDP]);
    notice(connections[UDP], SUPPLIER_USER_NOTICE, addresses[UDP], supplier.username);
    start_supp(max_sd, connections, master_set, addresses, supplier);
}

int main(int argc, char const *argv[]) 
{

    run_supp(atoi(argv[1]));
    
    return 0;
}