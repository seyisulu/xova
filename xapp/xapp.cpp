#include "stdafx.h"
#include "StockApp.h"

using namespace std;


int main()
{
	auto sxapp = std::unique_ptr<StockApp>(new StockApp());
	cout << "Stock Exchange App:" << endl << "Listening on http://localhost:33456" << endl;
	cout << "Press ENTER to exit." << endl;

	string line;
	getline(cin, line);

	sxapp.reset();
	return 0;
}

