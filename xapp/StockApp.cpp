#include "stdafx.h"
#include "StockApp.h"


StockApp::StockApp() : StockApp(U("33456"))
{
}


StockApp::StockApp(utility::string_t port)
{
	utility::string_t addr = U("http://localhost:");
	addr.append(port);
	uri_builder urb(addr);
	url = urb.to_uri();
	lst = http_listener(url.to_string());
	http_listener* lstptr = &lst;
	lst.open().then([=]()
	{
		lstptr->support(tr1::bind(&StockApp::handle_req, this, std::tr1::placeholders::_1));
	});
}


StockApp::~StockApp()
{
}

void StockApp::handle_req(http_request req)
{// Extract request path, query strings, request method, headers, and body
	auto pth = uri::split_path(uri::decode(req.relative_uri().path()));
	auto qry = uri::split_query(uri::decode(req.relative_uri().query()));
	auto mtd = req.method().c_str();
	auto hdr = req.headers();
	auto bdy = req.extract_json().get();
	auto cde = status_codes::OK;

	json::value res;

	if (pth.empty())
	{// Serve index.htm
		ifstream infile{ "assets/index.htm" };
		string file_contents{ istreambuf_iterator<char>(infile), istreambuf_iterator<char>() };
		this->respond(req, cde, file_contents);
		return;
	}
	else if (pth.size() == 1 && pth[0] == U("scripts.js"))
	{
		ifstream infile{ "assets/scripts.js" };
		string file_contents{ istreambuf_iterator<char>(infile), istreambuf_iterator<char>() };
		req.reply(cde, file_contents, "text/javascript");
		return;
	}
	else if (pth.size() == 1 && pth[0] == U("styles.css"))
	{
		ifstream infile{ "assets/styles.css" };
		string file_contents{ istreambuf_iterator<char>(infile), istreambuf_iterator<char>() };
		req.reply(cde, file_contents, "text/css");
		return;
	}

	this->respond(req, cde, "Error 404: Page not found.");
	return;
}

void StockApp::respond(const http_request& req, const status_code& status, const string& res) {
	//json::value resp;
	//resp[U("status")] = json::value::number(status);
	//resp[U("response")] = res;
	req.reply(status, res, "text/html");
}