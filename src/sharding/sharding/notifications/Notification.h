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
#ifndef __SHARDING_SHARDING_NOTIFICATION_H__
#define __SHARDING_SHARDING_NOTIFICATION_H__


#include "sharding/configuration/ShardingConstants.h"
#include "sharding/transport/Message.h"
#include "sharding/transport/MessageAllocator.h"
#include "migration/MigrationManager.h"
#include "core/util/Assert.h"
#include "boost/shared_ptr.hpp"


#define SP(TYPE) boost::shared_ptr<TYPE>

namespace srch2is = srch2::instantsearch;
using namespace srch2is;
using namespace std;
namespace srch2 {
namespace httpwrapper {

class Notification{
public:
    virtual ShardingMessageType messageType() const = 0;
    virtual ~Notification(){};
};

class ShardingNotification : public Notification{
public:
	ShardingNotification();
	virtual ~ShardingNotification(){};

	Message * serialize(MessageAllocator * allocator) const;

	Message * createMessage(MessageAllocator * allocator) const;

	void serializeHeaderInfo(Message * msg) const;

	void serializeContent(Message * msg) const;

	static void deserializeHeader(Message * msg, NodeId srcNode,
			NodeOperationId & srcAddress, NodeOperationId & destAddress, bool & bounced);

	template<class TYPE>
    static SP(TYPE) deserializeAndConstruct(Message * msg){
		void * buffer = Message::getBodyPointerFromMessagePointer(msg);
		return deserializeAndConstruct<TYPE>(buffer);
    }
	template<class TYPE>
    static SP(TYPE) deserializeAndConstruct(void * buffer){
		SP(TYPE) notification = create<TYPE>();
    	buffer = notification->deserializeHeader(buffer);
    	notification->deserializeBody(buffer);
    	return notification;
    }

	template<class TYPE>
    static SP(TYPE) create(){
		return SP(TYPE)(new TYPE());
    }

	virtual bool resolveNotification(SP(ShardingNotification) _notif)=0;

	static bool send(SP(ShardingNotification) notification);

	static bool send(SP(ShardingNotification) notification, const vector<NodeOperationId> & destinations );


	virtual bool hasResponse() const {
		return false;
	}

	void swapSrcDest();
	string getDescription();
	NodeOperationId getSrc() const;
	NodeOperationId getDest() const;
	void setSrc(const NodeOperationId & src) ;
	void setDest(const NodeOperationId & dest) ;
	void setBounced();
	void resetBounced();
	bool isBounced() const;

	void * serializeAll(void * buffer) const{
		buffer = serializeHeader(buffer);
		buffer = serializeBody(buffer);
		return buffer;
	}
	unsigned getNumberOfBytesAll() const{
		return getNumberOfBytesHeader() + getNumberOfBytesBody();
	}
	void * deserializeAll(void * buffer) {
		buffer = deserializeHeader(buffer);
		buffer = deserializeBody(buffer);
		return buffer;
	}

private:
    NodeOperationId srcOperationId;
    NodeOperationId destOperationId;
    bool bounced;

	void * serializeHeader(void * buffer) const;
	unsigned getNumberOfBytesHeader() const;
	void * deserializeHeader(void * buffer) ;

	virtual void * serializeBody(void * buffer) const{ return buffer;};
	virtual unsigned getNumberOfBytesBody() const{return 0;};
	virtual void * deserializeBody(void * buffer) {return buffer;};
};


class DummyShardingNotification : public ShardingNotification{
public:
    ShardingMessageType messageType() const {
    	return NULLType; // NOTE : A new ShardingMessageType must be added and used here.
    }
	bool resolveNotification(SP(ShardingNotification) _notif){
		ASSERT(false);
		// When this notification is received in this this node,
		// ShardManager calls this functions and whatever we want to do with this
		// notification must be implemented here (for example passing an ACK to the state-machine
		// if some node-iterator is waiting for this ack. or passing a command notification to
		// any of the internal processing modules like DPInteral, LockManager, ResourceMetadataManager and ...)
		return true;
	}
};

class NodeFailureNotification : public Notification{
public:
	NodeFailureNotification(const NodeId & failedNodeId):failedNodeId(failedNodeId){
	}
	NodeId getFailedNodeID() const{
		return this->failedNodeId;
	}
    ShardingMessageType messageType() const {
    	return ShardingNodeFailureNotificationMessageType;
    }
private:
	const NodeId failedNodeId;
};

class TimeoutNotification : public Notification{
public:
    ShardingMessageType messageType() const {
    	return ShardingTimeoutNotificationMessageType;
    }
};


class ShutdownNotification : public ShardingNotification {
public:
	ShardingMessageType messageType() const {
		return ShardingShutdownMessageType;
	}
	bool resolveNotification(SP(ShardingNotification) _notif);
};

}
}


#endif // __SHARDING_SHARDING_NOTIFICATION_H__
