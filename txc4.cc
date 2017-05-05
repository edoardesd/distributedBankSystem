//HOW TO RUN:
//opp_makemake -f -o output -I/mysql/include -L/mysql/lib/o -lmysqlclient
//
// make depend
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
#include <stdio.h>
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


    unsigned int port = 3306;
    
    unsigned int flag = 0;

    //MYSQL *conn;
    MYSQL_RES *res;
    MYSQL_ROW row;
    MYSQL * conn = mysql_init(NULL);
};

Define_Module(Txc4);


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


        if (!mysql_real_connect(conn, "localhost", "root", "distributedpass", "bank", port, unix_socket, flag)){

            fprintf(stderr, "%s [%d]\n", mysql_error(conn),mysql_errno(conn) );
            EV << "ERRPRE\n";
            printf("errore");
            exit(1);
        }

        if (mysql_query(conn, "show tables")) {
            fprintf(stderr, "%s\n", mysql_error(conn));
            exit(1);
        }


         res = mysql_use_result(conn);

        /* output table name */
        printf("MySQL Tables in mysql database:\n");
        while ((row = mysql_fetch_row(res)) != NULL){
            EV << "the rows is " << row[0] << " ok\n";

            printf("%s \n", row[0]);
        }

        }
}

void Txc4::handleMessage(cMessage *msg)
{
    counter--;
    if (counter == 0) {
        EV << getName() << "'s counter reached zero, deleting message\n";
        delete msg;
    }
    else {
        EV << "salame\n";
        EV << getName() << "'s counter is " << counter << ", sending back message\n";
        send(msg, "out");
    }


}

