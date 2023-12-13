#include "functions.c"

void load_recipes(struct Resturant* restaurant) 
{
    cJSON* root = load_json();
    int food_id = 0;
    cJSON* recipe_item = NULL;
    cJSON_ArrayForEach(recipe_item, root) 
    {
        struct Recipe* recipe = &(restaurant->foods[food_id]);
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
    restaurant->num_of_foods = food_id;
    cJSON_Delete(root);
}

void recv_udp_rest(int sock, struct Resturant* resturant)
{
    char buffer[1024] = {0};
    char cmd[CMD_MAX] = {0};
    memset(buffer, 0, 1024);
    memset(cmd, 0, CMD_MAX);

    recv(sock, buffer, 1024, 0);
    read_cmd(buffer, cmd);

    if(is_show_req(buffer, SHOW_RESTS_REST))
    {
        char port_str[USERNAME_LEN], massg[CMD_MAX] = {0};
        memset(massg, 0, CMD_MAX);
        my_substr(buffer, port_str, 3);
        int server_port = atoi(port_str); 
        int new_sock = connect_server(server_port);
        make_show_massg(massg, resturant->port, resturant->username);
        memset(buffer, 0, 1024);

        short_to_long(0, SHOULD_PRINT_SIGN, buffer, strlen(SHOULD_PRINT_SIGN));
        buffer[strlen(buffer)] = ' ';

        short_to_long(strlen(buffer), massg, buffer, strlen(massg));
        send(new_sock, buffer, strlen(buffer), MSG_NOSIGNAL);
    }
    else if(same_cmd(cmd, UNIQUITY_SIGN))
        compare_names(resturant->username, buffer);
    else if(same_cmd(cmd, SHOULD_PRINT_SIGN))
        print_consele(buffer);

}

void show_supps(int connections[NUM_OF_CONNCS], struct sockaddr_in 
    addresses[NUM_OF_CONNCS], struct Resturant* resturant)
{
    char buffer[1024] = {0};
    memset(buffer, 0, 1024);
    make_show_massg(buffer, resturant->port, SHOW_SUPPS);

    int a = sendto(connections[UDP], buffer, strlen(buffer), 0,
        (struct sockaddr *)&addresses[UDP], sizeof(addresses[UDP]));  
    
    char massg[CMD_MAX] = {0};
    memset(massg, 0, CMD_MAX);
    add_name_to_massg(resturant->username, massg, SHOW_SUPPS_LOG);
    write_log(massg);
}

void print_food(int number, char* food_name)
{
    char food_line[CMD_MAX] = {0};
    char number_str[USERNAME_LEN] = {0};
    memset(food_line, 0, CMD_MAX);
    memset(number_str, 0, USERNAME_LEN);

    int_to_str(number + 1, number_str);
    short_to_long(0, number_str, food_line, strlen(number_str));
    food_line[strlen(number_str)] = '-';
    food_line[strlen(number_str) + 1] = ' ';

    short_to_long(strlen(number_str) + 2,
        food_name, food_line, strlen(food_name));
    food_line[strlen(food_line)] = ':';
    food_line[strlen(food_line)] = '\n';

    write(1, food_line, strlen(food_line));
}

void print_ingr(struct Ingredient ingr)
{
    char ingr_line[CMD_MAX] = {0};
    memset(ingr_line, 0, CMD_MAX);

    ingr_line[0] = '\t';

    short_to_long(1, ingr.name, ingr_line, strlen(ingr.name));
    short_to_long(strlen(ingr_line), " : ", ingr_line, 3);

    char amount[USERNAME_LEN];
    int_to_str(ingr.amount, amount);

    short_to_long(strlen(ingr_line), amount, ingr_line, strlen(amount));
    ingr_line[strlen(ingr_line)] = '\n';

    write(1, ingr_line, strlen(ingr_line));
}

void print_recipes(struct Resturant resturant)
{
    char ingredient[CMD_MAX] = {0};
    
    memset(ingredient, 0, CMD_MAX);
    for(int i = 0; i < resturant.num_of_foods; i++)
    {   
        print_food(i, resturant.foods[i].name);
        for(int j = 0; j < resturant.foods[i].num_of_ingrs; j++)
        {
            print_ingr(resturant.foods[i].ingredients[j]);
        }
    }

    char massg[CMD_MAX] = {0};
    memset(massg, 0, CMD_MAX);
    add_name_to_massg(resturant.username, massg, SHOW_RECIPES_LOG);
    write_log(massg);
}

void add_ingr(int amount, char* name, struct Resturant* resturant)
{
    int ingr_id;

    for(ingr_id = 0; ingr_id < resturant->num_of_ingrs; ingr_id++)
    {
        if(same_cmd(resturant->ingredients[ingr_id].name, name))
            break;
    }
    resturant->ingredients[ingr_id].name = strdup(name);
    if(ingr_id == resturant->num_of_ingrs)
    {
        resturant->num_of_ingrs++;
        resturant->ingredients[ingr_id].amount = amount;
    }
    else
        resturant->ingredients[ingr_id].amount += amount;
}

void handle_answer(char* buffer, struct Resturant*
    resturant, int amount, char* ingr_name)
{
    char cmd[CMD_MAX] = {0};
    memset(cmd, 0, CMD_MAX);
    read_cmd(buffer, cmd);

    if(same_cmd(cmd, IM_BUSY_REST))
        write(1, SUPP_IS_BUSY, strlen(SUPP_IS_BUSY));
    else if(same_cmd(cmd, ACCEPTED_REST))
    {
        write(1, ACCEPTED_MASSG, strlen(ACCEPTED_MASSG));
        add_ingr(amount, ingr_name, resturant);
    }
    else if(same_cmd(cmd, REJECTED_REST))
        write(1, REJECTED_MASSG, strlen(REJECTED_MASSG));
}

int get_ingr_name(char* ingr_name, struct Resturant resturant)
{
    read(0, ingr_name, USERNAME_LEN);
    for(int i = 0; i < resturant.num_of_foods; i++)
    {
        for(int j = 0; j < resturant.foods[i].num_of_ingrs; j++)
        {
            if(same_cmd(ingr_name, resturant.foods[i].ingredients[j].name))
                return(VALID_INGR);
        }
    }
    return(INVALID_INGR);
}

void req_ingr(struct Resturant* resturant)
{
    int new_sock;
    char buffer[1024] = {0};
    char port_str[USERNAME_LEN] = {0};
    char ingr_name[USERNAME_LEN] = {0};
    char ingr_amount[USERNAME_LEN] = {0};
    memset(buffer, 0, 1024);
    memset(port_str, 0, USERNAME_LEN);
    memset(ingr_name, 0, USERNAME_LEN);
    memset(ingr_amount, 0, USERNAME_LEN);

    char massg[CMD_MAX] = {0};
    memset(massg, 0, CMD_MAX);
    add_name_to_massg(resturant->username, massg, REQUEST_INGR_LOG);
    write_log(massg);

    write(1, GET_PORT_OF_SUPP, strlen(GET_PORT_OF_SUPP));
    read(0, port_str, USERNAME_LEN);
    int port = atoi(port_str);
    new_sock = connect_server(port);
    while(new_sock == -1)
    {
        write(1, INVALID_PORT, strlen(INVALID_PORT));
        memset(port_str, 0, USERNAME_LEN);
        read(0, port_str, USERNAME_LEN);
        int port = atoi(port_str);
        new_sock = connect_server(port);
    }

    write(1, GET_NAME_OF_INGR, strlen(GET_NAME_OF_INGR));
    while(!get_ingr_name(ingr_name, *resturant))
        write(1, INVALID_INGR_MASSG, strlen(INVALID_INGR_MASSG));

    write(1, GET_NUM_OF_INGR, strlen(GET_NUM_OF_INGR));
    read(0, ingr_amount, USERNAME_LEN);

    short_to_long(0, NEW_INGR_REQ, buffer, strlen(NEW_INGR_REQ));
    send(new_sock, buffer, strlen(buffer), MSG_NOSIGNAL);

    signal(SIGALRM, timeout_handler);
    siginterrupt(SIGALRM, 1);
    alarm(90);
    int recive = recv(new_sock, buffer, 1024, 0);
    alarm(0);
    send(new_sock, END_REQ_REST, strlen(END_REQ_REST), MSG_NOSIGNAL);
    if(recive != -1) 
        handle_answer(buffer, resturant, atoi(ingr_amount), ingr_name);
}

void print_rest_ingrs(struct Resturant resturant)
{
    char buffer[1024] = {0};
    for(int i = 0; i < resturant.num_of_ingrs; i++)
    {
        if(resturant.ingredients[i].amount == 0)
            continue;

        memset(buffer, 0, 1024);

        short_to_long(0, resturant.ingredients[i].name,
            buffer, strlen(resturant.ingredients[i].name));
        if(buffer[strlen(buffer) - 1] == '\n')
            buffer[strlen(buffer) - 1] = ' ';
        else
            buffer[strlen(resturant.ingredients[i].name)] = ' ';

        char amount[USERNAME_LEN] = {0};
        memset(amount, 0, USERNAME_LEN);
        int_to_str(resturant.ingredients[i].amount, amount);
        short_to_long(strlen(buffer), amount, buffer, strlen(amount));

        buffer[strlen(buffer)] = '\n';
        write(1, buffer, strlen(buffer));
    }
    if( resturant.num_of_ingrs == 0)
        write(1, NO_INGRS_MASSG, strlen(NO_INGRS_MASSG));

    char massg[CMD_MAX] = {0};
    memset(massg, 0, CMD_MAX);
    add_name_to_massg(resturant.username, massg, SHOW_INGRS_LOG);
    write_log(massg);
}

void state_to_str(char* state_str, int state)
{
    if(state == ACCEPTED_STATE)
        short_to_long(0, ACCEPTED_HIST_MASSG,
            state_str, strlen(ACCEPTED_HIST_MASSG));
    else
        short_to_long(0, REJECTED_HIST_MASSG,
            state_str, strlen(REJECTED_HIST_MASSG));
}

void make_history_massg(char* buffer, struct Request request)
{
    char state_str[USERNAME_LEN] = {0};
    memset(state_str, 0, USERNAME_LEN);

    short_to_long(0, request.username, buffer, strlen(request.username));
    buffer[strlen(buffer)] = ' ';

    short_to_long(strlen(buffer), request.food_name, buffer, strlen(request.food_name));
    buffer[strlen(buffer)] = ' ';

    state_to_str(state_str, request.state);
    short_to_long(strlen(buffer), state_str, buffer, strlen(state_str));
    buffer[strlen(buffer)] = '\n';
}

void print_history(struct Resturant resturant)
{
    char buffer[1024] = {0};
    for(int i = 0; i < resturant.num_of_reqs; i++)
    {
        if(resturant.requests_list[i].state == WAITING_STATE)
            continue;
        memset(buffer, 0, 1024);
        make_history_massg(buffer, resturant.requests_list[i]);
        write(1, buffer, strlen(buffer));
    }
    char massg[CMD_MAX] = {0};
    memset(massg, 0, CMD_MAX);
    add_name_to_massg(resturant.username, massg, SHOW_HISTORY_LOG);
    write_log(massg);
}

int decrease_the_ingr(int amount, char* name, struct Resturant* resturant)
{
    char fixed_name[CMD_MAX] = {0};
    memset(fixed_name, 0, CMD_MAX);
    short_to_long(0, name, fixed_name, strlen(name));
    fixed_name[strlen(fixed_name)] = '\n';
    for(int i = 0; i < resturant->num_of_ingrs; i++)
    {
        if(same_cmd(fixed_name, resturant->ingredients[i].name))
        {
            if(resturant->ingredients[i].amount < amount)
                return(NOT_ENOUGH_INGRS);
            else
            {
                resturant->ingredients[i].amount -= amount;
                return(ENOUGH_INGRS);
            }
        }
    }
    return(NOT_ENOUGH_INGRS);
}

int decrease_ingrs(struct Resturant* resturant)
{
    int req_index = resturant->current_req_indx;
    int food_index;
    char food_name[1024] = {0};
    char ingr_name[1024] = {0};
    memset(food_name, 0, 1024);
    memset(ingr_name, 0, 1024);
    short_to_long(0, resturant->requests_list[req_index].
        food_name, food_name, strlen(resturant->
            requests_list[req_index].food_name));

    for(food_index = 0; food_index < resturant->num_of_foods; food_index++)
    {
        if(same_cmd(food_name, resturant->foods[food_index].name))
            break;
    }

    for(int i = 0; i < resturant->foods[food_index].num_of_ingrs; i++)
    {
        memset(ingr_name, 0, 1024);
        short_to_long(0, resturant->foods[food_index].
            ingredients[i].name, ingr_name, strlen(
            resturant->foods[food_index].ingredients[i].name));
        int ingr_amount = resturant->
            foods[food_index].ingredients[i].amount; 
        if(decrease_the_ingr(ingr_amount, 
            ingr_name, resturant) == NOT_ENOUGH_INGRS)
            return(NOT_ENOUGH_INGRS);
    }
    return(ENOUGH_INGRS);
}

int handle_req_answer(char* buffer, struct Resturant* resturant)
{
    char cmd[CMD_MAX] = {0};
    memset(cmd, 0, CMD_MAX);
    int right_answer = 0;
    int socket = resturant->requests_list[
        resturant->current_req_indx].socket;

    read_cmd(buffer, cmd);
    memset(buffer, 0, 1024);


    if(same_cmd(cmd, YES_ANSWER))
    {
        right_answer = 1;
        if(decrease_ingrs(resturant) == NOT_ENOUGH_INGRS)
        {
            write(1, NOT_ENOUGH_MASSG, strlen(NOT_ENOUGH_MASSG));
            short_to_long(0, REJECTED_FOOD_REST, buffer, strlen(REJECTED_FOOD_REST));
            resturant->requests_list[resturant->current_req_indx].state = REJECTED_STATE;
        }
        else
        {
            short_to_long(0, ACCEPTED_FOOD_REST, buffer, strlen(ACCEPTED_FOOD_REST));
            write(1, DONE_MASSG, strlen(DONE_MASSG));
            resturant->requests_list[resturant->current_req_indx].state = ACCEPTED_STATE;
        }
        short_to_long(strlen(buffer), resturant->username, buffer, strlen(resturant->username));

        send(socket, buffer, strlen(buffer), MSG_NOSIGNAL);
    }
    else if(same_cmd(cmd, NO_ANSWER))
    {
        right_answer = 1; 

        write(1, DONE_MASSG, strlen(DONE_MASSG));
        resturant->requests_list[resturant->current_req_indx].state = REJECTED_STATE;

        short_to_long(0, ACCEPTED_FOOD_REST, buffer, strlen(ACCEPTED_FOOD_REST));
        short_to_long(strlen(buffer), resturant->username, buffer, strlen(resturant->username));
        send(socket, buffer, strlen(buffer), MSG_NOSIGNAL);
    }
    return(right_answer);
}

int find_req_indx(int port, struct Resturant resturant)
{
    for(int i = 0; i < resturant.num_of_reqs; i++)
    {
        if(resturant.requests_list[i].port == port &&
            resturant.requests_list[i].state == WAITING_STATE)
            return(i);
    }
    return(NOT_FOUND);
}

int request_exist(struct Resturant resturant)
{
    for(int i = 0; i < resturant.num_of_reqs; i++)
    {
        if(resturant.requests_list[i].state == WAITING_STATE)
            return(EXISTS);
    }
    return(NOT_EXISTS);
}

void handle_req(struct Resturant* resturant)
{
    int new_sock, port, req_indx;
    char buffer[1024] = {0};
    char port_str[1024] = {0};
    memset(buffer, 0, 1024);
    memset(port_str, 0, 1024);
    if(!request_exist(*resturant))
        write(1, WITHOUT_REQ, strlen(WITHOUT_REQ));
    else
    {
        char massg[CMD_MAX] = {0};
        memset(massg, 0, CMD_MAX);
        add_name_to_massg(resturant->username, massg, ANSWER_ORDER_LOG);
        write_log(massg);

        write(1, CHOOSE_REQ_PORT, strlen(CHOOSE_REQ_PORT));
        read(0, port_str, 1024);
        port = atoi(port_str);
        new_sock = connect_server(port);
        req_indx = find_req_indx(port, *resturant);
        while(new_sock == -1 || req_indx == NOT_FOUND)
        {
            try_again(port_str);
            port = atoi(port_str);
            new_sock = connect_server(port);
            req_indx = find_req_indx(port, *resturant);
        }
        resturant->current_req_indx = req_indx;

        memset(buffer, 0, 1024);
        write(1, PLEASE_ANSWER, strlen(PLEASE_ANSWER));
        read(0, buffer, 1024);
        while(!handle_req_answer(buffer, resturant))
            try_again(buffer);   
    }
}

void show_reqs(struct Resturant* resturant)
{
    char buffer[1024] = {0};
    char str_port[USERNAME_LEN] = {0};
    for(int i = 0; i < resturant->num_of_reqs; i++)
    {
        if(resturant->requests_list[i].state != WAITING_STATE)
            continue;

        memset(buffer, 0, 1024);
        memset(str_port, 0, USERNAME_LEN);
        short_to_long(0, resturant->requests_list[i].username, 
            buffer, strlen(resturant->requests_list[i].username));

        buffer[strlen(buffer)] = ' ';

        int_to_str(resturant->requests_list[i].port, str_port);
        short_to_long(strlen(buffer), str_port, buffer, strlen(str_port));

        buffer[strlen(buffer)] = ' ';
    
        short_to_long(strlen(buffer), resturant->requests_list[i].food_name, 
            buffer, strlen(resturant->requests_list->food_name));

        
        buffer[strlen(buffer)] = '\n';

        write(1, buffer, strlen(buffer));
    }

    char massg[CMD_MAX] = {0};
    memset(massg, 0, CMD_MAX);
    add_name_to_massg(resturant->username, massg, SHOW_REQS_LOG);
    write_log(massg);
}

void handle_input_rest(int connections[NUM_OF_CONNCS], struct
    sockaddr_in addresses[NUM_OF_CONNCS], struct Resturant* resturant)
{
    char buffer[1024] = {0};
    char command[CMD_MAX] = {0}; 
    int a;

    memset(buffer, 0, 1024);
    read(0, buffer, 1024);
    memset(command, 0, CMD_MAX);
    read_cmd(buffer, command);
    if(same_cmd(command, CLOSE_CMD))
    {
        if(request_exist(*resturant))
            write(1, STILL_HAVE_REQS, strlen(STILL_HAVE_REQS));
        else
        {
            (*resturant).opened = NOT_OPENED;
            notice(connections[UDP], CLOSED_NOTICE, addresses[TCP], (*resturant).username);
        }
    }
    else if(same_cmd(command, SHOW_SUPPS))
        show_supps(connections, addresses, resturant);
    else if(same_cmd(command, SHOW_RECIPES))
        print_recipes(*resturant);
    else if(same_cmd(command, SHOW_INGRS))
        print_rest_ingrs(*resturant);
    else if(same_cmd(command, REQ_INGR))
        req_ingr(resturant);
    else if(same_cmd(command, ANSWER_REQ))
        handle_req(resturant);
    else if(same_cmd(command, SHOW_REQS))
        show_reqs(resturant);
    else if(same_cmd(command, PRINT_HISTORY))
        print_history(*resturant);
    else
    {
        a = sendto(connections[UDP], buffer, strlen(buffer), 0,
            (struct sockaddr *)&addresses[UDP], sizeof(addresses[UDP]));
        recv(connections[UDP], buffer, 1024, 0);
        memset(buffer, 0, 1024);
            char massg[CMD_MAX] = {0};
        memset(massg, 0, CMD_MAX);
        add_name_to_massg(resturant->username, massg, SEND_UDP_LOG);
        write_log(massg);
    }
}

void end_req(struct Resturant* resturant)
{
    for(int i = 0; i < resturant->num_of_reqs; i++)
    {
        if(resturant->requests_list[i].state == WAITING_STATE)
        {
            resturant->requests_list[i].state = REJECTED_STATE;
            break;
        }
    }
}

void recv_tcp_rest(int socket, struct Resturant* resturant)
{
    char buffer[1024] = {0};
    char cmd[CMD_MAX] = {0};
    memset(buffer, 0, 1024);
    memset(cmd, 0, CMD_MAX);

    recv(socket , buffer, 1024, 0);
    read_cmd(buffer, cmd);
    if(same_cmd(cmd, NEW_FOOD_REQ))
    {
        char username[USERNAME_LEN] = {0};
        char port[USERNAME_LEN] = {0};
        char food_name[USERNAME_LEN] = {0};
        memset(username, 0, USERNAME_LEN);
        memset(port, 0, USERNAME_LEN);
        memset(food_name, 0, USERNAME_LEN);
        
        my_substr(buffer, username, 4);
        my_substr(buffer, port, 5);
        my_substr(buffer, food_name, 6);
        food_name[strlen(food_name) - 1] = '\0';
        if(!resturant->opened)
        {
            send(socket, FOOD_REJECTED, strlen(FOOD_REJECTED), MSG_NOSIGNAL);
            send(socket, resturant->username,
                strlen(resturant->username), MSG_NOSIGNAL);
        }
        else
        {
            resturant->requests_list[resturant->num_of_reqs].username = strdup(username);
            resturant->requests_list[resturant->num_of_reqs].port = atoi(port);
            resturant->requests_list[resturant->num_of_reqs].food_name = strdup(food_name);
            resturant->requests_list[resturant->num_of_reqs].socket = socket;
            resturant->requests_list[resturant->num_of_reqs].state = WAITING_STATE;
            resturant->num_of_reqs++;
        }
    }
    else if(same_cmd(cmd, END_REQ_SUPP))
        end_req(resturant);
    else if(same_cmd(cmd, SHOULD_PRINT_SIGN))
        print_consele(buffer);
}

void start_restu(int max_sd, int connections[NUM_OF_CONNCS], fd_set master_set,
    struct sockaddr_in addresses[NUM_OF_CONNCS], struct Resturant resturant)
{
    fd_set working_set;
    while (1) 
    {
        int action_id = find_action(max_sd, master_set);
        int new_sock;

        switch (action_id)
        {
            case STDIN:
                handle_input_rest(connections, addresses, &resturant);
                break;
            case UDP_CONNEC:
                recv_udp_rest(connections[UDP], &resturant);
                break;
            case TCP_CONNEC:
                new_sock = accept_client(connections[TCP]);
                FD_SET(new_sock, &master_set);
                if (new_sock > max_sd)
                    max_sd = new_sock;
                break;
            default: //clients sending message
                recv_tcp_rest(action_id, &resturant);
                break;
        }
    } 
}

struct Resturant signup_restu(int port, int connections[
    NUM_OF_CONNCS], struct sockaddr_in udp_addr)
{
    struct Resturant resturant;
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

    memset(resturant.username, 0, USERNAME_LEN);
    long_to_short(0, buffer, resturant.username, 32);

    resturant.opened = OPENED;
    resturant.port = port;
    resturant.num_of_ingrs = 0;
    resturant.num_of_reqs = 0;

    return(resturant);
}

void run_restu(int udp_port)
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
    if(connections[TCP] > connections[UDP])
        max_sd = connections[TCP];
    else
        max_sd = connections[UDP];
    FD_SET(0, &master_set);
    FD_SET(connections[UDP], &master_set);
    FD_SET(connections[TCP], &master_set);

    struct Resturant resturant = signup_restu(tcp_port, connections, addresses[UDP]);
    load_recipes(&resturant);
    notice(connections[UDP], RESTUREN_USER_NOTICE, addresses[UDP], resturant.username);
    notice(connections[UDP], OPENED_NOTICE, addresses[UDP], resturant.username);
    start_restu(max_sd, connections, master_set, addresses, resturant);
}

int main(int argc, char const *argv[]) 
{

    run_restu(8080);
    
    return 0;
}