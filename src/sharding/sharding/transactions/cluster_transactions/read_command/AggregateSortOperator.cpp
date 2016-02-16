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
/*
 * AggregateSortOperator.cpp
 *
 *  Created on: Aug 6, 2014
 */

#include "AggregateSortOperator.h"
#include "NetworkOpertator.h"

namespace srch2is = srch2::instantsearch;
using namespace std;

using namespace srch2is;

namespace srch2 {
namespace httpwrapper {

ClusterSortOperator::~ClusterSortOperator(){
	for(unsigned i = 0; i < this->children.size(); i++){
		delete this->children[i];
	}
	this->children.clear();
	this->sortEvaluator = NULL;
}

bool ClusterSortOperator::open(ClusterPhysicalPlanExecutionParameter * params){
	for (unsigned i = 0; i < this->children.size(); i++){
		this->children[i]->open(params);
		while(true){
			QueryResult *queryResult = this->children[i]->getNext(params);
			if(queryResult == NULL){
				break;
			}
			if(find(this->sortedItems.begin(), this->sortedItems.end(), queryResult) != this->sortedItems.end()){
				Logger::sharding(Logger::Error, "Search includes duplicate results;");
				continue;
			}
			this->sortedItems.push_back(queryResult);
		}
	}
	sort(this->sortedItems.begin(), this->sortedItems.end(), ClusterSortOperator::ClusterSortOperatorItemCmp(this->sortEvaluator));
	this->sortedItemsItr = this->sortedItems.begin();
	return true;
}

QueryResult* ClusterSortOperator::getNext(ClusterPhysicalPlanExecutionParameter * params){
	if(this->sortedItemsItr == this->sortedItems.end()){
		return NULL;
	}
	QueryResult* result = *(this->sortedItemsItr);
	this->sortedItemsItr++;
	return result;
}

bool ClusterSortOperator::close(ClusterPhysicalPlanExecutionParameter * params){
	for(unsigned i = 0; i < this->children.size(); i++){
		this->children[i]->close(params);
	}
	return true;
}

bool ClusterSortOperator::addChild(NetworkOperator* networkOperator){
	ASSERT(networkOperator != NULL);
	this->children.push_back(networkOperator);
	return true;
}

}
}

