
//
// This file is part of an OMNeT++/OMNEST simulation example.
//
// Copyright (C) 2003-2015 Andras Varga
//
// This file is distributed WITHOUT ANY WARRANTY. See the file
// `license' for details on this and other legal matters.
//


simple Txc12
{
    parameters:
	@display("i=block/routing");
        @display("i=,cyan");
    gates:
        //input in[];  // declare in[] and out[] to be vector gates
        //output out;

	inout gate[];
}

simple Person extends Txc12
{
    parameters:
        @display("i=,gold");
	
}

network Tictoc12
{

    types:
        channel Channel extends ned.DelayChannel {
            delay = 100ms;
        }
    submodules:
        db[4]: Txc12;

	tocA: Person{
		@display("p=228, 175");
	}
	tocB: Person{
		@display("p=128, 175");
	}

    connections:
        db[0].gate++ <--> Channel <--> db[1].gate++;
        db[0].gate++ <--> Channel <--> db[2].gate++;
        db[0].gate++ <--> Channel <--> db[3].gate++;

	tocA.gate++ <--> Channel <--> db[0].gate++;
	tocA.gate++ <--> Channel <--> db[1].gate++;
	tocA.gate++ <--> Channel <--> db[2].gate++;
	tocA.gate++ <--> Channel <--> db[3].gate++;



}

