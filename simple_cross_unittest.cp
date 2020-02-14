#include "OrderBook.h"
#include "gtest/gtest.h"

 
TEST (OrderBook, defaultConstrutor) 
{ 
 OrderBook myOrderBook;
 
 EXPECT_EQ ("Unknown", myOrderBook.symbolName);
    
}
 
TEST (limit_order_t, defaultConstrutor) 
{ 
	limit_order_t myOrder;
	
    EXPECT_EQ (0, myOrder.price);
	EXPECT_EQ (0, myOrder.quantity);
	EXPECT_EQ (0, myOrder.order_id);
	EXPECT_EQ (BUY, myOrder.SIDE);
	EXPECT_EQ ("Unknown", myOrder.symbol);
	
    
}


class OrderBookTest : public testing::Test 
{

 public:
	
	std::unordered_map<std::string, OrderBook> all_books;
	
	std::string price = "100.00000";
	std::string quantity = "5";
	std::string strSide = "B";
	std::string oid = "10001";
	std::string symbol = "IBM";
	results_t output;
	
	virtual void SetUp()
	{
		//clear static memebers for each test.
		OrderBook::cancels.clear();
		OrderBook::all_order_ids.clear();
		//Clear output
		output.clear();
  
    }

};

TEST_F (OrderBookTest, processOrder_BadFormat) 
{ 
	OrderBook myOrderBook(symbol);
	
	std::string price = "ten";
	myOrderBook.processOrder(price,quantity,strSide,oid,output);
	//error message and no order added.
	EXPECT_EQ(1,output.size());
	EXPECT_EQ(0,myOrderBook.bids.size());
	EXPECT_EQ(0,myOrderBook.asks.size());
    
}

TEST_F (OrderBookTest, processOrder_NegSize) 
{ 
	OrderBook myOrderBook(symbol);
	
	std::string quantity = "-5";
	myOrderBook.processOrder(price,quantity,strSide,oid,output);
	//error message and no order added.
	EXPECT_EQ(1,output.size());
	EXPECT_EQ(0,myOrderBook.bids.size());
	EXPECT_EQ(0,myOrderBook.asks.size());

}

TEST_F (OrderBookTest, processOrder_InvalidSide) 
{ 
	OrderBook myOrderBook(symbol);
	
	std::string strSide = "N";
	myOrderBook.processOrder(price,quantity,strSide,oid,output);
	//error message and no order added.
	EXPECT_EQ(1,output.size());
	EXPECT_EQ(0,myOrderBook.bids.size());
	EXPECT_EQ(0,myOrderBook.asks.size());
	
}

TEST_F(OrderBookTest, processOrder)
{

  OrderBook myOrderBook(symbol);
  all_books[symbol] = myOrderBook;
  myOrderBook.processOrder(price,quantity,strSide,oid,output);
  EXPECT_EQ(0,output.size());
  EXPECT_EQ(1,myOrderBook.bids.size());
  EXPECT_EQ(1,OrderBook::all_order_ids.size());
  EXPECT_EQ(0,myOrderBook.asks.size());
  
  
}

TEST_F(OrderBookTest, processOrder_bid)
{

  OrderBook myOrderBook(symbol);
  all_books[symbol] = myOrderBook;
  myOrderBook.processOrder(price,quantity,strSide,oid,output);
  EXPECT_EQ(0,output.size());
  EXPECT_EQ(1,myOrderBook.bids.size());
  EXPECT_EQ(1,OrderBook::all_order_ids.size());
  EXPECT_EQ(0,myOrderBook.asks.size());
  
  
}

TEST_F(OrderBookTest, processOrder_ask)
{

  OrderBook myOrderBook(symbol);
  all_books[symbol] = myOrderBook;
  strSide = "S";
  myOrderBook.processOrder(price,quantity,strSide,oid,output);
  EXPECT_EQ(0,output.size());
  EXPECT_EQ(0,myOrderBook.bids.size());
  EXPECT_EQ(1,OrderBook::all_order_ids.size());
  EXPECT_EQ(1,myOrderBook.asks.size());
  
  
}

TEST_F(OrderBookTest, processOrder_badSide)
{

  OrderBook myOrderBook(symbol);
  all_books[symbol] = myOrderBook;
  strSide = "N";
  myOrderBook.processOrder(price,quantity,strSide,oid,output);
  //expect error message:
  EXPECT_EQ(1,output.size());
  
  EXPECT_EQ(0,myOrderBook.bids.size());
  EXPECT_EQ(0,myOrderBook.asks.size());
  
  
}

TEST_F(OrderBookTest, cancelNonExistingOrder)
{

  OrderBook myOrderBook(symbol);
  all_books[symbol] = myOrderBook;
  myOrderBook.processOrder(price,quantity,strSide,oid,output);
  ASSERT_EQ(0,output.size());
  
  bool found = OrderBook:: cancelOrder(0);
  //Nothing Marked for cancel:
  EXPECT_EQ(0,OrderBook::cancels.size());
  ASSERT_FALSE(found);
  
}

TEST_F(OrderBookTest, cancelExistingOrder)
{

  OrderBook myOrderBook(symbol);
  all_books[symbol] = myOrderBook;
  myOrderBook.processOrder(price,quantity,strSide,oid,output);
  ASSERT_EQ(0,output.size());
  
  bool found = OrderBook:: cancelOrder(stoi(oid));
  //Marked for cancel:
  EXPECT_EQ(1,OrderBook::cancels.size());
  ASSERT_TRUE(found);
  
}

TEST_F(OrderBookTest, removeCancelled_bid)
{
  OrderBook myOrderBook(symbol);
  all_books[symbol] = myOrderBook;
  myOrderBook.processOrder(price,quantity,strSide,oid,output);
  ASSERT_EQ(0,output.size());
  
  bool found = OrderBook:: cancelOrder(stoi(oid));
  //Marked for cancel:
  EXPECT_EQ(1,OrderBook::cancels.size());
  ASSERT_TRUE(found);
  
  //cancelled order removal:
  myOrderBook.removeCancelledOrders();
  EXPECT_EQ(0,OrderBook::cancels.size());
  
  
}

TEST_F(OrderBookTest, removeCancelled_ask)
{
  
  OrderBook myOrderBook(symbol);
  all_books[symbol] = myOrderBook;
  strSide = "S";
  myOrderBook.processOrder(price,quantity,strSide,oid,output);
  ASSERT_EQ(0,output.size());
  
  bool found = OrderBook:: cancelOrder(stoi(oid));
  //Marked for cancel:
  EXPECT_EQ(1,OrderBook::cancels.size());
  ASSERT_TRUE(found);
  
  myOrderBook.removeCancelledOrders();
  EXPECT_EQ(0,OrderBook::cancels.size());
  
  
}

TEST_F(OrderBookTest, getPendingOrders)
{
  
  OrderBook myOrderBook(symbol);
  all_books[symbol] = myOrderBook;
  
  myOrderBook.processOrder(price,quantity,strSide,oid,output);
  ASSERT_EQ(0,output.size());
  
  strSide = "S";
  price = "101.00000";
  
  myOrderBook.processOrder(price,quantity,strSide,oid,output);
  ASSERT_EQ(0,output.size());
  
  myOrderBook.getPendingOrders(output);
  //should have 2 pending orders.
  ASSERT_EQ(2,output.size());
  
}

TEST_F(OrderBookTest, fill_Bids)
{
  
  OrderBook myOrderBook(symbol);
  all_books[symbol] = myOrderBook;
  
  myOrderBook.processOrder(price,quantity,strSide,oid,output);
  ASSERT_EQ(0,output.size());
  
  std::shared_ptr<limit_order_t> Order = std::make_shared<limit_order_t>(10000000, 1,10002, SELL, symbol);  
  myOrderBook.fill_Bids(Order,output);
  //should get 2 fills.
  ASSERT_EQ(2,output.size());
  
  //still 1 bid and no offers in the book.
  EXPECT_EQ(1,myOrderBook.bids.size());
  EXPECT_EQ(0,myOrderBook.asks.size());
  
  
}

TEST_F(OrderBookTest, fill_Offers)
{
  
  OrderBook myOrderBook(symbol);
  all_books[symbol] = myOrderBook;
  strSide = "S";
  myOrderBook.processOrder(price,quantity,strSide,oid,output);
  ASSERT_EQ(0,output.size());
    
  std::shared_ptr<limit_order_t> Order = std::make_shared<limit_order_t>(10000000, 1,10002, BUY, symbol);  
  
  myOrderBook.fill_Offers(Order,output);
  //should get 2 fills.
  ASSERT_EQ(2,output.size());
  
   //still 0 bid and 1 offer in the book.
  EXPECT_EQ(0,myOrderBook.bids.size());
  EXPECT_EQ(1,myOrderBook.asks.size());
  
}

TEST_F(OrderBookTest, fill_Bids_largeSize)
{
  
  OrderBook myOrderBook(symbol);
  all_books[symbol] = myOrderBook;
  
  myOrderBook.processOrder(price,quantity,strSide,oid,output);
  ASSERT_EQ(0,output.size());
  
  std::shared_ptr<limit_order_t> Order = std::make_shared<limit_order_t>(10000000, 10,10002, SELL, symbol);  
  
  myOrderBook.fill_Bids(Order,output);
  //should get 2 fills.
  ASSERT_EQ(2,output.size());
  
  //remaing size as offer, no bid left.
  EXPECT_EQ(0,myOrderBook.bids.size());
  EXPECT_EQ(1,myOrderBook.asks.size());
  
  
}

 TEST_F(OrderBookTest, fill_Offers_largeSize)
{
  
  OrderBook myOrderBook(symbol);
  all_books[symbol] = myOrderBook;
  strSide = "S";
  myOrderBook.processOrder(price,quantity,strSide,oid,output);
  ASSERT_EQ(0,output.size());
  
  std::shared_ptr<limit_order_t> Order = std::make_shared<limit_order_t>(10000000, 10,10002, BUY, symbol);  
  
  myOrderBook.fill_Offers(Order,output);
  //should get 2 fills.
  ASSERT_EQ(2,output.size());
  
   //remaining size as bid and no offers in the book.
  EXPECT_EQ(1,myOrderBook.bids.size());
  EXPECT_EQ(0,myOrderBook.asks.size());
  
  
} 