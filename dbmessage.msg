//
// This file is part of an OMNeT++/OMNEST simulation example.
//
// Copyright (C) 2003 Ahmet Sekercioglu
// Copyright (C) 2003-2015 Andras Varga
//
// This file is distributed WITHOUT ANY WARRANTY. See the file
// `license' for details on this and other legal matters.
//HOW TO COMPILE
//opp_msgc tictoc13.msg

message DBMessage{
	int msg_type;

    char source;
    int db_source;

    int destination;
    char receiver;
    
    int pid;
    int amount;
   	int amount2; //used when check balance -> //amount of second one    
}


message UserMessage{
	int msg_type;
	int source;
	char destination;

    int pid;
}
