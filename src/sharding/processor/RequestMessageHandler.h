#ifndef __SHARDING_ROUTING_INTERNAL_MESSAGE_BROKER_H_
#define __SHARDING_ROUTING_INTERNAL_MESSAGE_BROKER_H_

#include "sharding/processor/DistributedProcessorInternal.h"
#include "transport/Message.h"
#include "transport/MessageAllocator.h"
#include "sharding/transport/CallbackHandler.h"
#include "sharding/transport/TransportManager.h"
#include "sharding/configuration/ConfigManager.h"
#include "ReplyMessageHandler.h"
#include "sharding/sharding/ShardManager.h"
#include "sharding/sharding/metadata_manager/ResourceMetadataManager.h"

#include "processor/serializables/SerializableGetInfoCommandInput.h"
#include "processor/serializables/SerializableGetInfoResults.h"

namespace srch2is = srch2::instantsearch;
using namespace std;


namespace srch2 {
namespace httpwrapper {


class RequestMessageHandler : public CallBackHandler {
public:

    RequestMessageHandler(ConfigManager & cm,TransportManager & tm,
    		DPInternalRequestHandler& internalDP,
    		ReplyMessageHandler & replyHandler)
    : configManager(cm), transportManager(tm), internalDP(internalDP), replyHandler(replyHandler){
    };


    template<typename Reply>
    bool sendReply(Reply * replyObject, NodeId waitingNode, unsigned requestMessageId, boost::shared_ptr<const ClusterResourceMetadata_Readview> clusterReadview){
    	if(replyObject == NULL){
    		ASSERT(false);
    		return false;
    	}
    	// if waiting node is the current node, we just call resolveMessage(objects) from replyMessageHandler
    	if(clusterReadview->getCurrentNodeId() == waitingNode){
    		replyHandler.resolveReply(replyObject, clusterReadview->getCurrentNodeId(), requestMessageId);
    	}else{
    		// send the reply to the waiting node
    		Message * replyMessage =
    				Message::getMessagePointerFromBodyPointer(
    						replyObject->serialize(transportManager.getMessageAllocator()));
    		replyMessage->setDPReplyMask();
    		replyMessage->setType(Reply::messageType());
    		replyMessage->setMessageId(transportManager.getUniqueMessageIdValue());
    		replyMessage->setRequestMessageId(requestMessageId);
    		// TODO : timeout zero is left in the API but not used.
    		transportManager.sendMessage(waitingNode, replyMessage, 0);
    		// delete message
    	    transportManager.getMessageAllocator()->deallocateByMessagePointer(replyMessage);
    	    // delete object
    	    deleteResponseRequestObjectBasedOnType(Reply::messageType(), replyObject);
    	}
    	return true;
    }

    bool resolveMessage(Message * msg, NodeId node);

    template<class Request>
    bool resolveMessage(Request * requestObj,
    		NodeId node,
    		unsigned requestMessageId,
    		const NodeTargetShardInfo & target,
    		ShardingMessageType type,
    		boost::shared_ptr<const ClusterResourceMetadata_Readview> clusterReadview){

    	bool resultFlag = true;
    	switch(type){
        case GetInfoCommandMessageType: // -> for GetInfoCommandInput object (used for getInfo)
        	resultFlag = sendReply<GetInfoCommandResults>(internalDP.internalGetInfoCommand(target, clusterReadview, (GetInfoCommand*)requestObj), node, requestMessageId, clusterReadview);
        	break;
        default:
            ASSERT(false);
            return false;
        }

//		deleteResponseRequestObjectBasedOnType(Request::messageType(), requestObj);

    	return false;
    }

private:

    DPInternalRequestHandler& internalDP;
    ConfigManager & configManager;
    TransportManager & transportManager;
    ReplyMessageHandler & replyHandler;
    void deleteResponseRequestObjectBasedOnType(ShardingMessageType type, void * responseObject);
};

}
}

#endif // __SHARDING_ROUTING_INTERNAL_MESSAGE_BROKER_H_
