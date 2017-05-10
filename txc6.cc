//
// This file is part of an OMNeT++/OMNEST simulation example.
//
// Copyright (C) 2003-2015 Andras Varga
//
// This file is distributed WITHOUT ANY WARRANTY. See the file
// `license' for details on this and other legal matters.
//

#include <stdio.h>
#include <string.h>
#include <omnetpp.h>
#include <mysql/mysql.h>


using namespace omnetpp;
static char *unix_socket = NULL;


/**
 * In the previous models, `tic' and `toc' immediately sent back the
 * received message. Here we'll add some timing: tic and toc will hold the
 * message for 1 simulated second before sending it back. In OMNeT++
 * such timing is achieved by the module sending a message to itself.
 * Such messages are called self-messages (but only because of the way they
 * are used, otherwise they are completely ordinary messages) or events.
 * Self-messages can be "sent" with the scheduleAt() function, and you can
 * specify when they should arrive back at the module.
 *
 * We leave out the counter, to keep the source code small.
 */
class Txc6 : public cSimpleModule
{
  private:
    cMessage *event;  // pointer to the event object which we'll use for timing
    cMessage *tictocMsg;  // variable to remember the message until we send it back
    int counter = 10;

  public:
    Txc6();
    virtual ~Txc6();

  protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
    virtual void finish_with_error(MYSQL *con);
    virtual int read_balance(char* my_query);


    unsigned int port = 3306;
    
    unsigned int flag = 0;

    //MYSQL *conn;
    MYSQL_RES *res;
    MYSQL_RES *query;
    MYSQL_ROW row;

    int num_fields;
    int my_balance;

};

Define_Module(Txc6);

Txc6::Txc6()
{
    // Set the pointer to nullptr, so that the destructor won't crash
    // even if initialize() doesn't get called because of a runtime
    // error or user cancellation during the startup process.
    event = tictocMsg = nullptr;
}

Txc6::~Txc6()
{
    // Dispose of dynamically allocated the objects
    cancelAndDelete(event);
    delete tictocMsg;
}

void Txc6::initialize()
{
    // Create the event object we'll use for timing -- just any ordinary message.
    event = new cMessage("event");

    // No tictoc message yet.
    tictocMsg = nullptr;

    if (strcmp("tic", getName()) == 0) {
        // We don't start right away, but instead send an message to ourselves
        // (a "self-message") -- we'll do the first sending when it arrives
        // back to us, at t=5.0s simulated time.
        EV << "Scheduling first send to t=5.0s\n";
        tictocMsg = new cMessage("tictocMsg");
        scheduleAt(5.0, event);
    }
}

void Txc6::handleMessage(cMessage *msg){

    counter--;

    if (counter!=0){
    // There are several ways of distinguishing messages, for example by message
    // kind (an int attribute of cMessage) or by class using dynamic_cast
    // (provided you subclass from cMessage). In this code we just check if we
    // recognize the pointer, which (if feasible) is the easiest and fastest
    // method.
    if (msg == event) {
        // The self-message arrived, so we can send out tictocMsg and nullptr out
        // its pointer so that it doesn't confuse us later.
        EV << "Wait period is over, sending back message\n";
        fprintf(stderr, "Counter: %d\n", counter);
        send(tictocMsg, "out");
        tictocMsg = nullptr;
    }
    else {
        // If the message we received is not our self-message, then it must
        // be the tic-toc message arriving from our partner. We remember its
        // pointer in the tictocMsg variable, then schedule our self-message
        // to come back to us in 1s simulated time.
        EV << "Message arrived, starting to wait 1 sec...\n";
        tictocMsg = msg;
        scheduleAt(simTime()+1.0, event);
        fprintf(stderr, "Counter2: %d\n", counter);
        my_balance = read_balance("SELECT balance FROM people WHERE name =\"edo\"");
    }
}
else{

    fprintf(stderr, "counter is zero, check: %d\n", counter);
     EV << getName() << "'s counter reached zero, deleting message\n";
        delete msg;
}
}

int Txc6::read_balance(char* my_query){
    int query_balance;

    MYSQL * conn = mysql_init(NULL);

    if (!mysql_real_connect(conn, "localhost", "root", "distributedpass", "bank", port, unix_socket, flag)){
            EV << "Connection error\n";
            fprintf(stderr, "Here\n" );
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

void Txc6::finish_with_error(MYSQL *con){
  fprintf(stderr, "%s\n", mysql_error(con));
  fprintf(stderr, "ERROR ehere\n" );
  mysql_close(con);
  exit(1);        
}


