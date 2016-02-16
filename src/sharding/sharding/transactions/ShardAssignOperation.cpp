/*
 * Copyright (c) 2016, SRCH2
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *    * Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *    * Redistributions in binary form must reproduce the above copyright
 *      notice, this list of conditions and the following disclaimer in the
 *      documentation and/or other materials provided with the distribution.
 *    * Neither the name of the SRCH2 nor the
 *      names of its contributors may be used to endorse or promote products
 *      derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL SRCH2 BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include "ShardAssignOperation.h"

#include "core/util/SerializationHelper.h"
#include "core/util/Assert.h"
#include "../../configuration/ShardingConstants.h"
#include "../metadata_manager/Cluster_Writeview.h"
#include "../ShardManager.h"
#include "../metadata_manager/DataShardInitializer.h"
#include "../state_machine/node_iterators/ConcurrentNotifOperation.h"
#include "./AtomicRelease.h"

namespace srch2is = srch2::instantsearch;
using namespace srch2is;
using namespace std;
namespace srch2 {
namespace httpwrapper {

ShardAssignOperation::ShardAssignOperation(const ClusterShardId & unassignedShard,
		ConsumerInterface * consumer):ProducerInterface(consumer), shardId(unassignedShard){
	ASSERT(this->getTransaction());
	this->locker = NULL;
	this->releaser = NULL;
	this->committer = NULL;
	this->successFlag = true;
	this->currentAction = PreStart;
	this->currentOpId = NodeOperationId(ShardManager::getCurrentNodeId(), OperationState::getNextOperationId());
}

ShardAssignOperation::~ShardAssignOperation(){
	if(this->locker != NULL){
		delete this->locker;
	}
	if(this->releaser != NULL){
		delete this->releaser;
	}
	if(this->committer != NULL){
		delete this->committer;
	}
}

void ShardAssignOperation::produce(){
	Logger::sharding(Logger::Step, "ShardAssign(opid=%s, shardId=%s)| Starting ...",
			currentOpId.toString().c_str(), shardId.toString().c_str());
	lock();
}

void ShardAssignOperation::lock(){ // ** start **
	this->locker = new AtomicLock(shardId , currentOpId, LockLevel_X, this);
	// locker calls all methods of LockResultCallbackInterface from us
	currentAction = Lock;
	this->locker->produce();
}
// for lock
void ShardAssignOperation::consume(bool granted){
	switch (currentAction) {
		case Lock:
			if(granted){
				if(checkStillValid()){
					commit();
				}else{
					this->successFlag = false;
					release();
				}
			}else{
				this->successFlag = false;
				finalize();
			}
			break;
		case Commit:
			if(! granted){
				this->successFlag = granted;
			}
			release();
			break;
		case Release:
			if(! granted){
				this->successFlag = granted;
			}
			finalize();
			break;
		default:
			ASSERT(false);
			if(! granted){
				this->successFlag = granted;
			}
			finalize();
			break;
	}
}
// ** if (granted)
void ShardAssignOperation::commit(){
	Logger::sharding(Logger::Detail, "ShardAssign(opid=%s, shardid=%s)| Committing shard assign change.",
			currentOpId.toString().c_str(), shardId.toString().c_str());
	// start metadata commit
	// prepare the shard change
	const Cluster_Writeview * writeview = ((WriteviewTransaction *)(this->getTransaction().get()))->getWriteview();
	string indexDirectory = ShardManager::getShardManager()->getConfigManager()->getShardDir(writeview->clusterName,
			writeview->cores.at(shardId.coreId)->getName(), &shardId);
	if(indexDirectory.compare("") == 0){
		indexDirectory = ShardManager::getShardManager()->getConfigManager()->createShardDir(writeview->clusterName,
				writeview->cores.at(shardId.coreId)->getName(), &shardId);
	}
	EmptyShardBuilder emptyShard(new ClusterShardId(shardId), indexDirectory);
	emptyShard.prepare(false);
	LocalPhysicalShard physicalShard(emptyShard.getShardServer(), emptyShard.getIndexDirectory(), "");
	// prepare the shard change
	ShardAssignChange * shardAssignChange = new ShardAssignChange(shardId, ShardManager::getCurrentNodeId());
	shardAssignChange->setPhysicalShard(physicalShard);
	currentAction = Commit;
	this->committer = new AtomicMetadataCommit(vector<NodeId>(), shardAssignChange,  this, true);
	this->committer->produce();
}
// ** end if
void ShardAssignOperation::release(){
	this->releaser = new AtomicRelease(shardId, currentOpId , this);
	// release the locks
	Logger::sharding(Logger::Detail, "ShardAssign(opid=%s, shardId=%s)| Releasing lock.", currentOpId.toString().c_str(), shardId.toString().c_str());
	currentAction = Release;
	this->releaser->produce();
}

void ShardAssignOperation::finalize(){ // ** return **
	Logger::sharding(Logger::Step, "ShardAssign(opid=%s, shardId=%s)| Done, successFlag : %s", this->successFlag ? "successfull": "failed" ,
			currentOpId.toString().c_str(), shardId.toString().c_str());
	this->getConsumer()->consume(this->successFlag);
}

bool ShardAssignOperation::checkStillValid(){
	const Cluster_Writeview * writeview = ((WriteviewTransaction *)(this->getTransaction().get()))->getWriteview();
	ClusterShardIterator itr(writeview);
	bool stateResult = false;
	bool srcNodeResult = false;
	ClusterShardId shardId;double load;ShardState state;bool isLocal;NodeId nodeId;
	itr.beginClusterShardsIteration();
	while(itr.getNextClusterShard(shardId, load, state, isLocal, nodeId)){
		if(shardId == this->shardId){
			if(state != SHARDSTATE_UNASSIGNED){
				return false;
			}else{
				return true;
			}
		}
	}
	return false;
}

}
}


