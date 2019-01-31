#pragma once
#include "stdafx.h"
#include "DBMgr.h"

using namespace std;
using namespace web;
using namespace http;
using namespace utility;
using namespace http::experimental::listener;

class StockXchange
{
public:
	StockXchange();
	StockXchange(string_t);
	virtual ~StockXchange();

protected:
	void handle_req(http_request);
	void respond(const http_request&, const status_code&, const json::value&);
	bool auth(string_t, string_t);
	bool join(string_t, string_t);
	bool buy(string_t, string_t, string_t, string_t);

	json::value portfolio(const string_t);

	template<typename T>
	json::value vector2json(const vector<T>);
	template<typename T>
	json::value map2json(const map<T, T>);

private:
	uri url;
	http_listener lst;
	unique_ptr<DBMgr> dbm;
};

