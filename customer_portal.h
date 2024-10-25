#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>

#define MAX_USERS 100
#define MAX_PASSWORD_LENGTH 50
#define MAX_FEEDBACK_LENGTH 200
#define MAX_TRANSACTIONS 100

typedef struct {
    long long account_number;
    char password[MAX_PASSWORD_LENGTH];
    double balance;
    char name[50];
    double loan_amount;   // Loan amount field
    int loan_status;      // 0 for no loan, 1 for loan approved
    int status;          // 0 for inactive, 1 for active
    char feedback[MAX_FEEDBACK_LENGTH]; // User feedback
    char transaction_history[MAX_TRANSACTIONS][100]; // Transaction history
    int transaction_count; // Number of transactions
} User;

User users[MAX_USERS];   // Array to store users
pthread_mutex_t userLock = PTHREAD_MUTEX_INITIALIZER; // Mutex for thread safety

// Function declarations
void loadUsers();
void saveUsers();
void initializeUsers(); // Function to initialize default users
int authenticateUser(int connectionFileDescriptor);
void viewBalance(int connectionFileDescriptor, int userIndex);
void deposit(int connectionFileDescriptor, int userIndex);
void withdraw(int connectionFileDescriptor, int userIndex);
void transferFunds(int connectionFileDescriptor, int userIndex);
void applyForLoan(int connectionFileDescriptor, int userIndex);
void changePassword(int connectionFileDescriptor, int userIndex);
void addFeedback(int connectionFileDescriptor, int userIndex);
void viewTransactionHistory(int connectionFileDescriptor, int userIndex);
void customer_portal(int connectionFileDescriptor);
int userCount = 0;
// Load users from the data file
// Load users from the data file
void loadUsers() {
    int fileDescriptor = open("userdata.dat", O_RDONLY);
    if (fileDescriptor == -1) {
        printf("User data file not found. Initializing users...\n");
        initializeUsers(); // Initialize users if the file doesn't exist
        return;
    }

    // Read the user data
    ssize_t bytesRead = read(fileDescriptor, users, sizeof(users));
    close(fileDescriptor);

    // Calculate the number of users loaded
    userCount = bytesRead / sizeof(User);
    printf("Debug: Loaded %d users\n", userCount); // Debug statement

    // Print loaded users for debugging
    for (int i = 0; i < userCount; i++) {
        printf("Account Number: %lld, Name: %s, Balance: %.2f\n",
               users[i].account_number, users[i].name, users[i].balance);
    }
}

// Save users to the data file
void saveUsers() {
    int fileDescriptor = open("userdata.dat", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fileDescriptor == -1) {
        perror("Error opening file for writing");
        return;
    }

    write(fileDescriptor, users, sizeof(users));
    close(fileDescriptor);
}

// Initialize some default users
void initializeUsers() {
    userCount = 3;
    // User 1
    users[0].account_number = 1001;
    strncpy(users[0].password, "password1", MAX_PASSWORD_LENGTH);
    users[0].balance = 5000.00;
    strncpy(users[0].name, "Alice", 50);
    users[0].loan_amount = 0.0;
    users[0].loan_status = 0;
    users[0].status = 1;
    users[0].transaction_count = 0;

    // User 2
    users[1].account_number = 1002;
    strncpy(users[1].password, "password2", MAX_PASSWORD_LENGTH);
    users[1].balance = 3000.00;
    strncpy(users[1].name, "Bob", 50);
    users[1].loan_amount = 0.0;
    users[1].loan_status = 0;
    users[1].status = 1;
    users[1].transaction_count = 0;

    // User 3
    users[2].account_number = 1003;
    strncpy(users[2].password, "password3", MAX_PASSWORD_LENGTH);
    users[2].balance = 10000.00;
    strncpy(users[2].name, "Charlie", 50);
    users[2].loan_amount = 0.0;
    users[2].loan_status = 0;
    users[2].status = 1;
    users[2].transaction_count = 0;

    // Save the initialized users to the file
    saveUsers();
}

// Authenticate user
int authenticateUser(int connectionFileDescriptor) {
    char readBuffer[1000];
    long long accountNumber;
    char password[MAX_PASSWORD_LENGTH];

    // Prompt for account number
    write(connectionFileDescriptor, "Enter account number: ", strlen("Enter account number: "));
    bzero(readBuffer, sizeof(readBuffer));
    read(connectionFileDescriptor, readBuffer, sizeof(readBuffer));
    accountNumber = atoll(readBuffer);

    // Prompt for password
    write(connectionFileDescriptor, "Enter password: ", strlen("Enter password: "));
    bzero(readBuffer, sizeof(readBuffer));
    read(connectionFileDescriptor, readBuffer, sizeof(readBuffer));
    strncpy(password, readBuffer, MAX_PASSWORD_LENGTH);
    password[strcspn(password, "\n")] = 0;  // Remove newline

    // Check user credentials
    for (int i = 0; i < MAX_USERS; i++) {
        if (users[i].account_number == accountNumber && strcmp(users[i].password, password) == 0) {
            return i; // Return index of the authenticated user
        }
    }

    write(connectionFileDescriptor, "Authentication failed.\n", strlen("Authentication failed.\n"));
    return -1; // Authentication failed
}

// View balance
void viewBalance(int connectionFileDescriptor, int userIndex) {
    char writeBuffer[1000];
    snprintf(writeBuffer, sizeof(writeBuffer), "Your balance is: %.2f\n", users[userIndex].balance);
    write(connectionFileDescriptor, writeBuffer, strlen(writeBuffer));
}

// Deposit function
void deposit(int connectionFileDescriptor, int userIndex) {
    char readBuffer[1000], writeBuffer[1000];
    double amount;

    // Prompt for deposit amount
    write(connectionFileDescriptor, "Enter amount to deposit: ", strlen("Enter amount to deposit: "));
    bzero(readBuffer, sizeof(readBuffer));
    read(connectionFileDescriptor, readBuffer, sizeof(readBuffer));
    amount = atof(readBuffer);

    pthread_mutex_lock(&userLock);
    if (amount > 0) {
        users[userIndex].balance += amount;
        snprintf(users[userIndex].transaction_history[users[userIndex].transaction_count++], 100, "Deposited: %.2f", amount);
        saveUsers();
        snprintf(writeBuffer, sizeof(writeBuffer), "Deposit successful. Your new balance: %.2f\n", users[userIndex].balance);
    } else {
        snprintf(writeBuffer, sizeof(writeBuffer), "Invalid deposit amount.\n");
    }
    pthread_mutex_unlock(&userLock);
    write(connectionFileDescriptor, writeBuffer, strlen(writeBuffer));
}

// Withdraw function
void withdraw(int connectionFileDescriptor, int userIndex) {
    char readBuffer[1000], writeBuffer[1000];
    double amount;

    // Prompt for withdrawal amount
    write(connectionFileDescriptor, "Enter amount to withdraw: ", strlen("Enter amount to withdraw: "));
    bzero(readBuffer, sizeof(readBuffer));
    read(connectionFileDescriptor, readBuffer, sizeof(readBuffer));
    amount = atof(readBuffer);

    pthread_mutex_lock(&userLock);
    if (amount > 0 && amount <= users[userIndex].balance) {
        users[userIndex].balance -= amount;
        snprintf(users[userIndex].transaction_history[users[userIndex].transaction_count++], 100, "Withdrew: %.2f", amount);
        saveUsers();
        snprintf(writeBuffer, sizeof(writeBuffer), "Withdrawal successful. Your new balance: %.2f\n", users[userIndex].balance);
    } else {
        snprintf(writeBuffer, sizeof(writeBuffer), "Invalid amount or insufficient balance.\n");
    }
    pthread_mutex_unlock(&userLock);
    write(connectionFileDescriptor, writeBuffer, strlen(writeBuffer));
}

// Transfer funds
void transferFunds(int connectionFileDescriptor, int userIndex) {
    char readBuffer[1000], writeBuffer[1000];
    long long targetAccountNumber;
    double amount;

    // Prompt for the target account number
    write(connectionFileDescriptor, "Enter the account number to transfer to: ", strlen("Enter the account number to transfer to: "));
    bzero(readBuffer, sizeof(readBuffer));
    read(connectionFileDescriptor, readBuffer, sizeof(readBuffer));
    targetAccountNumber = atoll(readBuffer);

    // Prompt for the amount to transfer
    write(connectionFileDescriptor, "Enter amount to transfer: ", strlen("Enter amount to transfer: "));
    bzero(readBuffer, sizeof(readBuffer));
    read(connectionFileDescriptor, readBuffer, sizeof(readBuffer));
    amount = atof(readBuffer);

    pthread_mutex_lock(&userLock);
    // Check if the target account exists
    int targetUserIndex = -1;
    for (int i = 0; i < MAX_USERS; i++) {
        if (users[i].account_number == targetAccountNumber) {
            targetUserIndex = i;
            break;
        }
    }

    if (targetUserIndex != -1 && amount > 0 && amount <= users[userIndex].balance) {
        users[userIndex].balance -= amount;  // Deduct from sender
        users[targetUserIndex].balance += amount;  // Add to receiver
        snprintf(users[userIndex].transaction_history[users[userIndex].transaction_count++], 100, "Transferred: %.2f to %lld", amount, targetAccountNumber);
        snprintf(users[targetUserIndex].transaction_history[users[targetUserIndex].transaction_count++], 100, "Received: %.2f from %lld", amount, users[userIndex].account_number);
        saveUsers();
        snprintf(writeBuffer, sizeof(writeBuffer), "Transfer successful. Your new balance: %.2f\n", users[userIndex].balance);
    } else {
        snprintf(writeBuffer, sizeof(writeBuffer), "Transfer failed. Check account number or balance.\n");
    }
    pthread_mutex_unlock(&userLock);
    write(connectionFileDescriptor, writeBuffer, strlen(writeBuffer));
}

// Apply for a loan
void applyForLoan(int connectionFileDescriptor, int userIndex) {
    char readBuffer[1000], writeBuffer[1000];
    double loanAmount;

    // Prompt for loan amount
    write(connectionFileDescriptor, "Enter loan amount: ", strlen("Enter loan amount: "));
    bzero(readBuffer, sizeof(readBuffer));
    read(connectionFileDescriptor, readBuffer, sizeof(readBuffer));
    loanAmount = atof(readBuffer);

    pthread_mutex_lock(&userLock);
    if (loanAmount > 0 && users[userIndex].loan_status == 0) { // Only if no existing loan
        users[userIndex].loan_amount = loanAmount;
        users[userIndex].loan_status = 1; // Mark loan as applied
        saveUsers();
        snprintf(writeBuffer, sizeof(writeBuffer), "Loan application successful for amount: %.2f\n", loanAmount);
    } else {
        snprintf(writeBuffer, sizeof(writeBuffer), "Loan application failed. Check amount or existing loan status.\n");
    }
    pthread_mutex_unlock(&userLock);
    write(connectionFileDescriptor, writeBuffer, strlen(writeBuffer));
}

// Change password
void changePassword(int connectionFileDescriptor, int userIndex) {
    char readBuffer[1000], writeBuffer[1000];
    char newPassword[MAX_PASSWORD_LENGTH];

    // Prompt for new password
    write(connectionFileDescriptor, "Enter new password: ", strlen("Enter new password: "));
    bzero(readBuffer, sizeof(readBuffer));
    read(connectionFileDescriptor, readBuffer, sizeof(readBuffer));
    strncpy(newPassword, readBuffer, MAX_PASSWORD_LENGTH);
    newPassword[strcspn(newPassword, "\n")] = 0;  // Remove newline

    pthread_mutex_lock(&userLock);
    strncpy(users[userIndex].password, newPassword, MAX_PASSWORD_LENGTH);
    saveUsers();
    pthread_mutex_unlock(&userLock);
    write(connectionFileDescriptor, "Password changed successfully.\n", strlen("Password changed successfully.\n"));
}

// Add feedback
void addFeedback(int connectionFileDescriptor, int userIndex) {
    char readBuffer[1000], writeBuffer[1000];

    // Prompt for feedback
    write(connectionFileDescriptor, "Enter your feedback: ", strlen("Enter your feedback: "));
    bzero(readBuffer, sizeof(readBuffer));
    read(connectionFileDescriptor, readBuffer, sizeof(readBuffer));
    strncpy(users[userIndex].feedback, readBuffer, MAX_FEEDBACK_LENGTH);
    users[userIndex].feedback[strcspn(users[userIndex].feedback, "\n")] = 0;  // Remove newline

    pthread_mutex_lock(&userLock);
    saveUsers();
    pthread_mutex_unlock(&userLock);
    write(connectionFileDescriptor, "Feedback submitted successfully.\n", strlen("Feedback submitted successfully.\n"));
}

// View transaction history
void viewTransactionHistory(int connectionFileDescriptor, int userIndex) {
    char writeBuffer[1000];
    snprintf(writeBuffer, sizeof(writeBuffer), "Transaction History:\n");
    write(connectionFileDescriptor, writeBuffer, strlen(writeBuffer));

    pthread_mutex_lock(&userLock);
    for (int i = 0; i < users[userIndex].transaction_count; i++) {
        snprintf(writeBuffer, sizeof(writeBuffer), "%s\n", users[userIndex].transaction_history[i]);
        write(connectionFileDescriptor, writeBuffer, strlen(writeBuffer));
    }
    pthread_mutex_unlock(&userLock);
}

// Main customer portal function
void customer_portal(int connectionFileDescriptor) {
    loadUsers(); // Load user data when entering the portal

    int userIndex = authenticateUser(connectionFileDescriptor);
    if (userIndex == -1) {
        return; // Exit if authentication fails
    }

    char readBuffer[1000];
    int choice;

    while (1) {
        // Menu
        const char* menu = "1. View Balance\n"
                           "2. Deposit Money\n"
                           "3. Withdraw Money\n"
                           "4. Transfer Funds\n"
                           "5. Apply for a Loan\n"
                           "6. Change Password\n"
                           "7. Add Feedback\n"
                           "8. View Transaction History\n"
                           "9. Logout\n"
                           "Enter your choice: ";
        write(connectionFileDescriptor, menu, strlen(menu));

        // Get user choice
        bzero(readBuffer, sizeof(readBuffer));
        read(connectionFileDescriptor, readBuffer, sizeof(readBuffer));
        choice = atoi(readBuffer);

        switch (choice) {
            case 1:
                viewBalance(connectionFileDescriptor, userIndex);
                break;
            case 2:
                deposit(connectionFileDescriptor, userIndex);
                break;
            case 3:
                withdraw(connectionFileDescriptor, userIndex);
                break;
            case 4:
                transferFunds(connectionFileDescriptor, userIndex);
                break;
            case 5:
                applyForLoan(connectionFileDescriptor, userIndex);
                break;
            case 6:
                changePassword(connectionFileDescriptor, userIndex);
                break;
            case 7:
                addFeedback(connectionFileDescriptor, userIndex);
                break;
            case 8:
                viewTransactionHistory(connectionFileDescriptor, userIndex);
                break;
            case 9:
                write(connectionFileDescriptor, "Logging out...\n", strlen("Logging out...\n"));
                shutdown(connectionFileDescriptor, SHUT_RDWR); // Shutdown the connection
                close(connectionFileDescriptor); // Close the file descriptor
                exit(0); // Exit the client process
            default:
                write(connectionFileDescriptor, "Invalid choice. Please try again.\n", strlen("Invalid choice. Please try again.\n"));
        }
    }
}

























