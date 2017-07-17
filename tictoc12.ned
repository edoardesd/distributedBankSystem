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
        int name = default(0);
        int send_msg = default(4);
        int delay = default(0);
    
    gates:
    	inout gate[];
}

simple A extends Person{
    parameters:
        @display("i=,red");
        @display("p=50, 50");
        bool sendMsgOnInit = default(true);
        name = 65;  //A
        delay = 20;
}

simple B extends Person{
    parameters:
    	@display("i=,gold");
        @display("p=400, 50");
        bool sendMsgOnInit = default(false);
        send_msg = default(4);
        name = 66; //B
        delay = 100;
}

simple C extends Person{
    parameters:
        @display("i=,white");
        @display("p=50, 400");
        bool sendMsgOnInit = default(false);
        send_msg = default(4);
        name = 67; //C
        delay = 5;
}

simple D extends Person{
    parameters:
        @display("i=,green");
        @display("p=400, 400");
        bool sendMsgOnInit = default(false);
        send_msg = default(4);
        name = 68; //D
        delay = 50;
}

simple E extends Person{
    parameters:
        @display("i=,blue");
        @display("p=250, 50");
        bool sendMsgOnInit = default(false);
        send_msg = default(4);
        name = 69; //E
        delay = 40;
}

simple F extends Person{
    parameters:
        @display("i=,orange");
        @display("p=50, 250");
        bool sendMsgOnInit = default(false);
        send_msg = default(4);
        name = 70; //F
        delay = 10;
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

        channel FastChannel extends ned.DelayChannel {
            delay = 20ms;
        }

        channel SlowChannel extends ned.DelayChannel {
            delay = 200ms;
        }


        channel VerySlowChannel extends ned.DelayChannel {
            delay = 1000ms;
        }
    submodules:
        db[4]: DB;

	personA: A;
	personB: B;
    personC: C;
    personD: D;
    //personE: E;
    //personF: F;

    connections:
    
	    personA.gate++ <--> SlowChannel <--> db[0].gate++;
        db[0].gate++ <--> FastChannel <--> db[1].gate++;
        db[0].gate++ <--> FastChannel <--> db[2].gate++;
        db[0].gate++ <--> FastChannel <--> db[3].gate++;
        
        personA.gate++ <--> SlowChannel <--> db[1].gate++;
        db[1].gate++ <--> FastChannel <--> db[2].gate++;
        db[1].gate++ <--> FastChannel <--> db[3].gate++;
        
        personA.gate++ <--> SlowChannel <--> db[2].gate++;
        db[2].gate++ <--> FastChannel <--> db[3].gate++;

	    personA.gate++ <--> SlowChannel <--> db[3].gate++;
	
	    personB.gate++ <--> SlowChannel <--> db[0].gate++;
	    personB.gate++ <--> SlowChannel <--> db[1].gate++;
	    personB.gate++ <--> SlowChannel <--> db[2].gate++;
	    personB.gate++ <--> SlowChannel <--> db[3].gate++;

        personC.gate++ <--> SlowChannel <--> db[0].gate++;
        personC.gate++ <--> SlowChannel <--> db[1].gate++;
        personC.gate++ <--> SlowChannel <--> db[2].gate++;
        personC.gate++ <--> SlowChannel <--> db[3].gate++;

        personD.gate++ <--> SlowChannel <--> db[0].gate++;
        personD.gate++ <--> SlowChannel <--> db[1].gate++;
        personD.gate++ <--> SlowChannel <--> db[2].gate++;
        personD.gate++ <--> SlowChannel <--> db[3].gate++;

        //personE.gate++ <--> VerySlowChannel <--> db[0].gate++;
        //personE.gate++ <--> VerySlowChannel <--> db[1].gate++;
        //personE.gate++ <--> VerySlowChannel <--> db[2].gate++;
        //personE.gate++ <--> VerySlowChannel <--> db[3].gate++;

        //personF.gate++ <--> VerySlowChannel <--> db[0].gate++;
        //personF.gate++ <--> VerySlowChannel <--> db[1].gate++;
        //personF.gate++ <--> VerySlowChannel <--> db[2].gate++;
        //personF.gate++ <--> VerySlowChannel <--> db[3].gate++;
}

