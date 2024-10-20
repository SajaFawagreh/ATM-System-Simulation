#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <stdbool.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <semaphore.h>
#include <sys/sem.h>
#include <fcntl.h>
#include <sys/msg.h>

// Structure for the account details
typedef struct account {	
    char accountNo[256];                    
    int encodedPIN;
    double funds;
    int attempts;
} account_t;

// Structure for the data in the message
typedef struct data {
	char operation[256];
    account_t account;
    char message[256];
} data_t;

// Structure for the message in the message queue
struct message {
    long mtype;
    data_t data;  
};

int main(int argc, char *argv[]) {
    // Get the absolute path of the key file
    char abs_path[100];
    realpath("key_file.txt", abs_path);
    
    // Generate a key for the message queue
    key_t key = ftok(abs_path, 1);

    if (key == -1) {
        perror("key");
        exit(1);
    }
    
    // Get the message queue
    int msqid = msgget(key, 0);

    if (msqid == -1) {
        perror("msgget");
        exit(1);
    }

    // Variables for user input and account details
    char input[100];
    char account_number[256];
    int pin_number;
    double funds_available;
    struct message msg;
    data_t data;
    bool accountNo = true;
    bool pinNo = true;
    size_t len;

    while(1) {
        // Get account details from user input
        while(accountNo){
            printf("Please enter the 5-digit account number for the account you want to add or update or 'X' to exit: ");
            fgets(input, sizeof(input), stdin);
            len = strlen(input);
            if (len > 0 && input[len - 1] == '\n') {
                input[len - 1] = '\0';
            }

            // Exit if the user enters 'X'
            if(strcmp(input, "X") == 0){
                exit(EXIT_SUCCESS);
            }

            // Ensures that the account number is exactly 5 digits.
            if(strlen(input) > 5 || strlen(input) < 5){
                printf("The account number must be exactly 5 digits.\n");
            }

            else{
                accountNo = false;
            }
        }
        accountNo = true;
        strcpy(account_number, input);

        while(pinNo){
            printf("Please enter the 3-digit PIN number: ");
            fgets(input, sizeof(input), stdin);

            len = strlen(input);
            if (len > 0 && input[len - 1] == '\n') {
                input[len - 1] = '\0';
            }

            // Ensures that the pin number is exactly 3 digits.
            if(strlen(input) > 3 || strlen(input) < 3){
                printf("The PIN number must be exactly 3 digits.\n");
            }

            else{
                pinNo = false;
            }
        }
        pinNo = true;
        sscanf(input, " %d", &pin_number);

        printf("Please enter the funds available: ");
        fgets(input, sizeof(input), stdin);
        sscanf(input, "%lf", &funds_available);

        // Prepare message to be sent to the message queue
        msg.mtype = 1;
        strcpy(data.account.accountNo, account_number);
        data.account.attempts = 0;
        data.account.encodedPIN = pin_number - 1;  // Adjust the encodedPIN value
        data.account.funds = funds_available;
        strcpy(data.operation, "UPDATE_DB");
        strcpy(data.message, "DBeditor");

        msg.data = data;

        // Calculate the message length
        int msg_length = sizeof(msg) - sizeof(long);

        // Send the message to the message queue
        if (msgsnd(msqid, &msg, msg_length, 0) == -1) {
            perror("msgsnd");
            exit(1);
        }
    }

    return 0;
}