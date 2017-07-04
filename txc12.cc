//
// This file is part of an OMNeT++/OMNEST simulation example.
//
//how to compile:
//opp_makemake -f -o output -I/mysql/include -L/mysql/lib/o -lmysqlclient
//
//how to run rapidly:
//make && ./output
//
// Copyright (C) 2003 Ahmet Sekercioglu
// Copyright (C) 2003-2015 Andras Varga
//
// This file is distributed WITHOUT ANY WARRANTY. See the file
// `license' for details on this and other legal matters.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <omnetpp.h>
#include <mysql/mysql.h>
#include <iostream>
#include <fstream>
#include <stdbool.h>
#include <time.h>
#include "tictoc13_m.h"

#define DB_COORDINATOR 0

using namespace omnetpp;
static char *unix_socket = NULL;

int transaction_counter = 0;
int transaction_array[200][5]; 
int global_pid = 0;





////////////////////// DATABASE METHODS //////////////////////

class DB : public cSimpleModule
{
  protected:
	virtual void forwardMessage(TicTocMsg13 *msg);
	virtual void initialize() override;
	virtual void handleMessage(cMessage *msg);
	virtual ErrorMessage *generateErrMessage(int src, char dest, int err_pid, int msg_type);
	virtual TicTocMsg13 *generateMessage(int src, int dest, int sndr_amnt, int rec_amnt, int operation_pid, char source, char receiver);
	virtual TicTocMsg13 *generateMessage(int sender, int my_type, char src, char rec, int pid, int amount);
	virtual TicTocMsg13 *generateMessage(bool flag, int pid);

	virtual MYSQL *createConnection(char *db_name);
	virtual MYSQL *handleConnection(int my_db);
	virtual void insert_transaction(int pid, char sender, char receiver, int money);
	virtual int read_balance(char* my_query, MYSQL *connection);
	virtual int transaction_amount(int pid);
	virtual int balance_amount(char person);
	virtual bool check_balance(int my_amount, char source);
	virtual char *create_insert(int pid, char sender, char receiver , int money);
	virtual char *create_query(char *first_part, char name_part);
	virtual char *create_query_transaction(char *first_part, int pid);
	virtual char *create_update(char *first_part, char *second_part, int amount, char name_part);
	virtual void update_balance(int transaction_amount, char source, char destination);
	virtual void finish_with_error(MYSQL *con);
	virtual int read_balance_char(char* my_query, MYSQL *connection); //aggiustabile


	virtual bool arevaluesequal(int val, int arr[][5],int size);
	virtual bool isvalueinarray(int val, int arr[][5], int size);
	virtual int whereisvalue(int val, int arr[200][5], int size);
	virtual int howmanytransactions(int my_pid, int arr[200][5], int size);

	virtual void rollback(int pid);
	virtual char read_sender(int pid);
	virtual char read_receiver(int pid);
	virtual void delete_transaction(int pid);



  private:
	unsigned int port = 3306;
	unsigned int flag = 0;
	int num_fields;
	int msg_type;
	int gate_size;
	bool final_flag = true;
	cMessage *event;

	//MYSQL *conn;
	MYSQL_RES *res;
	MYSQL_RES *query;
	MYSQL_ROW row;
};


Define_Module(DB);



MYSQL *DB::createConnection(char *db_name){

	MYSQL * con = mysql_init(NULL);

	if (con == NULL){
		fprintf(stderr, "mysql_init() failed\n");
		exit(1);
	}  

	if (!mysql_real_connect(con, "localhost", "root", "distributedpass", 
				db_name, port, unix_socket, CLIENT_MULTI_STATEMENTS)){
			finish_with_error(con);
		}
		else{
			//printf("\nConnection with the database established" );

			return con;
		}
	return NULL;
}

void DB::finish_with_error(MYSQL *con){
	printf("Connection error\n");
	fprintf(stderr, "%s\n", mysql_error(con));
	mysql_close(con);
	exit(1);        
}


void DB::initialize()
{
	EV << "HI, I'm number " << getIndex() << "\n";
	gate_size = gateSize("gate");

	//printf("HI, I'm number %d \n", getIndex());
	
}

MYSQL *DB::handleConnection(int my_db){

	MYSQL *con;
	switch (my_db){
		case 0: 
			con = createConnection((char*)"bank0");
			return con;

		case 1:
			con = createConnection((char*)"bank1");
			return con;

		case 2:
			con = createConnection((char*)"bank2");
			return con;

		case 3:
			con = createConnection((char*)"bank3");
			return con;

		default: 
			printf("\n50m3th1ng h45 g0n3 wr0ng!\n");
			con = NULL;
	}
	return con;

}



void DB::handleMessage(cMessage *msg){
	char msg_source;
	char msg_receiver;
	int pid;
	int amount;
	// Create the event object we'll use for timing -- just any ordinary message.
	event = new cMessage("timeout");

	TicTocMsg13 *ttmsg = check_and_cast<TicTocMsg13 *>(msg);
	msg_type = ttmsg->getMsg_type();

	
	printf("\n\n----------------------------------------------------------");
	printf("\nDB n.%d receives a message from (AGGIUSTARE)", getIndex());
	printf(" of type %d", msg_type);

	if (msg_type == 0) {
		pid = ttmsg->getPid();
		amount = ttmsg->getAmount();
		msg_receiver = ttmsg->getDestination();
		int pid_index = whereisvalue(pid, transaction_array, 200);
		bool transaction_equal = false;
		int number_of_transactions;
		
		msg_source = 'A';
		printf("\nEnd Timer!\n");

		printf("\nCheck dbs transaction money");
	

		transaction_equal = arevaluesequal(pid_index, transaction_array, 5);
		number_of_transactions = howmanytransactions(pid_index, transaction_array, 4); //perchè il primo non si deve contare
		printf("\nThere are %d transactions.", number_of_transactions);
		printf("\nAll transactions have the same money? ");
		printf("%s", transaction_equal ? "true" : "false");

		if (transaction_equal){
			//send msg to A 
			ErrorMessage *confirm_msg = generateErrMessage(getIndex(), msg_source, pid_index, 1);
			send(confirm_msg, "gate$o", getIndex());			//send msg to A

			ErrorMessage *notification_msg = generateErrMessage(getIndex(), msg_source, pid_index, 9);
			send(notification_msg, "gate$o", 4);			//send msg to B

		}
		else{
			TicTocMsg13 *rollback_msg= generateMessage(getIndex(), 5, msg_source, msg_receiver, pid, amount);
			printf("\nSend the broadcast message to the databases");

			for (int k = 1; k<gate_size-1; k++){

				TicTocMsg13 *copy = rollback_msg->dup();
				send(copy, "gate$o", k);
				bubble("Send rollback msg");
			}
			TicTocMsg13 *copy = rollback_msg->dup();
			scheduleAt(simTime(), copy);

			
			ErrorMessage *confirm_msg = generateErrMessage(getIndex(), msg_source, pid, 2);
			send(confirm_msg, "gate$o", getIndex());			//send msg to A
		}
	}

	if (msg_type == 1){
		pid = ttmsg->getPid();
		if (uniform(0, 1) < 0.1) {
        	EV << "\"Losing\" message\n";
        	printf("\nDB n.%d \"Lose\" message of type %d with pid %d \n", getIndex(), msg_type, pid);
        	bubble("message lost");  // making animation more informative...
        	delete ttmsg;
    	}
   		else {
		amount = ttmsg->getAmount();
				//if (getIndex()==2){
				//	amount = 1;
				//}
				msg_source = ttmsg->getSource();
				msg_receiver = ttmsg->getReceiver(); //e.g. B
				
				//printf("\nMsg_type: %d, Source %c, Destination: %d, Receiver: %c, PID: %d, Amount %ld\n", 
				//	msg_type, msg_source, ttmsg->getDestination(), msg_receiver, msg_pid, amount);
		
				delete ttmsg; //
		
				if(check_balance(amount, msg_source)){
					printf("\nBalance is greater than zero.");
					update_balance(amount, msg_source, msg_receiver);
					
					int index;
					insert_transaction(pid, msg_source, msg_receiver, amount);
					
					if (getIndex() == 0){
						//start timer
						if(isvalueinarray(pid, transaction_array, 200)){
							index = whereisvalue(pid, transaction_array, 200);
							transaction_array[index][getIndex()+1] = amount;
		
						}
						else{
							transaction_array[transaction_counter][0] = pid;
							transaction_array[transaction_counter][getIndex()+1] = amount;
							transaction_counter++;
		
							printf("\nDB n.%d starts a timer...", getIndex());
							TicTocMsg13 *self_msg= generateMessage(getIndex(), 0, msg_source, msg_receiver, pid, amount);
		
							scheduleAt(simTime()+5.0, self_msg);
						}
		
					}
					else{
						//other DBs send a message to the coordinator
						TicTocMsg13 *transaction_msg = generateMessage(getIndex(), 2 ,msg_source, msg_receiver, pid, amount);
						send(transaction_msg, "gate$o", DB_COORDINATOR);
						printf("\nDB n.%d sends a message to DB n.%d (coordinator)", getIndex(), DB_COORDINATOR);
						bubble("Send MSG");
		
					}
		
				}
				else{
					printf("Your balance is equal or less than 0, ERROR!\n");
					ErrorMessage *msg = generateErrMessage(getIndex(), msg_source, pid, 0);
					send(msg, "gate$o", getIndex()); //send a message to A
					bubble("Send Error");
				}
			}
	}

	if (msg_type == 2){

		int money_transfer = ttmsg->getAmount();
		char check_sender = ttmsg->getSource();
		char check_receiver = ttmsg->getReceiver();
		int source = ttmsg->getDb_source();
		pid = ttmsg->getPid();

		//printf("\nCheck if the operation with PID %d is %d euro", pid, money_transfer);

		//int query_transaction = transaction_amount(pid);
		

		printf("\nSource: %d", source );
		int index = 0;

		if(isvalueinarray(pid, transaction_array, 200)){
			index = whereisvalue(pid, transaction_array, 200);
			transaction_array[index][source+1] = money_transfer;

		}
		else{

			//db0 doesn't receive the first message from A
			printf("\nDB.%d non ha ricevuto il primo messaggio. rimando un self_msg", getIndex());
			TicTocMsg13 *self_msg= generateMessage(getIndex(), 0, check_sender, check_receiver, pid, money_transfer);
			scheduleAt(simTime()+5.0, self_msg);

			transaction_array[transaction_counter][0] = pid;
			transaction_array[transaction_counter][source+1] = money_transfer;

			printf("\nInserisco %d nell'array", transaction_array[transaction_counter][0]);
			transaction_counter++;
			index = whereisvalue(pid, transaction_array, 200);


		}

		printf("\nPid number: %d --- money:  %d ", transaction_array[index][0], transaction_array[index][source+1]);
	}


	if(msg_type == 5){
		pid = ttmsg->getPid();
		printf("\nOh, fuck! An error... I must rollback");
		rollback(pid);
	}
}

void DB::rollback(int pid){
	
	int amount = transaction_amount(pid);
	char sender = read_sender(pid);
	char receiver = read_receiver(pid);	
	printf("pid %d, sender %c, receiver %c\n",pid, sender, receiver );

	delete_transaction(pid);
	update_balance(amount, receiver, sender);
	printf("\nRollback performed correctly!");
}


void DB::delete_transaction(int pid){
	MYSQL *con = handleConnection(getIndex());

	//DELETE FROM table_name WHERE condition;
	char *delete_part = (char*)"DELETE FROM transaction WHERE pid=\'";
	char *delete_query= create_query_transaction(delete_part, pid);

	if (mysql_query(con, delete_query)){    
	  finish_with_error(con);
	}

	mysql_close(con);
	printf("\nTransaction number %d deleted", pid);

}

char DB::read_sender(int pid){
	MYSQL *con = handleConnection(getIndex());
	char *part_one = (char*)"SELECT sender FROM transaction WHERE pid=\'";
	char *balance_check= create_query_transaction(part_one, pid);
	return read_balance_char(balance_check, con);
}

char DB::read_receiver(int pid){
	MYSQL *con = handleConnection(getIndex());
	char *part_one = (char*)"SELECT receiver FROM transaction WHERE pid=\'";
	char *balance_check= create_query_transaction(part_one, pid);
	return read_balance_char(balance_check, con);
}

int DB::balance_amount(char person){
	MYSQL *con = handleConnection(getIndex());
	char *part_one = (char*)"SELECT balance FROM people WHERE name=\'";
	char *balance_check= create_query(part_one, person);
	return read_balance(balance_check, con);
}

int DB::transaction_amount(int pid){
	MYSQL *con = handleConnection(getIndex());
	char *part_one = (char*)"SELECT money_transaction FROM transaction WHERE pid=\'";
	char *transaction_query= create_query_transaction(part_one, pid);
	return read_balance(transaction_query, con);
}


void DB::update_balance(int transaction_amount, char source, char destination){
	MYSQL *con = handleConnection(getIndex());
	//UPDATE people SET balance = balance + transaction_amount WHERE name = source"
	char *increase_part = (char*)"UPDATE people SET balance = balance + ";
	char *decrease_part = (char*)"UPDATE people SET balance = balance - ";
	char *middle_part = (char*)" WHERE name = \'";
	
	char *increase_query = create_update(increase_part, middle_part, transaction_amount, destination);
	char *decrease_query = create_update(decrease_part, middle_part, transaction_amount, source);
	
	if (mysql_query(con, decrease_query)){    
	  finish_with_error(con);
	}
	else if (mysql_query(con, increase_query)){    
	  finish_with_error(con);
	}

	mysql_close(con);
	printf("\nBalance updated correctly in DB n.%d!", getIndex());
	//fprintf(stderr, "Connection closed!\n");
}

void DB::insert_transaction(int pid, char sender, char receiver, int money){
	MYSQL *con = handleConnection(getIndex());

	//query: INSERT INTO transaction(pid, sender, receiver, s_bal, r_bal) VALUES (999, 'A', 'B', 200, 300);
	char *insert_query = create_insert(pid, sender, receiver, money);

	if (mysql_query(con, insert_query)){    
	  finish_with_error(con);
	}

	
	mysql_close(con);
	//fprintf(stderr, "Connection closed!\n");

	printf("\nTransaction inserted correctly in DB n.%d!", getIndex());

}


char * DB::create_insert(int pid, char sender, char receiver , int money){
	char *close_query = (char*)"\')";
	char char_pid[10];
	char char_money[10];
	//char *quot = (char*)"\'";
	char *comma = (char*)"\', \'";

	sprintf(char_pid,"%d",pid); //convert int to a string
	sprintf(char_money,"%d",money); //convert int to a string

	size_t len;

	char *insert_part = (char*)"INSERT INTO transaction(pid, sender, receiver, money_transaction) VALUES (\'";
	char *full_insert = (char *) malloc(1+strlen(insert_part) + strlen(char_pid) + strlen(comma)+ 1 + strlen(comma) + 1 + strlen(comma) + strlen(char_money) + strlen(close_query));


	strcpy(full_insert, insert_part);
	strcat(full_insert, char_pid);
	strcat(full_insert, comma);
	len = strlen(full_insert);
	full_insert[len] = sender;
	full_insert[len + 1] = '\0';
	strcat(full_insert, comma);
	len = strlen(full_insert);
	full_insert[len] = receiver;
	full_insert[len + 1] = '\0';
	strcat(full_insert, comma);
	strcat(full_insert, char_money);
	strcat(full_insert, close_query);

	
	return full_insert;
}

char * DB::create_update(char *first_part, char *second_part, int amount, char name_part){
	char *close_query = (char*)"\'";
	char char_amount[10];
	sprintf(char_amount,"%d",amount); //convert int to a string

	size_t len = strlen(first_part);

	char * update_with_bal = (char *) malloc(1 + strlen(first_part) +strlen(char_amount)+strlen(second_part));
	strcpy(update_with_bal, first_part);
	strcat(update_with_bal, char_amount);
	strcat(update_with_bal, second_part);

	len = strlen(update_with_bal);

	char * update_with_name = (char *) malloc(1 + strlen(update_with_bal) +1);
	strcpy(update_with_name, update_with_bal);
	update_with_name[len] = name_part;
	update_with_name[len + 1] = '\0';


	char * update_query = (char *) malloc(1 + strlen(update_with_name) +strlen(close_query));
	strcpy(update_query, update_with_name);
	strcat(update_query, close_query);
	
	return (char*)update_query;
}

bool DB::check_balance(int my_amount, char source){
	MYSQL *con = handleConnection(getIndex());

	char *query_part = (char*)"SELECT balance FROM people WHERE name=\'";
		
	char *balance_query = create_query(query_part, source);

	int balance = read_balance(balance_query, con);


	if (balance - my_amount > 0){
		return true;
		
	}
	else {
		return false;
	}
}

char * DB::create_query_transaction(char *first_part, int pid){
	char *close_query = (char*)"\'";
	char char_pid[10];
	sprintf(char_pid,"%d",pid); //convert int to a string

	char * query_with_pid = (char *) malloc(1 + strlen(first_part) +strlen(char_pid)+ strlen(close_query));
	strcpy(query_with_pid, first_part);
	strcat(query_with_pid, char_pid);
	strcat(query_with_pid, close_query);


	return (char*)query_with_pid;
}

char * DB::create_query(char *first_part, char name_part){
	char *close_query = (char*)"\'";
		
	size_t len = strlen(first_part);

	char * query_with_name = (char *) malloc(1 + strlen(first_part) +1);
	strcpy(query_with_name, first_part);
	query_with_name[len] = name_part;

	query_with_name[len + 1] = '\0';
	char * query_string = (char *) malloc(1 + strlen(query_with_name) +strlen(close_query));
	strcpy(query_string, query_with_name);
	strcat(query_string, close_query);

	return (char*)query_string;
}

int DB::read_balance(char* my_query, MYSQL *connection){
	int query_balance;

	if (mysql_query(connection, my_query)){
		fprintf(stderr, "1\n\n");
				finish_with_error(connection);
		}

		query = mysql_store_result(connection);
  
		if (query == NULL) {
			fprintf(stderr, "2\n\n");
			finish_with_error(connection);
		}

		num_fields = mysql_num_fields(query);
  
		while ((row = mysql_fetch_row(query))){ 
			for(int i = 0; i < num_fields; i++){ 
				
				query_balance = std::stoi(row[i]);

			} 
		}

		mysql_free_result(query);
		mysql_close(connection);
		//fprintf(stderr, "Connection closed!\n");
		//fprintf(stderr, "%d\n", query_balance);
		return query_balance;
}

int DB::read_balance_char(char* my_query, MYSQL *connection){
	char query_balance;

	if (mysql_query(connection, my_query)){
		fprintf(stderr, "1\n\n");
				finish_with_error(connection);
		}

		query = mysql_store_result(connection);
  
		if (query == NULL) {
			fprintf(stderr, "2\n\n");
			finish_with_error(connection);
		}

		num_fields = mysql_num_fields(query);
  
		while ((row = mysql_fetch_row(query))){ 
			for(int i = 0; i < num_fields; i++){ 
				
				query_balance = row[i][0];
			} 
		}

		mysql_free_result(query);
		mysql_close(connection);
		//fprintf(stderr, "Connection closed!\n");
		//fprintf(stderr, "%d\n", query_balance);
		return query_balance;
}

ErrorMessage *DB::generateErrMessage(int src, char dest, int err_pid, int msg_type){
	ErrorMessage *msg = new ErrorMessage("user_msg");
	msg->setMsg_type(msg_type);
	msg->setSource(src);
	msg->setDestination(dest);
	msg->setPid(err_pid);

	return msg;
}

TicTocMsg13 *DB::generateMessage(bool flag, int pid){
	//no
	TicTocMsg13 *msg = new TicTocMsg13("flag_msg");
	msg->setMsg_type(3);
	msg->setDb_source(getIndex());
	msg->setDestination(0);
	msg->setFlag(flag);
	msg->setPid(pid);

	return msg;

}

TicTocMsg13 *DB::generateMessage(int sender, int my_type, char src, char rec, int pid, int amount){
	TicTocMsg13 *msg = new TicTocMsg13("msg");
	msg->setAmount(amount);
	msg->setMsg_type(my_type);
	msg->setDb_source(sender);
	msg->setSource(src);
	msg->setReceiver(rec);
	msg->setPid(pid);
	return msg;
}



TicTocMsg13 *DB::generateMessage(int src, int dest, int sndr_amnt, int rec_amnt, int operation_pid, char source, char receiver){
	//no
	TicTocMsg13 *msg = new TicTocMsg13("msg_check");
	msg->setMsg_type(2);
	msg->setDb_source(src);
	msg->setDestination(dest);
	msg->setSource(source);
	msg->setReceiver(receiver);
	msg->setAmount(sndr_amnt);
	msg->setAmount2(rec_amnt);	
	msg->setPid(operation_pid);
	return msg;

}

bool DB::isvalueinarray(int val, int arr[][5], int size){
	int i;
	for (i=0; i < size; i++) {
		if (arr[i][0] == val)
			return true;
	}
	return false;
}

int DB::whereisvalue(int val, int arr[][5], int size){
	int i;
	for (i=0; i < size; i++) {
		if (arr[i][0] == val)
			return i;
	}
	return 777;
}

bool DB::arevaluesequal(int my_pid, int arr[200][5], int size){
	int i;

	printf("\nPid array: %d ---- pid numero: %d", arr[my_pid][0], my_pid);
	for(i=1; i<size-1; i++){
		printf("\nEQ: Pos i: %d ---- pos i+1: %d", arr[my_pid][i], arr[my_pid][i+1]);
		if (arr[my_pid][i] != arr[my_pid][i+1]){

			return false;
		}
	}
	return true;
}

int DB::howmanytransactions(int my_pid, int arr[200][5], int size){
	int i;
	int bound = size+1;
	for(i=1; i<bound; i++){
		printf("\nHOW: Pos i: %d size: %d", arr[my_pid][i], size);
		if (arr[my_pid][i] == 0){
			size--;
		}
	}
	return size;
}


void DB::forwardMessage(TicTocMsg13 *msg)
{
	//something

	
}


////////////////////// PEOPLE METHODS //////////////////////

class Person : public cSimpleModule
{

  protected:
	virtual void forwardMessage(TicTocMsg13 *msg);
	virtual void initialize() override;
	virtual void handleMessage(cMessage *msg);
	virtual TicTocMsg13 *generateMessage(char src, int dest, int my_amount, char my_rec, int pid);
	virtual int read_pid();
	virtual void update_pid(int new_pid);

  private:
	cMessage *start;  // pointer to the event object which we'll use for timing
	cMessage *tictocMsg;  // variable to remember the message until we send it back
	char node_name;
	int gate_size;
};

Define_Module(Person);

int Person::read_pid(){

	int i;
	FILE *file;
	file = fopen("pid.txt", "r");
	fscanf (file, "%d", &i);    
	while (!feof (file)){  
		fscanf (file, "%d", &i);      
	}
	fclose(file);

	return i;

}

void Person::update_pid(int pid){
	FILE *file = fopen("pid.txt", "w");
	if (file == NULL){
		printf("Error opening file!\n");
		exit(1);
	}

	fprintf(file, "%d", pid);
	fclose(file);
	printf("\nNew pid: %d", pid);

}

void Person::initialize(){
	srand(time(NULL));

	node_name = par("name");
	printf("Initialize node %c\n", node_name);


	if (par("sendMsgOnInit").boolValue() == true) {
		printf("Program starts\n");
		start = new cMessage("start_msg");
		scheduleAt(0.0, start);
	}

}

void Person::forwardMessage(TicTocMsg13 *msg){

	int n = gateSize("gate");
	for (int k = 0; k<n; k = k+1){
		// $o and $i suffix is used to identify the input/output part of a two way gate
		//TicTocMsg13 *msg_forw = generateMessage(getIndex(), k);
		//send(msg_forw, "gate$o", k);
		printf("Forwarding message on gate[%d]\n", k);

		bubble("send message");
	}
	//delete msg;
}

void Person::handleMessage(cMessage *msg){

	if (msg == start) {
		
		if (par("sendMsgOnInit").boolValue() == true) {
		
			global_pid = read_pid();
			global_pid++;
			update_pid(global_pid);
			gate_size = gateSize("gate");
			int amount;

			do{
				amount = rand() % 20;
			}while(amount==0);


			char receiver = 'B';

			
			//Broadcast message = 99
			TicTocMsg13 *msg = generateMessage('A', 99, amount, receiver, global_pid);

			printf("Send the broadcast message to the databases\n");
			for (int k = 0; k<gate_size; k++){
			// $o and $i suffix is used to identify the input/output part of a two way gate
				TicTocMsg13 *copy = msg->dup();
				send(copy, "gate$o", k);
			
				bubble("Send messages");
			}

			printf("\nSend a new message");
			start = new cMessage("start_msg");
			scheduleAt(simTime()+(rand() % 30), start);
		}
	}
	else{

		ErrorMessage *ttmsg = check_and_cast<ErrorMessage *>(msg);
		int db_msg_type = ttmsg->getMsg_type();
		int pid_operation = ttmsg->getPid();
		
		printf("\nMsg from %d come back with destination %c", ttmsg->getSource(), ttmsg->getDestination());

		if(db_msg_type == 0){
			printf("\nYou can't performe operation with pid %d", pid_operation);
		}
		
		if(db_msg_type == 1){
			printf("\nOperation number %d is done!", pid_operation);
		}

		if(db_msg_type == 2){
			printf("\nWell, this is embarrassing! Sorry operation %d has gone wrong.", pid_operation);
		}

		if(db_msg_type == 9){
			printf("\nYou have received some money");
		}
		//forwardMessage(ttmsg);
	}

}

TicTocMsg13 *Person::generateMessage(char src, int dest, int my_balance, char my_rec, int my_pid)
{
	TicTocMsg13 *msg = new TicTocMsg13("msg");
	msg->setMsg_type(1);
	msg->setSource(src);
	msg->setDestination(dest);
	msg->setAmount(my_balance);
	msg->setReceiver(my_rec);
	msg->setPid(my_pid);

	return msg;
}


