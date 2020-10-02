#ifndef C_PLUS_PLUS_BELTS_SOLUTION_H
#define C_PLUS_PLUS_BELTS_SOLUTION_H

#include <fstream>
#include <iostream>
#include <vector>

#include "CustomUtils.h"
#include "Json.h"
#include "Request.h"
#include "test_runner.h"

std::vector<std::unique_ptr<Request>> ParseSpecificRequests(TypeConverter converter, Json::Document &document,
                                                            std::string key);

void ProcessModifyRequests(Database &db, std::vector<RequestHolder> &requests);

Json::Document ProcessReadRequests(Database &db, std::vector<RequestHolder> &requests);

void PrintResponses(const std::vector<std::string> &responses, std::ostream &stream = std::cout);

void TestParsing();
void TestIntegrationGenerator(std::string input, std::string expected);
void TestIntegrationPartD();
void TestIntegrationPartDRaw();

#endif  // C_PLUS_PLUS_BELTS_SOLUTION_H
