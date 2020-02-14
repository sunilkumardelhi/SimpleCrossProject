//#include "stdafx.h"

#include <fstream>
#include <iostream>
#include <vector>
#include <sstream>
#include <iterator>
#include "OrderBook.h"

typedef std::list<std::string> results_t;

class SimpleCross
{
	std::unordered_map<std::string, OrderBook> all_books;
	std::unordered_map<std::string, OrderBook>::iterator book_iter;


public:
	results_t action(const std::string& line)
	{
		if (line.size() == 0)
			return results_t();
		
		//convert action line to vector of tokens
		std::vector<std::string> tokens;
		std::istringstream iss(line);
		std::copy(std::istream_iterator<std::string>(iss),
			std::istream_iterator<std::string>(),
			back_inserter(tokens));

		results_t output;

		if (tokens[0] == "O")
		{
			if (tokens.size() < 6)
			{
				output.push_back("E  Unknown Request");
				return output;
			}
				

			//process the action request:
			std::string symbol = tokens[2];
			std::unordered_map<int32_t, std::shared_ptr<limit_order_t>>::iterator oid_iter;
			int32_t oid = stoi(tokens[1]);
			
			//check for duplicates
			oid_iter = OrderBook::all_order_ids.find(oid);
			if (oid_iter != OrderBook::all_order_ids.end())
			{
				output.push_back("E " + tokens[1] + " Duplicate order id");
				return output;
			}

			book_iter = all_books.find(symbol);
			if (book_iter != all_books.end())
			{
				book_iter->second.processOrder(tokens[5], tokens[4], tokens[3], tokens[1], output);
				return output;
			}
			else
			{
				//add a new book of symbol not found
				OrderBook tmpBook(symbol);
				tmpBook.processOrder(tokens[5], tokens[4], tokens[3], tokens[1], output);
				all_books[symbol] = tmpBook;
				return output;
			}

		}
		else if (tokens[0] == "X")
		{
			if (tokens.size() < 2)
			{
				output.push_back("E  Unknown Request");
				return output;
			}

			int32_t oid = stoi(tokens[1]);
			bool cancelled = OrderBook::cancelOrder(oid);
			if (cancelled)
				output.push_back(tokens[0] + " " + tokens[1]);
			else
				output.push_back("E " + tokens[1] + " Reject");
			return output;
		}
		else if (tokens[0] == "P")
		{

			if (tokens.size() != 1)
			{
				output.push_back("E  Unknown Request");
				return output;
			}

			for (book_iter = all_books.begin(); book_iter != all_books.end(); book_iter++)
			{
				book_iter->second.getPendingOrders(output);
			}
			return output;
		}
		else
		{
			output.push_back("E  Unknown Request");
			return output;
		}

		return(output);
	}
};

int main(int argc, char **argv)
{
	SimpleCross scross;
	std::string line;
	std::ifstream actions("actions.txt", std::ios::in);
	while (std::getline(actions, line))
	{
		results_t results = scross.action(line);
		//std::cout << "results.size() = " << results.size() << std::endl;
		for (results_t::const_iterator it = results.begin(); it != results.end(); ++it)
		{
			std::cout << *it << std::endl;
		}
	}
	//std::getchar();
	return 0;
}

