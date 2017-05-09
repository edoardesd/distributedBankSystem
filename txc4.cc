//HOW TO RUN:
//opp_makemake -f -o output -I/mysql/include -L/mysql/lib/o -lmysqlclient
//
// make depend o make
//
// ./output
//
//
// This file is part of an OMNeT++/OMNEST simulation example.
//
// Copyright (C) 2003-2015 Andras Varga
//
// This file is distributed WITHOUT ANY WARRANTY. See the file
// `license' for details on this and other legal matters.
//

#include <mysql/mysql.h>
#include <stdio.h>
#include <string.h>
#include <omnetpp.h>
#include <stdlib.h>



using namespace omnetpp;
static char *unix_socket = NULL;


/**
 * In this step you'll learn how to add input parameters to the simulation:
 * we'll turn the "magic number" 10 into a parameter.
 */
class Txc4 : public cSimpleModule
{
  private:
	int counter;

  protected:
	virtual void initialize() override;
	virtual void handleMessage(cMessage *msg) override;
	virtual void createConnection();
	virtual void finish_with_error(MYSQL *con);
	virtual int read_balance(char* my_query);


	unsigned int port = 3306;
	
	unsigned int flag = 0;

	//MYSQL *conn;
	MYSQL_RES *res;
	MYSQL_RES *query;
	MYSQL_ROW row;
	MYSQL * conn = mysql_init(NULL);
	int num_fields;

	int my_balance;

};

Define_Module(Txc4);

void Txc4::createConnection(){

	if (conn == NULL){
	  fprintf(stderr, "mysql_init() failed\n");
	  exit(1);
	}  


	if (!mysql_real_connect(conn, "localhost", "root", "distributedpass", "bank", port, unix_socket, flag)){
			EV << "Connection error\n";
		   finish_with_error(conn);
		}
		else{
			printf("Connection with the database established\n" );
		}

}

void Txc4::finish_with_error(MYSQL *con){
  fprintf(stderr, "%s\n", mysql_error(con));
  mysql_close(con);
  exit(1);        
}

int Txc4::read_balance(char* my_query){
	int query_balance;

	if (!mysql_real_connect(conn, "localhost", "root", "distributedpass", "bank", port, unix_socket, flag)){
			EV << "Connection error\n";
		   finish_with_error(conn);
	}else{
			printf("Connection with the database established\n" );
		}

	if (mysql_query(conn, my_query)){
		fprintf(stderr, "1\n\n");
				finish_with_error(conn);
		}

		query = mysql_store_result(conn);
  
		if (query == NULL) {
			fprintf(stderr, "2\n\n");
	 		finish_with_error(conn);
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
  		mysql_close(conn);
  		fprintf(stderr, "Closing connection\n\n");

  		fprintf(stderr, "%d\n", query_balance);
  		return query_balance;
}


void Txc4::initialize()
{
	// Initialize the counter with the "limit" module parameter, declared
	// in the NED file (tictoc4.ned).
	counter = par("limit");

	// we no longer depend on the name of the module to decide
	// whether to send an initial message
	if (par("sendMsgOnInit").boolValue() == true) {
		EV << "Sending initial messagello\n";
		cMessage *msg = new cMessage("tictocMsg");
		send(msg, "out");
	}
}


void Txc4::handleMessage(cMessage *msg){
	//int temp;
	//fprintf(stderr, "asd\n");
	
	//counter--;
	//WATCH(counter);
	//WATCH(my_balance);
	//WATCH(temp);

	
	char *query_string = (char*)"SELECT balance FROM people WHERE name=\"edo\"";

	read_balance(query_string);
	//fprintf(stderr, "my balance: %d\n", my_balance);


	EV << getName() << "'s counter is " << counter << ", sending back message\n";
		
		//temp = my_balance + (counter+3);
		//my_balance = temp;

		//fprintf(stderr, "Counter: %d\n", counter);
		//fprintf(stderr, "Temp: %d\n", temp);
		//fprintf(stderr, "balance: %d\n", my_balance);

		
		//fprintf(stderr, "updated balance: %d\n", my_balance);
		send(msg, "out");

		
		//strcpy(query_string, "UPDATE people SET balance = \"");
		//char str[10];
		//sprintf(str, "%d", my_balance);
		//fprintf(stderr, "Lol: %s\n", str);
		//strcat(query_string, str);
		//char str2[50];
		//strcpy(str2, " \" WHERE name = \"edo\"");
		//strcat(query_string, str2);

		//fprintf(stderr, "esd%s\n", query_string );

		//if (mysql_query(conn, query_string)){    
      	//	finish_with_error(conn);
  		//}


	


}

