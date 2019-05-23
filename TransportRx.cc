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
            int remaining = par("bufferSize").intValue() - buffer.getLength();
            fpkt->setRemainingBuffer(remaining);
            send(fpkt, "toOut$o");

            // dequeue packet
            cPacket *pkt = (cPacket*) buffer.pop();
            // send packet
            send(pkt, "toSink");

            // start new service
            serviceTime = pkt->getDuration();
            scheduleAt(simTime() + serviceTime, endServiceEvent);
        }
    } else {
        if (buffer.getLength() >= par("bufferSize").intValue()) {
            // queue is full. Drop msg
            delete msg;
            this->bubble("packet dropped");
        } else {
            // enqueue the packet
            buffer.insert(msg);
            // if the server is idle
            if (!endServiceEvent->isScheduled()) {
                // start the service
                scheduleAt(simTime() + 0, endServiceEvent);
            }
        }
    }
}

#endif /* TRANSRX */
