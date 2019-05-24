#ifndef TRANSRX
#define TRANSRX

#include <string.h>
#include <omnetpp.h>
#include <feedback_m.h>

using namespace omnetpp;

class TransportRx: public cSimpleModule {
private:
    cQueue buffer;
    cMessage *endServiceEvent;
    simtime_t serviceTime;
    cOutVector packetDropVector;
    cOutVector bufferSizeVector;
public:
    TransportRx();
    virtual ~TransportRx();
protected:
    virtual void initialize();
    virtual void finish();
    virtual void handleMessage(cMessage *msg);
};

Define_Module(TransportRx);

TransportRx::TransportRx() {
    endServiceEvent = NULL;
}

TransportRx::~TransportRx() {
    cancelAndDelete(endServiceEvent);
}

void TransportRx::initialize(){
    buffer.setName("buffer");
    packetDropVector.setName("dropCount");
    bufferSizeVector.setName("sizeCount");
    endServiceEvent = new cMessage("endService");
}

void TransportRx::finish(){
}

void TransportRx::handleMessage(cMessage * msg) {
    if (msg == endServiceEvent) {
        // if packet in buffer, send next one
        if (!buffer.isEmpty()) {
            FeedbackPkt *fpkt = new FeedbackPkt();
            fpkt->setByteLength(20);
            fpkt->setKind(2);

            send(fpkt, "toOut$o");

            // dequeue packet
            cPacket *pkt = (cPacket*) buffer.pop();
            // send packet
            send(pkt, "toSink");

            // start new service
            serviceTime = pkt->getDuration();
            scheduleAt(simTime() + serviceTime, endServiceEvent);
        }
    } else if (msg->getKind() == 1) {
        FeedbackPkt *fpkt = (FeedbackPkt *) msg;

        int remaining = par("bufferSize").intValue() - buffer.getLength();
        fpkt->setRemainingBuffer(remaining);

        send(fpkt, "toOut$o");

    } else {
        if (buffer.getLength() >= par("bufferSize").intValue()) {
            // queue is full. Drop msg
            delete msg;
            this->bubble("packet dropped");
            packetDropVector.record(1);
        } else {
            // enqueue the packet
            buffer.insert(msg);
            bufferSizeVector.record(buffer.getLength());
            // if the server is idle
            if (!endServiceEvent->isScheduled()) {
                // start the service
                scheduleAt(simTime() + 0, endServiceEvent);
            }
        }
    }
}

#endif /* TRANSRX */
