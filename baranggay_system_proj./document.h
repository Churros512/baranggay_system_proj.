#ifndef DOCUMENT_H
#define DOCUMENT_H

#include <string>

void generateClearance();

bool generateClearanceFor(const std::string& resident,
                          const std::string& purpose);

#endif
