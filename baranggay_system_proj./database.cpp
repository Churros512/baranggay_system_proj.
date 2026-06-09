#include "database.h"

#include <filesystem>
#include <iostream>
#include <fstream>
#include <sstream>

sqlite3* DB = nullptr;

static bool executeSQL(const std::string& sql,
                       int (*callback)(void*, int, char**, char**) = nullptr,
                       void* data = nullptr)
{
    char* errorMessage = nullptr;
    int result = sqlite3_exec(DB, sql.c_str(), callback, data, &errorMessage);
    if (result != SQLITE_OK) {
        if (errorMessage) {
            std::cerr << "SQLite error: " << errorMessage << "\n";
            sqlite3_free(errorMessage);
        }
        return false;
    }
    return true;
}

void initializeDatabase()
{
    sqlite3_open("barangay.db", &DB);

    std::string residentTable =
        "CREATE TABLE IF NOT EXISTS residents ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "firstname TEXT,"
        "lastname TEXT,"
        "age INTEGER,"
        "gender TEXT,"
        "household TEXT,"
        "voter TEXT);";
    executeSQL(residentTable);

    std::string documentTable =
        "CREATE TABLE IF NOT EXISTS documents ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "resident TEXT,"
        "documentType TEXT,"
        "status TEXT);";
    executeSQL(documentTable);

    std::cout << "Database Initialized\n";
}

bool addResident(const Resident& resident)
{
    std::ostringstream sql;
    sql << "INSERT INTO residents (firstname,lastname,age,gender,household,voter) VALUES('"
        << resident.firstname << "','"
        << resident.lastname << "',"
        << resident.age << "','"
        << resident.gender << "','"
        << resident.household << "','"
        << resident.voter << "');";
    return executeSQL(sql.str());
}

static int residentBuilder(void* data,
                           int argc,
                           char** argv,
                           char**)
{
    auto* residents = static_cast<std::vector<Resident>*>(data);
    Resident resident;
    resident.id = argv[0] ? std::stoi(argv[0]) : 0;
    resident.firstname = argv[1] ? argv[1] : "";
    resident.lastname = argv[2] ? argv[2] : "";
    resident.age = argv[3] ? std::stoi(argv[3]) : 0;
    resident.gender = argv[4] ? argv[4] : "";
    resident.household = argv[5] ? argv[5] : "";
    resident.voter = argv[6] ? argv[6] : "";
    residents->push_back(resident);
    return 0;
}

std::vector<Resident> getResidents()
{
    std::vector<Resident> residents;
    std::string sql = "SELECT id, firstname, lastname, age, gender, household, voter FROM residents;";
    executeSQL(sql, residentBuilder, &residents);
    return residents;
}

std::vector<Resident> searchResidents(const std::string& keyword)
{
    std::vector<Resident> residents;
    std::string sql =
        "SELECT id, firstname, lastname, age, gender, household, voter FROM residents "
        "WHERE firstname LIKE '%" + keyword + "%' OR lastname LIKE '%" + keyword + "%';";
    executeSQL(sql, residentBuilder, &residents);
    return residents;
}

bool addDocumentRequest(const Document& request)
{
    std::ostringstream sql;
    sql << "INSERT INTO documents (resident,documentType,status) VALUES('"
        << request.resident << "','"
        << request.documentType << "','"
        << request.status << "');";
    return executeSQL(sql.str());
}

static int documentBuilder(void* data,
                           int argc,
                           char** argv,
                           char**)
{
    auto* documents = static_cast<std::vector<Document>*>(data);
    Document request;
    request.id = argv[0] ? std::stoi(argv[0]) : 0;
    request.resident = argv[1] ? argv[1] : "";
    request.documentType = argv[2] ? argv[2] : "";
    request.status = argv[3] ? argv[3] : "";
    documents->push_back(request);
    return 0;
}

std::vector<Document> getDocuments()
{
    std::vector<Document> documents;
    std::string sql = "SELECT id, resident, documentType, status FROM documents;";
    executeSQL(sql, documentBuilder, &documents);
    return documents;
}

bool exportResidentsCSV(const std::string& filename)
{
    std::filesystem::create_directories("exports");
    std::vector<Resident> residents = getResidents();
    std::ofstream file(filename);
    if (!file.is_open()) {
        return false;
    }
    file << "ID,FIRSTNAME,LASTNAME,AGE,GENDER,HOUSEHOLD,VOTER\n";
    for (const auto& resident : residents) {
        file << resident.id << ","
             << resident.firstname << ","
             << resident.lastname << ","
             << resident.age << ","
             << resident.gender << ","
             << resident.household << ","
             << resident.voter << "\n";
    }
    file.close();
    return true;
}

void addResident()
{
    Resident resident;
    std::cout << "\nFIRST NAME: ";
    std::cin >> resident.firstname;
    std::cout << "LAST NAME: ";
    std::cin >> resident.lastname;
    std::cout << "AGE: ";
    std::cin >> resident.age;
    std::cout << "GENDER: ";
    std::cin >> resident.gender;
    std::cout << "HOUSEHOLD: ";
    std::cin >> resident.household;
    std::cout << "VOTER (YES/NO): ";
    std::cin >> resident.voter;

    if (addResident(resident)) {
        std::cout << "\nResident Added Successfully\n";
    }
}

static int callback(void*,
                    int argc,
                    char** argv,
                    char** azColName)
{
    for (int i = 0; i < argc; i++) {
        std::cout << azColName[i] << ": " << (argv[i] ? argv[i] : "NULL") << "\n";
    }
    std::cout << "----------------------\n";
    return 0;
}

void viewResidents()
{
    std::string sql = "SELECT * FROM residents;";
    executeSQL(sql, callback, nullptr);
}

void searchResident()
{
    std::string keyword;
    std::cout << "\nSearch Name: ";
    std::cin >> keyword;
    std::string sql =
        "SELECT * FROM residents "
        "WHERE firstname LIKE '%" + keyword + "%' OR lastname LIKE '%" + keyword + "%';";
    executeSQL(sql, callback, nullptr);
}

void addDocumentRequest()
{
    Document request;
    std::cout << "\nResident Name: ";
    std::cin.ignore();
    std::getline(std::cin, request.resident);
    std::cout << "Document Type: ";
    std::getline(std::cin, request.documentType);
    request.status = "PENDING";

    if (addDocumentRequest(request)) {
        std::cout << "\nDocument Request Added\n";
    }
}

void viewDocuments()
{
    std::string sql = "SELECT * FROM documents;";
    executeSQL(sql, callback, nullptr);
}
