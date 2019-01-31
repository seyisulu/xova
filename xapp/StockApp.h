#pragma once
#include "stdafx.h"

using namespace std;
using namespace web;
using namespace http;
using namespace utility;
using namespace http::experimental::listener;

class StockApp
{
public:
	StockApp();
	StockApp(string_t);
	virtual ~StockApp();

protected:
	void handle_req(http_request);
	void respond(const http_request&, const status_code&, const string&);

private:
	uri url;
	http_listener lst;
};

