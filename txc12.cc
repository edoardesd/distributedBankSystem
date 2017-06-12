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
#include <omnetpp.h>
#include <mysql/mysql.h>
#include "tictoc13_m.h"


using namespace omnetpp;
static char *unix_socket = NULL;




int global_pid = 0;

////////////////////// DATABASE METHODS //////////////////////

class DB : public cSimpleModule
{
  protected:
	virtual void forwardMessage(TicTocMsg13 *msg);
	virtual void initialize() override;
	virtual void handleMessage(cMessage *msg);
	virtual TicTocMsg13 *generateMessage(int src, int dest, long my_amount, char my_rec);
	
	virtual MYSQL *createConnection(char *db_name);
	virtual MYSQL *handleConnection(int my_db);
	virtual int read_balance(char* my_query, MYSQL *connection);
	virtual bool check_balance(int my_amount, char source);
	virtual char *create_query(char *first_part, char name_part);
	virtual char *create_update(char *first_part, char *second_part, int amount, char name_part);
	virtual void update_balance(int transaction_amount, char source, char destination);
	virtual void finish_with_error(MYSQL *con);

	unsigned int port = 3306;
	unsigned int flag = 0;
	int num_fields;


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
			printf("\nConnection with the database established\n" );

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
			printf("\n50m3th1ng g035 wr0ng!\n");
			con = NULL;
	}
	return con;

}

void DB::handleMessage(cMessage *msg)
{
	TicTocMsg13 *ttmsg = check_and_cast<TicTocMsg13 *>(msg);

	printf("\n%d received a message", getIndex());
	int msg_type = ttmsg->getMsg_type();
	long amount = ttmsg->getAmount();
	char msg_source = ttmsg->getSource();
	char msg_receiver = ttmsg->getReceiver(); //e.g. B

	if (msg_type == 1){
		printf("\nMsg_type: %d, Source %c, Destination: %d, Receiver: %c, PID: %d, Amount %ld\n", 
			msg_type, msg_source, ttmsg->getDestination(), msg_receiver, ttmsg->getPid(), amount);

		delete ttmsg;

		if(check_balance(amount, msg_source)){
			printf("Balance ok, let's do something\n");
			//UPDATE BALANCE
			update_balance(amount, msg_source, msg_receiver);
			//UPDATE table SET field = field + 1 WHERE id = $number
		}
		else{
			printf("Your balance is equal or less than 0, ERROR!\n");
			//DO SOMETHING
		}

		
	}

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

	char *query_part = (char*)"SELECT balance FROM people WHERE name=\"";
		
	char *balance_query = create_query(query_part, source);

	int balance = read_balance(balance_query, con);

	if (balance - my_amount > 0){
		return true;
		
	}
	else {
		return false;
	}
}

char * DB::create_query(char *first_part, char name_part){
	char *close_query = (char*)"\"";
		
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
          	printf("\n"); 
  		}

  		mysql_free_result(query);
  		mysql_close(connection);
  		fprintf(stderr, "Closing connection\n\n");

  		//fprintf(stderr, "%d\n", query_balance);
  		return query_balance;
}

TicTocMsg13 *DB::generateMessage(int src, int dest, long my_balance, char my_rec)
{
	TicTocMsg13 *msg = new TicTocMsg13("msg_gen");
	msg->setSource(src);
	msg->setDestination(dest);
	msg->setAmount(my_balance);
	msg->setReceiver(my_rec);
	global_pid++;
	msg->setPid(global_pid);
	return msg;
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
	virtual TicTocMsg13 *generateMessage(char src, int dest, long my_amount, char my_rec);

};

Define_Module(Person);



void Person::initialize()
{
	
	if (par("sendMsgOnInit").boolValue() == true) {
		// Boot the process scheduling the initial message as a self-message.
		
		int n = gateSize("gate");
		long amount = 5;
		char receiver = 'B';

		//Broadcast message = 99
		TicTocMsg13 *msg = generateMessage('A', 99, amount, receiver);
		for (int k = 0; k<n; k = k+1){
			// $o and $i suffix is used to identify the input/output part of a two way gate
			
			TicTocMsg13 *copy = msg->dup();
			send(copy, "gate$o", k);
			
			bubble("Send message");
		}
	}
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
	TicTocMsg13 *ttmsg = check_and_cast<TicTocMsg13 *>(msg);

	printf("Msg from %d come back with destination %d\n", ttmsg->getSource(), ttmsg->getDestination());
	//forwardMessage(ttmsg);

}

TicTocMsg13 *Person::generateMessage(char src, int dest, long my_balance, char my_rec)
{
	TicTocMsg13 *msg = new TicTocMsg13("msg_gen");
	msg->setMsg_type(1);
	msg->setSource(src);
	msg->setDestination(dest);
	msg->setAmount(my_balance);
	msg->setReceiver(my_rec);
	global_pid++;
	msg->setPid(global_pid);
	return msg;
}


