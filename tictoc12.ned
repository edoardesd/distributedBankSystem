// This file is part of an OMNeT++/OMNEST simulation example.
//
// Copyright (C) 2003-2015 Andras Varga
//
// This file is distributed WITHOUT ANY WARRANTY. See the file
// `license' for details on this and other legal matters.
//


simple Person {
    parameters:
        @display("i=block/routing");
    
    gates:
    	inout gate[];
}

simple A extends Person{
    parameters:
        @display("i=,red");
        @display("p=50, 50");
        bool sendMsgOnInit = default(true);
}

simple B extends Person{
    parameters:
    	@display("i=,gold");
        @display("p=400, 50");
        bool sendMsgOnInit = default(false);
}


simple DB {
    parameters:
        @display("i=,cyan");
        bool sendMsgOnInit = default(false);
    
    gates:
    	inout gate[];
}

network Tictoc12{

    types:
        channel Channel extends ned.DelayChannel {
            delay = 100ms;
        }
    submodules:
        db[4]: DB;

	personA: A;
	personB: B;

    connections:
    
	personA.gate++ <--> Channel <--> db[0].gate++;
        db[0].gate++ <--> Channel <--> db[1].gate++;
        db[0].gate++ <--> Channel <--> db[2].gate++;
        db[0].gate++ <--> Channel <--> db[3].gate++;
        
        personA.gate++ <--> Channel <--> db[1].gate++;
        db[1].gate++ <--> Channel <--> db[2].gate++;
        db[1].gate++ <--> Channel <--> db[3].gate++;
        
        personA.gate++ <--> Channel <--> db[2].gate++;
        db[2].gate++ <--> Channel <--> db[3].gate++;

	personA.gate++ <--> Channel <--> db[3].gate++;
	
	personB.gate++ <--> Channel <--> db[0].gate++;
	personB.gate++ <--> Channel <--> db[1].gate++;
	personB.gate++ <--> Channel <--> db[2].gate++;
	personB.gate++ <--> Channel <--> db[3].gate++;



}

