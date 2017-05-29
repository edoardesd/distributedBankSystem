//
// This file is part of an OMNeT++/OMNEST simulation example.
//

//opp_makemake -f -o output -I/mysql/include -L/mysql/lib/o -lmysqlclient

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

/**
 * Let's make it more interesting by using several (n) `tic' modules,
 * and connecting every module to every other. For now, let's keep it
 * simple what they do: module 0 generates a message, and the others
 * keep tossing it around in random directions until it arrives at
 * module 2.
 */
class DB : public cSimpleModule
{
  protected:
    virtual void forwardMessage(cMessage *msg);
    //virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
};



Define_Module(DB);




void DB::handleMessage(cMessage *msg)
{
    if (getIndex() == 3) {
        // Message arrived.
        EV << "Message " << msg << " arrived.\n";
        delete msg;
    }
    else {
        // We need to forward the message.
        forwardMessage(msg);
    }
}

void DB::forwardMessage(cMessage *msg)
{
    // In this example, we just pick a random gate to send it on.
    // We draw a random number between 0 and the size of gate `gate[]'.
    if (getIndex()==0){
        
        //send(msg, "gate$o", 2);
        
        int n = gateSize("gate");
        for (int k = 0; k<n; k = k+1){
            EV << "Forwarding message " << msg << " on gate[" << k << "]\n";
            EV << "I'm number " << getIndex() << "\n";
            // $o and $i suffix is used to identify the input/output part of a two way gate
            cMessage *msg_forw = new cMessage("newmsg");
            send(msg_forw, "gate$o", k);
            bubble("send message");
        }
        delete msg;
    }
    else{
    
    int n = gateSize("gate");
    int k = intuniform(0, n-1);

    EV << "Forwarding message " << msg << " on gate[" << k << "]\n";
    EV << "I'm number " << getIndex() << "\n";

    // $o and $i suffix is used to identify the input/output part of a two way gate
    send(msg, "gate$o", k);
    }
}


class Person : public cSimpleModule
{

  protected:
    virtual void forwardMessage(cMessage *msg);
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
};

Define_Module(Person);



void Person::initialize()
{
    if (par("sendMsgOnInit").boolValue() == true) {
        // Boot the process scheduling the initial message as a self-message.
        EV << "Sending initial mussage from number " << getIndex() <<"\n";
        char msgname[20];
        sprintf(msgname, "msg-%d", getIndex()); //getIndex is the number that identify the node (itself)
        cMessage *msg_self = new cMessage(msgname);
        scheduleAt(0.0, msg_self);
        //send(msg, "gate$o", 2);
    }
}
void Person::forwardMessage(cMessage *msg)
{
    // In this example, we just pick a random gate to send it on.
    // We draw a random number between 0 and the size of gate `gate[]'.
    if (getIndex()==0){
        
        //send(msg, "gate$o", 2);
        
        int n = gateSize("gate");
        for (int k = 0; k<n; k = k+1){
            EV << "Forwarding mussasasas " << msg << " on gate[" << k << "]\n";
            EV << "I'm number " << getIndex() << "\n";
            // $o and $i suffix is used to identify the input/output part of a two way gate
            cMessage *msg_forw = new cMessage("newmsg");
            send(msg_forw, "gate$o", k);
            bubble("send message");
        }
        delete msg;
    }
    else{
    
    int n = gateSize("gate");
    int k = intuniform(0, n-1);

    EV << "Forwarding message " << msg << " on gate[" << k << "]\n";
    EV << "I'm number " << getIndex() << "\n";

    // $o and $i suffix is used to identify the input/output part of a two way gate
    send(msg, "gate$o", k);
    }

}

void Person::handleMessage(cMessage *msg)
{
    if (getIndex() == 3) {
        // Message arrived.
        EV << "Message " << msg << " arrived.\n";
        delete msg;
    }
    else {
        // We need to forward the message.
        forwardMessage(msg);
    }
}


