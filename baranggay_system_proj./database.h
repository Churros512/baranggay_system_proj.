#ifndef DATABASE_H
#define DATABASE_H

#include <sqlite3.h>
#include <string>
#include <vector>

extern sqlite3* DB;

struct Resident
{
    int id;
    std::string firstname;
    std::string lastname;
    int age;
    std::string gender;
    std::string household;
    std::string voter;
};

struct Document
{
    int id;
    std::string resident;
    std::string documentType;
    std::string status;
};

void initializeDatabase();

bool addResident(const Resident& resident);
std::vector<Resident> getResidents();
std::vector<Resident> searchResidents(const std::string& keyword);

bool addDocumentRequest(const Document& request);
std::vector<Document> getDocuments();

bool exportResidentsCSV(const std::string& filename);

#endif
