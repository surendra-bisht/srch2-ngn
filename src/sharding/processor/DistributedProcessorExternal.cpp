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

#include "DistributedProcessorExternal.h"

/*
 * System and thirdparty libraries
 */
#include <sys/time.h>
#include <sys/queue.h>
#include <boost/algorithm/string.hpp>
#include <iostream>
#include <sstream>
#include <string>
#include <set>
#include "thirdparty/snappy-1.0.4/snappy.h"


/*
 * Utility libraries
 */
#include "util/Logger.h"
#include "util/CustomizableJsonWriter.h"
#include "util/RecordSerializer.h"
#include "util/RecordSerializerUtil.h"
#include "util/FileOps.h"

/*
 * Srch2 libraries
 */
#include "instantsearch/TypedValue.h"
#include "instantsearch/ResultsPostProcessor.h"
#include "ParsedParameterContainer.h"
#include "QueryParser.h"
#include "QueryValidator.h"
#include "QueryRewriter.h"
#include "QueryPlan.h"
#include "util/ParserUtility.h"
#include "HTTPRequestHandler.h"
#include "IndexWriteUtil.h"



#include "serializables/SerializableGetInfoCommandInput.h"
#include "serializables/SerializableGetInfoResults.h"

#include "sharding/sharding/ShardManager.h"
#include "sharding/configuration/ConfigManager.h"
#include "sharding/configuration/CoreInfo.h"
#include "sharding/processor/Partitioner.h"
#include "sharding/processor/aggregators/GetInfoAggregatorAndPrint.h"
#include "server/HTTPJsonResponse.h"

namespace srch2is = srch2::instantsearch;
using namespace srch2is;
using namespace srch2::util;

#define TIMEOUT_WAIT_TIME 2

namespace srch2 {
namespace httpwrapper {

//###########################################################################
//                       External Distributed Processor
//###########################################################################


DPExternalRequestHandler::DPExternalRequestHandler(ConfigManager & configurationManager,
		TransportManager& transportManager, DPInternalRequestHandler& dpInternal):
			dpMessageHandler(configurationManager, transportManager, dpInternal){
	this->configurationManager = &configurationManager;
	transportManager.registerCallbackForDPMessageHandler(&dpMessageHandler);
}


/*
  * 1. Receives a getinfo request from a client (not from another shard)
  * 2. broadcasts this request to DPInternalRequestHandler objects of other shards
  * 3. Gives ResultAggregator object to PendingRequest framework and it's used to aggregate the
  *       results. Results will be aggregator by another thread since it's not a blocking call.
  */
void DPExternalRequestHandler::externalGetInfoCommand(boost::shared_ptr<const ClusterResourceMetadata_Readview> clusterReadview,
		evhttp_request *req, unsigned coreId, PortType_t portType){


	bool debugRequest = false;
	switch (portType) {
		case InfoPort_Nodes_NodeID:
		{
			JsonResponseHandler httpResponse(req);
			httpResponse.setResponseAttribute(c_cluster_name, Json::Value(clusterReadview->getClusterName()));
			Json::Value nodes(Json::arrayValue);
			ShardManager::getShardManager()->getNodeInfoJson(nodes);
			httpResponse.setResponseAttribute(c_nodes, nodes);
			httpResponse.finalizeOK();
			return;
		}
		case DebugStatsPort:
			debugRequest = true;
			break;
		case InfoPort:
		case InfoPort_Cluster_Stats:
			break;
		default:
			ASSERT(false);
			break;
	}

	boost::shared_ptr<GetInfoJsonResponse > brokerSideInformationJson =
			boost::shared_ptr<GetInfoJsonResponse > (new GetInfoJsonResponse(req));

    vector<unsigned> coreIds;
    if(coreId == (unsigned) -1){
    	vector<const CoreInfo_t *> cores;
    	clusterReadview->getAllCores(cores);
    	for(unsigned cid = 0 ; cid < cores.size(); cid++){
    		coreIds.push_back(cores.at(cid)->getCoreId());
    	}
    }else{
    	coreIds.push_back(coreId);
    }


	vector<NodeTargetShardInfo> targets;
    for(unsigned cid = 0; cid < coreIds.size(); ++cid){
    	unsigned coreId = coreIds.at(cid);
		const CoreInfo_t *indexDataContainerConf = clusterReadview->getCore(coreId);
		const string coreName = indexDataContainerConf->getName();
		CorePartitioner * corePartitioner = new CorePartitioner(clusterReadview->getPartitioner(coreId));
		corePartitioner->getAllTargets(targets);
		delete corePartitioner;
    }

    boost::shared_ptr<GetInfoResponseAggregator> resultsAggregator(
    		new GetInfoResponseAggregator(configurationManager,brokerSideInformationJson, clusterReadview, coreId, debugRequest));
	if(targets.size() == 0){
		brokerSideInformationJson->addError(JsonResponseHandler::getJsonSingleMessage(HTTP_JSON_All_Shards_Down_Error));
		brokerSideInformationJson->finalizeOK();
	}else{
		brokerSideInformationJson->finalizeOK();
		time_t timeValue;
		time(&timeValue);
		timeValue = timeValue + TIMEOUT_WAIT_TIME;
		GetInfoCommand * getInfoInput = new GetInfoCommand(GetInfoRequestType_);
		resultsAggregator->addRequestObj(getInfoInput);
		bool routingStatus = dpMessageHandler.broadcast<GetInfoCommand, GetInfoCommandResults>(getInfoInput,
						true,
						true,
						resultsAggregator,
						timeValue,
						targets,
						clusterReadview);

		if(! routingStatus){
			brokerSideInformationJson->finalizeError("Internal Server Error.");
		}
	}

}

}
}
