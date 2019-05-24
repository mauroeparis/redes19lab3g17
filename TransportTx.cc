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
    int remainingBuffer;
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

    FeedbackPkt *fpkt = new FeedbackPkt();
    fpkt->setByteLength(20);
    fpkt->setKind(1);
    send(fpkt, "toOut$o");
}

void TransportTx::finish(){
}

void TransportTx::handleMessage(cMessage * msg) {
    if (msg == endServiceEvent) {
        // if packet in buffer, send next one
        if (!buffer.isEmpty() && remainingBuffer) {
            // dequeue packet
            cPacket *pkt = (cPacket*) buffer.pop();
            // send packet
            send(pkt, "toOut$o");
            // start new service
            serviceTime = pkt->getDuration();
            scheduleAt(simTime() + serviceTime, endServiceEvent);

            remainingBuffer--;
        }
    } else { // if msg is a data packet
        if (msg->getKind() == 1) {
            FeedbackPkt *fpkt = (FeedbackPkt *) msg;
            remainingBuffer = fpkt->getRemainingBuffer();

            this->bubble("Initial");

            delete msg;
        } else if (msg->getKind() == 2) {
            // msg is a feedback packet
            FeedbackPkt *fpkt = (FeedbackPkt *) msg;
            this->bubble("Feedback");

            remainingBuffer++;

            // if the server is idle
            if (!endServiceEvent->isScheduled()) {
                // start the service
                scheduleAt(simTime() + 0, endServiceEvent);
            }

            delete msg;

        } else {
            // msg is a data packet
            if (buffer.getLength() >= par("bufferSize").intValue()) {
                // queue is full. Drop msg
                delete msg;
                this->bubble("packet dropped");
                packetDropVector.record(1);
            } else {
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
}

#endif /* TRANSTX */
