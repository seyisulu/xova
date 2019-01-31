#include "stdafx.h"
#include "StockXchange.h"

using namespace std;

int main(int argc, char* argv[])
{
	auto sxsvr = std::unique_ptr<StockXchange>(new StockXchange());
	cout << "Stock Exchange Server:" << endl << "Listening on http://localhost:33455/api/v1" << endl;
	cout << "Press ENTER to exit." << endl;

	string line;
	getline(cin, line);

	sxsvr.reset();
    return 0;
}

