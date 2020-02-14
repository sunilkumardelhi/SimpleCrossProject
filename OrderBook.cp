//#include "stdafx.h"
#include "OrderBook.h"
#include <math.h>
#include <exception>

std::unordered_map <int32_t, std::shared_ptr<limit_order_t>> OrderBook::all_order_ids;
std::list<std::shared_ptr<limit_order_t>> OrderBook::cancels;

bool OrderBook::cancelOrder(int32_t oid)
{
	std::unordered_map<int32_t, std::shared_ptr<limit_order_t>>::iterator oid_iter;
	bool found = false;
	oid_iter = all_order_ids.find(oid);
	if (oid_iter != all_order_ids.end())
	{
		(oid_iter->second)->quantity = 0;
		cancels.push_back(oid_iter->second);
		oid_iter = all_order_ids.erase(oid_iter);
		found = true;
	}

	return found;

}

void OrderBook::removeCancelledOrders()
{
	std::list<std::shared_ptr<limit_order_t>> ::iterator it;
	for (it = cancels.begin(); it != cancels.end(); ) 
	{
		bool erased = false;
		if ((*it)->symbol == symbolName)
		{
			if ((*it)->SIDE == BUY)
			{
				bid_iter = bids.find((*it)->price);
				while (bid_iter != bids.end() && it != cancels.end() && bid_iter->first == (*it)->price)
				{
					if (bid_iter->second->order_id == (*it)->order_id)
					{
						bid_iter = bids.erase(bid_iter);
						it = cancels.erase(it);
						erased = true;
					}
					else
						++bid_iter;
				}
			}
			else
			{
				ask_iter = asks.find((*it)->price);
				while (ask_iter != asks.end() && it != cancels.end() && ask_iter->first == (*it)->price)
				{
					if (ask_iter->second->order_id == (*it)->order_id)
					{
						ask_iter = asks.erase(ask_iter);
						it = cancels.erase(it);
						erased = true;
					}
					else
						++ask_iter;
				}
			}

		}
		if (!erased)
			it++;

	}
}



void OrderBook::getPendingOrders(results_t& out)
{
	//list offers in reverse order of proirty (as needed in output)
	std::multimap<long, std::shared_ptr<limit_order_t>> ::reverse_iterator rit = asks.rbegin();

	for (; rit != asks.rend(); ++rit)
	{
		//exclude orders marked for cancellation
		if (rit->second->quantity == 0)
			continue;

		std::string price = std::to_string(static_cast<double>(rit->first) / Mult);
		std::string size = std::to_string(rit->second->quantity);
		std::string oid = std::to_string(static_cast<int>(rit->second->order_id));

		std::string finalMsg = "P " + oid + " " + symbolName + " S " + size + " " + price;
		out.push_back(finalMsg);


	}
	
	//then bids:
	for (bid_iter = bids.begin(); bid_iter != bids.end(); bid_iter++)
	{
		//exclude orders marked for cancellation
		if (bid_iter->second->quantity == 0)
			continue;

		std::string price = std::to_string(static_cast<double>(bid_iter->first) / Mult);
		std::string size = std::to_string(bid_iter->second->quantity);
		std::string oid = std::to_string(static_cast<int>(bid_iter->second->order_id));

		std::string finalMsg = "P " + oid + " " + symbolName + " B " + size + " " + price;
		out.push_back(finalMsg);


	}


}

void OrderBook::updateOrderBook(std::shared_ptr<limit_order_t> Order, results_t& out)
{
	//Clean the book
	removeCancelledOrders();

	if (Order->SIDE == BUY)
		fill_Offers(Order, out);
	else
		fill_Bids(Order, out);

}

void OrderBook::processOrder(std::string& price, std::string& quantity, std::string& side, std::string& oid, results_t& out)
{
	try
	{
		double p = stod(price);
		int q = stoi(quantity);
		if (q <= 0)
		{
			out.push_back("E " + oid + " Negative Size");
			return;
		}
		std::shared_ptr<limit_order_t> Order = std::make_shared<limit_order_t>(floor(p * Mult + 0.5), q, stoi(oid), BUY, symbolName);
		if (side == "B")
			Order->SIDE = BUY;
		else if (side == "S")
			Order->SIDE = SELL;
		else
		{
			std::string finalMsg = "E " + oid + " Unknown side";
			out.push_back(finalMsg);
			return;
		}
		updateOrderBook(Order, out);
	}
	catch (std::exception& e)
	{
		out.push_back("E " + oid + " Bad Action Format");
	}
}

void OrderBook::fill_Offers(std::shared_ptr<limit_order_t> Order, results_t& out)
{

	//Loop through offer price levels going up from best offer price and update bestoffer price in each iteration:
	if (!asks.empty())
	{
		ask_iter = asks.begin();
		while (ask_iter != asks.end() && Order->price >= ask_iter->second->price && Order->quantity > 0 )
		{
			//if Order quantity is more than this pending offer quantity 
			if (Order->quantity >= ask_iter->second->quantity)
			{
				Order->quantity -= ask_iter->second->quantity;
				addfillMsg(Order->order_id, ask_iter->second->quantity, ask_iter->second->price, out);
				addfillMsg(ask_iter->second->order_id, ask_iter->second->quantity, ask_iter->second->price, out);

				ask_iter->second->quantity = 0;
				all_order_ids.erase(ask_iter->second->order_id); //erase from static member
				ask_iter = asks.erase(ask_iter);
				
			}
			else
			{
				//update the order size of remaining orders in list
				ask_iter->second->quantity -= Order->quantity;
				addfillMsg(Order->order_id, Order->quantity, ask_iter->second->price, out);
				addfillMsg(ask_iter->second->order_id, Order->quantity, ask_iter->second->price, out);

				Order->quantity = 0;
			}
		}//end of while Loop for checking remaining orders in list
	}
	

	 //Call joinBids method for remaining unfilled order
	if (Order->quantity >0)
		joinBids(Order);
}

void OrderBook::fill_Bids(std::shared_ptr<limit_order_t> Order, results_t& out)
{
	if (!bids.empty())
	{
		//Loop through offer price levels going up from best offer price 
		bid_iter = bids.begin();
		while (bid_iter != bids.end() && Order->price <= bid_iter->second->price && Order->quantity > 0  )
		{
			//if Order quantity is more than this pending offer quantity 
			if (Order->quantity >= bid_iter->second->quantity)
			{
				Order->quantity -= bid_iter->second->quantity;

				addfillMsg(Order->order_id, bid_iter->second->quantity, bid_iter->second->price, out);
				addfillMsg(bid_iter->second->order_id, bid_iter->second->quantity, bid_iter->second->price, out);

				bid_iter->second->quantity = 0;
				all_order_ids.erase(bid_iter->second->order_id); //erase from static member
				bid_iter = bids.erase(bid_iter);
			}
			else
			{
				//update the order size of remaining orders in list
				bid_iter->second->quantity -= Order->quantity;
				addfillMsg(Order->order_id, Order->quantity, bid_iter->second->price, out);
				addfillMsg(bid_iter->second->order_id, Order->quantity, bid_iter->second->price, out);

				Order->quantity = 0;
			}
		}//end of while Loop for checking remaining orders in list

	}
	
	 //Call joinBids method for remaining unfilled order
	if (Order->quantity >0)
		joinOffers(Order);
}


void OrderBook::joinOffers(std::shared_ptr<limit_order_t> Order)
{
	//add to offers
	asks.insert(std::make_pair(Order->price, Order));
	//update static memeber all_order_ids
	all_order_ids.insert(std::make_pair(Order->order_id, Order));
}

void OrderBook::joinBids(std::shared_ptr<limit_order_t> Order)
{
	//add to bids
	bids.insert(std::make_pair(Order->price, Order));
	//add to static member all_order_ids
	all_order_ids.insert(std::make_pair(Order->order_id, Order));
}