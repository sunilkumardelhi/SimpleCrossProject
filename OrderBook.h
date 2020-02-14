//#include "stdafx.h"
#include <map>
#include <unordered_map>
#include <string>
#include <list>
#include <stdint.h>
#include <memory>



typedef std::list<std::string> results_t;

const long Mult = 100000;
struct bidCompare {
	bool operator()(const long& a, const long& b) const {
		return a > b;
	}
};



//Two possible sided Limit Orders:
enum side { BUY, SELL };
struct limit_order_t
{
	long price;			//implemeneted as Mult*actual decimal price 
	int quantity;		//quantity in limit order
	int32_t order_id;	//Order ID
	side SIDE;			//SIDE is either BUY or SELL
	std::string symbol;

	limit_order_t(long _p, int _q, int32_t _oid, side _side, std::string s)
		:price(_p), quantity(_q), order_id(_oid), SIDE(_side), symbol(s)
	{}
	
	limit_order_t()
	{
		price = 0;
		quantity = 0;
		order_id = 0;
		SIDE = BUY;
		symbol = "Unknown";
	}
};



class OrderBook
{
	

public:
	OrderBook(std::string symbol)
	{
		symbolName = symbol;
	}

	OrderBook() { symbolName = "Unknown"; }
	~OrderBook() {}

	std::string symbolName;
	//store orders by price key, value is pair of signed quantity (- for sell orders) and Order ID
	std::pair<int32_t, int> oid_size;
	std::multimap<long, std::shared_ptr<limit_order_t>, bidCompare> bids;
	std::multimap<long, std::shared_ptr<limit_order_t>> asks;

	static std::unordered_map <int32_t, std::shared_ptr<limit_order_t>> all_order_ids;
	static std::list<std::shared_ptr<limit_order_t>> cancels;

	std::multimap<long, std::shared_ptr<limit_order_t>, bidCompare> ::iterator bid_iter;
	std::multimap<long, std::shared_ptr<limit_order_t>> ::iterator ask_iter;

	typedef std::multimap<long, std::shared_ptr<limit_order_t>, bidCompare> ::iterator biditer_t;
	typedef std::multimap<long, std::shared_ptr<limit_order_t>> ::iterator askiter_t;


	static bool cancelOrder(int32_t oid);

	//crossing logic:
	void updateOrderBook(std::shared_ptr<limit_order_t> Order, results_t& out);
	void processOrder(std::string& price, std::string& quantity, std::string& side, std::string& oid, results_t& out);

	void fill_Offers(std::shared_ptr<limit_order_t> Order, results_t& out);

	void fill_Bids(std::shared_ptr<limit_order_t> Order, results_t& out);

	void joinBids(std::shared_ptr<limit_order_t> Order);
	void joinOffers(std::shared_ptr<limit_order_t> Order);
	void removeCancelledOrders();

	void getPendingOrders(results_t& out);

	void addfillMsg(int oid, int quantity, long price, results_t& out)
	{
		
		std::string fillmsg = "F " + std::to_string(oid) + " " + symbolName + " " + std::to_string(quantity) + " " + std::to_string(static_cast<double>(price) / Mult);
		out.push_back(fillmsg);
	}


};
