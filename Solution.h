#ifndef C_PLUS_PLUS_BELTS_SOLUTION_H
#define C_PLUS_PLUS_BELTS_SOLUTION_H

#include "test_runner.h"
#include <vector>
#include "Request.h"
#include "CustomUtils.h"

std::vector<std::unique_ptr<Request>> ParseRequests(const TypeConverter& converter, std::istream& input);

void ProcessModifyRequests(Database& db, std::vector<RequestHolder>& requests);

std::vector<std::string> ProcessReadRequests(Database& db, std::vector<RequestHolder>& requests);

void PrintResponses(const std::vector<std::string>& responses, std::ostream& stream = std::cout);

void TestIntegrationGenerator(std::string input, std::string expected);
void TestIntegrationPartD();

#endif //C_PLUS_PLUS_BELTS_SOLUTION_H
