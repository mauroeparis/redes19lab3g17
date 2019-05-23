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
    cOutVector packetDropVector;
    cOutVector bufferSizeVector;
    int waitingFeedback;
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
    waitingFeedback = 1;
}

TransportTx::~TransportTx() {
    cancelAndDelete(endServiceEvent);
}

void TransportTx::initialize(){
    buffer.setName("buffer");
    packetDropVector.setName("dropCount");
    bufferSizeVector.setName("sizeCount");
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
            waitingFeedback = 0;
        }
    } else { // if msg is a data packet
        if (msg->getKind() == 2) {
            // msg is a feedback packet
            FeedbackPkt *fpkt = (FeedbackPkt *) msg;

            int remainingBuffer = fpkt->getRemainingBuffer();
            this->bubble("Feedback");

            waitingFeedback = 1;

            delete msg;
            scheduleAt(simTime(), endServiceEvent);
        } else {
            // msg is a data packet
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
                if (waitingFeedback) {
                    waitingFeedback = 0;
                    // start the service
                    scheduleAt(simTime(), endServiceEvent);
                }
            }
        }
    }
}

#endif /* TRANSTX */
