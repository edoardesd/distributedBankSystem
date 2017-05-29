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

using namespace omnetpp;

#include "tictoc13_m.h"

////////////////////// DATABASE METHODS //////////////////////

class DB : public cSimpleModule
{
  protected:
	virtual void forwardMessage(TicTocMsg13 *msg);
	virtual void initialize() override;
	virtual void handleMessage(cMessage *msg);
	//virtual TicTocMsg13 *generateMessage();

};


Define_Module(DB);


void DB::initialize()
{
	EV << "HI, I'm number " << getIndex() << "\n";
	//printf("HI, I'm number %d \n", getIndex());
	
}

void DB::handleMessage(cMessage *msg)
{
	TicTocMsg13 *ttmsg = check_and_cast<TicTocMsg13 *>(msg);

	if (getIndex() == 3) {
		// Message arrived.
		EV << "Message " << ttmsg << " arrived.\n";
		delete ttmsg;
	}
	else {
		// We need to forward the message.
		forwardMessage(ttmsg);
	}
}




void DB::forwardMessage(TicTocMsg13 *msg)
{
	// In this example, we just pick a random gate to send it on.
	// We draw a random number between 0 and the size of gate `gate[]'.
	
	// Increment hop count.
    msg->setHopCount(msg->getHopCount()+1);

	int n = gateSize("gate");
	int k = intuniform(0, n-1);

	printf ("Gate size: %d\tDestination: %d \tIndex: %d\n", n, k, getIndex());

	EV << "Forwarding message " << msg << " on gate[" << k << "]\n";
	EV << "I'm number " << getIndex() << "\n";

	//printf("Hop count: %d \n", msg->getHopCount());

	// $o and $i suffix is used to identify the input/output part of a two way gate
	send(msg, "gate$o", k);
	
}


////////////////////// PEOPLE METHODS //////////////////////

class Person : public cSimpleModule
{

  protected:
	virtual void forwardMessage(TicTocMsg13 *msg);
	virtual void initialize() override;
	virtual void handleMessage(cMessage *msg);
	virtual TicTocMsg13 *generateMessage(int src, int dest);

};

Define_Module(Person);



void Person::initialize()
{
	printf ("my gatesize %d", gateSize("gateP"));
	EV << "HI, I'm number " << getIndex() << "\n";
	printf("HI, I'm number %d \n", getIndex());
	if (par("sendMsgOnInit").boolValue() == true) {
		// Boot the process scheduling the initial message as a self-message.
		EV << "Sending initial self message from " << getName() <<"\n";
		//char msgname[20];
		//sprintf(msgname, "msg-%s", getName()); //getIndex is the number that identify the node (itself)
        int n = gateSize("gateP");
		for (int k = 0; k<n; k = k+1){
			// $o and $i suffix is used to identify the input/output part of a two way gate
			TicTocMsg13 *msg_forw = generateMessage(getIndex(), k);
			send(msg_forw, "gateP$o", k);
			printf("Forwarding message on gate[%d]\n", k);

			bubble("send message");

			//send(msg, "gate$o", 2);
		}
	}
}

void Person::forwardMessage(TicTocMsg13 *msg)
{
	// In this example, we just pick a random gate to send it on.
	// We draw a random number between 0 and the size of gate `gate[]'.
	//EV << "I'm number " << getIndex() << "\n";

	int n = gateSize("gateP");
	for (int k = 0; k<n; k = k+1){
		// $o and $i suffix is used to identify the input/output part of a two way gate
		TicTocMsg13 *msg_forw = generateMessage(getIndex(), k);
		send(msg_forw, "gateP$o", k);
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

TicTocMsg13 *Person::generateMessage(int src, int dest)
{
    // Produce source and destination addresses.
    //int src = getIndex();  // our module index
    //int n = 6;  // module vector size
    //int dest = intuniform(0, n-1);
    //if (dest >= src)
    //    dest++;
    //char msgname[20];
    //sprintf(msgname, "tic-%d-to-%d", src, dest);
    // Create message object and set source and destination field.
    TicTocMsg13 *msg = new TicTocMsg13("msg_gen");
    msg->setSource(src);
    msg->setDestination(dest);
    return msg;
}


