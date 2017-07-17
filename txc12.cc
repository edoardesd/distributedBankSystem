//
//how to compile:
//opp_makemake -f -o output -I/mysql/include -L/mysql/lib/o -lmysqlclient
//
//how to run rapidly:
//make && ./output
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
#include "dbmessage_m.h"

#define DB_COORDINATOR 0
#define BROADCAST 255
#define DB_SIZE 4

//msg to db
#define SELF_MSG_TRANSACTION 0
#define TRANSACTION_MSG 1
#define CHECK_MSG 2
#define PID_REQ 3
#define PID_SPREAD 4
#define ROLLBACK_MSG 5
#define ACK_MSG 6
#define SELF_ACK_MSG 7
#define SELF_PID_MSG 8
#define OK_DB 9
#define START_MSG 99


//msg to user
#define ERROR 0
#define OPERATION_DONE 1
#define SOMETHING_WRONG 2
#define PID_REQ 3
#define PID_WRONG 4
#define DELETE_PID 5
#define MONEY 9


#define LOOSE_PROBABILITY 0.1


using namespace omnetpp;
static char *unix_socket = NULL;

int transaction_counter = 0;
int transaction_array[200][5]; 
int ack_array[200][5];
int pid_array[200][5];
int msg_queue[200][5];
int ack_counter;
int global_pid = 0;
int ack_number = 0;
int pid_counter = 0;
int pid_head = 0;
int msg_head = 0;
int msg_counter = 0;







////////////////////// DATABASE METHODS //////////////////////

class DB : public cSimpleModule
{
  protected:
	virtual void initialize() override;
	virtual void handleMessage(cMessage *msg);
	virtual UserMessage *generateUserMessage(int src, char dest, int err_pid, int msg_type);
	virtual DBMessage *generateMessage(int sender, int my_type, char src, char rec, int pid, int amount);


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

	virtual void performTransaction(int queue[200][5], int head);

	virtual bool arevaluesequal(int val, int arr[][5],int size);
	virtual bool isvalueinarray(int val, int arr[][5], int size);
	virtual int whereisvalue(int val, int arr[200][5], int size);
	virtual int howmanytransactions(int my_pid, int arr[200][5], int size);
	virtual bool lessthanzero(int my_pid, int arr[200][5], int size);
	virtual void printqueue(int val, int arr[200][5], int size);
	virtual void printmsgqueue(int arr[200][5], int size);
	virtual void putMsgInQueue(DBMessage *msg);



	virtual void rollback(int pid);
	virtual char read_sender(int pid);
	virtual char read_receiver(int pid);
	virtual void delete_transaction(int pid);

	virtual bool loosemessage(cMessage *msg, int type, int pid);
	virtual int getGate(char src);

	virtual int read_pid();
	virtual void update_pid(int new_pid);
	virtual void store_pid(int pid, char source);
	virtual bool check_pid(int pid, char source);



  private:
	unsigned int port = 3306;
	unsigned int flag = 0;
	int num_fields;
	int msg_type;
	int GATE_SIZE;
	bool final_flag = true;
	bool working = false;

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
	GATE_SIZE = gateSize("gate");
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

void DB::putMsgInQueue(DBMessage *msg){
	msg_queue[msg_counter][0] = msg->getPid();
	msg_queue[msg_counter][1] = msg->getSource();
	msg_queue[msg_counter][2] = msg->getReceiver();
	msg_queue[msg_counter][3] = msg->getAmount();

	msg_counter++;
	printf("\nmsg counter: %d (already updated)\n", msg_counter);
}

void DB::performTransaction(int queue[200][5], int head){
	working = true;
	int pid = queue[head][0];
	int amount = queue[head][3];
	char msg_source = queue[head][1];
	char msg_receiver = queue[head][2]; //e.g. B

			printf("my p: %d \n", pid);

			
			printf("\nMsg_type: %d, Source %c, Receiver: %c, PID: %d, Amount %d\n", 
				msg_type, msg_source, msg_receiver, pid, amount);
	


			if(check_pid(pid, msg_source)){

				if(check_balance(amount, msg_source)){
					printf("\nBalance is greater than zero.");
					update_balance(amount, msg_source, msg_receiver);
					
					int index;
					insert_transaction(pid, msg_source, msg_receiver, amount);
					
					if (getIndex() == DB_COORDINATOR){
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


							DBMessage *self_msg= generateMessage(getIndex(), SELF_MSG_TRANSACTION, msg_source, msg_receiver, pid, amount);
		
							scheduleAt(simTime()+5.0, self_msg);
						}
					}
					else{

						printf("my p: %d \n", pid );

						//other DBs send a message to the coordinator
						DBMessage *transaction_msg = generateMessage(getIndex(), CHECK_MSG ,msg_source, msg_receiver, pid, amount);
						send(transaction_msg, "gate$o", DB_COORDINATOR);
						printf("\nDB n.%d sends a message to DB n.%d (coordinator)", getIndex(), DB_COORDINATOR);
						bubble("Send MSG");
		
					}
				}
				else{
					printf("Your balance is equal or less than 0, ERROR!\n");
					UserMessage *msg = generateUserMessage(getIndex(), msg_source, pid, ERROR);

					send(msg, "gate$o", getGate(msg_source)); //send a message to A
					bubble("Send Error");

					working = false;
					msg_head++;
				}
			}
			else{
				//wrong pid (not the first one) 
				//put in queue

				//pid doesn't exists or it's timer is expired
				printf("\nThe pid %d doesn't exists", pid);
				//send msg to user
				UserMessage *wrong_pid = generateUserMessage(getIndex(), msg_source, pid, PID_WRONG);
				send(wrong_pid, "gate$o", getGate(msg_source));

			}
		
}

void DB::handleMessage(cMessage *msg){
	char msg_source;
	char msg_receiver;
	int pid;
	int amount;
	int db_source;

	// Create the event object we'll use for timing -- just any ordinary message.
	event = new cMessage("timeout");

	DBMessage *ttmsg = check_and_cast<DBMessage *>(msg);
	msg_type = ttmsg->getMsg_type();
	db_source = ttmsg->getDb_source();

	
	printf("\n\n----------------------------------------------------------");
	printf("\nDB n.%d receives a message from %d", getIndex(), db_source);
	printf(" of type %d", msg_type);

	if (msg_type == SELF_MSG_TRANSACTION) {
		pid = ttmsg->getPid();
		amount = ttmsg->getAmount();
		msg_source = ttmsg->getSource();
		msg_receiver = ttmsg->getReceiver();


		int pid_index = whereisvalue(pid, transaction_array, 200);
		bool transaction_equal = false;
		int number_of_transactions;
		
		printf("\nEnd Timer!\n");

		printf("\nChecking dbs transaction money...");
	

		transaction_equal = arevaluesequal(pid_index, transaction_array, 5);
		number_of_transactions = howmanytransactions(pid_index, transaction_array, DB_SIZE); //perchè il primo non si deve contare
		printf("\nThere are %d transactions.", number_of_transactions);
		printf("\nAll transactions have the same money? ");
		printf("%s", transaction_equal ? "true" : "false");



		//if (lessthanzero(pid_index; transaction_array; 5)){}
		//ricevo un numero minore di zero se almeno un db ha un valore negativo, rollback e send less than zero error to A


		if (transaction_equal){
			//send msg to A 
			pid_head++;
			UserMessage *confirm_msg = generateUserMessage(getIndex(), msg_source, pid, OPERATION_DONE);
			printf("\nMando messaggio a: %c sul gate %d",msg_source, getGate(msg_source) );
			send(confirm_msg, "gate$o", getGate(msg_source));			
			

			UserMessage *notification_msg = generateUserMessage(getIndex(), msg_receiver, pid, MONEY);
			printf("\nMando messaggio a: %c sul gate %d",msg_receiver, getGate(msg_receiver) );

			send(notification_msg, "gate$o", getGate(msg_receiver));			

			DBMessage *ok_msg= generateMessage(getIndex(), OK_DB, msg_source, msg_receiver, pid, amount);
			for (int k = 1; k<DB_SIZE; k++){

				DBMessage *copy = ok_msg->dup();
				send(copy, "gate$o", k);
				bubble("Send ok msg");
			}

			working = false;
			msg_head++;
		}
		else{
			DBMessage *rollback_msg= generateMessage(getIndex(), ROLLBACK_MSG, msg_source, msg_receiver, pid, amount);
			printf("\nSend the broadcast rollback message to all the databases");

			for (int k = 1; k<DB_SIZE; k++){

				DBMessage *copy = rollback_msg->dup();
				send(copy, "gate$o", k);
				bubble("Send rollback msg");
			}
			DBMessage *copy = rollback_msg->dup();
			scheduleAt(simTime(), copy);
			
			if(isvalueinarray(pid, ack_array, 200)){
				pid_index = whereisvalue(pid, ack_array, 200);
			}
			else{
				ack_counter++;
				pid_index = ack_counter;
			}
			ack_array[pid_index][0] = pid;
			ack_array[pid_index][1] = 1;

			DBMessage *ack_timer_msg= generateMessage(getIndex(), SELF_ACK_MSG, msg_source, msg_receiver, pid, amount);
			scheduleAt(simTime()+10.0, ack_timer_msg);

		}
	}

	if (msg_type == TRANSACTION_MSG){
		pid = ttmsg->getPid();
		printqueue(pid, pid_array, pid_head+5);
		if(!loosemessage(ttmsg, msg_type, pid)){
			//performTransaction(ttmsg);
			if(getIndex() == 0){
				putMsgInQueue(ttmsg);
			}
			DBMessage *start_msg= generateMessage(getIndex(), START_MSG, msg_source, msg_receiver, pid, amount);
			scheduleAt(simTime()+1.0, start_msg);
		}
		
	}

	if(msg_type == START_MSG){
		printf("\nDB n.%d is working? ",getIndex() );
		printf("%s", working ? "true" : "false");


		if(!working){
			printmsgqueue(msg_queue, msg_head +2);
			printf("\nPid head: %d", pid_head);
			printf("\nMessage head prima: %d", msg_head);
			if(isvalueinarray(pid_array[pid_head][0], msg_queue, 200)){
				msg_head = whereisvalue(pid_array[pid_head][0], msg_queue, 200);
				printf("\nMsg head dopo: %d", msg_head);
				performTransaction(msg_queue, msg_head);
			}
			printf("\nLa mia operazione non è ancora arrivata");
		}
		else {
			printf("\nI'm working!");
			DBMessage *start_msg= generateMessage(getIndex(), START_MSG, msg_source, msg_receiver, pid, amount);
			scheduleAt(simTime()+1.0, start_msg);
		}
	}

	if (msg_type == CHECK_MSG){
		pid = ttmsg->getPid();

		printf("\nPid: %d", pid);
		if(!loosemessage(ttmsg, msg_type, pid)){

			int money_transfer = ttmsg->getAmount();
			char check_sender = ttmsg->getSource();
			char check_receiver = ttmsg->getReceiver();
			int source = ttmsg->getDb_source();
			//pid = ttmsg->getPid();

			int index = 0;

			if(isvalueinarray(pid, transaction_array, 200)){
				index = whereisvalue(pid, transaction_array, 200);
				transaction_array[index][source+1] = money_transfer;

			}	
			else{
				//db0 doesn't receive the first message from A
				printf("\nDB.%d doesn't receive the first message from A. Start the timer now", getIndex());
				DBMessage *self_msg= generateMessage(getIndex(), DB_COORDINATOR, check_sender, check_receiver, pid, money_transfer);
				scheduleAt(simTime()+5.0, self_msg);

				transaction_array[transaction_counter][0] = pid;
				transaction_array[transaction_counter][source+1] = money_transfer;

				//printf("\nInserisco %d nell'array", transaction_array[transaction_counter][0]);
				transaction_counter++;
				index = whereisvalue(pid, transaction_array, 200);

			}	

			printf("\nPid number: %d --- money:  %d ", transaction_array[index][0], transaction_array[index][source+1]);
		}
	}


	if(msg_type == ROLLBACK_MSG){
		pid = ttmsg->getPid();
		if(!loosemessage(ttmsg, msg_type, pid)){
			printf("\nOh, f*ck! An error... I must rollback");
		
			rollback(pid);
			DBMessage *ack_msg= generateMessage(getIndex(), ACK_MSG, ttmsg->getSource(), ttmsg->getReceiver(), pid, ttmsg->getAmount());
			if (getIndex() != DB_COORDINATOR){
				send(ack_msg, "gate$o", 0);
				printf("\nSend rollback ACK to the coordinator");
			}
			working = false;
		}
	}
	
	if (msg_type == ACK_MSG){
		pid = ttmsg->getPid();
		int ack_sender = ttmsg->getDb_source();
		printf("\nReceived an ACK from DB n.%d", ack_sender);
		int pid_index;
		if(isvalueinarray(pid, ack_array, 200)){
			pid_index = whereisvalue(pid, ack_array, 200);
		}
		else{
			ack_counter++;
			pid_index = ack_counter;
		}
		ack_array[pid_index][0] = pid;
		ack_array[pid_index][ack_sender+1] = 1;
		//printf("\nACK? %d --- Pid: %d\n", ack_array[pid_index][ack_sender+1], ack_array[pid_index][0]);
		
	}

	if (msg_type == SELF_ACK_MSG){
		//ack timer
		printf("\nChecking the ACK");
		pid = ttmsg->getPid();
		amount = ttmsg->getAmount();
		msg_source = ttmsg->getSource();
		msg_receiver = ttmsg->getReceiver();
		int pid_index = 0;


		if(isvalueinarray(pid, ack_array, 200)){
			pid_index = whereisvalue(pid, ack_array, 200);
		}

		//send the rollback another time
		int num_of_ack=3;
		
		for (int k = 0; k<5; k++){
			if (ack_array[pid_index][k] == 0){
				num_of_ack--;
				//resend the rollback
				DBMessage *rollback_msg= generateMessage(getIndex(), ROLLBACK_MSG, msg_source, msg_receiver, pid, amount);
				send(rollback_msg, "gate$o", k-1);
				printf("\nSend the new rollback to DB n.%d", k-1 );

			}
		}
		
		//num_of_ack = howmanytransactions(pid, ack_array, 4);
		
		//num of db -1 (gate -2)
		if(num_of_ack==DB_SIZE-1){
			printf("\nI have all the ACKs");
			pid_head++;
			printf("\nSend the error message to the user");
			UserMessage *confirm_msg = generateUserMessage(getIndex(), msg_source, pid, SOMETHING_WRONG);
			send(confirm_msg, "gate$o", getGate(msg_source));	
			working = false;	
			msg_head++;	
		}
		else{
			printf("\nThere's a problem. I've received only %d ACKs.\n Start a new timer...", num_of_ack);
			//start ack timer
			DBMessage *ack_timer_msg= generateMessage(getIndex(), SELF_ACK_MSG, msg_source, msg_receiver, pid, amount);
			scheduleAt(simTime()+10.0, ack_timer_msg);
		}
	}

	if(msg_type == PID_REQ){
		msg_source = ttmsg->getSource();
		pid = read_pid();
		update_pid(pid);

		printf("\nI have received a PID request from %c", msg_source);
		UserMessage *pid_request = generateUserMessage(getIndex(), msg_source, pid, PID_REQ);
		send(pid_request, "gate$o", getGate(msg_source));

		//save pid in the array
		store_pid(pid, msg_source);

		DBMessage *pid_msg= generateMessage(getIndex(), PID_SPREAD, msg_source, BROADCAST, pid, 0);
		printf("\nSend the pid in broadcast to all the databases");

		DBMessage *pid_timer_msg= generateMessage(getIndex(), SELF_PID_MSG, msg_source, 'NULL', pid, 0);
		scheduleAt(simTime()+50.0, pid_timer_msg);

		for (int k = 1; k<DB_SIZE; k++){

			DBMessage *copy = pid_msg->dup();
			send(copy, "gate$o", k);
			bubble("Send pid to DBs");
		}
	}

	if(msg_type == PID_SPREAD){
		msg_source = ttmsg->getSource();
		pid = ttmsg->getPid();

		printf("\nI have received a PID update from %c", msg_source);
		printf("\nSave pid %d and source %d in pos %d", pid_array[pid_counter-1][0], pid_array[pid_counter-1][1], pid_counter-1);

	}	

	if(msg_type == SELF_PID_MSG){
		//delete il pid che sto aspettando
		pid = ttmsg->getPid();
		msg_source = ttmsg->getSource();

		printqueue(pid, pid_array, pid_head+5);

		printf("\nExpired pid %d's timer ", pid);

		printf("\npid head: %d, pid: %d source %c\n",pid_head, pid_array[pid_head][0], msg_source );
		if (pid_array[pid_head][0] == pid){
			printf("but I don't received that operation yet");
			//pid_index = whereisvalue(pid)
			pid_head++;
			printf("\nNew head of list: %d", pid_head);
			printf("\nRemove pid n. %d", pid);
			//TODO: send a message to source and say "i've deleted ur pid")
			UserMessage *delete_pid = generateUserMessage(getIndex(), msg_source, pid, DELETE_PID);
			send(delete_pid, "gate$o", getGate(msg_source));


		}
		else{
			printf("and operation is already done :)");
		}
	}

	if(msg_type == OK_DB){
		working = false;
	}
}

bool DB::check_pid(int pid, char source){
	if(pid_array[pid_head][0] == pid){
		if (pid_array[pid_head][1] == (int)source)
			return true;
	}
	else{
		printf("\nSomething isn't correct");
		return false;
	}
}

void DB::store_pid(int pid, char source){
	pid_array[pid_counter][0] = pid;
	pid_array[pid_counter][1] = (int)source;
	pid_counter++;

	printf("\nSave pid %d and source %d in pos %d", pid_array[pid_counter-1][0], pid_array[pid_counter-1][1], pid_counter-1);
}

int DB::read_pid(){

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

void DB::update_pid(int n){
	n++;
	FILE *file = fopen("pid.txt", "w");
	if (file == NULL){
		printf("Error opening file!\n");
		exit(1);
	}

	fprintf(file, "%d", n);
	fclose(file);
}

bool DB::loosemessage(cMessage *msg, int type, int pid){
	if (uniform(0, 1) < LOOSE_PROBABILITY) {
        EV << "\"Losing\" message\n";
        printf("\nDB n.%d \"Lose\" message of type %d with pid %d \n", getIndex(), type, pid);
        bubble("message lost");  // making animation more informative...
        delete msg;
        return true;
    }
    return false;
}

int DB::getGate(char src){
	switch(src){
		case 65:
			return getIndex();

		case 66:
			return GATE_SIZE-3;

		case 67:
			return GATE_SIZE-2;

		case 68:
			return GATE_SIZE-1;

		//case 69:
			//return GATE_SIZE-2;

		//case 70:
			//return GATE_SIZE-1;

		default :
			return 0;
	}
}

void DB::rollback(int pid){
	int amount = transaction_amount(pid);
	char sender = read_sender(pid);
	char receiver = read_receiver(pid);	
	//printf("pid %d, sender %c, receiver %c\n",pid, sender, receiver );

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

UserMessage *DB::generateUserMessage(int src, char dest, int err_pid, int msg_type){
	UserMessage *msg = new UserMessage("user_msg");
	msg->setMsg_type(msg_type);
	msg->setSource(src);
	msg->setDestination(dest);
	msg->setPid(err_pid);

	return msg;
}


DBMessage *DB::generateMessage(int sender, int my_type, char src, char rec, int pid, int amount){
	DBMessage *msg = new DBMessage("msg");
	msg->setAmount(amount);
	msg->setMsg_type(my_type);
	msg->setDb_source(sender);
	msg->setSource(src);
	msg->setReceiver(rec);
	msg->setPid(pid);
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
			printf("\nTroavato");
			return i;
	}
	printf("\nQuas");
	return 777;
}


void DB::printqueue(int val, int arr[200][5], int size){
	int i;
	for (i=0; i < size; i++) {
		
		printf("\n%d -- %d", arr[i][0], arr[i][1]);
		if(i == pid_head){
			printf("  <--- ");

		}		
	}
}

//int DB::whereistransaction(int val, int arr[200][5], int size){
//	for (int i = 0; i<size; i++){
//		if (arr[i][0] = val)
//			printf("\nEccolo" );
//			return i;
//	}
//}

void DB::printmsgqueue(int arr[200][5], int size){
	int i;
	for (i=0; i < size; i++) {
		
		printf("\n%d -- %d -- %d --- %d", arr[i][0], arr[i][1], arr[i][2], arr[i][3]);
		if(i == msg_head){
			printf("  <--- ");

		}		
	}
}

bool DB::arevaluesequal(int my_pid, int arr[200][5], int size){
	int i;

	for(i=1; i<size-1; i++){
		//printf("\nEQ: Pos i: %d ---- pos i+1: %d", arr[my_pid][i], arr[my_pid][i+1]);
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
		//printf("\nHOW: Pos i: %d size: %d", arr[my_pid][i], size);
		if (arr[my_pid][i] == 0){
			size--;
		}
	}
	return size;
}

bool DB::lessthanzero(int my_pid, int arr[200][5], int size){
	for(int i=1; i<size; i++){
		if (arr[my_pid][i]<0){
			return false;
		}
	}
	return true;
}


////////////////////// PEOPLE METHODS //////////////////////

class Person : public cSimpleModule
{

  protected:
	virtual void initialize() override;
	virtual void handleMessage(cMessage *msg);
	virtual DBMessage *generateMessage(char src, int dest, int my_amount, char my_rec, int pid);
	virtual DBMessage *askPid(char src, int dest);
	virtual void sendTransaction(int my_pid);

  private:
	cMessage *start;  // pointer to the event object which we'll use for timing
	//cMessage *DBMessage;  
	char node_name;
	int GATE_SIZE;
	int delay;
	bool do_operation = true;
	bool wait_pid = false;
	bool have_pid = false;
};

Define_Module(Person);



void Person::initialize(){
	srand(time(NULL));

	node_name = par("name");
	delay = par("delay");
	printf("Initialize node %c\n", node_name);


	
	printf("Program starts\n");
	start = new cMessage("start_msg");
	scheduleAt(delay, start);
	

}

void Person::sendTransaction(int my_pid){
	if (do_operation) {
		do_operation = false;

		printf("\n---------------------------------------------");
		printf("\nUser %c generates a new transaction ", node_name);
		//global_pid = read_pid();
		//global_pid++;
		//update_pid(global_pid);
		printf("with pid %d ", my_pid);
		GATE_SIZE = gateSize("gate");
		int amount;
		char receiver;

		do{
			amount = rand() % 20;
		}while(amount==0);
		printf("and money %d. ", amount);
		
		do{
			receiver = rand() % (68-65 + 1) + 65;
		}while(receiver == (int)node_name);

		printf("The eceiver is %c\n", receiver);
		
		//Broadcast message = 99
		DBMessage *msg_transaction = generateMessage(node_name, BROADCAST, amount, receiver, my_pid);

		printf("Send the broadcast message to the databases\n");
		for (int k = 0; k<GATE_SIZE; k++){
		// $o and $i suffix is used to identify the input/output part of a two way gate
			DBMessage *copy = msg_transaction->dup();
			send(copy, "gate$o", k);
		
			bubble("Send messages");
		}

		start = new cMessage("new_start_msg");
		scheduleAt(simTime()+(rand() % 20) + delay + 20, start);

	}
	else{
		start = new cMessage("oostart_msg");
		scheduleAt(simTime()+(rand() % 20) + delay, start);
		}
}

void Person::handleMessage(cMessage *msg){

	//if msg == return pid -> fai tutta la roba di prima (send transaction)
	if (msg == start) {
		
		if(wait_pid){
			printf("User %c wait the pid\n", node_name);
			start = new cMessage("wait_pid_msg");
			scheduleAt(simTime()+(rand() % 20), start);
		}
		else{
			//ask_pid
			printf("\n---------------------------------------------");
			printf("\nUser %c asks for a pid", node_name);
			DBMessage *pid_req = askPid(node_name, DB_COORDINATOR);
			send(pid_req, "gate$o", DB_COORDINATOR);

			wait_pid = true;
		}

	}
	else{

		UserMessage *ttmsg = check_and_cast<UserMessage *>(msg);
		int db_msg_type = ttmsg->getMsg_type();
		int pid_operation = ttmsg->getPid();
		
		printf("\n---------------------------------------------");
		printf("\nUser %c (dest msg: %c) receives a message from DB %d ",node_name, ttmsg->getDestination(), ttmsg->getSource());

		//0
		if(db_msg_type == ERROR){
			printf("\nYou can't performe operation with pid %d", pid_operation);
			do_operation = true;

		}
		//1
		if(db_msg_type == OPERATION_DONE){
			printf("\nOperation number %d is done!", pid_operation);
			do_operation = true;
		}
		//2
		if(db_msg_type == SOMETHING_WRONG){
			printf("\nWell, this is embarrassing! Sorry operation %d has gone wrong.", pid_operation);	
			do_operation = true;

		}
		//9
		if(db_msg_type == MONEY){
			printf("\nYou have received some money");
		}

		//3
		if(db_msg_type == PID_WRONG){
			printf("\nThe pid %d is wrong!", pid_operation);
		}

		//3
		if(db_msg_type == PID_REQ){
			printf("\nYou have received the PID number: %d!", pid_operation);
			//pid_number
			have_pid = true;
			wait_pid = false;
			//send auto msg
			//start = new cMessage("have_pid_msg");
			//scheduleAt(simTime(), start);
			if(node_name != 'A') {
				sendTransaction(pid_operation);
			}
		}

		if(db_msg_type == DELETE_PID){
			printf("\nOperation pid n.%d is expired (timeout ended)", pid_operation);
			have_pid = false;
			wait_pid = false;


			start = new cMessage("restart_msg");
			scheduleAt(simTime()+(rand() % 20) + delay, start);
		}
	}

}

DBMessage *Person::generateMessage(char src, int dest, int my_balance, char my_rec, int my_pid)
{
	DBMessage *msg = new DBMessage("transaction");
	msg->setMsg_type(TRANSACTION_MSG);
	msg->setSource(src);
	msg->setDb_source(node_name); //per vedere se riesco a togliere una delle due sources
	msg->setDestination(dest);
	msg->setAmount(my_balance);
	msg->setReceiver(my_rec);
	msg->setPid(my_pid);

	return msg;
}

DBMessage *Person::askPid(char src, int dest)
{
	DBMessage *msg = new DBMessage("pid_req");
	msg->setMsg_type(3);
	msg->setSource(src);
	msg->setDb_source(node_name); //per vedere se riesco a togliere una delle due sources
	msg->setDestination(dest);
	return msg;
}


