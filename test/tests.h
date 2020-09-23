#pragma once

#include "json.h"
#include "descriptions.h"
#include "json.h"
#include "requests.h"
#include "sphere.h"
#include "transport_catalog.h"
#include "utils.h"
#include "utils/test_runner.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>



void TestParsing();
void TestIntegrationGenerator(const std::string &testDataFolderName);
void TestIntegrationTest1();
void TestIntegrationTest2();
void TestIntegrationTest3();
void TestIntegrationTest4();