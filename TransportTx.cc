#ifndef TRANSTX
#define TRANSTX

#include <string.h>
#include <omnetpp.h>
#include <feedback_m.h>

using namespace omnetpp;

class TransportTx: public cSimpleModule {
private:
    cQueue buffer;
    cMessage *endServiceEvent;
    simtime_t serviceTime;
public:
    TransportTx();
    virtual ~TransportTx();
protected:
    virtual void initialize();
    virtual void finish();
    virtual void handleMessage(cMessage *msg);
};

Define_Module(TransportTx);

TransportTx::TransportTx() {
    endServiceEvent = NULL;
}

TransportTx::~TransportTx() {
    cancelAndDelete(endServiceEvent);
}

void TransportTx::initialize(){
    endServiceEvent = new cMessage("endService");
}

void TransportTx::finish(){
}

void TransportTx::handleMessage(cMessage * msg) {
    if (msg == endServiceEvent) {
        // if packet in buffer, send next one
        if (!buffer.isEmpty()) {
            // dequeue packet
            cPacket *pkt = (cPacket*) buffer.pop();
            // send packet
            send(pkt, "toOut$o");
            // start new service
            serviceTime = pkt->getDuration();
            scheduleAt(simTime() + serviceTime, endServiceEvent);
        }
    } else { // if msg is a data packet
        if (msg->getKind() == 2) {
            // msg is a feedback packet
            FeedbackPkt *fpkt = (FeedbackPkt *) msg;

            int remainingBuffer = fpkt->getRemainingBuffer();
            this->bubble("Feedback");
            // (...)

            delete(msg);

        } else {
            // msg is a data packet
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
}

#endif /* TRANSTX */
