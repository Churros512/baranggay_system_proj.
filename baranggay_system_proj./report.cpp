#include "report.h"
#include "database.h"

#include <cctype>
#include <fstream>
#include <iostream>
#include <filesystem>

void showPopulationReport()
{
    std::vector<Resident> residents = getResidents();
    int male = 0;
    int female = 0;
    int other = 0;

    for (const Resident& resident : residents) {
        std::string gender = resident.gender;
        for (char& ch : gender) {
            ch = static_cast<char>(tolower(ch));
        }

        if (gender == "male" || gender == "m") {
            male++;
        } else if (gender == "female" || gender == "f") {
            female++;
        } else {
            other++;
        }
    }

    std::cout << "\nPOPULATION REPORT\n";
    std::cout << "\nTotal Residents: " << residents.size() << "\n";
    std::cout << "Male   : " << male << "\n";
    std::cout << "Female : " << female << "\n";
    std::cout << "Other  : " << other << "\n";
}

void exportCSV()
{
    std::filesystem::create_directories("exports");
    if (exportResidentsCSV("exports/residents.csv")) {
        std::cout << "\nCSV Exported Successfully\n";
    } else {
        std::cout << "\nCSV Export Failed\n";
    }
}
