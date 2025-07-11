#include "employee.h"  // Include the EmployeeDB class definition
#include <iostream>
#include <string>

int main() {
    // Create an instance of the Employee Database
    EmployeeDB db;

    // String to store user input
    std::string input;

    // Main program loop - runs until user chooses to exit
    while (true) {
        // Display the menu options
        std::cout << "\nEmployee Database System\n";
        std::cout << "1. Add Employee\n";
        std::cout << "2. Display Employees\n";
        std::cout << "3. Delete Employee\n";
        std::cout << "4. Index Records\n";
        std::cout << "5. Search by SSN\n";
        std::cout << "6. Run Query\n";
        std::cout << "7. Exit\n";
        std::cout << "Enter choice: ";

        // Get user input (whole line to avoid parsing issues)
        std::getline(std::cin, input);

        // Try-catch block to handle any exceptions
        try {
            // Process user choice
            if (input == "1") {
                // Add a new employee
                db.addEmployee();
            }
            else if (input == "2") {
                // Display all employees
                db.displayEmployees();
            }
            else if (input == "3") {
                // Delete an employee by ID
                db.deleteEmployee();
            }
            else if (input == "4") {
                // Index records by a specific field
                std::cout << "Field to index (1=Salary, 2=Age, 3=LastName, 4=SSN): ";
                int field;
                std::cin >> field;  // Get field choice
                std::cin.ignore();  // Clear input buffer

                std::cout << "Sort order (0=Descending, 1=Ascending): ";
                bool ascending;
                std::cin >> ascending;  // Get sort direction
                std::cin.ignore();     // Clear input buffer

                // Perform the indexing
                db.indexByField(field, ascending);
            }
            else if (input == "5") {
                // Search for employee by SSN
                db.searchBySSN();
            }
            else if (input == "6") {
                // Execute a SQL-like query
                std::cout << "Enter query: ";
                std::string query;
                std::getline(std::cin, query);  // Get full query line
                db.runQuery(query);             // Execute the query
            }
            else if (input == "7") {
                // Exit the program
                break;
            }
            else {
                // Handle invalid menu choices
                std::cout << "Invalid choice!\n";
            }
        }
        catch (const std::exception& e) {
            // Catch and display any errors that occur
            std::cerr << "Error: " << e.what() << "\n";
        }
    }

    return 0;  // Successful program termination
}
