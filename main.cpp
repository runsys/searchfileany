#include <QCoreApplication>
#include <QRegularExpression>
#include <QDir>
#include <QFile>
#include <iostream>
#include <string>
#include <QDateTime>
#include <QThread>
#include <QDebug>

#ifdef _MSC_VER
#pragma comment(lib,"M:\\work\\tool\\Qt5.14.2\\5.14.2\\msvc2017_64\\lib\\Qt5Core.lib")
#endif

QString RegexReplace(QString source,QRegularExpressionMatchIterator &pama, QString replacewithstr){
    QString newpath;
    int macnt=0;
    int preend=0;
    for(;pama.hasNext();){
        macnt+=1;
        auto ma=pama.next();
        auto start=ma.capturedStart();
        auto end=ma.capturedEnd();
        if(end-start==0){
            continue;
        }
        newpath+=source.mid(preend,start-preend);
        preend=end;
        //qDebug()<<"newpath after cap:"<<newpath<<"start:"<<start<<" end:"<<end;

        auto replacewithstr2=replacewithstr;
        QString replacewithstr3;
        for(int ri=0;ri<replacewithstr2.size();ri++){
            //qDebug()<<"ri"<<ri;
            if(ri+1<replacewithstr2.size() && replacewithstr2[ri]=='\\' && (replacewithstr2[ri+1].isDigit() || replacewithstr2[ri+1]=='\\'  || replacewithstr2[ri+1]=='#' || replacewithstr2[ri+1]=='%' || replacewithstr2[ri+1]=='n'  || replacewithstr2[ri+1]=='r' || replacewithstr2[ri+1]=='t'  )){
                if(replacewithstr2[ri+1].isDigit()){
                    replacewithstr3.push_back(replacewithstr2[ri+1]);
                }else if(replacewithstr2[ri+1]=='n'){
                    replacewithstr3.push_back('\n');
                }else if(replacewithstr2[ri+1]=='r'){
                    replacewithstr3.push_back('\r');
                }else if(replacewithstr2[ri+1]=='t'){
                    replacewithstr3.push_back('\t');
                }else if(replacewithstr2[ri+1]=='\\'){
                    replacewithstr3.push_back('\\');
                }else if(replacewithstr2[ri+1]=='%'){
                    replacewithstr3.push_back('%');
                }else if(replacewithstr2[ri+1]=='#'){
                    replacewithstr3.push_back('#');
                }
                ri+=1;
            }else if(ri+2<replacewithstr2.size() && replacewithstr2[ri]=='$' && replacewithstr2[ri+1].isDigit() && replacewithstr2[ri+2].isDigit()){
                QString matag=replacewithstr2.mid(ri,3);
                auto mastr=ma.captured(matag.mid(1).toInt());
                //qDebug()<<"matag:"<<matag<<" mastr"<<mastr;
                replacewithstr3.push_back(mastr);
                ri+=2;
            }else if(ri+1<replacewithstr2.size() && replacewithstr2[ri]=='$' && replacewithstr2[ri+1].isDigit()){
                QString matag=replacewithstr2.mid(ri,2);
                auto mastr=ma.captured(matag.mid(1).toInt());
                //qDebug()<<"matag:"<<matag<<" mastr"<<mastr;
                replacewithstr3.push_back(mastr);
                ri+=1;
            }else if(ri+2<replacewithstr2.size() && replacewithstr2[ri]=='#' && replacewithstr2[ri+1].isDigit() && replacewithstr2[ri+2].isDigit()){
                QString matag=replacewithstr2.mid(ri,3);
                auto mastr=ma.captured(matag.mid(1).toInt());
                //qDebug()<<"matag:"<<matag<<" mastr"<<mastr;
                replacewithstr3.push_back(mastr);
                ri+=2;
            }else if(ri+1<replacewithstr2.size() && replacewithstr2[ri]=='#' && replacewithstr2[ri+1].isDigit()){
                QString matag=replacewithstr2.mid(ri,2);
                auto mastr=ma.captured(matag.mid(1).toInt());
                //qDebug()<<"matag:"<<matag<<" mastr"<<mastr;
                replacewithstr3.push_back(mastr);
                ri+=1;
            }else{
                replacewithstr3.push_back(replacewithstr2[ri]);
            }
        }
        //qDebug()<<"replacewithstr3:"<<replacewithstr3;
        newpath.push_back(replacewithstr3.toUtf8());
    }
    if(macnt==0){
        newpath=source;
    }else{
        newpath.push_back(source.mid(preend));
    }
    return newpath;
}


bool MoveFile(QString oldpath,QString newpath) {
    qDebug()<<oldpath<<" "<<newpath;
    QFile ff(oldpath);
    ff.open(QFile::ReadOnly);
    auto ffsize=ff.size();

    QFile ff2(newpath);
    QString dirpa=newpath.replace(ff2.fileName(),"");
    if(dirpa=="")dirpa=".";
    QDir dir(dirpa);
    qDebug()<<dir.absolutePath();
    dir.mkpath(dir.absolutePath());
    ff2.open(QFile::WriteOnly|QFile::Truncate);
    for(int i=0;i<ffsize;i+=4*1024*1024){
        auto data=ff.read(4*1024*1024);
        if(data.size()==0)break;
        ff2.write(data);
    }
    ff.close();
    ff2.close();
    qDebug()<<oldpath<<" "<<newpath;
    return true;
}


int searchfileany(QStringList *results,QString rootdir,QString curdir,QRegularExpression *pathregex,QRegularExpression *filenameregex,QRegularExpression *contentregex,bool directoryonly,bool fileonly,bool showdetail,int maxdeep,int curdeep,QDateTime timebegin,QDateTime timeend,QString replacewithstr,bool replacewithnorename,QString newpathreplacewith,bool newpathnomove){
    if(curdeep>maxdeep)return 0;
    QDir fd(curdir);
    auto fdls=fd.entryInfoList();
    if(showdetail){
        std::cout<<"do search folder:"<<curdir.toStdString()<<" child file size:"<<fdls.size()<<std::endl;
    }
    for(int i=0;i<fdls.size();i++){
        auto fullpath=fdls[i].absoluteFilePath();
        //std::cout<<"fullpath"<<fullpath.toStdString()<<std::endl;
        if(fdls[i].fileName()=="."||fdls[i].fileName()==".."||curdir==fullpath)continue;
        //std::cout<<"fullpath"<<fullpath.toStdString()<<std::endl;
        if(pathregex==nullptr)continue;
        auto pama=pathregex->globalMatch(fullpath);
        if(newpathreplacewith==""){
            if(pama.hasNext()){
                if(fdls[i].isFile()){
                    if(timebegin.isValid() && fdls[i].fileTime(QFile::FileModificationTime).secsTo(timebegin)>0){
                        continue;
                    }
                    if(timeend.isValid() && fdls[i].fileTime(QFile::FileModificationTime).secsTo(timeend)<0){
                        continue;
                    }
                    if(showdetail){
                        std::cout<<"child file:"<<fullpath.toStdString()<<std::endl;
                    }
                    if(contentregex!=nullptr){
                        bool bnamepass=false;
                        if(filenameregex!=nullptr){
                            auto fnma=filenameregex->globalMatch(fdls[i].fileName());
                            if(fnma.hasNext()){
                                if(showdetail){
                                    std::cout<<"file name pass:"<<fullpath.toStdString()<<std::endl;
                                }
                                bnamepass=true;
                            }
                        }else{
                            bnamepass=true;
                        }
                        if(bnamepass){
                            QFile ff(fullpath);
                            ff.open(QFile::ReadOnly);
                            if(ff.isOpen()){
                                auto ffsize=ff.size();
                                QByteArray prerd;
                                bool bclose=false;
                                for(qint64 fi=0;fi<ffsize;fi+=32*1024*1024){
                                    auto curd=ff.read(32*1024*1024);
                                    if(prerd.size()>1024){
                                        curd=prerd.mid(prerd.size()-1024)+curd;
                                    }
                                    auto ffnma=contentregex->globalMatch(QString(curd));
                                    if (prerd.size()==0 && ffsize==curd.size() && replacewithstr!=""){
                                        bclose=true;
                                        ff.close();
                                        QByteArray newctt;
                                        int preend=0;
                                        int macnt=0;
                                        for(;ffnma.hasNext();){
                                            macnt+=1;
                                            auto ma=ffnma.next();
                                            auto start=ma.capturedStart();
                                            auto end=ma.capturedEnd();
                                            newctt+=curd.mid(preend,start-preend);
                                            preend=end;

                                            auto replacewithstr2=replacewithstr;
                                            QString replacewithstr3;
                                            for(int ri=0;ri<replacewithstr2.size();ri++){
                                                if(ri+1<replacewithstr2.size() && replacewithstr2[ri]=='\\' && (replacewithstr2[ri+1].isDigit() || replacewithstr2[ri+1]=='\\'  || replacewithstr2[ri+1]=='%' || replacewithstr2[ri+1]=='n'  || replacewithstr2[ri+1]=='r' || replacewithstr2[ri+1]=='t'  )){
                                                    if(replacewithstr2[ri+1].isDigit()){
                                                        replacewithstr3.push_back(replacewithstr2[ri+1]);
                                                    }else if(replacewithstr2[ri+1]=='n'){
                                                        replacewithstr3.push_back('\n');
                                                    }else if(replacewithstr2[ri+1]=='r'){
                                                        replacewithstr3.push_back('\r');
                                                    }else if(replacewithstr2[ri+1]=='t'){
                                                        replacewithstr3.push_back('\t');
                                                    }else if(replacewithstr2[ri+1]=='\\'){
                                                        replacewithstr3.push_back('\\');
                                                    }else if(replacewithstr2[ri+1]=='%'){
                                                        replacewithstr3.push_back('%');
                                                    }
                                                    ri+=1;
                                                }else if(ri+2<replacewithstr2.size() && replacewithstr2[ri]=='$' && replacewithstr2[ri+1].isDigit() && replacewithstr2[ri+2].isDigit()){
                                                    QString matag=replacewithstr2.mid(ri,3);
                                                    if(showdetail){
                                                        std::cout<<"matag:"<<matag.toStdString()<<std::endl;
                                                    }
                                                    auto mastr=ma.captured(matag.mid(1).toInt());
                                                    replacewithstr3.push_back(mastr);
                                                    ri+=2;
                                                }else if(ri+1<replacewithstr2.size() && replacewithstr2[ri]=='$' && replacewithstr2[ri+1].isDigit()){
                                                    QString matag=replacewithstr2.mid(ri,2);
                                                    if(showdetail){
                                                        std::cout<<"matag:"<<matag.toStdString()<<std::endl;
                                                    }
                                                    auto mastr=ma.captured(matag.mid(1).toInt());
                                                    replacewithstr3.push_back(mastr);
                                                    ri+=1;
                                                }else if(ri+2<replacewithstr2.size() && replacewithstr2[ri]=='#' && replacewithstr2[ri+1].isDigit() && replacewithstr2[ri+2].isDigit()){
                                                    QString matag=replacewithstr2.mid(ri,3);
                                                    if(showdetail){
                                                        std::cout<<"matag:"<<matag.toStdString()<<std::endl;
                                                    }
                                                    auto mastr=ma.captured(matag.mid(1).toInt());
                                                    replacewithstr3.push_back(mastr);
                                                    ri+=2;
                                                }else if(ri+1<replacewithstr2.size() && replacewithstr2[ri]=='#' && replacewithstr2[ri+1].isDigit()){
                                                    QString matag=replacewithstr2.mid(ri,2);
                                                    if(showdetail){
                                                        std::cout<<"matag:"<<matag.toStdString()<<std::endl;
                                                    }
                                                    auto mastr=ma.captured(matag.mid(1).toInt());
                                                    replacewithstr3.push_back(mastr);
                                                    ri+=1;
                                                }else{
                                                    replacewithstr3.push_back(replacewithstr2[ri]);
                                                }
                                            }

                                            newctt.push_back(replacewithstr3.toUtf8());

                                        }
                                        if(macnt==0){
                                            newctt=curd;
                                        }else{
                                            newctt.push_back(curd.mid(preend));

                                            QFile ff2(fullpath+".sfanew");
                                            ff2.open(QFile::WriteOnly);
                                            if(ff2.isOpen()){
                                                ff2.write(newctt);
                                                ff2.close();
                                                if(replacewithnorename==false){
                                                    ff.remove();
                                                    auto rnrl=ff2.rename(fullpath);
                                                    if(showdetail){
                                                        std::cout<<"rename result:"<<rnrl<<std::endl;
                                                    }
                                                }
                                            }
                                            std::cout<<fullpath.toStdString()<<std::endl;
                                            results->push_back(fullpath);
                                        }
                                        prerd=curd;
                                        break;
                                    }else {
                                        if(ffnma.hasNext()){
                                            if(directoryonly==false || fileonly==true){
                                                if(showdetail){
                                                    std::cout<<"found file:"<<fullpath.toStdString()<<std::endl;
                                                }
                                                std::cout<<fullpath.toStdString()<<std::endl;
                                                results->push_back(fullpath);
                                            }
                                            bclose=true;
                                            ff.close();
                                            break;
                                        }
                                        prerd=curd;
                                    }
                                }
                                if(bclose==false)ff.close();
                            }
                        }
                    }else{
                        if(directoryonly==false || fileonly==true){
                            if(filenameregex!=nullptr){
                                auto fnma=filenameregex->globalMatch(fdls[i].fileName());
                                if(fnma.hasNext()){
                                    if(showdetail){
                                        std::cout<<"found name pass file:"<<fullpath.toStdString()<<std::endl;
                                    }
                                    std::cout<<fullpath.toStdString()<<std::endl;
                                    results->push_back(fullpath);
                                }
                            }else{
                                if(showdetail){
                                    std::cout<<"found file:"<<fullpath.toStdString()<<std::endl;
                                }
                                std::cout<<fullpath.toStdString()<<std::endl;
                                results->push_back(fullpath);
                            }
                        }
                    }
                }else{
                    if(showdetail){
                        std::cout<<"child folder:"<<fullpath.toStdString()<<std::endl;
                    }
                    if(fileonly==false || directoryonly==true){
                        if(filenameregex!=nullptr){
                            auto fnma=filenameregex->globalMatch(fdls[i].fileName());
                            if(fnma.hasNext()){
                                if(showdetail){
                                    std::cout<<"found name pass folder:"<<fullpath.toStdString()<<std::endl;
                                }
                                std::cout<<fullpath.toStdString()<<std::endl;
                                results->push_back(fullpath);
                            }
                        }else{
                            if(showdetail){
                                std::cout<<"found folder:"<<fullpath.toStdString()<<std::endl;
                            }
                            std::cout<<fullpath.toStdString()<<std::endl;
                            results->push_back(fullpath);
                        }
                    }

                    searchfileany(results,rootdir,fullpath,pathregex,filenameregex, contentregex,directoryonly,fileonly,showdetail,maxdeep,curdeep+1,timebegin,timeend,replacewithstr,replacewithnorename,newpathreplacewith,newpathnomove);
                }
            }
        }else{
            if(pama.hasNext()){
                if(fdls[i].isFile()){
                    if(timebegin.isValid() && fdls[i].fileTime(QFile::FileModificationTime).secsTo(timebegin)>0){
                        continue;
                    }
                    if(timeend.isValid() && fdls[i].fileTime(QFile::FileModificationTime).secsTo(timeend)<0){
                        continue;
                    }
                    if(showdetail){
                        std::cout<<"child file:"<<fullpath.toStdString()<<std::endl;
                    }
                    if(contentregex!=nullptr){
                        bool bnamepass=false;
                        if(filenameregex!=nullptr){
                            auto fnma=filenameregex->globalMatch(fdls[i].fileName());
                            if(fnma.hasNext()){
                                if(showdetail){
                                    std::cout<<"file name pass:"<<fullpath.toStdString()<<std::endl;
                                }
                                bnamepass=true;
                            }
                        }else{
                            bnamepass=true;
                        }
                        if(bnamepass){
                            QFile ff(fullpath);
                            ff.open(QFile::ReadOnly);
                            if(ff.isOpen()){
                                auto ffsize=ff.size();
                                QByteArray prerd;
                                bool bclose=false;
                                for(qint64 fi=0;fi<ffsize;fi+=32*1024*1024){
                                    auto curd=ff.read(32*1024*1024);
                                    if(prerd.size()>1024){
                                        curd=prerd.mid(prerd.size()-1024)+curd;
                                    }
                                    auto ffnma=contentregex->globalMatch(QString(curd));
                                    if (prerd.size()==0 && ffsize==curd.size() && replacewithstr!=""){
                                        bclose=true;
                                        ff.close();
                                        QByteArray newctt;
                                        int preend=0;
                                        int macnt=0;
                                        for(;ffnma.hasNext();){
                                            macnt+=1;
                                            auto ma=ffnma.next();
                                            auto start=ma.capturedStart();
                                            auto end=ma.capturedEnd();
                                            newctt+=curd.mid(preend,start-preend);
                                            preend=end;

                                            auto replacewithstr2=replacewithstr;
                                            QString replacewithstr3;
                                            for(int ri=0;ri<replacewithstr2.size();ri++){
                                                if(ri+1<replacewithstr2.size() && replacewithstr2[ri]=='\\' && (replacewithstr2[ri+1].isDigit() || replacewithstr2[ri+1]=='\\'  || replacewithstr2[ri+1]=='%' || replacewithstr2[ri+1]=='n'  || replacewithstr2[ri+1]=='r' || replacewithstr2[ri+1]=='t'  )){
                                                    if(replacewithstr2[ri+1].isDigit()){
                                                        replacewithstr3.push_back(replacewithstr2[ri+1]);
                                                    }else if(replacewithstr2[ri+1]=='n'){
                                                        replacewithstr3.push_back('\n');
                                                    }else if(replacewithstr2[ri+1]=='r'){
                                                        replacewithstr3.push_back('\r');
                                                    }else if(replacewithstr2[ri+1]=='t'){
                                                        replacewithstr3.push_back('\t');
                                                    }else if(replacewithstr2[ri+1]=='\\'){
                                                        replacewithstr3.push_back('\\');
                                                    }else if(replacewithstr2[ri+1]=='%'){
                                                        replacewithstr3.push_back('%');
                                                    }
                                                    ri+=1;
                                                }else if(ri+2<replacewithstr2.size() && replacewithstr2[ri]=='$' && replacewithstr2[ri+1].isDigit() && replacewithstr2[ri+2].isDigit()){
                                                    QString matag=replacewithstr2.mid(ri,3);
                                                    if(showdetail){
                                                        std::cout<<"matag:"<<matag.toStdString()<<std::endl;
                                                    }
                                                    auto mastr=ma.captured(matag.mid(1).toInt());
                                                    replacewithstr3.push_back(mastr);
                                                    ri+=2;
                                                }else if(ri+1<replacewithstr2.size() && replacewithstr2[ri]=='$' && replacewithstr2[ri+1].isDigit()){
                                                    QString matag=replacewithstr2.mid(ri,2);
                                                    if(showdetail){
                                                        std::cout<<"matag:"<<matag.toStdString()<<std::endl;
                                                    }
                                                    auto mastr=ma.captured(matag.mid(1).toInt());
                                                    replacewithstr3.push_back(mastr);
                                                    ri+=1;
                                                }else if(ri+2<replacewithstr2.size() && replacewithstr2[ri]=='#' && replacewithstr2[ri+1].isDigit() && replacewithstr2[ri+2].isDigit()){
                                                    QString matag=replacewithstr2.mid(ri,3);
                                                    if(showdetail){
                                                        std::cout<<"matag:"<<matag.toStdString()<<std::endl;
                                                    }
                                                    auto mastr=ma.captured(matag.mid(1).toInt());
                                                    replacewithstr3.push_back(mastr);
                                                    ri+=2;
                                                }else if(ri+1<replacewithstr2.size() && replacewithstr2[ri]=='#' && replacewithstr2[ri+1].isDigit()){
                                                    QString matag=replacewithstr2.mid(ri,2);
                                                    if(showdetail){
                                                        std::cout<<"matag:"<<matag.toStdString()<<std::endl;
                                                    }
                                                    auto mastr=ma.captured(matag.mid(1).toInt());
                                                    replacewithstr3.push_back(mastr);
                                                    ri+=1;
                                                }else{
                                                    replacewithstr3.push_back(replacewithstr2[ri]);
                                                }
                                            }

                                            newctt.push_back(replacewithstr3.toUtf8());

                                        }
                                        if(macnt==0){
                                            newctt=curd;
                                        }else{
                                            newctt.push_back(curd.mid(preend));

                                            QFile ff2(fullpath+".sfanew");
                                            ff2.open(QFile::WriteOnly);
                                            if(ff2.isOpen()){
                                                ff2.write(newctt);
                                                ff2.close();
                                                if(replacewithnorename==false){
                                                    ff.remove();
                                                    auto rnrl=ff2.rename(fullpath);
                                                    if(showdetail){
                                                        std::cout<<"rename result:"<<rnrl<<std::endl;
                                                    }
                                                }
                                            }
                                            std::cout<<fullpath.toStdString()<<std::endl;

                                            QString newpath=RegexReplace(fullpath,pama,newpathreplacewith);
                                            if(showdetail){
                                                std::cout<<"new path:"<<newpath.toStdString()<<std::endl;
                                            }
                                            if(fullpath!=newpath){
                                                if(newpathnomove==false){
                                                    MoveFile(fullpath,newpath);
                                                }
                                                std::cout<<fullpath.toStdString()<<" new path:"<<newpath.toStdString()<<std::endl;
                                            }else{
                                                std::cout<<fullpath.toStdString()<<std::endl;
                                            }
                                            results->push_back(fullpath);
                                        }
                                        prerd=curd;
                                        break;
                                    }else {
                                        if(ffnma.hasNext()){
                                            if(directoryonly==false || fileonly==true){
                                                if(showdetail){
                                                    std::cout<<"found file:"<<fullpath.toStdString()<<std::endl;
                                                }
                                                QString newpath=RegexReplace(fullpath,pama,newpathreplacewith);
                                                if(showdetail){
                                                    std::cout<<"new path:"<<newpath.toStdString()<<std::endl;
                                                }
                                                if(fullpath!=newpath){
                                                    if(newpathnomove==false){
                                                        MoveFile(fullpath,newpath);
                                                    }
                                                    std::cout<<fullpath.toStdString()<<" new path:"<<newpath.toStdString()<<std::endl;
                                                }else{
                                                    std::cout<<fullpath.toStdString()<<std::endl;
                                                }
                                                results->push_back(fullpath);
                                            }
                                            bclose=true;
                                            ff.close();
                                            break;
                                        }
                                        prerd=curd;
                                    }
                                }
                                if(bclose==false)ff.close();
                            }
                        }
                    }else{
                        if(directoryonly==false || fileonly==true){
                            if(filenameregex!=nullptr){
                                auto fnma=filenameregex->globalMatch(fdls[i].fileName());
                                if(fnma.hasNext()){
                                    if(showdetail){
                                        std::cout<<"found name pass file:"<<fullpath.toStdString()<<std::endl;
                                    }
                                    QString newpath=RegexReplace(fullpath,pama,newpathreplacewith);
                                    if(showdetail){
                                        std::cout<<"new path:"<<newpath.toStdString()<<std::endl;
                                    }
                                    if(fullpath!=newpath){
                                        if(newpathnomove==false){
                                            MoveFile(fullpath,newpath);
                                        }
                                        std::cout<<fullpath.toStdString()<<" new path:"<<newpath.toStdString()<<std::endl;
                                    }else{
                                        std::cout<<fullpath.toStdString()<<std::endl;
                                    }
                                    results->push_back(fullpath);
                                }
                            }else{
                                if(showdetail){
                                    std::cout<<"found file:"<<fullpath.toStdString()<<std::endl;
                                }
                                QString newpath=RegexReplace(fullpath,pama,newpathreplacewith);
                                if(showdetail){
                                    std::cout<<"new path:"<<newpath.toStdString()<<std::endl;
                                }
                                if(fullpath!=newpath){
                                    if(newpathnomove==false){
                                        MoveFile(fullpath,newpath);
                                    }
                                    std::cout<<fullpath.toStdString()<<" new path:"<<newpath.toStdString()<<std::endl;
                                }else{
                                    std::cout<<fullpath.toStdString()<<std::endl;
                                }

                                results->push_back(fullpath);
                            }
                        }
                    }
                }else{
                    if(showdetail){
                        std::cout<<"child folder:"<<fullpath.toStdString()<<std::endl;
                    }
                    if(fileonly==false || directoryonly==true){
                        if(filenameregex!=nullptr){
                            auto fnma=filenameregex->globalMatch(fdls[i].fileName());
                            if(fnma.hasNext()){
                                if(showdetail){
                                    std::cout<<"found name pass folder:"<<fullpath.toStdString()<<std::endl;
                                }
                                std::cout<<fullpath.toStdString()<<std::endl;
                                results->push_back(fullpath);
                            }
                        }else{
                            if(showdetail){
                                std::cout<<"found folder:"<<fullpath.toStdString()<<std::endl;
                            }
                            std::cout<<fullpath.toStdString()<<std::endl;
                            results->push_back(fullpath);
                        }
                    }

                    searchfileany(results,rootdir,fullpath,pathregex,filenameregex, contentregex,directoryonly,fileonly,showdetail,maxdeep,curdeep+1,timebegin,timeend,replacewithstr,replacewithnorename,newpathreplacewith,newpathnomove);
                }
            }
        }
    }
    return 0;
}
int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    if(a.arguments().size()==1){
        std::cout<<"help:\nsearchfileany folder [--pathregex= -pr=] [--filenameregex= -fr=] [--contentregex= -cr=] [--directoryonly -do] [--fileonly(default) -fo] [--dirandfile -df] [--deep=] [--timebegin=yyyy-MM-ddThh:mm] [--timeend=yyyy-MM-ddThh:mm] [--replacewith= match got $0-99 #0-99 -rw] [--replacewithnorename] [--newpathreplacewith= match got $0-99 #0-99 -nprw] [--newpathnomove]\n";
        return 0;
    }
    bool directoryonly=false,fileonly=true,showdetail=false,replacewithnorename=false,newpathnomove=false;
    QString pathrestr,filenamerestr,cttrestr,replacewithstr,newpathreplacewith;
    int deep=1<<30;
    QDateTime timebegin,timeend;
    for(int i=0;i<a.arguments().size();i+=1){
        if(a.arguments()[i].startsWith("--pathregex=")){
            pathrestr=a.arguments()[i].mid(strlen("--pathregex="));
        }else if(a.arguments()[i].startsWith("-pr=")){
            pathrestr=a.arguments()[i].mid(strlen("-pr="));
        }else if(a.arguments()[i].startsWith("--filenameregex=")){
            filenamerestr=a.arguments()[i].mid(strlen("--filenameregex="));
        }else if(a.arguments()[i].startsWith("-fr=")){
            filenamerestr=a.arguments()[i].mid(strlen("-fr="));
        }else if(a.arguments()[i].startsWith("--contentregex=")){
            cttrestr=a.arguments()[i].mid(strlen("--contentregex="));
        }else if(a.arguments()[i].startsWith("--deep=")){
            deep=a.arguments()[i].mid(strlen("--deep=")).toInt();
        }else if(a.arguments()[i].startsWith("--timebegin=")){
            timebegin=QDateTime::fromString(a.arguments()[i].mid(strlen("--timebegin=")),"yyyy-MM-ddThh:mm");
        }else if(a.arguments()[i].startsWith("--timeend=")){
            timeend=QDateTime::fromString(a.arguments()[i].mid(strlen("--timeend=")),"yyyy-MM-ddThh:mm");
        }else if(a.arguments()[i].startsWith("--replacewithstr=")){
            replacewithstr=a.arguments()[i].mid(strlen("--replacewith="));
        }else if(a.arguments()[i].startsWith("-rw=")){
            replacewithstr=a.arguments()[i].mid(strlen("-rw="));
        }else if(a.arguments()[i].startsWith("--newpathreplacewith=")){
            newpathreplacewith=a.arguments()[i].mid(strlen("--newpathreplacewith="));
        }else if(a.arguments()[i].startsWith("-nprw=")){
            newpathreplacewith=a.arguments()[i].mid(strlen("-nprw="));
        }else if(a.arguments()[i].startsWith("--directoryonly")){
            directoryonly=true;
        }else if(a.arguments()[i].startsWith("--newpathnomove")){
            newpathnomove=true;
        }else if(a.arguments()[i].startsWith("--replacewithnorename")){
            replacewithnorename=true;
        }else if(a.arguments()[i].startsWith("--fileonly")){
            fileonly=true;
        }else if(a.arguments()[i].startsWith("--showdetail")){
            showdetail=true;
        }else if(a.arguments()[i].startsWith("--dirandfile")){
            directoryonly=true;
            fileonly=true;
        }else if(a.arguments()[i].startsWith("-cr=")){
            cttrestr=a.arguments()[i].mid(strlen("-cr="));
        }else if(a.arguments()[i].startsWith("-do")){
            directoryonly=true;
        }else if(a.arguments()[i].startsWith("-fo")){
            fileonly=true;
        }else if(a.arguments()[i].startsWith("-sd")){
            showdetail=true;
        }else if(a.arguments()[i].startsWith("-df")){
            directoryonly=true;
            fileonly=true;
        }else if(a.arguments()[i].startsWith("-dp=")){
            deep=a.arguments()[i].mid(strlen("-dp=")).toInt();
        }else if(a.arguments()[i].startsWith("-tb=")){
            timebegin=QDateTime::fromString(a.arguments()[i].mid(strlen("-tb=")),"yyyy-MM-ddThh:mm");
        }else if(a.arguments()[i].startsWith("-te=")){
            timeend=QDateTime::fromString(a.arguments()[i].mid(strlen("-te=")),"yyyy-MM-ddThh:mm");
        }
    }
    if(deep<=0)deep=1;
    QStringList results;
    QDir sd(a.arguments()[1]);
    auto sdpath=sd.absolutePath();

    if(showdetail){
        std::cout<<"do search parameter path regex:"<<pathrestr.toStdString()<<std::endl<<"file name regex:"<<filenamerestr.toStdString()<<std::endl<<"content regex:"<<cttrestr.toStdString()<<std::endl<<"folder only:"<<directoryonly<<std::endl<<"file only:"<<fileonly<<std::endl<<"root folder:"<<sdpath.toStdString()<<std::endl<<"replace with:"<<replacewithstr.toStdString()<<std::endl<<"newpathreplacewith:"<<newpathreplacewith.toStdString()<<std::endl;
    }
    if(cttrestr!="" && pathrestr=="")pathrestr=".*";
    if(filenamerestr!="" && pathrestr=="")pathrestr=".*";
    QRegularExpression *pathre=nullptr,*filenamere=nullptr,*cttre=nullptr;
    if(pathrestr!=""){
        pathre=new QRegularExpression(pathrestr,QRegularExpression::MultilineOption|QRegularExpression::DotMatchesEverythingOption|QRegularExpression::CaseInsensitiveOption);
    }
    if(filenamerestr!=""){
        filenamere=new QRegularExpression(filenamerestr,QRegularExpression::MultilineOption|QRegularExpression::DotMatchesEverythingOption|QRegularExpression::CaseInsensitiveOption);
    }
    if(cttrestr!=""){
        cttre=new QRegularExpression(cttrestr,QRegularExpression::MultilineOption|QRegularExpression::DotMatchesEverythingOption|QRegularExpression::CaseInsensitiveOption);
    }
    if(showdetail){
        std::cout<<"search folder "<<sdpath.toStdString()<<" results:"<<std::endl;
    }
    searchfileany(&results,sdpath,sdpath,pathre,filenamere,cttre,directoryonly,fileonly,showdetail,deep,1,timebegin,timeend,replacewithstr,replacewithnorename,newpathreplacewith,newpathnomove);
    // for(int i=0;i<results.size();i++){
    //     std::cout<<results[i].toStdString()<<std::endl;
    // }
    if(showdetail){
        std::cout<<"total:"<<results.size()<<std::endl;
    }

    //return a.exec();
}
