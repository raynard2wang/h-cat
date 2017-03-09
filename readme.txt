v.0.0.1(2017.3.6 11:45:02)
1.add output data with hex view.
2.add format analysis.
it only can analysis format below:
struct bin
{
	struct block_head[2]
	{
		char start_time[7];
		char end_time[7];
	}
	struct block_item[]
	{
		char logic_card_no[4];
		char purchase_trade_no[2];
		char load_trade_no[2];
		char last_trade_time[7];
		char balance[4];
	}
}


2017.3.9 12:08
update username& email
