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

//msg_type
//0: timer before send check msg
//1: message from A to db
//2: message from db to db
//3: flag msg from db123 to db0
//4: final_flag timer

message UserMessage{
	int msg_type;
	int source;
	char destination;

    int pid;
}

//0: balance less than 0
//1: operation ok
//2: Well, this is embarrassing! Sorry something has gone wrong.
