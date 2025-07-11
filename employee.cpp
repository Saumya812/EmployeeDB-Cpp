#include "employee.h"
#include <queue>
#include <sstream>
#include <cctype>
#include <cstring>
#include <algorithm>


// Constructor - Initializes the database and determines the next available ID
EmployeeDB::EmployeeDB() {
    // Open the employee data file in binary mode
                   // Default starting ID if no records exist
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        nextId = 1;               // If file doesn't exist, start IDs from 1
        return;
    }

    Employee emp;
    nextId = 1;
    // Read through all records to find the highest ID
    while (file.read(reinterpret_cast<char*>(&emp), sizeof(Employee))) {
        // Update nextId to be one higher than the maximum found ID
        if (emp.id >= nextId) {
            nextId = emp.id + 1;
        }
    }
    file.close();         // Always close the file when done
}

// Returns the next available ID and increments the counter
int EmployeeDB::getNextId() {
    return nextId++;                // Returns current ID, then increments for next call
}

// Adds a new employee to the database with validation
void EmployeeDB::addEmployee() {
    Employee emp;
    emp.id = getNextId();         // Assign the next available ID
    emp.isDeleted = false;        // New records are active by default

    // Get employee details with input validation
    std::cout << "Enter First Name (max 20 chars): ";
    std::cin.getline(emp.firstName, 21);
    std::cout << "Enter Last Name (max 20 chars): ";   // 20 chars + null terminator
    std::cin.getline(emp.lastName, 21);

    // Validate SSN format
    bool validSSN = false;
    while (!validSSN) {
        std::cout << "Enter SSN (format XXX-XX-XXXX): ";
        std::cin.getline(emp.ssn, 12);
        validSSN = (strlen(emp.ssn) == 11) &&
            (isdigit(emp.ssn[0]) && isdigit(emp.ssn[1]) && isdigit(emp.ssn[2])) &&
            (emp.ssn[3] == '-') &&
            (isdigit(emp.ssn[4]) && isdigit(emp.ssn[5])) &&
            (emp.ssn[6] == '-') &&
            (isdigit(emp.ssn[7]) && isdigit(emp.ssn[8]) && isdigit(emp.ssn[9]) && isdigit(emp.ssn[10]));

        if (!validSSN) {
            std::cout << "Invalid SSN format. Please try again.\n";
        }
        else {
            // Check for duplicate SSN
            std::ifstream file(filename, std::ios::binary);
            Employee temp;
            while (file.read(reinterpret_cast<char*>(&temp), sizeof(Employee))) {
                if (!temp.isDeleted && strcmp(temp.ssn, emp.ssn) == 0) {
                    std::cout << "SSN already exists in database.\n";
                    validSSN = false;
                    break;
                }
            }
            file.close();
        }
    }

    std::cout << "Enter Salary: ";
    std::cin >> emp.salary;
    std::cout << "Enter Age: ";
    std::cin >> emp.age;
    std::cin.ignore();        // Clear leftover newline character

    // Append new record to binary file
    std::ofstream file(filename, std::ios::binary | std::ios::app);
    if (!file) {
        std::cerr << "Error opening file for writing.\n";
        return;
    }
    // Write the employee struct as binary data
    file.write(reinterpret_cast<const char*>(&emp), sizeof(Employee));
    file.close();

    std::cout << "Employee added successfully. ID: " << emp.id << "\n";
}

// Displays employees with optional sorting
void EmployeeDB::displayEmployees(bool indexed, int field) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        // Open file for reading all employees
        std::cout << "No employee data found.\n";
        return;
    }

    std::vector<Employee> employees;
    Employee emp;

    // Read all active (non-deleted) employees into vector
    while (file.read(reinterpret_cast<char*>(&emp), sizeof(Employee))) {
        if (!emp.isDeleted) {
            employees.push_back(emp);
        }
    }
    file.close();

    if (employees.empty()) {
        std::cout << "No employees to display.\n";
        return;
    }

    // Sort if indexing requested
    if (indexed) {
        switch (field) {
        case 1: // Sort by salary
            std::sort(employees.begin(), employees.end(),
                [](const Employee& a, const Employee& b) { return a.salary < b.salary; });
            break;
        case 2: // Sort by Age
            std::sort(employees.begin(), employees.end(),
                [](const Employee& a, const Employee& b) { return a.age < b.age; });
            break;
        case 3: // Sort by Last Name
            std::sort(employees.begin(), employees.end(),
                [](const Employee& a, const Employee& b) { return strcmp(a.lastName, b.lastName) < 0; });
            break;
        case 4: // Sort by SSN
            std::sort(employees.begin(), employees.end(),
                [](const Employee& a, const Employee& b) { return strcmp(a.ssn, b.ssn) < 0; });
            break;
        }
    }

    // Display all employee records
    for (const auto& e : employees) {
        std::cout << "ID: " << e.id << "\n";
        std::cout << "Name: " << e.firstName << " " << e.lastName << "\n";
        std::cout << "SSN: " << e.ssn << "\n";
        std::cout << "Salary: $" << e.salary << "\n";
        std::cout << "Age: " << e.age << "\n";
        std::cout << "------------------------\n";
    }
}

// Deletes an employee record by marking it as deleted (logical deletion)
void EmployeeDB::deleteEmployee() {
    int idToDelete;
    std::cout << "Enter Employee ID to delete: ";
    std::cin >> idToDelete;
    std::cin.ignore();

    // Open file in read+write mode to modify records
    std::fstream file(filename, std::ios::binary | std::ios::in | std::ios::out);
    if (!file) {
        std::cout << "Error opening file.\n";
        return;
    }

    Employee emp;
    bool found = false;
    // Search through all records
    while (file.read(reinterpret_cast<char*>(&emp), sizeof(Employee))) {
        if (emp.id == idToDelete && !emp.isDeleted) {
            found = true;
            emp.isDeleted = true;    // Mark record as deleted

            // Move write pointer back to start of current record
            file.seekp(-static_cast<int>(sizeof(Employee)), std::ios::cur);
            // Write modified record back to file
            file.write(reinterpret_cast<const char*>(&emp), sizeof(Employee));
            std::cout << "Employee deleted successfully.\n";
            break;
        }
    }

    if (!found) {
        std::cout << "Employee not found or already deleted.\n";
    }
    file.close();      // Always close the file
}


// Merge function for merge sort - combines two sorted subarrays
void EmployeeDB::merge(std::vector<std::pair<int, std::string>>& arr, int l, int m, int r, bool ascending) {
    int n1 = m - l + 1;            // Size of left subarray
    int n2 = r - m;                // Size of right subarray

    // Create temporary arrays
    std::vector<std::pair<int, std::string>> L(n1), R(n2);

    // Copy data to temp arrays
    for (int i = 0; i < n1; i++)
        L[i] = arr[l + i];
    for (int j = 0; j < n2; j++)
        R[j] = arr[m + 1 + j];

    // Merge the temp arrays back into arr
    int i = 0, j = 0, k = l;         // Initial indexes
    while (i < n1 && j < n2) {
        // Compare elements based on sort order (ascending/descending)
        bool condition = ascending ? (L[i].second < R[j].second) : (L[i].second > R[j].second);
        if (condition) {
            arr[k] = L[i];      // Take from left array
            i++;
        }
        else {
            arr[k] = R[j];     // Take from right array
            j++;
        }
        k++;
    }

    // Copy remaining elements of L[] if any
    while (i < n1) {
        arr[k] = L[i];
        i++;
        k++;
    }

    // Copy remaining elements of R[] if any
    while (j < n2) {
        arr[k] = R[j];
        j++;
        k++;
    }
}

// Recursive merge sort implementation
void EmployeeDB::mergeSort(std::vector<std::pair<int, std::string>>& arr, int l, int r, bool ascending) {
    if (l >= r) return;            // Base case: subarray of size 1 is already sorted

    int m = l + (r - l) / 2;      // Find midpoint
    mergeSort(arr, l, m, ascending);         // Sort first half
    mergeSort(arr, m + 1, r, ascending);     // Sort second half
    merge(arr, l, m, r, ascending);          // Merge sorted halves
}

// Indexes employees by specified field using merge sort
void EmployeeDB::indexByField(int field, bool ascending) {
    // Open employee data file
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        std::cout << "No employee data found.\n";
        return;
    }

    // Create vector to store ID-field pairs for sorting
    std::vector<std::pair<int, std::string>> idAndField;
    Employee emp;

    // Read all active employees and extract relevant field
    while (file.read(reinterpret_cast<char*>(&emp), sizeof(Employee))) {
        if (!emp.isDeleted) {
            std::string fieldValue;
            switch (field) {
            case 1: fieldValue = std::to_string(emp.salary); break; // Salary
            case 2: fieldValue = std::to_string(emp.age); break;   // Age
            case 3: fieldValue = emp.lastName; break;              // Last Name
            case 4: fieldValue = emp.ssn; break;                  // SSN
            default: break;
            }
            idAndField.emplace_back(emp.id, fieldValue);
        }
    }
    file.close();

    if (idAndField.empty()) {
        std::cout << "No employees to index.\n";
        return;
    }

    // Sort the ID-field pairs using merge sort
    mergeSort(idAndField, 0, idAndField.size() - 1, ascending);

    // Display sorted results
    std::cout << "Employees indexed by field " << field << " ("
        << (ascending ? "ascending" : "descending") << "):\n";
    std::ifstream displayFile(filename, std::ios::binary);
    for (const auto& pair : idAndField) {
        // Seek to the correct record position
        displayFile.seekg((pair.first - 1) * sizeof(Employee));
        displayFile.read(reinterpret_cast<char*>(&emp), sizeof(Employee));

        // Display the requested field
        std::cout << "ID: " << emp.id << " - ";
        switch (field) {
        case 1: std::cout << "Salary: $" << emp.salary; break;
        case 2: std::cout << "Age: " << emp.age; break;
        case 3: std::cout << "Last Name: " << emp.lastName; break;
        case 4: std::cout << "SSN: " << emp.ssn; break;
        }
        std::cout << "\n";
    }
    displayFile.close();
}

// Searches for an employee by SSN using binary search
void EmployeeDB::searchBySSN() {
    char searchSSN[12];               // Buffer to store SSN input (11 chars + null terminator)
    std::cout << "Enter SSN to search (XXX-XX-XXXX): ";
    std::cin.getline(searchSSN, 12);        // Get SSN from user

    // First index by SSN for binary search
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        std::cout << "No employee data found.\n";
        return;
    }

    // Create vector to store (employee ID, SSN) pairs
    std::vector<std::pair<int, std::string>> ssnIndex;
    Employee emp;

    // Read all active employees and store their SSNs with IDs
    while (file.read(reinterpret_cast<char*>(&emp), sizeof(Employee))) {
        if (!emp.isDeleted) {
            ssnIndex.emplace_back(emp.id, emp.ssn);    // Store ID-SSN pair
        }
    }
    file.close();

    if (ssnIndex.empty()) {
        std::cout << "No employees to search.\n";
        return;
    }

    // Sort SSNs for binary search
    mergeSort(ssnIndex, 0, ssnIndex.size() - 1, true);

    // Binary search implementation
    int left = 0;                     // Start of search range
    int right = ssnIndex.size() - 1;  // End of search range
    bool found = false;               // Search result flag
    int foundId = -1;                 // ID of found employee

    while (left <= right) {
        int mid = left + (right - left) / 2;           // Calculate midpoint (avoids overflow)

        // Compare search SSN with middle element's SSN
        int cmp = strcmp(ssnIndex[mid].second.c_str(), searchSSN);

        if (cmp == 0) {                // Exact match found
            found = true;
            foundId = ssnIndex[mid].first;        // Store the employee ID
            break;
        }
        else if (cmp < 0) {            // Search SSN is in right half
            left = mid + 1;
        }
        else {                        // Search SSN is in left half
            right = mid - 1;
        }
    }

    if (found) {
        // Display the found record
        std::ifstream file(filename, std::ios::binary);    // Seek to record position
        file.seekg((foundId - 1) * sizeof(Employee));      // Read record
        file.read(reinterpret_cast<char*>(&emp), sizeof(Employee));
        std::cout << "Employee found:\n";
        std::cout << "ID: " << emp.id << "\n";
        std::cout << "Name: " << emp.firstName << " " << emp.lastName << "\n";
        std::cout << "SSN: " << emp.ssn << "\n";
        std::cout << "Salary: $" << emp.salary << "\n";
        std::cout << "Age: " << emp.age << "\n";
        file.close();
    }
    else {
        std::cout << "Employee with SSN " << searchSSN << " not found.\n";
    }
}

// Executes SQL-like queries on employee records
void EmployeeDB::runQuery(const std::string& query) {
    std::istringstream iss(query);
    std::string token;
    std::vector<std::string> tokens;

    while (iss >> token) {       // Split query by spaces
        tokens.push_back(token);
    }

    // Validate basic query structure: must start with SELECT and have minimum tokens
    if (tokens.size() < 3 || tokens[0] != "SELECT") {
        std::cout << "Invalid query format.\n";
        return;
    }

    // Determine if query wants all fields or specific ones
    bool selectAll = tokens[1] == "*";
    std::string fieldName;
    if (!selectAll) {
        fieldName = tokens[1];         // Get the specific field to select
    }

    // Parse WHERE clause if present
    bool hasWhere = false;
    std::string whereField, whereValue;
    if (tokens.size() > 3 && tokens[3] == "WHERE") {
        hasWhere = true;
        whereField = tokens[4];         // Field to filter by
        whereValue = tokens[6];         // Value to match

        // Remove quotes from string values if present
        if (whereValue.front() == '\'' && whereValue.back() == '\'') {
            whereValue = whereValue.substr(1, whereValue.length() - 2);
        }
    }

    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        std::cout << "No employee data found.\n";
        return;
    }

    Employee emp;
    bool foundAny = false;              // Track if any matches were found

    // Process each employee record
    while (file.read(reinterpret_cast<char*>(&emp), sizeof(Employee))) {
        if (!emp.isDeleted) {           // Skip deleted records
            bool matchesWhere = true;

            // Apply WHERE clause filtering if present
            if (hasWhere) {
                if (whereField == "ssn") {
                    matchesWhere = strcmp(emp.ssn, whereValue.c_str()) == 0;
                }
                else if (whereField == "lastName") {
                    matchesWhere = strcmp(emp.lastName, whereValue.c_str()) == 0;
                }
                else if (whereField == "salary") {
                    matchesWhere = emp.salary == std::stof(whereValue);     // Convert string to float
                }
                else if (whereField == "age") {
                    matchesWhere = emp.age == std::stoi(whereValue);       // Convert string to int
                }
            }


            // Display record if it matches the query
            if (matchesWhere) {
                foundAny = true;
                if (selectAll) {
                    // Display all fields
                    std::cout << "ID: " << emp.id << "\n";
                    std::cout << "Name: " << emp.firstName << " " << emp.lastName << "\n";
                    std::cout << "SSN: " << emp.ssn << "\n";
                    std::cout << "Salary: $" << emp.salary << "\n";
                    std::cout << "Age: " << emp.age << "\n";
                    std::cout << "------------------------\n";
                }
                else {
                    // Display only the requested field
                    if (fieldName == "firstName") {
                        std::cout << emp.firstName << "\n";
                    }
                    else if (fieldName == "lastName") {
                        std::cout << emp.lastName << "\n";
                    }
                    else if (fieldName == "ssn") {
                        std::cout << emp.ssn << "\n";
                    }
                    else if (fieldName == "salary") {
                        std::cout << emp.salary << "\n";
                    }
                    else if (fieldName == "age") {
                        std::cout << emp.age << "\n";
                    }
                }
            }
        }
    }

    if (!foundAny) {
        std::cout << "No matching records found.\n";
    }
    file.close();
}

// Processes user menu choices from the queue
void EmployeeDB::processMenuQueue() {
    while (!menuQueue.empty()) {           // Process all queued commands
        int choice = menuQueue.front();    // Get next choice
        menuQueue.pop();                   // Remove from queue

        switch (choice) {
        case 1: addEmployee();     // Add new employee
            break;
        case 2: displayEmployees();    // Show all employees
            break;
        case 3: deleteEmployee();     // Delete employee
            break;
        case 4: {              // Index records by field
            std::cout << "Index by:\n1. Salary\n2. Age\n3. Last Name\n4. SSN\n";
            int field;
            std::cin >> field;      // Get field choice
            std::cin.ignore();      // Clear input buffer
            std::cout << "Sort order (0=descending, 1=ascending): ";
            bool ascending;
            std::cin >> ascending;     // Get sort direction
            std::cin.ignore();
            indexByField(field, ascending);    // Perform indexing
            break;
        }
        case 5: searchBySSN();     // Search by SSN
            break;
        case 6: std::cout << "Exiting...\n"; exit(0);    // Quit program
        default: std::cout << "Invalid choice: " << choice << "\n";    // Handle invalid input
        }
    }
}
