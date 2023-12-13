#include "functions.c"

struct Recipe foods[MAX_FOODS];
int num_of_foods;

void load_foods() 
{
    cJSON* root = load_json();
    int food_id = 0;
    cJSON* recipe_item = NULL;
    cJSON_ArrayForEach(recipe_item, root) 
    {
        struct Recipe* recipe = &(foods[food_id]);
        strncpy(recipe->name, recipe_item->string, BUFFER_NAME);

        int ingredient_id = 0;
        cJSON* ingredient_item = NULL;
        cJSON_ArrayForEach(ingredient_item, recipe_item) 
        {
            recipe->ingredients[ingredient_id].name = strdup(ingredient_item->string);
            recipe->ingredients[ingredient_id].amount = ingredient_item->valueint;
            ingredient_id++;
        }
        recipe->num_of_ingrs = ingredient_id;
        food_id++;
    }
    num_of_foods = food_id;
    cJSON_Delete(root);
}

void recv_udp_cust(int sock, struct Customer* customer)
{
    char buffer[1024] = {0};
    char cmd[CMD_MAX] = {0};
    memset(buffer, 0, 1024);
    memset(cmd, 0 ,CMD_MAX);

    recv(sock, buffer, 1024, 0);
    read_cmd(buffer, cmd);
    
    if(same_cmd(cmd, UNIQUITY_SIGN))
        compare_names(customer->username, buffer);
    else if(same_cmd(cmd, SHOULD_PRINT_SIGN))
        print_consele(buffer);

}

void recv_tcp_cust(int socket, struct Customer* customer)
{
    char buffer[1024] = {0};
    char cmd[CMD_MAX] = {0};
    memset(buffer, 0, 1024);
    memset(cmd, 0, CMD_MAX);
    recv(socket , buffer, 1024, 0);
    read_cmd(buffer, cmd);

    if(same_cmd(cmd, SHOULD_PRINT_SIGN))
        print_consele(buffer);
}

void print_foods(struct Customer customer)
{
    char buffer[1024] = {0};
    char number[USERNAME_LEN] = {0};
    for(int i = 0; i < num_of_foods; i++)
    {
        memset(buffer, 0, 1024);
        memset(number, 0, USERNAME_LEN);

        int_to_str(i + 1, number);
        short_to_long(0, number, buffer, strlen(number));

        buffer[strlen(buffer)] = '-';
        buffer[strlen(buffer)] = ' ';

        short_to_long(strlen(buffer), foods[i].name,
            buffer, strlen(foods[i].name));
        
        buffer[strlen(buffer)] = '\n';

        write(1, buffer, strlen(buffer));
    }
    char massg[CMD_MAX] = {0};
    memset(massg, 0, CMD_MAX);
    add_name_to_massg(customer.username, massg, SHOW_MENU_LOG);
    write_log(massg);
}

void show_rests(int connections[NUM_OF_CONNCS], struct sockaddr_in 
    addresses[NUM_OF_CONNCS], struct Customer* customer)
{
    char buffer[1024] = {0};
    memset(buffer, 0, 1024);
    make_show_massg(buffer, customer->port, SHOW_RESTS);

    int a = sendto(connections[UDP], buffer, strlen(buffer), 0,
        (struct sockaddr *)&addresses[UDP], sizeof(addresses[UDP]));  

    char massg[CMD_MAX] = {0};
    memset(massg, 0, CMD_MAX);
    add_name_to_massg(customer->username, massg, SHOW_RESTS_LOG);
    write_log(massg);
}

int get_food_name(char* food_name)
{
    memset(food_name, 0, USERNAME_LEN);
    read(0, food_name, USERNAME_LEN);
    for(int i = 0; i <num_of_foods; i++)
    {
        if(same_cmd(food_name, foods[i].name))
            return(VALID_INGR);
    }
    return(INVALID_INGR);
}

void handle_answer(char* buffer, int sock)
{
    char cmd[CMD_MAX] = {0};
    char rest_name[USERNAME_LEN] = {0};
    memset(cmd, 0, CMD_MAX);
    memset(rest_name, 0, USERNAME_LEN);
    read_cmd(buffer, cmd);

    if((!same_cmd(cmd, FOOD_ACCEPTED))&&(!same_cmd(cmd, FOOD_REJECTED)))
        return;

    my_substr(buffer, rest_name, 3);
    memset(buffer, 0, 1024);
    short_to_long(0, rest_name, buffer, strlen(rest_name));
    if(same_cmd(cmd, FOOD_REJECTED))
        short_to_long(strlen(buffer), FOOD_REJECT_MASSG,
            buffer, strlen(FOOD_REJECT_MASSG));
    else
        short_to_long(strlen(buffer), FOOD_ACCEPT_MASSG,
        buffer, strlen(FOOD_ACCEPT_MASSG));
    write(1, buffer, strlen(buffer));
}

void order_food(struct Customer customer)
{
    int new_sock;
    char buffer[1024] = {0};
    char port_str[USERNAME_LEN] = {0};
    char username[USERNAME_LEN] = {0};
    char food_name[USERNAME_LEN] = {0};
    memset(buffer, 0, 1024);
    memset(port_str, 0, USERNAME_LEN);
    memset(username, 0, USERNAME_LEN);
    memset(port_str, 0, USERNAME_LEN);
    memset(food_name, 0, USERNAME_LEN);

    char massg[CMD_MAX] = {0};
    memset(massg, 0, CMD_MAX);
    add_name_to_massg(customer.username, massg, ORDERING_LOG);
    write_log(massg);


    write(1, GET_PORT_OF_REST, strlen(GET_PORT_OF_REST));
    read(0, port_str, USERNAME_LEN);
    int port = atoi(port_str);
    new_sock = connect_server(port);
    while(new_sock == -1)
    {
        write(1, INVALID_PORT, strlen(INVALID_PORT));
        read(0, port_str, USERNAME_LEN);
        int port = atoi(port_str);
        new_sock = connect_server(port);
    }

    write(1, GET_NAME_OF_FOOD, strlen(GET_NAME_OF_FOOD));
    while(!get_food_name(food_name))
        write(1, INVALID_INGR_MASSG, strlen(INVALID_INGR_MASSG));

    short_to_long(0, NEW_FOOD_REQ, buffer, strlen(NEW_FOOD_REQ));

    buffer[strlen(buffer)] = ' ';

    short_to_long(strlen(buffer), customer.
        username, buffer, strlen(customer.username));

    buffer[strlen(buffer)] = ' ';

    int_to_str(customer.port, port_str);
    short_to_long(strlen(buffer), port_str, buffer, strlen(port_str));

    buffer[strlen(buffer) - 1] = ' ';

    short_to_long(strlen(buffer), food_name, buffer, strlen(food_name));
    
    send(new_sock, buffer, strlen(buffer), MSG_NOSIGNAL);

    signal(SIGALRM, timeout_handler);
    siginterrupt(SIGALRM, 1);
    alarm(120);
    int recive = recv(new_sock, buffer, 1024, 0);
    alarm(0);
    send(new_sock, END_REQ_REST, strlen(END_REQ_REST), MSG_NOSIGNAL);
    if(recive != -1) 
        handle_answer(buffer, new_sock);
}

void handle_input_cust(int connections[NUM_OF_CONNCS], struct
    sockaddr_in addresses[NUM_OF_CONNCS], struct Customer* customer)
{
    char buffer[1024] = {0};
    char command[CMD_MAX] = {0}; 
    int a;

    memset(buffer, 0, 1024);
    read(0, buffer, 1024);
    memset(command, 0, CMD_MAX);
    read_cmd(buffer, command);

    if(same_cmd(command, SHOW_FOODS))
        print_foods(*customer);
    else if(same_cmd(command, SHOW_RESTS))
        show_rests(connections, addresses, customer);
    else if (same_cmd(command, ORDER_FOOD))
        order_food(*customer);
    else
    {
        a = sendto(connections[UDP], buffer, strlen(buffer), 0,
            (struct sockaddr *)&addresses[UDP], sizeof(addresses[UDP]));
        recv(connections[UDP], buffer, 1024, 0);
        memset(buffer, 0, 1024);

        char massg[CMD_MAX] = {0};
        memset(massg, 0, CMD_MAX);
        add_name_to_massg(customer->username, massg, SEND_UDP_LOG);
        write_log(massg);
    }
}

void start_supp(int max_sd, int connections[NUM_OF_CONNCS], fd_set master_set,
    struct sockaddr_in addresses[NUM_OF_CONNCS], struct Customer customer)
{

    while (1) 
    {
        int action_id = find_action(max_sd, master_set);
        int new_sock;

        switch (action_id)
        {
            case STDIN:
                handle_input_cust(connections, addresses, &customer);
                break;
            case UDP_CONNEC:
                recv_udp_cust(connections[UDP], &customer);
                break;
            case TCP_CONNEC:
                new_sock = accept_client(connections[TCP]);
                FD_SET(new_sock, &master_set);
                if (new_sock > max_sd)
                    max_sd = new_sock;
                break;
            default: //clients sending message
                recv_tcp_cust(action_id, &customer);
                break;
        }
    } 
}

struct Customer signup_cust(int port, int connections[
    NUM_OF_CONNCS], struct sockaddr_in udp_addr)
{
    struct Customer customer;
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

    memset(customer.username, 0, USERNAME_LEN);
    long_to_short(0, buffer, customer.username, 32);

    customer.port = port;

    return(customer);
}

void run_cust(int udp_port)
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

    struct Customer customer = signup_cust(tcp_port, connections, addresses[UDP]);
    notice(connections[UDP], CUSTOMER_USER_NOTICE, addresses[UDP], customer.username);
    start_supp(max_sd, connections, master_set, addresses, customer);
}

int main(int argc, char const *argv[])
{
    load_foods();

    run_cust(atoi(argv[1]));

    return(1);
}