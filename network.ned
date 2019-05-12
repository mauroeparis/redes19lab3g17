simple pepe
{
    parameters:
        volatile double vd_arg;
        @display("i=misc/node_vs,blue");
    gates:
        output o1;
        output o2;
        input i1;
}

module node
{
    gates:
        output out;
    submodules:
        pepe1: pepe {
            @display("p=50,100");
        }
        pepe2: pepe {
            @display("p=50,50");
        }
    connections:
        pepe1.o1 --> pepe2.i1;
        pepe2.o2 --> pepe1.i1;
        pepe2.o1 --> out;
        

}

simple Generator
{
    parameters:
        volatile double generationInterval; // sec
        @display("i=block/source");
    gates:
        output out;
}

simple Queue
{
    parameters:
        volatile double serviceTime; // sec
        @display("i=block/queue;q=buffer");
    gates:
        input in;
        output out;
}

simple Sink
{
    parameters:
        @display("i=block/sink");
    gates:
        input in;
}

network Network
{
    @display("bgl=2");
    submodules:
        gen: Generator {
            @display("p=30,30");
        }
        queue: Queue {
            @display("p=130,30");
        }
        sink: Sink {
            @display("p=230,30");
        }
    connections:
        gen.out --> queue.in;
        queue.out --> sink.in;
}
