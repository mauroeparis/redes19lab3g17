simple Generator
{
    parameters:
        volatile double generationInterval; // sec
        volatile int packetByteSize;
        @display("i=block/source");
    gates:
        output out;
}

simple Queue
{
    parameters:
        volatile int bufferSize;
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

simple TransportTx
{
    parameters:
        int bufferSize;
        @display("i=block/queue;q=buffer");
    gates:
        input toApp;
        inout toOut;
}

simple TransportRx
{
    parameters:
        int bufferSize;
        @display("i=block/queue;q=buffer");
    gates:
        output toSink;
        inout toOut;
}

module NodeTx
{
    parameters:
        @display("i=block/routing");
    gates:
        inout gate;
    submodules:
        gen: Generator {
            parameters:
                @display("p=75,50");
        }
        TransTx: TransportTx {
            parameters:
                @display("p=75,190");
        }
    connections:
        gen.out --> TransTx.toApp;
        TransTx.toOut <--> gate;
}

module NodeRx
{
    parameters:
        @display("i=block/routing");
    gates:
        inout gate;
    submodules:
        sink: Sink {
            parameters:
                @display("p=75,190");
        }
        TransRx: TransportRx {
            parameters:
                @display("p=75,50");
        }
    connections:
        gate <--> TransRx.toOut;
        TransRx.toSink --> {  datarate = 0.5Mbps; } --> sink.in;
}

network Network
{
    @display("bgl=2");
    submodules:
        nodeTx: NodeTx {
            @display("p=30,75");
        }
        nodeRx: NodeRx {
            @display("p=230,75");
        }
        queue0: Queue {
            @display("p=130,35"); // modify display string
        }
        queue1: Queue {
            @display("p=130,115"); // modify display string
        }
    connections:

        nodeTx.gate$o --> { datarate = 1Mbps; delay = 100us; } --> queue0.in;
        nodeRx.gate$i <-- { datarate = 1Mbps; delay = 100us; } <-- queue0.out;
        
        nodeRx.gate$o --> { datarate = 1Mbps; delay = 100us; } --> queue1.in;
        nodeTx.gate$i <-- { datarate = 1Mbps; delay = 100us; } <-- queue1.out;
        
        
       	
}
