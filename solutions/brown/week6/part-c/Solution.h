#ifndef C_PLUS_PLUS_BELTS_SOLUTION_H
#define C_PLUS_PLUS_BELTS_SOLUTION_H

#include "CustomUtils.h"
#include "Request.h"
#include "test_runner.h"
#include <vector>

std::vector<std::unique_ptr<Request>> ParseRequests(const TypeConverter &converter, std::istream &input);

void ProcessModifyRequests(Database &db, std::vector<RequestHolder> &requests);

std::vector<std::string> ProcessReadRequests(Database &db, std::vector<RequestHolder> &requests);

void PrintResponses(const std::vector<std::string> &responses, std::ostream &stream = std::cout);

void TestParseRequests_withModifyConverter();
void TestParseRequests_withReadConverter();
void TestIntegrationGenerator(std::string input, std::string expected);
void TestIntegrationPartC();
void TestIntegrationPartC_byGrader();

#endif // C_PLUS_PLUS_BELTS_SOLUTION_H
