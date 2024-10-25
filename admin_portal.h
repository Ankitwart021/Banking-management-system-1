#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>

#define MAX_ADMINISTRATORS 5

typedef struct {
    char name[50];
    int admin_id;
    char password[MAX_PASSWORD_LENGTH];
} Administrator;

Administrator administrators[MAX_ADMINISTRATORS];
int adminCount = 0;

// Function declarations
void initializeAdministrators();
void loadAdministrators();
void saveAdministrators();
int authenticateAdministrator(int connectionFileDescriptor);
void addNewEmployee(int connectionFileDescriptor);
void modifyCustomerEmployeeDetails(int connectionFileDescriptor);
void manageUserRoles(int connectionFileDescriptor);
void changePasswordA(int connectionFileDescriptor, int adminIndex);
void admin_portal(int connectionFileDescriptor);

// Initialize default administrators
void initializeAdministrators() {
    adminCount = 1;
    // Administrator 1
    administrators[0].admin_id = 1;
    strncpy(administrators[0].name, "Admin One", sizeof(administrators[0].name));
    strncpy(administrators[0].password, "adminpass1", MAX_PASSWORD_LENGTH);

    saveAdministrators();
}

// Load administrators from the data file
void loadAdministrators() {
    int fileDescriptor = open("admindata.dat", O_RDONLY);
    if (fileDescriptor == -1) {
        printf("Administrator data file not found. Initializing administrators...\n");
        initializeAdministrators();
        return;
    }
    ssize_t bytesRead = read(fileDescriptor, administrators, sizeof(administrators));
    close(fileDescriptor);

    // Calculate the number of administrators loaded
    adminCount = bytesRead / sizeof(Administrator);
    printf("Debug: Loaded %d administrators\n", adminCount); // Debug statement
}

// Save administrators to the data file
void saveAdministrators() {
    int fileDescriptor = open("admindata.dat", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fileDescriptor == -1) {
        perror("Error opening file for writing");
        return;
    }
    write(fileDescriptor, administrators, sizeof(administrators));
    close(fileDescriptor);
}

// Authenticate administrator
int authenticateAdministrator(int connectionFileDescriptor) {
    char readBuffer[1000];
    int adminID;
    char password[MAX_PASSWORD_LENGTH];

    write(connectionFileDescriptor, "Enter Administrator ID: ", strlen("Enter Administrator ID: "));
    bzero(readBuffer, sizeof(readBuffer));
    read(connectionFileDescriptor, readBuffer, sizeof(readBuffer));
    adminID = atoi(readBuffer);

    write(connectionFileDescriptor, "Enter Password: ", strlen("Enter Password: "));
    bzero(password, sizeof(password));
    read(connectionFileDescriptor, password, sizeof(password));
    password[strcspn(password, "\n")] = 0;

    for (int i = 0; i < adminCount; i++) {
        if (administrators[i].admin_id == adminID && strcmp(administrators[i].password, password) == 0) {
            write(connectionFileDescriptor, "Login successful!\n", strlen("Login successful!\n"));
            return i;
        }
    }
    write(connectionFileDescriptor, "Invalid credentials.\n", strlen("Invalid credentials.\n"));
    return -1;
}

// Add new bank employee
void addNewEmployee(int connectionFileDescriptor) {
    Employee newEmployee;
    char readBuffer[1000];

    // Input details for the new employee
    write(connectionFileDescriptor, "Enter Employee ID: ", strlen("Enter Employee ID: "));
    bzero(readBuffer, sizeof(readBuffer));
    read(connectionFileDescriptor, readBuffer, sizeof(readBuffer));
    newEmployee.employee_id = atoi(readBuffer);

    write(connectionFileDescriptor, "Enter Name: ", strlen("Enter Name: "));
    bzero(readBuffer, sizeof(readBuffer));
    read(connectionFileDescriptor, readBuffer, sizeof(readBuffer));
    strncpy(newEmployee.name, readBuffer, sizeof(newEmployee.name));
    newEmployee.name[strcspn(newEmployee.name, "\n")] = 0;

    write(connectionFileDescriptor, "Enter Password: ", strlen("Enter Password: "));
    bzero(readBuffer, sizeof(readBuffer));
    read(connectionFileDescriptor, readBuffer, sizeof(readBuffer));
    strncpy(newEmployee.password, readBuffer, sizeof(newEmployee.password));
    newEmployee.password[strcspn(newEmployee.password, "\n")] = 0;

    newEmployee.loan_count = 0; // Initial loan count

    // Lock file for thread safety
    pthread_mutex_lock(&userLock);

    // Add the new employee to the employee array
    employees[employeeCount++] = newEmployee;

    // Save employees to file
    saveEmployees();

    pthread_mutex_unlock(&userLock);

    write(connectionFileDescriptor, "New employee added successfully.\n", strlen("New employee added successfully.\n"));
}

// Modify customer or employee details
void modifyCustomerEmployeeDetails(int connectionFileDescriptor) {
    char readBuffer[1000];
    int choice;
    long long accountNumber;
    int foundIndex = -1;

    // Prompt for modification choice
    write(connectionFileDescriptor, "Modify:\n1. Customer\n2. Employee\nEnter your choice: ", strlen("Modify:\n1. Customer\n2. Employee\nEnter your choice: "));
    bzero(readBuffer, sizeof(readBuffer));
    read(connectionFileDescriptor, readBuffer, sizeof(readBuffer));
    choice = atoi(readBuffer);

    if (choice == 1) {
        // Modify customer details
        write(connectionFileDescriptor, "Enter Account Number of Customer to Modify: ", strlen("Enter Account Number of Customer to Modify: "));
        bzero(readBuffer, sizeof(readBuffer));
        read(connectionFileDescriptor, readBuffer, sizeof(readBuffer));
        accountNumber = atoll(readBuffer);

        // Search for the customer by account number
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

        // Modify customer details (name and balance)
        write(connectionFileDescriptor, "Enter New Name: ", strlen("Enter New Name: "));
        bzero(readBuffer, sizeof(readBuffer));
        read(connectionFileDescriptor, readBuffer, sizeof(readBuffer));
        strncpy(users[foundIndex].name, readBuffer, sizeof(users[foundIndex].name));
        users[foundIndex].name[strcspn(users[foundIndex].name, "\n")] = 0;

        write(connectionFileDescriptor, "Enter New Balance: ", strlen("Enter New Balance: "));
        bzero(readBuffer, sizeof(readBuffer));
        read(connectionFileDescriptor, readBuffer, sizeof(readBuffer));
        users[foundIndex].balance = atof(readBuffer);

        pthread_mutex_unlock(&userLock);
        write(connectionFileDescriptor, "Customer details updated successfully.\n", strlen("Customer details updated successfully.\n"));
    } else if (choice == 2) {
        // Modify employee details
        write(connectionFileDescriptor, "Enter Employee ID to Modify: ", strlen("Enter Employee ID to Modify: "));
        bzero(readBuffer, sizeof(readBuffer));
        read(connectionFileDescriptor, readBuffer, sizeof(readBuffer));
        int employeeID = atoi(readBuffer);

        // Search for the employee by ID
        pthread_mutex_lock(&userLock);
        for (int i = 0; i < employeeCount; i++) {
            if (employees[i].employee_id == employeeID) {
                foundIndex = i;
                break;
            }
        }

        if (foundIndex == -1) {
            pthread_mutex_unlock(&userLock);
            write(connectionFileDescriptor, "Employee not found.\n", strlen("Employee not found.\n"));
            return;
        }

        // Modify employee details (name and password)
        write(connectionFileDescriptor, "Enter New Name: ", strlen("Enter New Name: "));
        bzero(readBuffer, sizeof(readBuffer));
        read(connectionFileDescriptor, readBuffer, sizeof(readBuffer));
        strncpy(employees[foundIndex].name, readBuffer, sizeof(employees[foundIndex].name));
        employees[foundIndex].name[strcspn(employees[foundIndex].name, "\n")] = 0;

        write(connectionFileDescriptor, "Enter New Password: ", strlen("Enter New Password: "));
        bzero(readBuffer, sizeof(readBuffer));
        read(connectionFileDescriptor, readBuffer, sizeof(readBuffer));
        strncpy(employees[foundIndex].password, readBuffer, sizeof(employees[foundIndex].password));
        employees[foundIndex].password[strcspn(employees[foundIndex].password, "\n")] = 0;

        pthread_mutex_unlock(&userLock);
        write(connectionFileDescriptor, "Employee details updated successfully.\n", strlen("Employee details updated successfully.\n"));
    } else {
        write(connectionFileDescriptor, "Invalid choice.\n", strlen("Invalid choice.\n"));
    }
}

void manageUserRoles(int connectionFileDescriptor) {
    char readBuffer[1000];
    int choice;

    // Prompt for role management choice
    write(connectionFileDescriptor, "Manage Roles:\n1. Promote Employee to Manager\n2. Demote Manager to Employee\nEnter your choice: ", strlen("Manage Roles:\n1. Promote Employee to Manager\n2. Demote Manager to Employee\nEnter your choice: "));
    bzero(readBuffer, sizeof(readBuffer));
    read(connectionFileDescriptor, readBuffer, sizeof(readBuffer));
    choice = atoi(readBuffer);

    if (choice == 1) {
        // Promote Employee to Manager
        write(connectionFileDescriptor, "Enter Employee ID to Promote: ", strlen("Enter Employee ID to Promote: "));
        bzero(readBuffer, sizeof(readBuffer));
        read(connectionFileDescriptor, readBuffer, sizeof(readBuffer));
        int employeeID = atoi(readBuffer);

        // Search for the employee by ID
        pthread_mutex_lock(&userLock);
        int foundIndex = -1;
        for (int i = 0; i < employeeCount; i++) {
            if (employees[i].employee_id == employeeID) {
                foundIndex = i;
                break;
            }
        }

        if (foundIndex == -1) {
            pthread_mutex_unlock(&userLock);
            write(connectionFileDescriptor, "Employee not found.\n", strlen("Employee not found.\n"));
            return;
        }

        // Promote employee to manager
        if (managerCount < MAX_MANAGERS) {
            managers[managerCount].manager_id = employees[foundIndex].employee_id;
            strncpy(managers[managerCount].name, employees[foundIndex].name, sizeof(managers[managerCount].name));
            strncpy(managers[managerCount].password, employees[foundIndex].password, MAX_PASSWORD_LENGTH);
            managerCount++;

            // Debug statement to verify manager data
            printf("Debug: Promoted Employee ID %d to Manager ID %d\n", employees[foundIndex].employee_id, managers[managerCount - 1].manager_id);

            // Remove employee from employee array
            for (int i = foundIndex; i < employeeCount - 1; i++) {
                employees[i] = employees[i + 1];
            }
            employeeCount--;

            saveEmployees();
            saveManagers(); // Ensure this is called after updating manager data
            pthread_mutex_unlock(&userLock);
            write(connectionFileDescriptor, "Employee promoted to manager successfully.\n", strlen("Employee promoted to manager successfully.\n"));
        } else {
            pthread_mutex_unlock(&userLock);
            write(connectionFileDescriptor, "Manager limit reached. Cannot promote employee.\n", strlen("Manager limit reached. Cannot promote employee.\n"));
        }
    } else if (choice == 2) {
        // Demote Manager to Employee
        write(connectionFileDescriptor, "Enter Manager ID to Demote: ", strlen("Enter Manager ID to Demote: "));
        bzero(readBuffer, sizeof(readBuffer));
        read(connectionFileDescriptor, readBuffer, sizeof(readBuffer));
        int managerID = atoi(readBuffer);

        // Search for the manager by ID
        pthread_mutex_lock(&userLock);
        int foundIndex = -1;
        for (int i = 0; i < managerCount; i++) {
            if (managers[i].manager_id == managerID) {
                foundIndex = i;
                break;
            }
        }

        if (foundIndex == -1) {
            pthread_mutex_unlock(&userLock);
            write(connectionFileDescriptor, "Manager not found.\n", strlen("Manager not found.\n"));
            return;
        }

        // Demote manager to employee
        if (employeeCount < MAX_EMPLOYEES) {
            employees[employeeCount].employee_id = managers[foundIndex].manager_id;
            strncpy(employees[employeeCount].name, managers[foundIndex].name, sizeof(employees[employeeCount].name));
            strncpy(employees[employeeCount].password, managers[foundIndex].password, MAX_PASSWORD_LENGTH);
            employees[employeeCount].loan_count = 0; // Initial loan count
            employeeCount++;

            // Debug statement to verify employee data
            printf("Debug: Demoted Manager ID %d to Employee ID %d\n", managers[foundIndex].manager_id, employees[employeeCount - 1].employee_id);

            // Remove manager from manager array
            for (int i = foundIndex; i < managerCount - 1; i++) {
                managers[i] = managers[i + 1];
            }
            managerCount--;

            saveEmployees();
            saveManagers(); // Ensure this is called after updating manager data
            pthread_mutex_unlock(&userLock);
            write(connectionFileDescriptor, "Manager demoted to employee successfully.\n", strlen("Manager demoted to employee successfully.\n"));
        } else {
            pthread_mutex_unlock(&userLock);
            write(connectionFileDescriptor, "Employee limit reached. Cannot demote manager.\n", strlen("Employee limit reached. Cannot demote manager.\n"));
        }
    } else {
        write(connectionFileDescriptor, "Invalid choice.\n", strlen("Invalid choice.\n"));
    }
}

// Change administrator password
void changePasswordA(int connectionFileDescriptor, int adminIndex) {
    char readBuffer[1000], writeBuffer[1000];
    char newPassword[MAX_PASSWORD_LENGTH];

    // Prompt for new password
    write(connectionFileDescriptor, "Enter new password: ", strlen("Enter new password: "));
    bzero(readBuffer, sizeof(readBuffer));
    read(connectionFileDescriptor, readBuffer, sizeof(readBuffer));
    strncpy(newPassword, readBuffer, MAX_PASSWORD_LENGTH);
    newPassword[strcspn(newPassword, "\n")] = 0;  // Remove newline

    pthread_mutex_lock(&userLock);
    strncpy(administrators[adminIndex].password, newPassword, MAX_PASSWORD_LENGTH);
    saveAdministrators();
    pthread_mutex_unlock(&userLock);
    write(connectionFileDescriptor, "Password changed successfully.\n", strlen("Password changed successfully.\n"));
}

// Administrator portal function
void admin_portal(int connectionFileDescriptor) {
    loadUsers();   // Load user data
    loadEmployees(); // Load employee data
    loadAdministrators(); // Load administrator data

    int adminIndex = authenticateAdministrator(connectionFileDescriptor);
    if (adminIndex == -1) {
        return;  // Exit if authentication fails
    }

    char readBuffer[1000];
    int choice;

    while (1) {
        const char* menu = "1. Add New Bank Employee\n"
                           "2. Modify Customer/Employee Details\n"
                           "3. Manage User Roles\n"
                           "4. Change Password\n"
                           "5. Logout\n"
                           "6. Exit\n"
                           "Enter your choice: ";
        write(connectionFileDescriptor, menu, strlen(menu));

        // Get administrator choice
        bzero(readBuffer, sizeof(readBuffer));
        read(connectionFileDescriptor, readBuffer, sizeof(readBuffer));
        choice = atoi(readBuffer);

        switch (choice) {
            case 1:
                addNewEmployee(connectionFileDescriptor);
                break;
            case 2:
                modifyCustomerEmployeeDetails(connectionFileDescriptor);
                break;
            case 3:
                manageUserRoles(connectionFileDescriptor);
                break;
            case 4:
                changePasswordA(connectionFileDescriptor, adminIndex);
                break;
            case 5:
                write(connectionFileDescriptor, "Logging out...\n", strlen("Logging out...\n"));
                return;
            case 6:
                write(connectionFileDescriptor, "Logging out...\n", strlen("Logging out...\n"));
                shutdown(connectionFileDescriptor, SHUT_RDWR); // Shutdown the connection
                close(connectionFileDescriptor); // Close the file descriptor
                exit(0); // Exit the client process
            default:
                write(connectionFileDescriptor, "Invalid choice. Try again.\n", strlen("Invalid choice. Try again.\n"));
        }
    }
}