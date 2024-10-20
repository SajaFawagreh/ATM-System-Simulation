#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <semaphore.h>
#include <sys/sem.h>
#include <fcntl.h>
#include <sys/msg.h>
#include <assert.h>

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

// Node structure for linking accounts in a queue
typedef struct node {	
    account_t *account;
    struct node *next;
} node_t;

// Structure for a queue containing linked account nodes
typedef struct {  
    node_t *front;
    node_t *rear;
    int size;
} queue_t;

// Allocates a new queue on the heap and returns a pointer to it
queue_t *alloc_queue(void) {
    queue_t *queue = malloc(sizeof(queue_t));  
    assert(queue != NULL);
    queue->front = NULL;  
    queue->rear = NULL;  
    queue->size = 0;  
    return queue;
}

/**
 * Allocates a new account structure on the heap and returns a pointer to it.
 *  
 * @param accountNo   The account number.
 * @param encodedPIN  The encoded PIN.
 * @param funds       The available funds.
 * @return            A pointer to the newly created account structure.
 */
account_t *new_account(const char accountNo[], int encodedPIN, double funds) {
    account_t *account = malloc(sizeof(account_t));
    assert(account != NULL);
    strcpy(account->accountNo, accountNo);
    account->encodedPIN = encodedPIN;
    account->funds = funds;
    account->attempts = 0;
    return account;
}

/** 
 * Enqueues (appends) a node to the end of the queue.
 * 
 * @param queue   A pointer to the queue.
 * @param account A pointer to the account node to enqueue.
 */ 
void enqueue(queue_t *queue, node_t *account)
{
    assert(queue != NULL);
    if (queue->size == 0) {
        queue->front = account;
    } else {
        queue->rear->next = account;
    }
    queue->rear = account;
    queue->size++;
}

/**
 * Removes white spaces from a string.
 * 
 * @param str The string to remove white spaces from.
 */
void removeWhiteSpace(char *str) {
    int i, j = 0;
    for (i = 0; str[i] != '\0'; ++i) {
        if (str[i] != ' ' && str[i] != '\n') {
            str[j++] = str[i];
        }
    }
    str[j] = '\0';
}

/** 
 * Reads a CSV file, creates account structures for each entry, enqueues them into a queue, and returns a pointer to the queue.
 * 
 * @param filename The name of the input CSV file.
 * @return         A pointer to the queue containing account structures.
 */
queue_t *read_CSV_file(const char filename[]) {
    account_t *account;
    node_t *p;
    queue_t *queue = alloc_queue();
    char accountNo[256];
    int encodedPIN;
    double funds;

    FILE *file = fopen(filename, "r");

    if (file == NULL) {
        printf("Failed to open the file.\n");
        return NULL;
    }

    char buffer[1024];
    fgets(buffer, sizeof(buffer), file);

    while (fscanf(file, "%255[^,],%d,%lf", accountNo, &encodedPIN, &funds) == 3) {
        removeWhiteSpace(accountNo);
        p = malloc(sizeof(node_t));
        account = new_account(accountNo, encodedPIN, funds);
        p->account = account;
        p->next = NULL;
        enqueue(queue, p);
    }

    fclose(file);

    return queue;
}

/**
 * Writes account data from a queue to a CSV file.
 * 
 * @param filename The name of the output CSV file.
 * @param queue    A pointer to the queue containing account structures.
 */
void write_CSV_file(const char filename[], queue_t *queue) {

    FILE *file = fopen(filename, "w");

    if (file == NULL) {
        printf("Failed to open the file.\n");
        exit(1);
    }

    fprintf(file, "Account No.,Encoded PIN,Funds available\n");

    node_t *temp = queue->front;

    while (temp != NULL) {
        account_t *account = temp->account;
        int accountNoAsInt;
        if (sscanf(account->accountNo, "%d", &accountNoAsInt) != 1) {
            fprintf(file, "%s,", account->accountNo);
        } else {
            fprintf(file, "%05d,", accountNoAsInt);
        }
        fprintf(file, "%d,", account->encodedPIN);
        fprintf(file, "%.2lf\n", (double)(account->funds));

        temp = temp->next;
    }

    fclose(file);
}

/**
 * Finds and returns an account with a specific account number in the queue.
 * 
 * @param queue      A pointer to the queue containing account structures.
 * @param accountNo  The account number to search for.
 * @return           A pointer to the found account, or NULL if not found.
 */
account_t *find_account(queue_t *queue, const char accountNo[]) {
    node_t *temp = queue->front;
    while (temp != NULL) {
        if (strcmp(temp->account->accountNo, accountNo) == 0) {
            return temp->account;
        }
        temp = temp->next;
    }

    return NULL;
}

/**
 * Prints the details of an account.
 * 
 * @param account  The account to print.
 */
void printAccount(account_t account) {
    printf("[ \n");
    printf("Account number: %s\n", account.accountNo);
    printf("PIN number: %d\n", account.encodedPIN);
    printf("Funds: %.2lf\n", account.funds);
    printf("Attempts: %d\n", account.attempts);
    printf("] \n");
}

/** 
 * Prints the details of all accounts in the queue.
 * 
 * @param queue  A pointer to the queue.
 */
void queue_print(const queue_t *queue) {
    assert(queue != NULL);
    assert(queue->size != 0);
    node_t *current;
    account_t account;
    for (current = queue->front;
         current != queue->rear;
         current = current->next) {
        account = *(current->account);
        printAccount(account);
    }
    account = *(current->account);
    printAccount(account);
}

int main(int argc, char *argv[]) {
    // Generate a key based on the path to the key file
    char abs_path[100];
    realpath("key_file.txt", abs_path);
    key_t key = ftok(abs_path, 1);

    if (key == -1) {
        perror("key");
        exit(1);
    }
    
    // Gets the message queue based on the key
    int msqid = msgget(key, 0);

    if (msqid == -1) {
        perror("msgget");
        exit(1);
    }

    // Initialize a queue and read data from CSV file
    queue_t *queue = alloc_queue();
    queue = read_CSV_file("DataBase.csv");

    while (1) {
        // Message structure for inter-process communication
        struct message msg;
        // Size of the message excluding the long type
        int msg_length = sizeof(msg) - sizeof(long);
        
        // Receive a message from the message queue
        if (msgrcv(msqid, &msg, msg_length, 1, 0) == -1) {
            perror("msgrcv");
            exit(1);
        }

        // Find the account in the queue based on account number
        account_t *account = find_account(queue, msg.data.account.accountNo);

        // Check if the message came from the ATM
        if (strcmp(msg.data.message, "ATM") == 0) {
            // Check if the account exists
            if (account != NULL) {
                // Check the operation type
                if (strcmp(msg.data.operation, "PIN") == 0) {
                    // Validate PIN
                    if (account->encodedPIN == (msg.data.account.encodedPIN - 1)) {
                        account->attempts = 0;  // Reset PIN attempts
                        strcpy(msg.data.message, "OK");
                    } 
                    else {
                        account->attempts++;
                        printf("Attempt number %d\n", account->attempts);

                        // Check PIN attempts and update message accordingly
                        if (account->attempts < 3) {
                            strcpy(msg.data.message, "PIN_WRONG");
                        } 
                        else {
                            account->accountNo[0] = 'X';
                            write_CSV_file("DataBase.csv", queue);
                            strcpy(msg.data.message, "BLOCKED");
                        }
                    }
                } 
                else if (strcmp(msg.data.operation, "BALANCE") == 0) {
                    // Retrieve account balance
                    msg.data.account.funds = account->funds;
                } 
                else if (strcmp(msg.data.operation, "WITHDRAW") == 0) {
                    // Process withdrawal
                    if (msg.data.account.funds > account->funds) {
                        strcpy(msg.data.message, "NSF");  // Insufficient funds
                    } 
                    else {
                        msg.data.account.funds = account->funds - msg.data.account.funds;
                        account->funds = msg.data.account.funds;
                        write_CSV_file("DataBase.csv", queue);
                        strcpy(msg.data.message, "FUNDS_OK");
                    }
                }
            } 

            else {
                // Account does not exist
                strcpy(msg.data.message, "NOT_EXIST");
            }

            // Set the message type for response
            msg.mtype = 2;

            // Send the response message to the queue
            if (msgsnd(msqid, &msg, msg_length, 0) == -1) {
		        perror("msgsnd");
		        exit(1);
	        }
        } 
        
        // Check if the message came from the DBeditor
        else if (strcmp(msg.data.message, "DBeditor") == 0) {
            if (account != NULL) {
                // Update existing account details
                account->encodedPIN = msg.data.account.encodedPIN;
                account->funds = msg.data.account.funds;
                write_CSV_file("DataBase.csv", queue);
                exit(EXIT_SUCCESS);
            } 
            
            else {
                // Create a new account if it doesn't exist
                account_t *new_acc = new_account(msg.data.account.accountNo, msg.data.account.encodedPIN, msg.data.account.funds);
                node_t *node;
                node = malloc(sizeof(node_t));
                node->account = new_acc;
                node->next = NULL;
                enqueue(queue, node);
                write_CSV_file("DataBase.csv", queue);
                exit(EXIT_SUCCESS);
            }
        }
    }

    return 0;
}
