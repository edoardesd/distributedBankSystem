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
	virtual int read_balance(char* my_query, MYSQL *connection);
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



	if (!mysql_real_connect(con, "localhost", "root", "distributedpass", db_name, port, unix_socket, flag)){
			finish_with_error(con);
		}
		else{
			printf("Connection with the database established\n" );

			return con;
		}

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

void DB::handleMessage(cMessage *msg)
{
	TicTocMsg13 *ttmsg = check_and_cast<TicTocMsg13 *>(msg);

	printf("\nI received a message");

	printf("\nSource %c, Destination: %d, Receiver: %c, PID: %d, Amount %ld", 
		ttmsg->getSource(), ttmsg->getDestination(), ttmsg->getReceiver(), ttmsg->getPid(), ttmsg->getAmount());

	delete ttmsg;

	MYSQL *con;
	//add msg type
	switch (getIndex()){
		case 0: 
			con = createConnection((char*)"bank0");
			break;

		case 1:
			con =createConnection((char*)"bank1");
			break;

		case 2:
			con =createConnection((char*)"bank2");
			break;

		case 3:
			con =createConnection((char*)"bank3");
			break;

		default: 
			printf("\n50m3th1ng g035 wr0ng!\n");
	}
	
	char *query_string = (char*)"SELECT balance FROM people WHERE name=\"A\"";

	int actual_balance = read_balance(query_string, con);

}

int DB::read_balance(char* my_query, MYSQL *connection){
	int query_balance;

	/*
	if (!mysql_real_connect(conn, "localhost", "root", "distributedpass", "bank", port, unix_socket, flag)){
			EV << "Connection error\n";
		   finish_with_error(conn);
	}else{
			printf("Connection with the database established\n" );
		}
	*/

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
          		//printf("%d ", query_balance); 

      		} 
          	printf("\n"); 
  		}

  		mysql_free_result(query);
  		mysql_close(connection);
  		fprintf(stderr, "Closing connection\n\n");

  		fprintf(stderr, "%d\n", query_balance);
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
		long amount = 200;
		char receiver = 'B';

		TicTocMsg13 *msg = generateMessage('A', 99, amount, receiver);
		for (int k = 0; k<n; k = k+1){
			// $o and $i suffix is used to identify the input/output part of a two way gate
			
			TicTocMsg13 *copy = msg->dup();
			send(copy, "gate$o", k);
			
			printf("Send message on gate[%d]\n", k);
			bubble("send message");
			printf("Process ID: %d\n", msg->getPid() );

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
	msg->setSource(src);
	msg->setDestination(dest);
	msg->setAmount(my_balance);
	msg->setReceiver(my_rec);
	global_pid++;
	msg->setPid(global_pid);
	return msg;
}


