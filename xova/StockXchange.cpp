#include "StockXchange.h"

using namespace std;

StockXchange::StockXchange() : StockXchange(U("33455"))
{
}


StockXchange::StockXchange(utility::string_t port)
{
	dbm = unique_ptr<DBMgr>(new DBMgr());
	utility::string_t addr = U("http://localhost:");
	addr.append(port);
	uri_builder urb(addr);
	urb.append_path(U("api"));
	urb.append_path(U("v1"));
	url = urb.to_uri();
	lst = http_listener(url.to_string());
	http_listener* lstptr = &lst;
	lst.open().then([=]()
	{
		lstptr->support(tr1::bind(&StockXchange::handle_req, this, std::tr1::placeholders::_1));
	});
}


void StockXchange::handle_req(http_request req)
{// Extract request path, query strings, request method, headers, and body
	auto pth = uri::split_path(uri::decode(req.relative_uri().path()));
	auto qry = uri::split_query(uri::decode(req.relative_uri().query()));
	auto mtd = req.method().c_str();
	auto hdr = req.headers();
	auto bdy = req.extract_json().get();
	auto cde = status_codes::OK;

	json::value res; // TODO: remove <debug>
	res[U("path")] = this->vector2json(pth);
	res[U("query")] = this->map2json(qry);
	res[U("method")] = json::value(mtd);

	if (1 == pth.size() && U("register") == pth[0])
	{// Relax authentication requirements for registration
		auto usr = bdy[U("username")].as_string();
		auto pwd = bdy[U("password")].as_string();
		auto join_rslt = this->join(usr, pwd);
		0 == join_rslt ? res[U("message")] = json::value(U("Failed")) : res[U("message")] = join_rslt;
	}
	else if (hdr.has(U("Authorization")))
	{// Constrain all requests to REST server to use Basic Auth
		char* token_next, *passphrase;
		auto basic = conversions::to_utf8string(hdr.find(U("Authorization"))->second);
		auto token = strtok_s(&basic[0], " ", &token_next);

		if (token_next != NULL)
		{// If using the Basic Auth scheme, continue
			auto coded = string(token_next);
			auto uncds = conversions::to_string_t(coded); // Platform dependent UNICODE string
			auto chvec = conversions::from_base64(uncds); // Store vector of unsigned char
			std::string plain(chvec.begin(), chvec.end());
			auto username = strtok_s(&plain[0], ":", &passphrase);

			auto username_t = conversions::to_string_t(username);
			auto passphrase_t = conversions::to_string_t(passphrase);

			if (this->auth(username_t, passphrase_t))
			{// If credentials are valid, continue to operation
				if (pth.empty())
				{
					res[U("message")] = json::value(U("API v1: Stock Exchange Server"));
				}
				else if (pth.size() == 1)
				{
					res[U("controller")] = json::value(pth[0]);
					if (pth[0] == U("auth"))
					{
						res[U("message")] = json::value(U("Authorized"));
						res[U("token")] = json::value(conversions::to_string_t(coded));
					}
					else if (pth[0] == U("buy"))
					{
						auto cde_t = bdy[U("stockCode")].as_string();
						auto qty_t = bdy[U("quantity")].as_string();
						auto prc_t = bdy[U("price")].as_string();
						if (this->buy(username_t, cde_t, qty_t, prc_t))
						{
							res[U("message")] = json::value(U("Success"));
						}
						else 
						{
							res[U("message")] = json::value(U("Failed"));
						}
					}
					else if (pth[0] == U("sell"))
					{

					}
					else if (pth[0] == U("portfolio"))
					{
						res[U("message")] = this->portfolio(username_t);
					}
					else if (pth[0] == U("transactions"))
					{

					}
				}				
			}
			else
			{
				res[U("message")] = json::value(U("Unauthorized"));
				cde = status_codes::Unauthorized;
			}
		}
	}
	else // Not authenticated and not registering
	{
		res[U("message")] = json::value(U("Unauthorized"));
		cde = status_codes::Unauthorized;
	}

	this->respond(req, cde, res);
	return;
}


void StockXchange::respond(const http_request& req, const status_code& status, const json::value& res) {
	json::value resp;
	resp[U("status")] = json::value::number(status);
	resp[U("response")] = res;
	req.reply(status, resp);
}


bool StockXchange::auth(string_t userName, string_t password)
{
	SHA1 sha1;
	bool resp = false;
	auto res = dbm->find_trader(userName);
	auto passHash = sha1(conversions::to_utf8string(password));
	auto passHash_t = conversions::to_utf16string(passHash);

	if (res.has_field(U("password")))
	{
		if (res[U("password")].as_string() == passHash_t)
		{
			resp = true;
		}
	}
	return resp;
}

bool StockXchange::join(string_t userName, string_t password)
{
	SHA1 sha1;
	bool resp = false;
	auto passHash = sha1(conversions::to_utf8string(password));
	auto passHash_t = conversions::to_string_t(passHash);
	if (0 != dbm->join_trader(userName, passHash_t))
	{
		resp = true;
	}
	return resp;
}

bool StockXchange::buy(string_t userName, string_t stockCode, string_t quantity, string_t price)
{
	return dbm->buy_stock(userName, stockCode, quantity, price) == 0 ? false : true;
}

json::value StockXchange::portfolio(const string_t userName)
{	
	return dbm->list_portfolio(userName);
}


template<typename T>
json::value StockXchange::vector2json(const vector<T> vec) {
	json::value jvec;
	int i = 0;
	for (const auto& v : vec) {
		jvec[i++] = json::value(v);
	}
	return jvec;
}


template<typename T>
json::value StockXchange::map2json(const map<T, T> map) {
	json::value jmap;
	for (const auto& m : map) {
		jmap[m.first] = json::value(m.second);
	}
	return jmap;
}


StockXchange::~StockXchange()
{
	lst.close().wait();
}

