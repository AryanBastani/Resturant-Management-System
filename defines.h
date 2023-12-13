#ifndef DEFINES_H_INCLUDE
#define DEFINES_H_INDLUDE

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
#include <cjson/cJSON.h>

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

#define NOT_OPENED 0

#define SAME 1
#define NOT_SAME 0

#define GLOBAL_IP "255.255.255.255"

#define SHOWING 1
#define NOT_SHOWING 0

#define NUM_OF_CONNCS 2
#define UDP 0
#define TCP 1

#define BUFFER_NAME        32
#define RECIPE_FILEPATH  "../../../Assets/recipes.json"


struct Ingredient
{
    char* name;
    int amount;
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
};

struct Supplier
{
    char username[USERNAME_LEN];
    int port;
};

#endif