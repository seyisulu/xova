#include "DBMgr.h"

static int DBcb(void *data, int argc, char **argv, char **azColName) {
	int i;
	json::value rval;
	auto rval_vec = (vector<json::value>*) data;
	for (i = 0; i < argc; i++) {
		if (argv[i])
		{
			rval[utility::conversions::to_string_t(azColName[i])] =
				json::value(utility::conversions::to_string_t(argv[i]));
		}
		else
		{
			rval[utility::conversions::to_string_t(azColName[i])] =
				json::value();
		}
	}
	rval_vec->push_back(rval);
	return 0;
}

DBMgr::DBMgr()
{
	int rc = sqlite3_open_v2("db.sqlite3", &dbptr, SQLITE_OPEN_READWRITE, NULL);
	assert(rc == SQLITE_OK);
	opened = true;
}


json::value DBMgr::find_trader(utility::string_t userName) {
	wstringstream strm;
	sqlite3_stmt* stmt;

	strm << U("SELECT username, password FROM trader WHERE username = '") << userName << U("'");

	auto s = strm.str();
	const auto query = &s[0];

	json::value rval;
	{
		if (sqlite3_prepare16(dbptr, query, -1, &stmt, 0) == SQLITE_OK)
		{
			int ctotal = sqlite3_column_count(stmt);
			int res = 0;
			res = sqlite3_step(stmt);
			if (res == SQLITE_ROW)
			{
				for (int i = 0; i < ctotal; i++)
				{
					string k = (char*)sqlite3_column_name(stmt, i);
					string v = (char*)sqlite3_column_text(stmt, i);

					auto json_key = json::value(utility::conversions::to_utf16string(k));
					auto json_val = json::value(utility::conversions::to_utf16string(v));

					rval[json_key.as_string()] = json_val;
				}
			}
		}
	}
	return rval;
}


json::value DBMgr::list_portfolio(utility::string_t userName)
{
	auto query_vec = vector<utility::string_t>{ 
		U("SELECT * FROM portfolio WHERE username='"), userName, U("'") };
	auto query = this->vec2query(query_vec);
	auto res = this->run_query(&query[0]);
	return this->vec2json(res);
}


int DBMgr::join_trader(utility::string_t userName, utility::string_t password)
{
	wstringstream strm;
	sqlite3_stmt* stmt;

	strm << U("INSERT INTO trader(username, password, balancecash) VALUES ('") 
		 << userName << U("','") << password << U("','100000')");

	auto s = strm.str();
	const auto query = &s[0];

	int rval;
	if (sqlite3_prepare16(dbptr, query, -1, &stmt, 0) == SQLITE_OK)
	{
		int res = sqlite3_step(stmt);
		rval = res;
		sqlite3_finalize(stmt);
		return rval;
	}
	return 0;
}


int DBMgr::buy_stock(
	utility::string_t userName, 
	utility::string_t stockCode, 
	utility::string_t quantity, 
	utility::string_t price)
{
	auto query_vec = vector<utility::string_t>{
		U("SELECT balancecash FROM trader WHERE username='"), userName, U("'") };
	auto qry_ = this->vec2query(query_vec);
	auto qty_ = this->wstrtoi(quantity);
	auto prc_ = this->wstrtof(price);
	auto cst_ = qty_ * prc_;
	auto bal_ = 0;
	auto rvl_ = SQLITE_OK;
	// Making sure the quantity, cost and price values are valid
	auto cst_string = utility::conversions::to_string_t(to_string(cst_));
	auto prc_string = utility::conversions::to_string_t(to_string(prc_));
	auto qty_string = utility::conversions::to_string_t(to_string(qty_));

	auto res_balance = this->run_query(&qry_[0]); 
	// Invalid traders aren't authorized anyway so we always get a result
	auto balance_csh = this->wstrtof(res_balance[0][U("balancecash")].as_string());
	if (balance_csh > cst_)
	{
		auto query_buy_vec = vector<utility::string_t>{
			U("UPDATE trader SET balancecash = balancecash - "), cst_string, U(" WHERE username = '"), userName, U("'; "),
			U("INSERT INTO portfolio (username, stockcode, quantity, totalcost) VALUES('"), userName, U("', '"), stockCode, U("', '"), qty_string, U("', '"), cst_string, U("'); "),
			U("UPDATE stock SET lastsaleprice = "), prc_string, U(" WHERE stockcode = '"), stockCode, U("'; "), };
		auto query_buy = this->vec2query(query_buy_vec);
		this->run_query(&query_buy[0]);
		return qty_;
	}
	return 0;
}


json::value DBMgr::vec2json(const vector<json::value>& vec)
{
	auto max = vec.size();
	json::value rval;
	for (int iota = 0; iota < max; iota++)
	{
		rval[iota] = vec[iota];
	}
	return rval;
}


string DBMgr::vec2query(const vector<utility::string_t>& vec)
{
	wstringstream strm;
	for (const auto &v : vec)
	{
		strm << v;
	}
	auto s = strm.str();
	auto s8 = utility::conversions::to_utf8string(s);
	return s8;
}


vector<json::value> DBMgr::run_query(const char* sql)
{
	char *zErrMsg = 0;
	vector<json::value> data;
	auto rc = sqlite3_exec(dbptr, sql, DBcb, (void*) &data, &zErrMsg);
	if (rc != SQLITE_OK) {
		fprintf(stderr, "SQL error: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
	}
	return data;
}


int DBMgr::wstrtoi(const utility::string_t wstr)
{
	auto utf8qty = utility::conversions::to_utf8string(wstr);
	return atoi(&utf8qty[0]);
}


float DBMgr::wstrtof(const utility::string_t wstr)
{
	auto utf8qty = utility::conversions::to_utf8string(wstr);
	return atof(&utf8qty[0]);
}


DBMgr::~DBMgr()
{
	if (true == opened)
	{
		sqlite3_close(dbptr);
	}
}
