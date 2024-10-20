#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>
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

// Function to send and receive messages to/from the message queue
data_t message(int msqid, const char accountNo[], int encodedPIN, double funds, const char operation[]) {
	struct message msg;
    struct message msg1;
	data_t data;

    strcpy(data.account.accountNo, accountNo);
    data.account.encodedPIN = encodedPIN;
    data.account.funds = funds;
    data.account.attempts = 0;
    strcpy(data.operation, operation);
    strcpy(data.message, "ATM");

	msg.mtype = 1;
	msg.data = data;

    int msg_length = sizeof(msg) - sizeof(long);
    int msg_length1 = sizeof(msg1) - sizeof(long);

	if (msgsnd(msqid, &msg, msg_length, 0) == -1) {
		perror("msgsnd");
		exit(1);
	}

	while (1) {
        if (msgrcv(msqid, &msg1, msg_length1, 2, 0) == -1) {
            perror("msgrcv");
            exit(1);
        }
		return msg1.data;
	}

	perror("msgrec");
	exit(1);
}

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
    int msqid = msgget(key, IPC_CREAT | 0644);

    // Check if the message queue was created successfully
    if (msqid == -1) {
        perror("msgget");
        exit(1);
    }

    // Fork process for the DBserver
    pid_t pid1 = fork();

    if (pid1 < 0) {
        perror("Error forking process 1");
        exit(EXIT_FAILURE);

    } 
    
    else if (pid1 == 0) {
        // Execute DBserver
        execlp("./DBserver", "DBserver", NULL);
        perror("exec failed");
    } 

    else {
        // Variables for user input and account details
        char input[100];
        char account_number[256];
        int pin_number;
        double withdraw_amount;
        char operation[256];
        size_t len;
        bool accountNo = true;
        bool pinNo = true;
        
        // Set the initial operation to "ACCOUNT"
        strcpy(operation, "ACCOUNT");

        while(1) {
            // Check the current operation
            if(strcmp(operation, "ACCOUNT") == 0){
                while(accountNo){
                    printf("Please enter your 5-digit Account Number or 'X' to exit: ");
                    fgets(input, sizeof(input), stdin);
                    len = strlen(input);
                    if (len > 0 && input[len - 1] == '\n') {
                        input[len - 1] = '\0';
                    }

                    // Exit if the user enters 'X'
                    if(strcmp(input, "X") == 0){
                        kill(pid1, SIGTERM);
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
                strcpy(operation, "PIN");
            }

            else if (strcmp(operation, "PIN") == 0){
                while(pinNo){
                    printf("Please enter your 3-digit PIN number: ");
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
                // Get the result from the message queue
                data_t result = message(msqid, account_number, pin_number, 0, "PIN");

                // Check the result and update the operation accordingly
                // If the result is "OK", it will proceed to the banking operations (Balance, Withdraw)
                if (strcmp(result.message, "OK") == 0) {
                    printf("Valid PIN\n");
                    strcpy(operation, "OK"); 
                } 

                // If the result is "PIN_WRONG", it will go back to the Account 
                else if (strcmp(result.message, "PIN_WRONG") == 0){
                    printf("Invalid PIN\n");
                    strcpy(operation, "ACCOUNT"); 
                }
                
                // If the result is "BLOCKED", it will go back to the Account 
                else if (strcmp(result.message, "BLOCKED") == 0) {
                    printf("Account Blocked\n");
                    strcpy(operation, "ACCOUNT"); 
                }

                // If the result is "NOT_EXIST", it will go back to the Account 
                else if (strcmp(result.message, "NOT_EXIST") == 0) {
                    printf("Account does not exist\n");
                    strcpy(operation, "ACCOUNT"); 
                }
            }

            else if (strcmp(operation, "OK") == 0){
                printf("Please choose a banking operation from the following options:\n");
                printf("1. Balance\n");
                printf("2. Withdraw\n");
                fgets(input, sizeof(input), stdin);

                len = strlen(input);
                if (len > 0 && input[len - 1] == '\n') {
                    input[len - 1] = '\0';
                }

                // Check the user input for Balance or Withdraw
                if(strcmp(input, "Balance") == 0){
                    // Get the result from the message queue
                    data_t result = message(msqid, account_number, pin_number, 0, "BALANCE");
                    printf("Your current balance is %.2lf \n", result.account.funds);
                    strcpy(operation, "ACCOUNT");
                }

                else if(strcmp(input, "Withdraw") == 0){
                    printf("Please enter the amount you want to withdraw: ");
                    fgets(input, sizeof(input), stdin);
                    sscanf(input, " %lf", &withdraw_amount);
                    // Get the result from the message queue
                    data_t result = message(msqid, account_number, pin_number, withdraw_amount, "WITHDRAW");
                
                    // Check the result and display the appropriate message
                    if (strcmp(result.message, "NSF") == 0) {
                        printf("Withdrawal operation unsuccessful\n");
                        printf("Insufficient funds\n");
                    } 
                    else if (strcmp(result.message, "FUNDS_OK") == 0){
                        printf("Withdrawal operation successful\n");
                        printf("Your current balance is %.2lf \n", result.account.funds);
                    }

                    // Update the operation to "ACCOUNT"
                    strcpy(operation, "ACCOUNT");
                }
            }
        }
    }

    // Terminate the DBserver process
    kill(pid1, SIGTERM);

    return 0;
}
