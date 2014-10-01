/*
 * DataConnectorThread.h
 *
 *  Created on: Jun 24, 2014
 *      Author: Chen Liu at SRCH2
 */

#ifndef __DATACONNECTORTHREAD_H__
#define __DATACONNECTORTHREAD_H__

#include <string>
#include "DataConnector.h"
#include "ServerInterfaceInternal.h"

//Arguments passed to the thread
struct ConnectorThreadArguments {
    ServerInterface* server;
    bool createNewIndexFlag;
    std::string sharedLibraryFullPath;
};

//Called by the pthread_create, create the database connector
void * spawnConnector(void *arg);

class DataConnectorThread {
public:
    //Create thread if interface built successfully.
    static void getDataConnectorThread(void * server);
    //The main function run by the thread, get connector and start listener.
    static void bootStrapConnector(ConnectorThreadArguments * connThreadArg);
    static void saveConnectorTimeStamp();
private:
    static bool checkIndexExistence(void * server);
    static void getDataConnector(std::string & path, void * pdlHandle,
            DataConnector *& connector);
    static void closeDataConnector(void * pdlHandle, DataConnector *& connector);
    static std::vector<DataConnector *> connectors;
};

#endif /* __DATACONNECTORTHREAD_H__ */
