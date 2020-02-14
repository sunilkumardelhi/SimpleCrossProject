# SimpleCrossProject
Creates order book for a ticker

Reads action.txt files showing orders for ticker and uses it to populate current state of order book for the ticker.


Design and Implementation Details:

This project uses bid and ask multimaps to store OrderBook information for each symbol. 
These OrderBook multimaps are keyed on Price and use custom compare for bids.
Entry to multimaps sorted by price aggressiveness ensures price-time of FIFO market.
Orders of same price are stored by time of insertion; thus implementing time priority.

Different OrderBooks for different symbols are stored in unordered_map (hash table) called all_books.
This allows constant time lookup of OrderBook by symbol.

OrderBook is a class that stores bid and ask multimaps. This class also has static member
all_order_ids which is also a unordered_map of all Orders in the book keyed on OrderID.
OrderIDs are gauranteed to be unique and this hash table provides constant time lookup of all 
OrderIDs in all the books of different symbols. Constant lookup is needed to cheaply check for duplicate
OrderIDs and process cancels based only on OrderID info.

Orders that are marked for cancels pushed to list of cancels and their size is set to zero. 
Cancels are removed from bid and ask multimaps before processing any new order for that book.
Searching the bid and ask multimaps by price of cancels saves time. Algo further searches for exact
orderID in the multimaps which can have multiple orders of same price.

This list of cancels is also static member that contain list of all cancels for all symbols.





 
