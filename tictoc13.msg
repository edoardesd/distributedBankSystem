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

message TicTocMsg13{
	int msg_type;


    char source;
    int db_source;

    int destination;
    char receiver;
    
    int pid;
    long amount;
   	long amount2; //used when check balance -> //amount of second one

    
    int hopCount = 0;
}

//msg_type
//1: message from A to db
//2: message from db to db

message ErrorMessage{
	int msg_type;
	int source;
	char destination;

    int pid;
}

