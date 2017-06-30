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
#include "tictoc13_m.h"

#define DB_COORDINATOR 0

using namespace omnetpp;
static char *unix_socket = NULL;

int transaction_counter = 0;
int transaction_array[200][4] = { {0,0,5,6}, {1,2,3,4}, {23,4}, {3,6},{4,8}};




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
	virtual char * create_insert(int pid, char sender, char receiver , int money);
	virtual char *create_query(char *first_part, char name_part);
	virtual char * create_query_transaction(char *first_part, int pid);
	virtual char *create_update(char *first_part, char *second_part, int amount, char name_part);
	virtual void update_balance(int transaction_amount, char source, char destination);
	virtual void finish_with_error(MYSQL *con);

	virtual bool arevaluesequal(int val, int arr[][4],int size);
	virtual bool isvalueinarray(int val, int arr[][4], int size);
	virtual int whereisvalue(int val, int arr[200][4], int size);


  private:
	unsigned int port = 3306;
	unsigned int flag = 0;
	int num_fields;
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
	// Create the event object we'll use for timing -- just any ordinary message.
    event = new cMessage("timeout");

	TicTocMsg13 *ttmsg = check_and_cast<TicTocMsg13 *>(msg);
	printf("\n\n----------------------------------------------------------");
	printf("\nDB n.%d receives a message", getIndex());
	int msg_type = ttmsg->getMsg_type();
	printf(" of type %d", msg_type);

	if (msg_type == 0) {
		int timer_pid = ttmsg->getPid();
		int pid_index = whereisvalue(timer_pid, transaction_array, 200);
		bool transaction_equal ;
		msg_source = 'A';
		printf("\nEnd Timer!\n");

		printf("\nCheck dbs transaction money");
		for (int i=0; i<4; i++){
			printf("\nArray in pos: %d", transaction_array[pid_index][i]);
		}

		transaction_equal = arevaluesequal(timer_pid, transaction_array, 4);
		printf("la flag e' ");
		printf("%s", transaction_equal ? "true" : "false");

		if (transaction_equal){
			//send msg to A 
			ErrorMessage *confirm_msg = generateErrMessage(getIndex(), msg_source, pid_index, 1);
			send(confirm_msg, "gate$o", getIndex());			//send msg to A

		}
		else{
			ErrorMessage *confirm_msg = generateErrMessage(getIndex(), msg_source, pid_index, 3);
			send(confirm_msg, "gate$o", getIndex());			//send msg to A
		}
		/*
		int msg_pid = ttmsg->getPid();
		msg_source = ttmsg->getSource();
		msg_receiver = ttmsg->getReceiver();
		
		delete ttmsg;
		//send msg to other dbs
		
		int sender_amount = balance_amount(msg_source);

		int receiver_amount = balance_amount(msg_receiver);
				
		int n = gateSize("gate");
		for (int k = 1; k<n-1; k = k+1){ 
			TicTocMsg13 *msg_check = generateMessage(getIndex(), k, sender_amount, receiver_amount, msg_pid, msg_source, msg_receiver);
			send(msg_check, "gate$o", k);
			printf("\nSend check msg to %d", k);
			bubble("Send check message");
		}

		TicTocMsg13 *self_msg= generateMessage(getIndex(), 4, msg_source, msg_receiver, msg_pid, sender_amount);
		scheduleAt(simTime()+5.0, self_msg);
		*/
	}

	if (msg_type == 1){
		int amount = ttmsg->getAmount();
		msg_source = ttmsg->getSource();
		msg_receiver = ttmsg->getReceiver(); //e.g. B
		int msg_pid = ttmsg->getPid();
		//printf("\nMsg_type: %d, Source %c, Destination: %d, Receiver: %c, PID: %d, Amount %ld\n", 
		//	msg_type, msg_source, ttmsg->getDestination(), msg_receiver, msg_pid, amount);

		delete ttmsg;

		if(check_balance(amount, msg_source)){
			printf("\nBalance is greater than zero.");
			update_balance(amount, msg_source, msg_receiver);
			printf("\nUpdate balance!");
			
			insert_transaction(msg_pid, msg_source, msg_receiver, amount);
			//start timer
			if (getIndex() == 0){
				printf("\nDB n.%d starts a timer...", getIndex());
				TicTocMsg13 *self_msg= generateMessage(getIndex(), 0, msg_source, msg_receiver, msg_pid, amount);
				scheduleAt(simTime()+5.0, self_msg);

	        }
	        else{
	        	TicTocMsg13 *transaction_msg = generateMessage(getIndex(), 2 ,msg_source, msg_receiver, msg_pid, amount);
	        	send(transaction_msg, "gate$o", DB_COORDINATOR);
	    	}

		}
		else{
			printf("Your balance is equal or less than 0, ERROR!\n");
			ErrorMessage *msg = generateErrMessage(getIndex(), msg_source, msg_pid, 0);
			send(msg, "gate$o", getIndex()); //send a message to A
			bubble("Send Error");
		}
	}

	if (msg_type == 2){
		bool is_balance_equal = false;

		int money_transfer = ttmsg->getAmount();
		char check_sender = ttmsg->getSource();
		char check_receiver = ttmsg->getReceiver();
		char source = ttmsg->getDb_source();
		int msg_pid = ttmsg->getPid();

		printf("\nCheck if the operation with PID %d is %d euro", msg_pid, money_transfer);

		int query_transaction = transaction_amount(msg_pid);
		

		printf("\nSource: %d", source );
		int index = 0;
		if(isvalueinarray(msg_pid, transaction_array, 200)){
			index = whereisvalue(msg_pid, transaction_array, 200);
			printf("index e': %d\n", index );
			transaction_array[index][source] = money_transfer;

			printf("\nIf");
		}
		else{
			printf("\nElse %d", msg_pid);
			transaction_array[transaction_counter][0] = msg_pid;
			transaction_array[transaction_counter][source] = money_transfer;

			printf("\nInserisco%d", transaction_array[transaction_counter][0]);
			transaction_counter++;
		}

		printf("\nIf %d --- %d ", transaction_array[index][0], transaction_array[index][source]);

		printf("\nThe transaction is %d euro", query_transaction);
		/*
		if (balance_sender == balance_amount(check_sender)){
			if(balance_receiver == balance_amount(check_receiver)){
				is_balance_equal = true;
				printf("it's ok, balances are equals\n");
			}
			else{
				printf("receiver balance is different\n");
			}
		}
		else{
			printf("sender balance is different\n");
		}


		//send message to 0
		TicTocMsg13 *flag_msg= generateMessage(is_balance_equal, msg_pid);
		send(flag_msg, "gate$o", 0);
	*/
	}

	if (msg_type == 3){
		bool actual_flag = ttmsg->getFlag();
		if (!actual_flag){
			final_flag = false;
		}
	}

	if(msg_type == 4){
		if (final_flag){
			printf("All the balance are ok\n");
			//send ok message to B
		}
		else{
			printf("At least one balance is not ok\n");
			// delete operation with pid from all the db
		}
	}
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

  	printf("\nTransaction inserted!\n");
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
	size_t len = strlen(first_part);

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

ErrorMessage *DB::generateErrMessage(int src, char dest, int err_pid, int msg_type){
	ErrorMessage *msg = new ErrorMessage("user_msg");
	msg->setMsg_type(msg_type);
	msg->setSource(src);
	msg->setDestination(dest);
	msg->setPid(err_pid);

	return msg;
}

TicTocMsg13 *DB::generateMessage(bool flag, int pid){
	TicTocMsg13 *msg = new TicTocMsg13("flag_msg");
	msg->setMsg_type(3);
	msg->setDb_source(getIndex());
	msg->setDestination(0);
	msg->setFlag(flag);
	msg->setPid(pid);

	return msg;

}

TicTocMsg13 *DB::generateMessage(int sender, int my_type, char src, char rec, int pid, int amount){
	TicTocMsg13 *msg = new TicTocMsg13("_msg");
	msg->setAmount(amount);
	msg->setMsg_type(my_type);
	msg->setDb_source(sender);
	msg->setSource(src);
	msg->setReceiver(rec);
	msg->setPid(pid);
	return msg;
}



TicTocMsg13 *DB::generateMessage(int src, int dest, int sndr_amnt, int rec_amnt, int operation_pid, char source, char receiver){
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

bool DB::isvalueinarray(int val, int arr[][4], int size){
    int i;
    for (i=0; i < size; i++) {
        if (arr[i][0] == val)
            return true;
    }
    return false;
}

int DB::whereisvalue(int val, int arr[][4], int size){
    int i;
    for (i=0; i < size; i++) {
        if (arr[i][0] == val)
            return i;
    }
    return 777;
}

bool DB::arevaluesequal(int my_pid, int arr[200][4], int size){
	int i;
	for(i=0; i<size-1; i++){
		if (arr[my_pid][i] != arr[my_pid][i+1]){
			return false;
		}
	}
	return true;
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

	return i;

}

void Person::update_pid(int new_pid){
	FILE *file = fopen("pid.txt", "w");
	if (file == NULL){
    printf("Error opening file!\n");
    exit(1);
	}

	fprintf(file, "%d", new_pid);

}

void Person::initialize()
{
	node_name = par("name");
	printf("Initialize node %c\n", node_name);
	
	if (par("sendMsgOnInit").boolValue() == true) {
		printf("Program starts\n");
	}


	start = new cMessage("start_msg");
	scheduleAt(0.0, start);

}

void Person::forwardMessage(TicTocMsg13 *msg)
{
	// In this example, we just pick a random gate to send it on.
	// We draw a random number between 0 and the size of gate `gate[]'.
	//EV << "I'm number " << getIndex() << "\n";

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

void Person::handleMessage(cMessage *msg)
{

	 if (msg == start) {
       int global_pid = read_pid();
		if (par("sendMsgOnInit").boolValue() == true) {
			// Boot the process scheduling the initial message as a self-message.
		
			int n = gateSize("gate");
			int amount = 10;
			char receiver = 'B';

			//Broadcast message = 99
			TicTocMsg13 *msg = generateMessage('A', 99, amount, receiver, global_pid);

			printf("Send the broadcast message to the databases\n");
			for (int k = 0; k<n; k = k+1){
			// $o and $i suffix is used to identify the input/output part of a two way gate
			
				TicTocMsg13 *copy = msg->dup();
				send(copy, "gate$o", k);
			
				bubble("Send messages");
			}
		}
    }
	else{

		ErrorMessage *ttmsg = check_and_cast<ErrorMessage *>(msg);
		int db_msg_type = ttmsg->getMsg_type();
		int pid_operation = ttmsg->getPid();
		
		if(db_msg_type == 0){
			printf("Msg from %d come back with destination %c\n", ttmsg->getSource(), ttmsg->getDestination());
			printf("You can't performe operation with pid %d\n", pid_operation);
		}
		
		if(db_msg_type == 1){
			printf("\nOperation number %d is done!", pid_operation);
		}

		if(db_msg_type == 2){
			printf("\nWell, this is embarrassing! Sorry operation %d has gone wrong.", pid_operation);
		}
		//forwardMessage(ttmsg);
	}

}

TicTocMsg13 *Person::generateMessage(char src, int dest, int my_balance, char my_rec, int my_pid)
{
	TicTocMsg13 *msg = new TicTocMsg13("msg_gen");
	msg->setMsg_type(1);
	msg->setSource(src);
	msg->setDestination(dest);
	msg->setAmount(my_balance);
	msg->setReceiver(my_rec);
	my_pid++;
	msg->setPid(my_pid);

	update_pid(my_pid);
	return msg;
}


