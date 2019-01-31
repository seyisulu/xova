#pragma once
#include "stdafx.h"

using namespace std;
using namespace web;

class DBMgr
{
public:
	DBMgr();
	virtual ~DBMgr();
	json::value find_trader(utility::string_t);
	json::value list_portfolio(utility::string_t);
	int join_trader(utility::string_t, utility::string_t);
	int buy_stock(utility::string_t, utility::string_t, utility::string_t, utility::string_t);

private:
	bool opened = false;
	sqlite3* dbptr = NULL;
	json::value vec2json(const vector<json::value>&);
	string vec2query(const vector<utility::string_t>&);
	vector<json::value> run_query(const char*);
	int wstrtoi(const utility::string_t);
	float wstrtof(const utility::string_t);
};

