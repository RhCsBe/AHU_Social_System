#include "workthread.h"

WorkThread::WorkThread(QObject *parent)
    : QObject{parent}
{

}

void WorkThread::mergeDataPackage(QString key, QByteArray dataArray)
{
    QDataStream fileHead(&dataArray,QFile::ReadOnly);
    FileType fileType;
    qint16 space;
    int fileSize;
    fileHead>>fileType>>space>>fileSize;
    dataArray.remove(0,10);
    //QFile* file=nullptr;

    switch (fileType)
    {
    case JsonDataHead:
        parseMessage(key,dataArray.left(fileSize));
        break;
    case FileInfoHead:
    {
        //上面对数据进行了处理，原来数据流的定位已经不准了，要重新开一个
        QDataStream readFile(&dataArray,QFile::ReadOnly);
        int type,fileSize;
        QString fileName,sendToAccount;
        readFile>>type>>fileSize>>fileName>>sendToAccount;
        this->fileSize.insert(key,QPair(0,fileSize));
        QFile* file=new QFile();
        this->fileArray.insert(key,file);
        switch(type)
        {
            default:
            {
                file->setFileName(Protocol::getAllUserPath()+"/"+fileName);
                file->open(QFile::WriteOnly);
                break;
            }
        }
    }
    break;
    case FileDataHead:
    {
        QFile* file=fileArray.value(key);
        file->write(dataArray);
        this->fileSize[key].first+=NoHeadBufferSize;
    }
    break;
    case FileEndDataHead:
    {
        QFile* file=fileArray.value(key);
        file->write(dataArray.left(this->fileSize[key].second-this->fileSize[key].first));
        file->close();
        this->fileArray.remove(key);
        this->fileSize.remove(key);
        delete file;
    }
    break;
    default:
        break;
    }

}

void WorkThread::parseMessage(QString key, QByteArray dataArray)
{
    QJsonObject jsonData=QJsonDocument::fromJson(dataArray).object();
    int type=jsonData.value("type").toInt();
    //qDebug()<<"type:"<<type;
    switch(type)
    {
        case Reconnection://断线重连
        case LoginAccount://用户登录
        {
            //读取登录信息
            QString account=jsonData.value("account").toString();
            QString password=jsonData.value("password").toString();
            qint64 lastLoginTime=jsonData.value("lastLoginTime").toString().toLongLong();
            qint64 loginTime=jsonData.value("loginTime").toString().toLongLong();
            bool remember=jsonData.value("remember").toBool();
            bool autoLogin=jsonData.value("autoLogin").toBool();
            bool firstLogin=jsonData.value("firstLogin").toBool();
            qDebug()<<account<<"    "<<password;

            //验证登录信息
            if(sql.judgeUserExist(account,password))
            {
                //result:-1——登录失败，0——重复登录，1——登录成功
                QString fileName="";
                QString headPhoto="";

                //判断是否重复登录
                if(sql.isUserOnLine(account))
                {
                    replyJson(key,InfoType(type),QString::number(0),"","",account,fileName);
                    break;
                }

                //判断是否第一次登录，或者用户信息是否过时，过时则同步
                if(firstLogin || !sql.judgeUpdateTimeOfUser(account,lastLoginTime))
                {
                    //判断是否发送login文件
                    qDebug()<<"用户信息过时";
                    QString temp=Protocol::getWorkPath()+"/"+account+"/login.json";
                    //更新login文件
                    updateFile(temp,QJsonDocument(sql.getUserDateFile(account,remember,autoLogin)).toJson());

                    //添加进发送文件的名单
                    fileName+=temp+"?";

                    //判断是否发送用户头像
                    QString str=sql.getUserHeadPhoto(account);
                    if(str!=""&&Protocol::isFileExist("allUser",str))
                    {
                        headPhoto+=Protocol::getAllUserPath()+"/"+str+"?";
                    }
                }
                //判断是否第一次登录，联系人信息是否过时
                if(firstLogin || !sql.judgeUpdateTimeOfRelation(account,lastLoginTime))
                {
                    //判断是否发送friend文件
                    qDebug()<<"联系人信息过时";
                    QString temp=Protocol::getWorkPath()+"/"+account+"/friend.json";
                    //更新friend文件
                    updateFile(temp,QJsonDocument(sql.getFriendDataFile(account)).toJson());
                    fileName+=temp+"?";

                    //判断是否发送好友头像
                    QStringList list=sql.getAllFriendHeadPhoto(account);
                    for(auto i:list)
                    {
                        if(i!=""&&Protocol::isFileExist("allUser",i))
                        {
                            headPhoto+=Protocol::getAllUserPath()+"/"+i+"?";
                        }
                    }

                    //更新group和添加相关头像
                    temp=Protocol::getWorkPath()+"/"+account+"/group.json";
                    //这里添加头像包含了路径，不用像上面那个遍历添加了
                    updateFile(temp,sql.getGroupDataFile(account,headPhoto));
                    fileName+=temp+"?";
                }
                //头像名单不为空去掉末尾的问号,并发送信号
                if(headPhoto!="")
                {
                    headPhoto.chop(1);
                    emit sendToServer(key,InfoType::AllHeadPhoto,"","",QByteArray(),"",headPhoto);
                }

                //文件名单不为空去掉末尾的问号
                if(fileName!="")
                {
                    fileName.remove(fileName.length()-1,1);
                }
                qDebug()<<fileName;
                replyJson(key,InfoType(type),QString::number(1),"","",account,fileName);
                emit myInformation("登录成功");

                //将该用户套接字添加进在线用户套接字映射表
                emit addUserOnline(account,key);
                //添加消息记录映射表
                messageArray.insert(account,QHash<QString,QJsonArray*>());
                //同步历史信息
                sendMessageFile(account,lastLoginTime);
                //添加个人动态记录映射表
                dynamicArray.insert(account,new QJsonArray());
                //同步历史动态
                sendDynamicFile(account,lastLoginTime);
                //设置用户状态
                sql.userUpLine(account,loginTime);

                //发送存储的任务
                sendMessageTask(account);
                //1.5秒后发送存储在数据库的消息任务
//                QTimer::singleShot(1500,this,[&](){
//                    sendMessageTask(account);
//                },);
            }
            else
            {
                replyJson(key,InfoType(type),QString::number(-1));
                emit myInformation("登录失败");
                //QMessageBox::information(nullptr,"提示","登录失败");
            }
            break;
        }
        case SendMessage://发送消息
        {
            //qDebug()<<"接收到发来的信息："<<jsonData.value("message").toString();
            QString sender=jsonData.value("sender").toString();
            QString receiver=jsonData.value("receiver").toString();

            //先将messageTask转成message，去除多余信息
            QJsonObject json=messageTaskToMessage(jsonData);
            //将消息添加进发送者的消息缓存中
            addMessage(sender,receiver,json);
            //qDebug()<<"消息缓存完毕";
            if(receiver[0]=='G')
            {
                sendGroupMessage(jsonData);
            }
            else
            {
                //判断接收者是否在线
                if(sql.isUserOnLine(receiver))
                {
                    //在线转发并存储在接收者的消息缓存中
                    sendToServer(key,type,sender,receiver,QJsonDocument(jsonData).toJson(),"","");

                    addMessage(receiver,sender,json);
                }
                else
                {
                    //不在则将消息转成任务存储到mysql任务表中，等待接收者上线后转发
                    sql.addMessageTask(jsonData);
                }
            }
            break;
        }
        case Registration:
        {
            //读取json信息
            QString id=jsonData.value("id").toString();
            QString identity=jsonData.value("identity").toString();
            QString message=jsonData.value("message").toString();
            //判断注册信息是否正确
            if(sql.judgeUserRegistrationMessage(id,identity,message))
            {
                //判断用户是否已存在
                if(sql.userExist(id))
                {
                    replyJson(key,InfoType::Registration,QString::number(0));//账号已存在
                }
                else
                {
                    //往数据库写入新用户信息,并创建用户数据文件夹
                    userRegister(id,message.split("\n").at(1));
                    replyJson(key,InfoType::Registration,QString::number(1));//注册成功
                }
            }
            else
            {
                replyJson(key,InfoType::Registration,QString::number(-1));//信息错误，注册失败
            }
            break;
        }
        case SearchUser:
        {
            //读取json信息
            QString account=jsonData.value("account").toString();
            QString message=jsonData.value("message").toString();
            QString fileName=Protocol::getWorkPath()+"/"+account+"/searchUser.json";
            QString headPhotoList="";
            updateFile(fileName,sql.searchUser(message,headPhotoList));//更新信息
            //头像名单不为空去掉末尾的问号
            if(!headPhotoList.isEmpty())
                headPhotoList.chop(1);
            //先发头像再发信息
            emit sendToServer(key,InfoType::AllHeadPhoto,"",account,QByteArray(),"",headPhotoList);
            replyJson(key,type,"","","",account,fileName);//发送信息
            break;
        }
        case SearchGroup:
        {
            //读取json信息
            QString account=jsonData.value("account").toString();
            QString message=jsonData.value("message").toString();
            QString fileName=Protocol::getWorkPath()+"/"+account+"/searchGroup.json";
            QString headPhotoList="";
            updateFile(fileName,sql.searchGroup(message,headPhotoList));//更新信息
            //头像名单不为空去掉末尾的问号
            if(!headPhotoList.isEmpty())
                headPhotoList.chop(1);
            //先发头像再发信息
            emit sendToServer(key,InfoType::AllHeadPhoto,"",account,QByteArray(),"",headPhotoList);
            replyJson(key,type,"","","",account,fileName);//发送信息
            break;
        }
        case AskForGroupData:
        {
            //读取json信息
            QString account=jsonData.value("account").toString();
            QString message=jsonData.value("message").toString();
            QString fileName=Protocol::getWorkPath()+"/"+account+"/askForGroupData.json";
            QString headPhotoList="";
            updateFile(fileName,QJsonDocument(sql.askForGroupData(message,headPhotoList)).toJson());//更新信息
            //头像名单不为空去掉末尾的问号
            if(!headPhotoList.isEmpty())
                headPhotoList.chop(1);
            //先发头像再发信息
            emit sendToServer(key,InfoType::AllHeadPhoto,"",account,QByteArray(),"",headPhotoList);
            replyJson(key,type,"","","",account,fileName);//发送信息
            break;
        }
        case AskForUserData:
        {
            //读取json信息
            QString account=jsonData.value("account").toString();
            QString message=jsonData.value("message").toString();
            QString fileName=Protocol::getWorkPath()+"/"+account+"/askForUserData.json";
            QString headPhotoList="";
            updateFile(fileName,QJsonDocument(sql.askForUserData(message,headPhotoList)).toJson());//更新信息
            //头像名单不为空去掉末尾的问号
            if(!headPhotoList.isEmpty())
                headPhotoList.chop(1);
            //先发头像再发信息
            emit sendToServer(key,InfoType::AllHeadPhoto,"",account,QByteArray(),"",headPhotoList);
            replyJson(key,type,"","","",account,fileName);//发送信息
            break;
        }
        case AddFriend:
        case JoinGroup:
        {
            //读取json信息
            QString sender=jsonData.value("sender").toString();//发送者
            QString receiver=jsonData.value("receiver").toString();//接收者
            qint64 time=jsonData.value("time").toString().toLongLong();//时间戳
            QString message=jsonData.value("message").toString();//消息：apply或者agree
            QString messageType=jsonData.value("messageType").toString();//群号(如果有的话)
            //直接在本地处理
            if(message=="agree")
            {
                if(type==AddFriend)
                {
                    //更新数据库信息
                    sql.addFriend(sender,receiver,time);
                    //判断双方是否在线，在线则发送新的文件，并通知对方更新
                    //若不在线，下次登录发现信息超时，也会自动更新
                    if(sql.isUserOnLine(sender))
                    {
                        QString file=Protocol::getWorkPath()+"/"+sender+"/friend.json";
                        updateFile(file,QJsonDocument(sql.getFriendDataFile(sender)).toJson());
                        replyJson(key,InfoType::UpdateFriend,"","","",sender,file);
                    }
                    if(sql.isUserOnLine(receiver))
                    {
                        QString file=Protocol::getWorkPath()+"/"+receiver+"/friend.json";
                        updateFile(file,QJsonDocument(sql.getFriendDataFile(receiver)).toJson());
                        replyJson(key,InfoType::UpdateFriend,"","","",receiver,file);
                    }
                }
                else
                {
                    //更新数据库信息
                    sql.joinGroup(receiver,messageType,time);
                    //发送群聊信息
                    QStringList list=sql.getAllMember(messageType);
                    for(int i=0;i<list.size();i++)
                    {
                        //更新所有成员的relation信息
                        sql.setRelationUpdateTime(list[i],time);
                        //在线的成员更新group文件，并发送，然后通知客户端更新
                        if(sql.isUserOnLine(list[i]))
                        {
                            QString file=Protocol::getWorkPath()+"/"+list[i]+"/group.json";
                            QString headPhoto="";
                            updateFile(file,sql.getGroupDataFile(list[i],headPhoto));
                            if(!headPhoto.isEmpty())
                            {
                                headPhoto.chop(1);
                                emit sendToServer("",InfoType::AllHeadPhoto,"",list[i],"","",headPhoto);//发送头像
                            }
                            replyJson(key,InfoType::UpdateGroup,"","","",list[i],file);//发送通知
                        }
                    }
                }
            }
            //转发
            else
            {
                if(sql.isUserOnLine(receiver))
                {
                    if(!sql.getUserHeadPhoto(sender).isEmpty())
                    {
                        emit sendToServer(key,InfoType::AllHeadPhoto,sender,receiver,QByteArray(),"",Protocol::getAllUserPath()+"/"+sql.getUserHeadPhoto(sender));
                    }
                    if(type==AddFriend)
                    {
                        emit sendToServer(key,type,sender,receiver,QJsonDocument(sql.taskToAddFriend(jsonData)).toJson(),"","");
                    }
                    else
                    {
                        emit sendToServer(key,type,sender,receiver,QJsonDocument(sql.taskToJoinGroup(jsonData)).toJson(),"","");
                    }
                }
                else
                {
                    sql.addMessageTask(jsonData);
                }
            }
            break;
        }
        case SendDynamic:
        {
            sendDynamic(jsonData);
            break;
        }
        case CreateGroup:
        {
            //读取json信息
            QString sender=jsonData.value("sender").toString();//发送者
            QString receiver=jsonData.value("receiver").toString();//接收者
            qint64 time=jsonData.value("time").toString().toLongLong();//时间戳
            QString message=jsonData.value("message").toString();//消息：apply或者agree
            QString groupAccount=sql.createGroup(message,sender,time);
            QStringList list=receiver.split(";");
            for(int i=0;i<list.size();i++)
            {
                sql.joinGroup(list[i],groupAccount,time);
                if(sql.isUserOnLine(list[i]))
                {
                    QString headPhoto="";
                    QString file=Protocol::getWorkPath()+"/"+list[i]+"/group.json";
                    updateFile(file,sql.getGroupDataFile(list[i],headPhoto));
                    if(!headPhoto.isEmpty())
                    {
                        headPhoto.chop(1);
                        emit sendToServer("",InfoType::AllHeadPhoto,"",list[i],"","",headPhoto);//发送头像
                    }
                    replyJson(key,InfoType::UpdateGroup,"","","",list[i],file);//发送通知
                }
            }
            if(sql.isUserOnLine(sender))
            {
                QString headPhoto="";
                QString file=Protocol::getWorkPath()+"/"+sender+"/group.json";
                updateFile(file,sql.getGroupDataFile(sender,headPhoto));
                if(!headPhoto.isEmpty())
                {
                    headPhoto.chop(1);
                    emit sendToServer("",InfoType::AllHeadPhoto,"",sender,"","",headPhoto);//发送头像
                }
                replyJson(key,InfoType::UpdateGroup,"","","",sender,file);//发送通知
            }
            break;
        }
        case ModifyUserData:
        {
            //读取登录信息
            QString account=jsonData.value("account").toString();
            bool remember=jsonData.value("remember").toBool();
            bool autoLogin=jsonData.value("autoLogin").toBool();
            qint64 time=jsonData.value("updateTime").toString().toLongLong();
            //更新个人信息
            sql.modifyUserData(account,jsonData);
            QString file=Protocol::getWorkPath()+"/"+account+"/login.json";
            //更新login文件
            updateFile(file,QJsonDocument(sql.getUserDateFile(account,remember,autoLogin)).toJson());
            file=sql.getUserHeadPhoto(account);
            if(!file.isEmpty())
            {
                file=Protocol::getAllUserPath()+"/"+file;
                emit sendToServer(key,InfoType::AllHeadPhoto,"",account,QByteArray(),"",file);
            }
            //更新login表并且让客户端重读
            emit sendToServer(key,InfoType::LoginAccount,"",account,QByteArray(),"",Protocol::getWorkPath()+"/"+account+"/login.json");
            emit replyJson(key,InfoType::UpdateUserData,"","","",account,"");

            //更新所有好友的联系人更新时间
            QStringList list=sql.getAllFriend(account);
            for(int i=0;i<list.size();i++)
            {
                sql.setRelationUpdateTime(list[i],time);
            }
            break;
        }
        case ModifyGroupData:
        {
            //读取登录信息
            QString account=jsonData.value("account").toString();
            qint64 time=jsonData.value("updateTime").toString().toLongLong();
            sql.modifyGroupData(account,jsonData);
            QStringList list=sql.getAllMember(account);
            for(int i=0;i<list.size();i++)
            {
                //只修改relation表的更新时间，等他们下次上线自己获取
                sql.setRelationUpdateTime(list[i],time);
            }
            //发回给修改者（群主）
            QString headPhoto="";
            QString sender=sql.getGroupHostAccount(account);
            QString file=Protocol::getWorkPath()+"/"+sender+"/group.json";
            updateFile(file,sql.getGroupDataFile(sender,headPhoto));
            if(!headPhoto.isEmpty())
            {
                headPhoto.chop(1);
                //emit sendToServer("",InfoType::AllHeadPhoto,"",sender,"","",headPhoto);//发送头像
            }
            replyJson(key,InfoType::UpdateGroup,"","","",sender,file);//发送通知
            break;
        }
        default:
            break;
    }
}

void WorkThread::replyJson(QString key, int type, QString message, QString messageType, QString account, QString targetAccount, QString fileName)
{
    QJsonObject jsonData;
    jsonData.insert("type",type);
    switch(type)
    {
        case Reconnection:
        case Registration:
        case LoginAccount:
        {
            jsonData.insert("result",message);
            break;
        }
        default:
            break;
    }
    QJsonDocument jsonDocument(jsonData);
    QByteArray dataArray=jsonDocument.toJson();
    emit sendToServer(key,type,account,targetAccount,dataArray,messageType,fileName);
}

void WorkThread::updateFile(QString fileName, QByteArray data)
{
    QFile file(fileName);
    if(file.open(QFile::WriteOnly))
    {
        file.write(data);
        file.close();
    }
}

void WorkThread::openMessageFile(QString sender, QString receiver)
{
    QJsonArray* json=new QJsonArray();
    messageArray[sender].insert(receiver,json);
}

void WorkThread::addMessage(QString sender,QString receiver,QJsonObject json)
{
    //不存在对应的消息记录映射表则创建
    if(!messageArray.value(sender).contains(receiver))
        openMessageFile(sender,receiver);
    //添加消息
    messageArray.value(sender).value(receiver)->append(json);
}

QJsonObject WorkThread::messageTaskToMessage(QJsonObject json)
{
    QJsonObject message;
    QString sender=json.value("sender").toString();
    if(sender[0]=='G')//如果发送方是群聊，那么真实的发送者是messageType
        sender=json.value("messageType").toString();
    message.insert("account",sender);
    message.insert("message",json.value("message").toString());
    message.insert("time",json.value("time").toString());
    return message;
}

void WorkThread::sendGroupMessage(QJsonObject json)
{
    QString sender=json.value("sender").toString();//用户
    QString receiver=json.value("receiver").toString();//群号
    QStringList list=sql.getAllMember(receiver);
    qint64 time=json.value("time").toString().toLongLong();//时间戳/任务号
    for (int i = 0; i<list.size(); i++)
    {
        QJsonObject temp;
        if(list[i]==sender)//不会发给自己
            continue;
        else
        {
            //将真实的发送者替换成群号，方便接收，真实的发送者是messageType
            temp["sender"]=json.value("receiver").toString();
            temp["receiver"]=list[i];
            temp["message"]=json.value("message").toString();
            temp["messageType"]=json.value("sender").toString();
            temp["file"]=json.value("file");
            temp["type"]=json.value("type");
            if(sql.isUserOnLine(list[i]))
            {
                temp["time"]=QString::number(time);
                //发送消息并添加进接收者的缓存中
                emit sendToServer("",InfoType::SendMessage,"",list[i],QJsonDocument(temp).toJson(),"","");
                addMessage(temp.value("receiver").toString(),temp.value("sender").toString(),messageTaskToMessage(temp));
            }
            else
            {
                //time是任务表的主键，不能重复
                time++;
                temp["time"]=QString::number(time);
                //不在则将消息转成任务存储到mysql任务表中，等待接收者上线后转发
                sql.addMessageTask(temp);
            }
        }
    }
}

void WorkThread::sendDynamic(QJsonObject json)
{
    QString sender=json.value("sender").toString();//用户
    qint64 time=json.value("time").toString().toLongLong();
    //dynamicArray[sender]->append(sql.taskToSendDynamic(json));//放入该用户动态缓存中
    QStringList list=sql.getAllFriend(sender);
    for(int i=0;i<list.size();i++)
    {
        QJsonObject temp=json;
        //在线则发送
        if(sql.isUserOnLine(list[i]))
        {
            //转成客户端接受的格式发送
            temp=sql.taskToSendDynamic(json);
            sendToServer("",InfoType::SendDynamic,sender,list[i],QJsonDocument(temp).toJson(),"","");
            //移除垃圾数据并加入接收者的动态缓存中
            temp.remove("type");
            dynamicArray[list[i]]->append(temp);
        }
        else
        {
            time++;
            temp["time"]=QString::number(time);
            //更换接受者，并存放进任务表中
            temp["receiver"]=list[i];
            sql.addMessageTask(temp);
        }
    }

    //移除垃圾数据并加入发送者的动态缓存中
    json=sql.taskToSendDynamic(json);
    json.remove("type");
    dynamicArray[sender]->append(json);
}

void WorkThread::sendMessageFile(QString account, qint64 lastLoginTime)
{
    if(!sql.judgeLastLoginTimeOfUser(account,lastLoginTime))
    {
        QDir dir;
        dir.setPath(Protocol::getWorkPath()+"/"+account+"/message");
        QStringList message=dir.entryList(QDir::Dirs);
        message.removeOne(".");
        message.removeOne("..");
        QVector<qint64> timeSort;
        for(QString i:message)
        {
            timeSort.push_back(i.toLongLong());
        }
        std::sort(timeSort.begin(),timeSort.end());
        for(qint64 num:timeSort)
        {
            if(num<=lastLoginTime)
                continue;
            else
            {
                QDir dir;
                dir.setPath(Protocol::getWorkPath()+"/"+account+"/message/"+QString::number(num));
                QStringList allFile=dir.entryList(QDir::Files);
                allFile.removeOne(".");
                allFile.removeOne("..");
                QString files="";
                for(QString file:allFile)
                {
                    files+=dir.path()+"/"+file;
                    files+="?";
                }
                files.chop(1);
                emit sendToServer("",InfoType::HistoryMessage,"",account,QByteArray(),QString::number(num),files);
            }
        }
        //发送完所有历史消息文件后发送一个json数据包通知客户端读取历史消息数据
        QJsonObject json;
        json.insert("type",InfoType::HistoryMessage);
        emit sendToServer("",InfoType::HistoryMessage,"",account,QJsonDocument(json).toJson(),"","");
    }
}

void WorkThread::sendDynamicFile(QString account, qint64 lastLoginTime)
{
    if(!sql.judgeLastLoginTimeOfUser(account,lastLoginTime))
    {
        QDir dir;
        dir.setPath(Protocol::getWorkPath()+"/"+account+"/dynamic");
        QStringList list=dir.entryList(QDir::Files);
        list.removeOne(".");
        list.removeOne("..");
        QVector<qint64> timeSort;
        for(QString i:list)
        {
            timeSort.push_back(i.split(".").first().toLongLong());
        }
        std::sort(timeSort.begin(),timeSort.end());
        QString files="";
        for(qint64 num:timeSort)
        {
            if(num<=lastLoginTime)
                continue;
            else
            {
                files+=dir.path()+"/"+QString::number(num)+".json";
                files+="?";
            }
        }
        if(files.isEmpty())
            return;
        files.chop(1);
        emit sendToServer("",InfoType::HistoryDynamic,"",account,QByteArray(),"",files);
        //发送完所有历史动态文件后发送一个json数据包通知客户端读取历史动态数据
        QJsonObject json;
        json.insert("type",InfoType::HistoryDynamic);
        emit sendToServer("",InfoType::HistoryDynamic,"",account,QJsonDocument(json).toJson(),"","");
    }
}

void WorkThread::sendMessageTask(QString account)
{
    QVector<QJsonObject> array=sql.sendMessageTask(account);
    //遍历发送消息
    for(QJsonObject json: array)
    {
        QString sender=json.value("sender").toString();
        QString receiver=json.value("receiver").toString();
        int type=json.value("type").toInt();
        //存储到消息缓存中,记得先将messageTask转成message再存储
        if(type==InfoType::SendMessage)
        {
            emit sendToServer("",type,sender,receiver,QJsonDocument(json).toJson(),"","");
            addMessage(receiver,sender,messageTaskToMessage(json));
        }
        else if(type==InfoType::SendDynamic)
        {
            QJsonObject temp=sql.taskToSendDynamic(json);
            emit sendToServer("",type,sender,receiver,QJsonDocument(temp).toJson(),"","");
            temp.remove("type");
            dynamicArray[receiver]->append(temp);
        }
        else if(type==InfoType::AddFriend)
        {
            QJsonObject temp=sql.taskToAddFriend(json);
            emit sendToServer("",type,sender,receiver,QJsonDocument(temp).toJson(),"","");
        }
        else if(type==InfoType::JoinGroup)
        {
            QJsonObject temp=sql.taskToJoinGroup(json);
            emit sendToServer("",type,sender,receiver,QJsonDocument(temp).toJson(),"","");
        }
        else
        {

        }
    }
}



void WorkThread::userDownLine(QString account)
{
    //先将用户的状态设置为下线
    sql.userDownLine(account);

    //然后根据用户登录时间查看是否有消息需要存储
    QString loginTime;
    qint64 temp=sql.getUserLoginTime(account);
    //temp小于0代表sql出现错误，直接返回
    if(temp<0)
        return;
    else
        loginTime=QString::number(temp);
    //查看是否有消息通信
    if(messageArray.value(account).size()>0)
    {
        //以登录时间戳作为文件夹名创建
        QDir dir;
        QString path=Protocol::getWorkPath()+"/"+account+"/message/"+loginTime;
        dir.mkdir(path);
        qDebug()<<path;
        //遍历写入所有与好友的通信消息，文件名：好友账号.json
        for(QString friendAccount:messageArray[account].keys())
        {
            QFile file(path+"/"+friendAccount+".json");
            file.open(QFile::WriteOnly|QFile::Truncate);
            file.write(QJsonDocument(*(messageArray[account].value(friendAccount))).toJson());
            file.close();
            delete messageArray[account].value(friendAccount);//释放指针
        }
    }

    //清空消息缓存
    messageArray.remove(account);

    //查看是否有动态信息
    if(dynamicArray.value(account)->size()>0)
    {
        QFile file(Protocol::getWorkPath()+"/"+account+"/dynamic/"+loginTime+".json");
        file.open(QFile::WriteOnly);
        file.write(QJsonDocument((*dynamicArray[account])).toJson());
        file.close();
        delete dynamicArray[account];
    }

    //清空动态缓存
    dynamicArray.remove(account);
}

void WorkThread::userRegister(QString account, QString password)
{
    //往数据库写入新用户信息
    sql.userRegister(account,password);

    //创建用户文件夹
    QDir dir;
    dir.setPath(Protocol::getWorkPath());
    if(!dir.exists(dir.path()+"/"+account))
    {
        if(!dir.mkdir(dir.path()+"/"+account))
        {
            qDebug()<<"创建用户"<<account<<"文件夹失败";
        }
        else
        {
            if(!dir.mkdir(dir.path()+"/"+account+"/message"))
                qDebug()<<"创建用户"<<account<<"文件夹下的message文件夹失败";
            if(!dir.mkdir(dir.path()+"/"+account+"/FileRecv"))
                qDebug()<<"创建用户"<<account<<"文件夹下的FileRecv文件夹失败";
            if(!dir.mkdir(dir.path()+"/"+account+"/dynamic"))
                qDebug()<<"创建用户"<<account<<"文件夹下的dynamic文件夹失败";
        }
    }
}

