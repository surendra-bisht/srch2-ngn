/*
 * MigrationManager.h
 *
 *  Created on: Jul 1, 2014
 *      Author: srch2
 */

#ifndef __MIGRATION_MIGRATIONMANAGER_H__
#define __MIGRATION_MIGRATIONMANAGER_H__

#include <vector>
#include <string>
#include <iostream>

#include "sharding/metadata_manager/Shard.h"
#include "transport/TransportManager.h"
//#include "sharding/ShardManager.h"

namespace srch2 {
namespace httpwrapper {


enum MIGRATION_STATUS {
	MM_STATUS_SUCCESS,
	MM_STATUS_FAILURE,  //e.g network failure or serialization failure
	MM_STATUS_ABORTED,  // aborted by SHM
	MM_STATUS_BUSY      // MM is busy. Try again later.
};


// migration status data structure
struct ShardMigrationStatus{
	unsigned srcOperationId;
    unsigned dstOperationId;
	NodeId sourceNodeId;
	NodeId destinationNodeId;
	boost::shared_ptr<Srch2Server> shard;
	MIGRATION_STATUS status;
	ShardMigrationStatus() {}
	void * serialize(void * buffer) const;
	unsigned getNumberOfBytes() const;
	void * deserialize(void * buffer);
	ShardMigrationStatus & operator=(const ShardMigrationStatus & status);
	ShardMigrationStatus(const ShardMigrationStatus & status);
};

// redistribution data structure
//struct Stats{
//	ShardId shardId;
//	unsigned nodeId;
//	unsigned recCount;
//};
//
//struct ShardDistributionStatus{
//	MIGRATION_STATUS status;
//	unsigned statsCount;
//	// for source this is destination stats and for the destination this is source stats
//	Stats statsArray[0];
//};

enum MIGRATION_STATE{

	MM_STATE_MIGRATION_BEGIN,

	//sender's state
	MM_STATE_INIT_ACK_WAIT,  MM_STATE_INIT_ACK_RCVD,
	MM_STATE_INFO_ACK_WAIT,  MM_STATE_INFO_ACK_RCVD,
	MM_STATE_COMPONENT_TRANSFERRED_ACK_RCVD, MM_STATE_SHARD_TRANSFERRED_ACK_RCVD,

	//Receiver's state
	MM_STATE_INIT_RCVD,  MM_STATE_INFO_WAIT,
	MM_STATE_INFO_RCVD,  MM_STATE_COMPONENT_RECEIVING,
	MM_STATE_SHARD_RCVD,
};

// Forward declarations
class ShardManager;
class TransportManager;
class Partitioner;

const short MM_MIGRATION_PORT_START = 53000;

// MM internal structure to
class MigrationSessionInfo {
public:
	ClusterShardId shardId;
	boost::shared_ptr<Srch2Server> shard;
	unsigned srcOperationId;
	unsigned dstOperationId;
	unsigned shardCompCount;
	unsigned shardCompSize;
	string shardCompName;

	MIGRATION_STATE status;

	unsigned beginTimeStamp;  // epoch value
	unsigned endTimeStamp;    // epoch value

	unsigned remoteNode;
	unsigned remoteAddr;
	short listeningPort;
};

// callback handler from TM
class MigrationManager;
class MMCallBackForTM : public CallBackHandler {
public:
	MMCallBackForTM(MigrationManager *migrationMgr);
	bool resolveMessage(Message * msg, NodeId node);
private:
	MigrationManager *migrationMgr;
};

class MigrationService {
public:
	MigrationService(MigrationManager *migrationMgr) { this->migrationMgr = migrationMgr; }
	void sendShard(ClusterShardId shardId, unsigned destinationNodeId);
	void receiveShard(ClusterShardId shardId, unsigned remoteNode);
private:
	MigrationManager *migrationMgr;
};

class MigrationManager {
	friend class MMCallBackForTM;
	friend class MigrationService;
public:
	/*
	 * 1. Nonblocking API for migrating shard. Notifies ShardManager when migration is done.
	 * 2. ShardManager must expose method "void Notify(ShardMigrationStatus &)"
	 */
	void migrateShard(unsigned uri, boost::shared_ptr<Srch2Server> shard, NodeId destinationNodeId,
			unsigned srcOperationId , unsigned dstOperationId);

	/*  NOT IMPLEMENTED
	 * 1. Non Blocking API. Notifies ShardManager when distribution is done.
	 * 2. ShardManager must expose method "void Notify(ShardDistributionStatus &)"
	 * 3. partitioner is used for deciding shardId/NodeId
	 */
	void reDistributeShard(ClusterShardId shardId, boost::shared_ptr<Srch2Server> shard, Partitioner *partitioner, unsigned destination);

	/*  NOT IMPLEMENTED
	 *  1. ShardManager on remote node provides Shard object to which migrated record should be assigned.
	 *  2. non-blocking API.
	 */
	void registerShardForMigratedRecords(ClusterShardId shardId, boost::shared_ptr<Srch2Server> shard);

	/*  NOT IMPLEMENTED
	 *  1. ShardManager should call this to stop accepting new records into the given shard.
	 *  2. MM releases shared pointer to the requested shard.
	 */
	void unRegisterShardForMigratedRecords(ClusterShardId shardId);

	/*  NOT IMPLEMENTED
	 * ShardManager should call this API to request aborting the migration of the given shardId.
	 * This is a non blocking API. When migration stops, MM notifies SHM via callback with migration
	 * status as ABORTED
	 */
	void stopMigration(ClusterShardId shardId);

	/*  NOT IMPLEMENTED
	 * ShardManager should call this API to request aborting the migration of the given shardId to
	 * the given destinatonNodeId only.
	 * This is a non blocking API. When migration stops, MM notifies SHM via callback with migration
	 * status as ABORTED
	 */
	void stopMigration(ClusterShardId shardId, unsigned destinatonNodeId);

	MigrationManager(ShardManager *shardManager, TransportManager *transport, ConfigManager *config);

	~MigrationManager();

private:
	// MM private functions
	int openTCPSendChannel(unsigned addr, short port);
	int openSendChannel();
	void openTCPReceiveChannel(int& , short&);
	void openReceiveChannel(int& , short&);
	void sendComponentInfoAndWaitForAck(const string& sessionKey);
	void sendComponentDoneMsg(const string& sessionKey);
	void sendInitMessageAck(const string& sessionKey);
	void sendInfoAckMessage(const string& sessionKey);
	int acceptTCPConnection(int tcpSocket , short receivePort);
	void doInitialHandShake(const string& sessionKey);
	string initMigrationSession(ClusterShardId shardId,unsigned srcOperationId,
			unsigned dstOperationId, unsigned remoteNode, unsigned shardCompCount);
	bool hasActiveSession(const ClusterShardId& shardId, unsigned node);
	bool hasActiveSession(const ClusterShardId& shardId, unsigned node, string& sessionKey);
	void sendMessage(unsigned destinationNodeId, Message *message);
	const CoreInfo_t *getIndexConfig(ClusterShardId shardId);
	void populateStatus(ShardMigrationStatus& status, unsigned srcOperationId,
			unsigned dstOperationId, unsigned destinationNodeId, boost::shared_ptr<Srch2Server> shard,
			MIGRATION_STATUS migrationResult);
	void notifySHMAndCleanup(string sessionKey, MIGRATION_STATUS migrationResult);
	// Hash function for key of type ShardId to be used by boost::unordered_map
	string getSessionKey( const ClusterShardId& shardId, unsigned node) const
	{
		stringstream ss;
		ss << shardId.coreId << shardId.partitionId << shardId.replicaId << node;
		return  ss.str();
	}

	Message * allocateMessage(unsigned size) {
		return MessageAllocator().allocateMessage(size);
	}

	void deAllocateMessage(Message *p) {
		MessageAllocator().deallocateByMessagePointer(p);
	}

	// MM private members
	int sendSocket;
	int receiveSocket;
	short receivePort;
	TransportManager * transport;
	ShardManager* shardManager;
	MMCallBackForTM *transportCallback;
	ConfigManager *configManager;
	boost::unordered_map<string, MigrationSessionInfo> migrationSession;
	enum SessionLockState {
		LOCKED,
		UNLOCKED
	};
	SessionLockState _sessionLock;
};

}
}
#endif /* __MIGRATION_MIGRATIONMANAGER_H__ */
