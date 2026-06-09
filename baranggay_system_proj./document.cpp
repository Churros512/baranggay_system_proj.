#include "document.h"

#include <fstream>
#include <iostream>
#include <filesystem>

bool generateClearanceFor(const std::string& resident,
                          const std::string& purpose)
{
    std::filesystem::create_directories("clearances");
    std::string filename = "clearances/" + resident + ".txt";
    std::ofstream file(filename);
    if (!file.is_open()) {
        return false;
    }

    file << "=============================\n";
    file << "   BARANGAY CLEARANCE\n";
    file << "=============================\n\n";
    file << "This certifies that " << resident << " is a resident of this barangay.\n\n";
    file << "Purpose: " << purpose << "\n\n";
    file << "Approved by:\n";
    file << "Barangay Captain\n";
    file.close();
    return true;
}

void generateClearance()
{
    std::string resident;
    std::string purpose;

    std::cout << "\nResident Name: ";
    std::cin.ignore();
    std::getline(std::cin, resident);
    std::cout << "Purpose: ";
    std::getline(std::cin, purpose);

    if (generateClearanceFor(resident, purpose)) {
        std::cout << "\nClearance Generated\n";
    } else {
        std::cout << "\nFailed to generate clearance\n";
    }
}
