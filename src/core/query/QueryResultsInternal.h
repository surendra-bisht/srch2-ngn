
// $Id: QueryResultsInternal.h 3513 2013-06-29 00:27:49Z jamshid.esmaelnezhad $

/*
 * The Software is made available solely for use according to the License Agreement. Any reproduction
 * or redistribution of the Software not in accordance with the License Agreement is expressly prohibited
 * by law, and may result in severe civil and criminal penalties. Violators will be prosecuted to the
 * maximum extent possible.
 *
 * THE SOFTWARE IS WARRANTED, IF AT ALL, ONLY ACCORDING TO THE TERMS OF THE LICENSE AGREEMENT. EXCEPT
 * AS WARRANTED IN THE LICENSE AGREEMENT, SRCH2 INC. HEREBY DISCLAIMS ALL WARRANTIES AND CONDITIONS WITH
 * REGARD TO THE SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES AND CONDITIONS OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT.  IN NO EVENT SHALL SRCH2 INC. BE LIABLE FOR ANY
 * SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA
 * OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER ACTION, ARISING OUT OF OR IN CONNECTION
 * WITH THE USE OR PERFORMANCE OF SOFTWARE.

 * Copyright © 2010 SRCH2 Inc. All rights reserved
 */

#ifndef __QUERYRESULTSINTERNAL_H__
#define __QUERYRESULTSINTERNAL_H__

#include <instantsearch/QueryResults.h>
//#include "operation/TermVirtualList.h"
#include <instantsearch/Stat.h>
#include <instantsearch/Ranker.h>
#include <instantsearch/Score.h>
#include "index/ForwardIndex.h"
#include "util/Assert.h"

#include <vector>
#include <queue>
#include <string>
#include <set>


namespace srch2
{
namespace instantsearch
{
class Query;
class IndexSearcherInternal;
class TermVirtualList;

struct QueryResult {
    string externalRecordId;
    unsigned internalRecordId;
    Score _score;

    std::vector<std::string> matchingKeywords;
    std::vector<unsigned> attributeBitmaps;
    std::vector<unsigned> editDistances;
    
    // only the results of MapQuery have this
    double physicalDistance; // TODO check if there is a better way to structure the "location result"
    


    QueryResult(const QueryResult& copy_from_me){
//    	ASSERT(copy_from_me._score != NULL);
    	externalRecordId = copy_from_me.externalRecordId;
    	internalRecordId = copy_from_me.internalRecordId;
    	_score = copy_from_me._score;

    	matchingKeywords = copy_from_me.matchingKeywords;
    	attributeBitmaps = copy_from_me.attributeBitmaps;
    	editDistances = copy_from_me.editDistances;

    }
    QueryResult(){
    };



    Score getResultScore() const
    {
    	return _score;
    }
    // this operator should be consistent with two others in TermVirtualList.h and InvertedIndex.h
    bool operator<(const QueryResult& queryResult) const
    {
        float leftRecordScore, rightRecordScore;
        unsigned leftRecordId  = internalRecordId;
        unsigned rightRecordId = queryResult.internalRecordId;
		Score _leftRecordScore = this->_score;
		Score _rightRecordScore = queryResult._score;
		return DefaultTopKRanker::compareRecordsGreaterThan(_leftRecordScore,  leftRecordId,
		                                    _rightRecordScore, rightRecordId);//TODO: this one should be returned.


    }
    
};
 
class QueryResultsInternal : public QueryResults
{
public:
    QueryResultsInternal(IndexSearcherInternal *indexSearcherInternal, Query *query);
    virtual ~QueryResultsInternal();

    std::vector<TermVirtualList* > *getVirtualListVector() { return virtualListVector; };

    QueryResult* getQueryResult(unsigned position);
    unsigned getNumberOfResults() const;
    std::string getRecordId(unsigned position) const;
    unsigned getInternalRecordId(unsigned position) const;
    std::string getInMemoryRecordString(unsigned position) const;
    
    string getResultScoreString(unsigned position) const;
    Score getResultScore(unsigned position) const;
    
    void getMatchingKeywords(const unsigned position, vector<string> &matchingKeywords) const;
    void getEditDistances(const unsigned position, vector<unsigned> &editDistances) const;
    void getMatchedAttributeBitmaps(const unsigned position, std::vector<unsigned> &matchedAttributeBitmaps) const;
    void getMatchedAttributes(const unsigned position, std::vector<vector<unsigned> > &matchedAttributes) const;
    
    // only the results of MapQuery have this
    double getPhysicalDistance(const unsigned position) const; // TODO check if there is a better way to structure the "location result"
    
    void setNextK(const unsigned k); // set number of results in nextKResultsHeap
    void insertResult(QueryResult &queryResult); //insert queryResult to the priority queue
    bool hasTopK(const float maxScoreForUnvisitedRecords);
    
    void fillVisitedList(std::set<unsigned> &visitedList); // fill visitedList with the recordIds in sortedFinalResults
    void finalizeResults(const ForwardIndex *forwardIndex);

    const Query* getQuery() const {
        return this->query;
    }
    
    // DEBUG function. Used in CacheIntegration_Test
    bool checkCacheHit(IndexSearcherInternal *indexSearcherInternal, Query *query);
    
    void printStats() const;
    
    void printResult() const;

    void addMessage(const char* msg)
    {
        this->stat->addMessage(msg);
    }
    
    std::vector<QueryResult> sortedFinalResults;
    std::vector<TermVirtualList* > *virtualListVector;
    
    Stat *stat;
    
 private:
    Query* query;
    unsigned nextK;

    const IndexSearcherInternal *indexSearcherInternal;

    // OPT use QueryResults Pointers.
    // TODO: DONE add an iterator to consume the results by converting ids using getExternalRecordId(recordId)
    std::priority_queue<QueryResult, std::vector<QueryResult> > nextKResultsHeap;
};
}}
#endif /* __QUERYRESULTSINTERNAL_H__ */