#ifndef C_PLUS_PLUS_BELTS_SOLUTION_H
#define C_PLUS_PLUS_BELTS_SOLUTION_H

#include "test_runner.h"
#include <vector>
#include <iostream>
#include <fstream>
#include "Request.h"
#include "CustomUtils.h"
#include "Json.h"
#include "Settings.h"

void InitializeSettings(const Json::Document& document, const std::string& key);

std::vector<std::unique_ptr<Request>>
ParseSpecificRequests(const TypeConverter& converter, Json::Document& document, std::string key);

void ProcessModifyRequests(Database& db, std::vector<RequestHolder>& requests);

Json::Document ProcessReadRequests(Database& db, std::vector<RequestHolder>& requests);

void PrintResponses(const std::vector<std::string>& responses, std::ostream& stream = std::cout);
Json::Document GeneralProcess(std::istream& input);

void TestParsing();
void TestIntegrationGenerator(const std::string& testDataFolderName);
void TestIntegrationTest1();
void TestIntegrationTest2();
void TestIntegrationTest3();
void TestIntegrationTest4();

#endif //C_PLUS_PLUS_BELTS_SOLUTION_H
