#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>


#define MAX_EMPLOYEES 10
#define MAX_PASSWORD_LENGTH 50
#define MAX_FEEDBACK_LENGTH 200
#define MAX_LOANS 30



typedef struct {
    char name[50];
    int employee_id;
    char password[MAX_PASSWORD_LENGTH];
    int assigned_loans[MAX_LOANS]; // Array to hold assigned loan account numbers
    int loan_count; // Counter for assigned loans
} Employee;

Employee employees[MAX_EMPLOYEES];
int employeeCount = 0;
// Function declarations
void initializeEmployees();
void loadUsers();
void saveUsers();
void initializeUsers(); // Initialize default users
void loadEmployees();   // Load employee data
void saveEmployees();   // Save employee data
int authenticateEmployee(int connectionFileDescriptor);
void addNewCustomer(int connectionFileDescriptor);
void modifyCustomer(int connectionFileDescriptor);
void processLoanApplications(int connectionFileDescriptor);
void employee_portal(int connectionFileDescriptor);

// Load users from the data file
/*void loadUsers() {
    int fileDescriptor = open("userdata.dat", O_RDONLY);
    if (fileDescriptor == -1) {
        printf("User data file not found. Initializing users...\n");
        initializeUsers();
        return;
    }
    read(fileDescriptor, users, sizeof(users));
    close(fileDescriptor);
}*/

// Save users to the data file
/*void saveUsers() {
    int fileDescriptor = open("userdata.dat", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fileDescriptor == -1) {
        perror("Error opening file for writing");
        return;
    }
    write(fileDescriptor, users, sizeof(users));
    close(fileDescriptor);
}*/

// Load employees from the data file
// Load employees from the data file
void loadEmployees() {
    int fileDescriptor = open("employeedata.dat", O_RDONLY);
    if (fileDescriptor == -1) {
        printf("Employee data file not found. Initializing employees...\n");
        initializeEmployees();
        return;
    }
    ssize_t bytesRead = read(fileDescriptor, employees, sizeof(employees));
    close(fileDescriptor);

    // Calculate the number of employees loaded
    employeeCount = bytesRead / sizeof(Employee);
    printf("Debug: Loaded %d employees\n", employeeCount); // Debug statement
}


// Save employees to the data file
void saveEmployees() {
    int fileDescriptor = open("employeedata.dat", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fileDescriptor == -1) {
        perror("Error opening file for writing");
        return;
    }
    write(fileDescriptor, employees, sizeof(employees));
    close(fileDescriptor);
}
void initializeEmployees() {
    employeeCount = 3;
    // Employee 1
    employees[0].employee_id = 1; // Assign a unique employee ID
    strncpy(employees[0].name, "Alice Johnson", sizeof(employees[0].name));
    strncpy(employees[0].password, "password1", MAX_PASSWORD_LENGTH);
    employees[0].loan_count = 0; // Initial loan count

    // Employee 2
    employees[1].employee_id = 2; // Assign a unique employee ID
    strncpy(employees[1].name, "Bob Smith", sizeof(employees[1].name));
    strncpy(employees[1].password, "password2", MAX_PASSWORD_LENGTH);
    employees[1].loan_count = 0; // Initial loan count

    // Employee 3
    employees[2].employee_id = 3; // Assign a unique employee ID
    strncpy(employees[2].name, "Charlie Brown", sizeof(employees[2].name));
    strncpy(employees[2].password, "password3", MAX_PASSWORD_LENGTH);
    employees[2].loan_count = 0; // Initial loan count
    saveEmployees();
}


// Add new customer
void addNewCustomer(int connectionFileDescriptor) {
    
    User newUser;
    char readBuffer[1000];
    
    // Input details for the new customer
    write(connectionFileDescriptor, "Enter Account Number: ", strlen("Enter Account Number: "));
    bzero(readBuffer, sizeof(readBuffer));
    read(connectionFileDescriptor, readBuffer, sizeof(readBuffer));
    newUser.account_number = atoll(readBuffer);

    write(connectionFileDescriptor, "Enter Name: ", strlen("Enter Name: "));
    bzero(readBuffer, sizeof(readBuffer));
    read(connectionFileDescriptor, readBuffer, sizeof(readBuffer));
    strncpy(newUser.name, readBuffer, sizeof(newUser.name));
    newUser.name[strcspn(newUser.name, "\n")] = 0;

    write(connectionFileDescriptor, "Enter Initial Balance: ", strlen("Enter Initial Balance: "));
    bzero(readBuffer, sizeof(readBuffer));
    read(connectionFileDescriptor, readBuffer, sizeof(readBuffer));
    newUser.balance = atof(readBuffer);

    write(connectionFileDescriptor, "Enter Password: ", strlen("Enter Password: "));
    bzero(readBuffer, sizeof(readBuffer));
    read(connectionFileDescriptor, readBuffer, sizeof(readBuffer));
    strncpy(newUser.password, readBuffer, sizeof(newUser.password));
    newUser.password[strcspn(newUser.password, "\n")] = 0;

    newUser.loan_amount = 0.0;
    newUser.loan_status = 0;
    newUser.status = 1;  // Active
    newUser.transaction_count = 0;

    // Lock file for thread safety
    pthread_mutex_lock(&userLock);

    // Add the new customer to the user array
    users[userCount++] = newUser;

    // Save users to file
    saveUsers();

    pthread_mutex_unlock(&userLock);

    write(connectionFileDescriptor, "New customer added successfully.\n", strlen("New customer added successfully.\n"));


}
void changePasswordE(int connectionFileDescriptor, int employeeIndex) {
    char readBuffer[1000], writeBuffer[1000];
    char newPassword[MAX_PASSWORD_LENGTH];

    // Prompt for new password
    write(connectionFileDescriptor, "Enter new password: ", strlen("Enter new password: "));
    bzero(readBuffer, sizeof(readBuffer));
    read(connectionFileDescriptor, readBuffer, sizeof(readBuffer));
    strncpy(newPassword, readBuffer, MAX_PASSWORD_LENGTH);
    newPassword[strcspn(newPassword, "\n")] = 0;  // Remove newline

    pthread_mutex_lock(&userLock);
    strncpy(users[employeeIndex].password, newPassword, MAX_PASSWORD_LENGTH);
    saveEmployees();
    pthread_mutex_unlock(&userLock);
    write(connectionFileDescriptor, "Password changed successfully.\n", strlen("Password changed successfully.\n"));
}
void viewCustomerTransactions(int connectionFileDescriptor) {
    char readBuffer[1000];
    long long accountNumber;
    int foundIndex = -1;

    // Prompt for account number
    write(connectionFileDescriptor, "Enter Account Number of Customer: ", strlen("Enter Account Number of Customer: "));
    bzero(readBuffer, sizeof(readBuffer));
    if (read(connectionFileDescriptor, readBuffer, sizeof(readBuffer)) <= 0) {
        write(connectionFileDescriptor, "Error reading account number.\n", strlen("Error reading account number.\n"));
        return;
    }
    accountNumber = atoll(readBuffer);
    printf("Debug: Read account number: %lld\n", accountNumber); // Debug statement

    // Search for the customer by account number
    pthread_mutex_lock(&userLock);
    for (int i = 0; i < userCount; i++) {
        printf("Debug: Checking user %d with account number %lld\n", i, users[i].account_number); // Debug statement
        if (users[i].account_number == accountNumber) {
            foundIndex = i;
            break;
        }
    }

    if (foundIndex == -1) {
        pthread_mutex_unlock(&userLock);
        write(connectionFileDescriptor, "Customer not found.\n", strlen("Customer not found.\n"));
        return;
    }

    // Display transaction history
    char writeBuffer[1000];
    snprintf(writeBuffer, sizeof(writeBuffer), "Transaction History for Account Number: %lld\n", accountNumber);
    write(connectionFileDescriptor, writeBuffer, strlen(writeBuffer));

    for (int i = 0; i < users[foundIndex].transaction_count; i++) {
        snprintf(writeBuffer, sizeof(writeBuffer), "%s\n", users[foundIndex].transaction_history[i]);
        write(connectionFileDescriptor, writeBuffer, strlen(writeBuffer));
    }
    pthread_mutex_unlock(&userLock);
}
void modifyCustomer(int connectionFileDescriptor) {
    
    char readBuffer[1000];
    long long accountNumber;
    int foundIndex = -1;

    // Prompt for account number
    write(connectionFileDescriptor, "Enter Account Number of Customer to Modify: ", strlen("Enter Account Number of Customer to Modify: "));
    bzero(readBuffer, sizeof(readBuffer));
    read(connectionFileDescriptor, readBuffer, sizeof(readBuffer));
    accountNumber = atoll(readBuffer);
    printf("Debug: Read account number: %lld\n", accountNumber); // Debug statement

    // Search for the customer by account number
    pthread_mutex_lock(&userLock);
    for (int i = 0; i < userCount; i++) {
        printf("Debug: Checking user %d with account number %lld\n", i, users[i].account_number); // Debug statement
        if (users[i].account_number == accountNumber) {
            foundIndex = i;
            break;
        }
    }

    if (foundIndex == -1) {
        pthread_mutex_unlock(&userLock);
        write(connectionFileDescriptor, "Customer not found.\n", strlen("Customer not found.\n"));
        return;
    }

    // Modify customer details (name and balance)
    write(connectionFileDescriptor, "Enter New Name: ", strlen("Enter New Name: "));
    bzero(readBuffer, sizeof(readBuffer));
    read(connectionFileDescriptor, readBuffer, sizeof(readBuffer));
    printf("Debug: Read new name: %s\n", readBuffer); // Debug statement
    strncpy(users[foundIndex].name, readBuffer, sizeof(users[foundIndex].name));
    users[foundIndex].name[strcspn(users[foundIndex].name, "\n")] = 0;
    printf("Debug: Updated user name: %s\n", users[foundIndex].name); // Debug statement

    pthread_mutex_unlock(&userLock);
    write(connectionFileDescriptor, "Customer details updated successfully.\n", strlen("Customer details updated successfully.\n"));

}

// Process loan applications
void processLoanApplications(int connectionFileDescriptor) {
    char readBuffer[1000];
    long long accountNumber;
    int foundIndex = -1;

    // Prompt for account number of customer with loan application
    write(connectionFileDescriptor, "Enter Account Number of Customer for Loan Approval/Rejection: ", strlen("Enter Account Number of Customer for Loan Approval/Rejection: "));
    bzero(readBuffer, sizeof(readBuffer));
    read(connectionFileDescriptor, readBuffer, sizeof(readBuffer));
    accountNumber = atoll(readBuffer);

    // Search for the customer
    pthread_mutex_lock(&userLock);
    for (int i = 0; i < userCount; i++) {
        if (users[i].account_number == accountNumber) {
            foundIndex = i;
            break;
        }
    }

    if (foundIndex == -1) {
        pthread_mutex_unlock(&userLock);
        write(connectionFileDescriptor, "Customer not found.\n", strlen("Customer not found.\n"));
        return;
    }

    // Check if the user has a loan application
    if (users[foundIndex].loan_status != 1) {
        pthread_mutex_unlock(&userLock);
        write(connectionFileDescriptor, "No loan application found for this customer.\n", strlen("No loan application found for this customer.\n"));
        return;
    }

    // Approve or reject loan
    write(connectionFileDescriptor, "Approve Loan? (yes/no): ", strlen("Approve Loan? (yes/no): "));
    bzero(readBuffer, sizeof(readBuffer));
    read(connectionFileDescriptor, readBuffer, sizeof(readBuffer));

    if (strncmp(readBuffer, "yes", 3) == 0) {
        users[foundIndex].loan_status = 2;  // Loan approved
        snprintf(users[foundIndex].transaction_history[users[foundIndex].transaction_count++], 100, "Loan Approved: %.2f", users[foundIndex].loan_amount);
        write(connectionFileDescriptor, "Loan approved successfully.\n", strlen("Loan approved successfully.\n"));
    } else {
        users[foundIndex].loan_status = 3;  // Loan rejected
        write(connectionFileDescriptor, "Loan rejected.\n", strlen("Loan rejected.\n"));
    }

    // Save users to file
    saveUsers();
    pthread_mutex_unlock(&userLock);
}

void viewAssignedLoans(int connectionFileDescriptor, int employeeIndex) {
    char writeBuffer[1000];
    snprintf(writeBuffer, sizeof(writeBuffer), "Assigned Loan Applications:\n");
    write(connectionFileDescriptor, writeBuffer, strlen(writeBuffer));

    pthread_mutex_lock(&userLock);
    for (int i = 0; i < employees[employeeIndex].loan_count; i++) {
        int loanAccountNumber = employees[employeeIndex].assigned_loans[i];
        for (int j = 0; j < userCount; j++) {
            if (users[j].account_number == loanAccountNumber) {
                snprintf(writeBuffer, sizeof(writeBuffer), "Account Number: %lld, Loan Amount: %.2f, Loan Status: %d\n",
                         users[j].account_number, users[j].loan_amount, users[j].loan_status);
                write(connectionFileDescriptor, writeBuffer, strlen(writeBuffer));
                break;
            }
        }
    }
    pthread_mutex_unlock(&userLock);
}


// Employee portal function
void employee_portal(int connectionFileDescriptor) {
    loadUsers();   // Load user data
    loadEmployees(); // Load employee data

    int employeeIndex = authenticateEmployee(connectionFileDescriptor);
    if (employeeIndex == -1) {
        return;  // Exit if authentication fails
    }

    char readBuffer[1000];
    int choice;

    while (1) {
        const char* menu = "1. Add New Customer\n"
                           "2. Modify Customer Details\n"
                           "3. Process Loan Applications\n"
                           "4. change password\n"
                           "5. View Assigned Loans\n"
                           "6. View Customer Transactions\n"
                           "7. Logout\n"
                           "8. Exit\n"
                           "Enter your choice: ";
        write(connectionFileDescriptor, menu, strlen(menu));

        // Get employee choice
        bzero(readBuffer, sizeof(readBuffer));
        read(connectionFileDescriptor, readBuffer, sizeof(readBuffer));
        choice = atoi(readBuffer);

        switch (choice) {
            case 1:
                addNewCustomer(connectionFileDescriptor);
                break;
            case 2:
                modifyCustomer(connectionFileDescriptor);
                break;
            case 3:
                processLoanApplications(connectionFileDescriptor);
                break;
            case 4:
                changePasswordE(connectionFileDescriptor,  employeeIndex);
                break;
            case 5:
                viewAssignedLoans(connectionFileDescriptor, employeeIndex);
                break;
            case 6:
                viewCustomerTransactions(connectionFileDescriptor);
                break;
            
            case 7:
                write(connectionFileDescriptor, "Logging out...\n", strlen("Logging out...\n"));
                return;
            case 8:
                write(connectionFileDescriptor, "Logging out...\n", strlen("Logging out...\n"));
                shutdown(connectionFileDescriptor, SHUT_RDWR); // Shutdown the connection
                close(connectionFileDescriptor); // Close the file descriptor
                exit(0); // Exit the client process
            default:
                write(connectionFileDescriptor, "Invalid choice. Try again.\n", strlen("Invalid choice. Try again.\n"));
            
            
        }
    }
}

// Authenticate employee
int authenticateEmployee(int connectionFileDescriptor) {
    char readBuffer[1000];
    int employeeID;
    char password[MAX_PASSWORD_LENGTH];

    write(connectionFileDescriptor, "Enter Employee ID: ", strlen("Enter Employee ID: "));
    bzero(readBuffer, sizeof(readBuffer));
    read(connectionFileDescriptor, readBuffer, sizeof(readBuffer));
    employeeID = atoi(readBuffer);

    write(connectionFileDescriptor, "Enter Password: ", strlen("Enter Password: "));
    bzero(password, sizeof(password));
    read(connectionFileDescriptor, password, sizeof(password));
    password[strcspn(password, "\n")] = 0;

    for (int i = 0; i < MAX_EMPLOYEES; i++) {
        if (employees[i].employee_id == employeeID && strcmp(employees[i].password, password) == 0) {
            write(connectionFileDescriptor, "Login successful!\n", strlen("Login successful!\n"));
            return i;
        }
    }
    write(connectionFileDescriptor, "Invalid credentials.\n", strlen("Invalid credentials.\n"));
    return -1;
}




