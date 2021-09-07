// Shuvam Raj Satyal, 13102529
// Cedric Lim, 24026891

#include <unistd.h> 
#include <stdio.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <string.h> 
#include <sys/types.h>
#include <netdb.h>

#define PORT 30000
#define size_of_AAPL 503
#define size_of_TWTR 503
#define MAXLINE 30

struct date {
    char date_str[10];
    int year;
    int month;
    int day;
};

struct stock {
    int price;
};

int parse_date(char *str, struct date *dates, int index) { // minor fix: adjust to months with 30 or 31 days, february with 28 and 29?
    // Extract the first token, the year
    int curr = 0; // 0 for year, 1 for month, 2 for day
    char *found; // pointer to parse the given date
    char str_copy[10];
    strcpy(str_copy, str);

    while( (found = strsep(&str,"-")) != NULL ) {
        if (curr == 0) { // year
            if (atoi(found) > 2020 || atoi(found) < 2018) {
                return -1;
            }
            else {
                dates[index].year = atoi(found);
                // printf("year: %s\n",found);
            }
        }

        else if (curr == 1) { // month
            if (atoi(found) > 12 || atoi(found) < 1) {
                return -1;
            }
            else {
                dates[index].month = atoi(found);
                // printf("month: %s\n", found);
            }
        }

        else if (curr == 2) { // day
            if (atoi(found) > 31 || atoi(found) < 1) {
                return -1;
            }
            else {
                dates[index].day = atoi(found);
                // printf("day: %s\n", found);
            }
        }
        curr++;
    }

    // printf("str: %s\n", str_copy);
    strcpy(dates[index].date_str, str_copy);
    return 1;
}

void collect_data(char data_to_read[], int size, FILE** fp, struct date *dates, struct stock *stocks) { // collects data from a file
    int row = 0;
    int col = 0;
    int index = -1;
    
    while (fgets(data_to_read, size, *fp) != NULL) {         // reads the whole file
        col = 0;
        row++;

        if (row == 1) { // avoid reading the data headers  
            continue;
        }

        index++;

        char *value = strtok(data_to_read, ", ");           // reads a line
        while (value) {                                     // reads a single word
            // printf("col #: %d", col);                               
            if (col == 0) { // date column
                // printf("col 1: %s ", value); 

                if (parse_date(value, dates, index) == -1) {
                    // not a valid date
                    index--;
                    break;
                } 
            }

            if (col == 4) { // price column
                // printf("col 5 %s", value);  
                stocks[index].price = atoi(value);
            }
            
            value = strtok(NULL, ", ");
            col++;
        }
        // printf("%s", data_to_read);
        // printf("\n");
    }
}

void prices(char* date, char* date_pointer, char* result, struct date *company_dates, struct stock *company_stocks, int size) { // FIX
    // display stock prices on a given date
    // if params dont exists, server responds with "Unknown" to client screen
    // printf("Date: %s\n", date);

    //check date first
    int curr = 0; // 0 for year, 1 for month, 2 for day
    char *found; // pointer to parse the given date
    int isValid = 1;
    int isLeapYear = 0;
    int inFeb = 0;
    int isMonth30 = 0;
    int isUnknown = 0;
    // char str_copy[10];
    // strcpy(str_copy, str);

    // found = strsep(&date_pointer,"-");
    // printf("f: %s", (found));
    while( (found = strsep(&date_pointer,"-")) != NULL ) {
        
        if (curr == 0) { // year
            if (atoi(found) <= 1000 ) {
                isValid = 0;
            }
            else if (atoi(found) > 2020 || atoi(found) < 2018) {
                isUnknown = 1;
            }
            else if (((atoi(found) % 4 == 0) && (atoi(found) % 100 != 0)) || (atoi(found) % 400 == 0)) {
                isLeapYear = 1;
            }
        }

        else if (curr == 1) { // month
            if (atoi(found) > 12 || atoi(found) < 1) {
                isValid = 0;
            }
            else if (atoi(found) == 2) {
               inFeb = 1;
            }
            else if (atoi(found) == 4 || atoi(found) == 6 || atoi(found) == 9 || atoi(found) == 11) {
                isMonth30 = 1;
            }
        }

        else if (curr == 2) { // day
            if (atoi(found) > 31 || atoi(found) < 1) {
                isValid = 0;
            }
            else if (inFeb == 1 && isLeapYear == 1) {
                if (atoi(found) > 29) {
                    isValid = 0;
                }
            }
            else if (inFeb == 1 && isLeapYear == 0) {
                if (atoi(found) > 28) {
                    isValid = 0;
                }
            }
            else if (isMonth30 == 1) {
                if (atoi(found) > 30) {
                    isValid = 0;
                }
            }
        }
        else {
            break;
        }
        curr++;
    }

    if (isValid == 1) {
        int entry_exists = 0;
        int i;
        for (i = 0; i < size; i++) {
            
            if ( strcmp(company_dates[i].date_str, date) == 0 ) {
                // printf("company_date: %s\n", company_dates[i].date_str);

                char buffer[30];
                // printf("struct price: %d\n", company_stocks[i].price);

                sprintf(buffer, "%d", company_stocks[i].price);
                // printf("Price: %s\n", buffer);

                strcpy(result, buffer);
                // printf("result: %s", result);

                entry_exists = 1;
            }
        }
        if (entry_exists == 0) {
            strcpy(result, "Unknown");
        }
    }
    else if (isValid == 0){
        strcpy(result, "Invalid syntax");
    }
    else if (isUnknown == 1) {
        strcpy(result, "Unknown");
    }
    
}

int max_profit_helper(struct stock *company_stocks, int size) {
    // calc max profit of a share
    int max_profit = 0;
    int min_price = company_stocks[0].price;
    int price;
    int profit;
    // printf("helper max\n");
    int i;
    // printf("min: %d", min_price);
    for (i = 1; i < size; i++) { 
        // printf("i: %d", i);
        price = company_stocks[i].price;

        if (price < min_price) {
            min_price = price;
        } 

        profit = price - min_price;
        if (profit > max_profit) {
            max_profit = profit;
            // printf("Max: %d", max_profit);
        }
    }
    return max_profit;
}

void max_profit(char* stock_name, char *result, struct stock *company_stocks, int size) {

    int res = max_profit_helper(company_stocks, size);
    // printf("res: %d\n", res);

    char buffer[30];
   
    sprintf(buffer, "Maximum Profit for %s: %d", stock_name, res);
    
    strcpy(result, buffer);
}

int check_stock_name(char *token, char *company1, char *company2) {
    return strcmp(token, company1) == 0 || strcmp(token, company2) == 0;
}

int check_date_format(char* date) { // TODO
    return 1;
}

void copyCompanyName(char *name, const char *file_name) { // AAPL.csv (8 - 4)
    int i;
    for (i = 0; i < ( strlen(file_name) - 4) ; i++) {
        name[i] = file_name[i];
    }
    name[( strlen(file_name) - 4)] = '\0';
}


int main(int argc, char const *argv[]) // ./server AAPL.csv TWTR.csv 30000
{ 
    // printf("argv[1]:%s\n", argv[1]); //AAPL.CSV
    // printf("argv[2]:%s\n", argv[2]); //TWTR.csv
    // printf("argv[3]:%s\n", argv[3]); //30000 // NEED TO MODIFY CLIENT/SERVER connection code from socket tutorial

    char company1[MAXLINE];
    char company2[MAXLINE];

    copyCompanyName(company1, argv[1]);
    copyCompanyName(company2, argv[2]);

    // printf("company1: %s\n", company1);
    // printf("company2: %s\n", company2);

    printf("server started\n\n");

    struct date AAPL_dates[size_of_AAPL];
    struct stock AAPL_stocks[size_of_AAPL];

    struct date TWTR_dates[size_of_TWTR];
    struct stock TWTR_stocks[size_of_TWTR];
    
    // read from csv files
    FILE *AAPL_file_pointer; 
    FILE *TWTR_file_pointer;
    
    AAPL_file_pointer = fopen(argv[1], "r");
    TWTR_file_pointer = fopen(argv[2], "r");

    if (AAPL_file_pointer == NULL || TWTR_file_pointer == NULL) {
        printf("Failed to open one or both of the following:\n%s\n%s\n", argv[1], argv[2]);
        // throw error?
    }
    else {
        // printf("Files opened:\n%s\n%s\n", argv[1], argv[2]);

        char AAPL_to_read[size_of_AAPL];
        char TWTR_to_read[size_of_TWTR];

        // fills in the dates and stocks structs for each company
        collect_data(AAPL_to_read, size_of_AAPL, &AAPL_file_pointer, AAPL_dates, AAPL_stocks); 
        collect_data(TWTR_to_read, size_of_TWTR, &TWTR_file_pointer, TWTR_dates, TWTR_stocks);

        fclose(AAPL_file_pointer);
        fclose(TWTR_file_pointer);
    }

    // START OF SOCKETS
    int server_fd, new_socket, valread, *new_sock; 
    struct sockaddr_in address; 
    int opt = 1; 
    int addrlen = sizeof(address); 
       
    // Creating socket file descriptor 
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) 
    { 
        perror("socket failed"); 
        exit(EXIT_FAILURE); 
    } 
       
    // Forcefully attaching socket to the port 30000 
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, // | SO_REUSEPORT
                                                  &opt, sizeof(opt))) 
    { 
        perror("setsockopt"); 
        exit(EXIT_FAILURE); 
    } 

    address.sin_family = AF_INET; 
    address.sin_addr.s_addr = INADDR_ANY; 
    address.sin_port = htons( atoi(argv[3]) ); // PORT
       
    // Forcefully attaching socket to the port 30000 
    if (bind(server_fd, (struct sockaddr *)&address,  
                                 sizeof(address))<0) 
    { 
        perror("bind failed"); 
        exit(EXIT_FAILURE); 
    } 

    if (listen(server_fd, 3) < 0) // 2nd param: defines the maximum length to which the queue of pending connections for sockfd may grow. 
    { 
        perror("listen"); 
        exit(EXIT_FAILURE); 
    } 

    if ((new_socket = accept(server_fd, (struct sockaddr *)&address,  
                       (socklen_t*)&addrlen))<0) 
    { 
        perror("accept"); 
        exit(EXIT_FAILURE); 
    } 

    // used AAPL_dates and AAPL_stocks structs to respond to user input
   
    // char buffer[1024] = {0}; // this is content sent to the client
    while (1) {

        char buffer[1024] = {0}; // placement before or in while loop?
        valread = read( new_socket , buffer, 1024); // reads content sent from the client
        // printf("%s\n", buffer ); // shows client commands, FIX: show only certain commands that work?
        char buffer_copy[1024];
        strcpy(buffer_copy, buffer);

        char *buf = buffer;
        // printf("buf: %s\n", buf);
        char result[1024] = {0};

        char *found;
        char cmd[30];
        char stock_name[30];
        char date[30];

        int curr = 0;

        while( (found = strsep(&buf," ")) != NULL ) {
            // printf("found: %s\n",found);

            if (curr == 0) {
                strcpy(cmd, found);
                // printf("command: %s\n", cmd);
            }
            else if (curr == 1) {
                strcpy(stock_name, found);
                // printf("stock_name: %s\n", stock_name);
            }
            else if (curr == 2) {
                strcpy(date, found);
                // printf("date %s\n", date);
            }
            else {
                break;
            }
            curr++;
        }

        if (strcmp(cmd, "Prices") == 0) { // check date here
            // printf("Prices Function\n");
            // assign result
            if (strcmp(stock_name, company1) == 0) { 
                // printf("Company1...\n");
                char date_copy[30];
                strcpy(date_copy, date);
                char *d = date;
                prices(date_copy, d, result, AAPL_dates, AAPL_stocks, size_of_AAPL);
            }
            else if (strcmp(stock_name, company2) == 0) {
                // printf("Company2...\n");
                char date_copy[30];
                strcpy(date_copy, date);
                char *d = date;
                prices(date_copy, d, result, TWTR_dates, TWTR_stocks, size_of_TWTR);
            }
            else {
                strcpy(result, "Invalid syntax");
            }
            
        }

        else if (strcmp(cmd, "MaxProfit") == 0) {
            // printf("MaxProfit Function\n");

            if (strcmp(stock_name, company1) == 0) { 
                // printf("Company1...Profit\n");
                max_profit(stock_name, result, AAPL_stocks, size_of_AAPL);
            }
            else if (strcmp(stock_name, company2) == 0) {
                // printf("Company2...Profit\n");
                max_profit(stock_name, result, TWTR_stocks, size_of_TWTR);
            }
            else {
                strcpy(result, "Invalid syntax");
            }
        }
        else {
            // printf("Invalid command");
            // assign result
            strcpy(result, "Invalid command");
        }

        if (strcmp(result, "Invalid syntax") != 0) {
            printf("%s\n", buffer_copy );
        }
        
        // strcpy(result, "Testing Server Sent Message");
        // printf("Result to send: %s", result);
        send(new_socket , result , strlen(result) , 0 ); // result of calc is recorded in "hello"
        // printf("Server: Hello message sent\n"); 
    } 
    return 0; 
} 

