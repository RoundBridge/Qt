#include <thread>
#include "common.h"
#include "crc.h"
#include "controller.h"
#include "actuator.h"
#include "joint.h"
#include "attitude.h"


EndFactory::EndFactory() {
    registerCreator(MAIN_ACTUATOR_NAME, [](Controller* c, Link* link) {
        return std::make_unique<Actuator>(c, MAIN_ACTUATOR_IP, MAIN_ACTUATOR_PORT, link);
    });
    registerCreator(JOINT_NAME, [](Controller* c, Link* link) {
        return std::make_unique<joint>(c, JOINT_IP, JOINT_PORT, link);
    });
    registerCreator(ATTITUDE_NAME, [](Controller* c, Link* link) {
        return std::make_unique<attitude>(c, ATTITUDE_IP, ATTITUDE_PORT, link);
    });
}

std::unique_ptr<End> EndFactory::create(const std::string& type, Controller* controller, Link* link) {
    auto it = creators.find(type);
    if (it != creators.end()) {
        return it->second(controller, link);
    }
    return nullptr;
}

void EndWorker::doWork() {
    Qt::HANDLE id = QThread::currentThreadId();
    qDebug() << "EndWorker thread " << id << "start.";

    for (;;) {
        if (mStop) {
            mEnd->doWorkProc(true);
            qDebug() << "EndWorker thread " << id << "exit.";
            emit workFinished(); // 通知主线程任务结束
            return;
        } else {
            mEnd->doWorkProc(false);
        }
        QThread::msleep(mWorkSleepGapMs);
    }
}

void EndWorker::stopWork() {
    // QMutexLocker locker(&mMutex);
    mStop = true;
}

End::End(Controller* c, const char* remoteIp, quint16 remotePort, Link* link, const std::string& id):mEndName(id) {
    mCtrl = c;
    mLink = link;

    mCmd = 0, mSeq = 1, mRodState = ROD_STATE_NONE;
    mRemotePort = remotePort;
    mRemoteIp.setAddress(remoteIp);

    mConnectState = Link_Unknown;
    mHeartBeatMs = 0;
    mExeState = EXE_SUCCESS;

    // qDebug() << "End " << mEndName << "created";
}

End::~End() {

}

void End::updateEndConnectState(bool valid, qint64 ms) {
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
        qDebug() << "End " << mEndName << " not connected"; //多一对引号
        // qDebug() << "End size " << mEndName.size();
        // qDebug() << "End " << mEndName.c_str() << " not connected";
        return false;
    }

    // 摇杆命令暂定为公用命令，统一在父类 End 处理
    if (ctrlCmd == CTRL_ROD) {
        if (mRodState == ROD_STATE_NONE) {
            if (!mWorker) {
                qDebug() << "Null mWorker!";
                return false;
            }
            mWorker->stopWork();
            return true;
        }
        // 1. 创建线程和任务对象
        QThread* workerThread = new QThread();
        mWorker = new EndWorker(this);

        // 2. 将任务对象转移到子线程
        mWorker->moveToThread(workerThread);

        // 3. 关联信号槽：线程启动时执行任务，任务结束时退出线程
        QObject::connect(workerThread, &QThread::started, mWorker, &EndWorker::doWork);
        QObject::connect(mWorker, &EndWorker::workFinished, workerThread, &QThread::quit);

        // 4. 线程退出后自动释放资源（避免内存泄漏）
        QObject::connect(workerThread, &QThread::finished, mWorker, &EndWorker::deleteLater);
        QObject::connect(workerThread, &QThread::finished, workerThread, &QThread::deleteLater);

        // 5. 启动线程（开始执行任务）
        workerThread->start();
        return true;
    }

    return processCmd(ctrlCmd);
}

uint32_t End::makeCmd(uint32_t c, uint32_t s, int32_t msgType, QByteArray& out, QJsonObject* extra, QByteArray* byte) {
    uint32_t bodyL = 0, hdrL = 0;
    QDMessageHdr hdr;
    QJsonObject root;
    QJsonDocument doc;
    QByteArray array;
    QByteArray crc_data;
    QByteArray send_data_head1;
    QByteArray send_data_head2;

    if (c == MAX_UINT32) {
        return 0;
    }

    hdrL = sizeof(QDMessageHdr);

    if (msgType == QD_MESSAGE_TYPE_JSON) {
        root.insert("from", CENTER_NAME);
        root.insert("to", mEndName.c_str());
        root.insert("type", "request");
        root.insert("cmd", QJsonValue::fromVariant(QVariant(c)));
        root.insert("result", 1);
        root["seq"] = QJsonValue::fromVariant(QVariant(s));
        if(extra) root.insert("extra", *extra);
        doc.setObject(root);
        // array = doc.toJson(QJsonDocument::Compact);
        array = doc.toJson(QJsonDocument::Indented); //Indented方式便于观察

        // 观察
        mCtrl->dispCmdInfo(mEndName, array);
    } else {
        if(byte) array = *byte;
    }
    bodyL = array.size();

    hdr.magic = 0x4E5A4451;
    hdr.magic = qToBigEndian(hdr.magic);
    hdr.len = bodyL + hdrL;
    hdr.len = qToBigEndian(hdr.len);
    hdr.type = msgType;
    hdr.type = qToBigEndian(hdr.type);

    QDataStream out1(&send_data_head1, QIODevice::WriteOnly);
    QDataStream out2(&send_data_head2, QIODevice::WriteOnly);

    out1 << hdr.len << hdr.type;
    crc_data = send_data_head1 + array;
    hdr.crc32 = getCRC32((unsigned char *)(crc_data.data()), bodyL + 8);
    hdr.crc32 = qToBigEndian(hdr.crc32);
    out2 << hdr.magic << hdr.crc32;
    out = send_data_head2 + crc_data;


    return out.size();
}

void End::updateEndExeState(uint32_t endCmd, uint32_t seq, uint32_t state) {
    (void)endCmd; (void)seq; (void)state;
}

bool End::setParam(uint32_t key, void* data, uint32_t dataLen) {
    (void)key; (void)data; (void)dataLen;
    return false;
}

bool End::getParam(uint32_t key, void* data, uint32_t dataLen) {
    (void)key; (void)data; (void)dataLen;
    return false;
}

void End::parseExtraInfo(uint32_t endCmd, QJsonObject &e) {
    (void)endCmd; (void)e;
}

uint32_t End::getMappedCmd(uint32_t ctrlCmd) {
    (void)ctrlCmd;
    return 0;
}

bool End::processCmd(uint32_t ctrlCmd) {
    (void)ctrlCmd;
    return false;
}

bool End::doWorkProc(bool stop) {
    (void)stop;
    return false;
}

bool End::reConnect() {
    return false;
}

bool End::stop() {
    bool ret = false;
    uint32_t len = 0;
    QByteArray out;

    len = makeCmd(getMappedCmd(CTRL_STOP), ++mSeq, QD_MESSAGE_TYPE_JSON, out);
    if (len && mLink->send((uint8_t*)out.data(), (uint32_t)out.size(), mRemoteIp, mRemotePort)) {
        qDebug() << "stop end " << mEndName << " executed";
        ret = true;
    }
    return ret;
}

bool End::recover() {
    bool ret = false;
    uint32_t len = 0;
    QByteArray out;

    len = makeCmd(getMappedCmd(CTRL_RECOVER), ++mSeq, QD_MESSAGE_TYPE_JSON, out);
    if (len && mLink->send((uint8_t*)out.data(), (uint32_t)out.size(), mRemoteIp, mRemotePort)) {
        qDebug() << "recover end " << mEndName << " executed";
        ret = true;
    }
    return ret;
}

bool End::pause() {
    bool ret = false;
    uint32_t len = 0;
    QByteArray out;

    len = makeCmd(getMappedCmd(CTRL_PAUSE), ++mSeq, QD_MESSAGE_TYPE_JSON, out);
    if (len && mLink->send((uint8_t*)out.data(), (uint32_t)out.size(), mRemoteIp, mRemotePort)) {
        qDebug() << "pause end " << mEndName << " executed";
        ret = true;
    }
    return ret;
}

bool End::resume() {
    bool ret = false;
    uint32_t len = 0;
    QByteArray out;

    len = makeCmd(getMappedCmd(CTRL_RESUME), ++mSeq, QD_MESSAGE_TYPE_JSON, out);
    if (len && mLink->send((uint8_t*)out.data(), (uint32_t)out.size(), mRemoteIp, mRemotePort)) {
        qDebug() << "resume end " << mEndName << " executed";
        ret = true;
    }
    return ret;
}

bool End::reset() {
    bool ret = false;
    uint32_t len = 0;
    QByteArray out;

    len = makeCmd(getMappedCmd(CTRL_RESET), ++mSeq, QD_MESSAGE_TYPE_JSON, out);
    if (len && mLink->send((uint8_t*)out.data(), (uint32_t)out.size(), mRemoteIp, mRemotePort)) {
        qDebug() << "reset end " << mEndName << " executed";
        ret = true;
    }
    return ret;
}

bool End::query() {
    bool ret = false;
    uint32_t len = 0;
    QByteArray out;

    len = makeCmd(getMappedCmd(CTRL_QUERY), mSeq, QD_MESSAGE_TYPE_JSON, out);
    if (len && mLink->send((uint8_t*)out.data(), (uint32_t)out.size(), mRemoteIp, mRemotePort)) {
        ret = true;
    }
    return ret;
}
