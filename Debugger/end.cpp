#include <thread>
#include "common.h"
#include "controller.h"
#include "actuator.h"


End* End::createEnd(Controller* controller, Link* link, int id) {
    End* e = nullptr;
    if (id == End_actuator) {
        e = new Actuator(controller, MAIN_ACTUATOR_IP, MAIN_ACTUATOR_PORT, link);
    }
    return e;
}

void End::destroyEnd(End* e) {
    delete e;
}

End::End(Controller* c, int id) {
    mEndId = id;
    mCtrl = c;
    mConnectState = Link_Unknown;
    mHeartBeatMs = 0;
}

End::~End() {

}

void End::updateEndState(bool valid, qint64 ms) {
    if (valid) {
        mHeartBeatMs = ms;
        mConnectState = Link_Connect;
    } else {
        if (ms - mHeartBeatMs > 10000 && mConnectState != Link_Connecting && ms - mLastReconnectMs > 30000) {
            // 重连可能耗时，利用线程处理
            mLastReconnectMs = ms;
            std::thread t(&End::reConnect, this);
            t.detach();
        }
    }
}

bool End::execute(uint32_t ctrlCmd) {
    if (mConnectState != Link_Connect) {
        qDebug() << "End " << mEndId << " not connected";
        return false;
    }
    return processCmd(ctrlCmd);
}

uint32_t End::getMappedCmd(uint32_t ctrlCmd) {
    (void)ctrlCmd;
    return 0;
}

bool End::processCmd(uint32_t ctrlCmd) {
    (void)ctrlCmd;
    return false;
}

bool End::reConnect() {
    return false;
}

bool End::query() {
    return false;
}
