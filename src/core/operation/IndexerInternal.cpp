//$Id: IndexerInternal.cpp 3456 2013-06-14 02:11:13Z jiaying $

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


#include "operation/IndexerInternal.h"

namespace srch2
{
namespace instantsearch
{
    
INDEXWRITE_RETVAL IndexReaderWriter::commit()
{    
    writelock();
    
    INDEXWRITE_RETVAL commitReturnValue = this->index->_commit();

    if (commitReturnValue == OP_FAIL && this->mergeThreadStarted == false) {
    	/*
    	 *  If _commit fails, then we are in past bulk load stage. We should call merge function but
    	 *  only if there is NO dedicated merge thread running. Merge thread can be
    	 *  instantiated by a wrapper layer which allows it to not call commit (or merge) explicitly.
    	 */
    	bool updateHistogramFlag = shouldUpdateHistogram();
    	if(updateHistogramFlag == true){
    		this->resetMergeCounterForHistogram();
    	}
    	this->merge(updateHistogramFlag);
    }
    this->writesCounterForMerge = 0;
    
    writeunlock();

    return commitReturnValue;
}

INDEXWRITE_RETVAL IndexReaderWriter::addRecord(const Record *record, Analyzer* analyzer)
{
    writelock();
    this->writesCounterForMerge++;
    INDEXWRITE_RETVAL returnValue = this->index->_addRecord(record, analyzer);

    writeunlock();
    
    return returnValue;
}

INDEXWRITE_RETVAL IndexReaderWriter::deleteRecord(const std::string &primaryKeyID)
{
    writelock();
    this->writesCounterForMerge++;

    INDEXWRITE_RETVAL returnValue = this->index->_deleteRecord(primaryKeyID);
    
    writeunlock();
    
    return returnValue;
}

INDEXWRITE_RETVAL IndexReaderWriter::deleteRecordGetInternalId(const std::string &primaryKeyID, unsigned &internalRecordId)
{
    writelock();
    this->writesCounterForMerge++;

    INDEXWRITE_RETVAL returnValue = this->index->_deleteRecordGetInternalId(primaryKeyID, internalRecordId);

    writeunlock();

    return returnValue;
}

INDEXWRITE_RETVAL IndexReaderWriter::recoverRecord(const std::string &primaryKeyID, unsigned internalRecordId)
{
    writelock();
    this->writesCounterForMerge++;

    INDEXWRITE_RETVAL returnValue = this->index->_recoverRecord(primaryKeyID, internalRecordId);

    writeunlock();

    return returnValue;
}

INDEXLOOKUP_RETVAL IndexReaderWriter::lookupRecord(const std::string &primaryKeyID)
{
    // although it's a read-only OP, since we need to check the writeview
    // we need to acquire the writelock
    // but do NOT need to increase the merge counter

    writelock(); 
    
    INDEXLOOKUP_RETVAL returnValue = this->index->_lookupRecord(primaryKeyID);

    writeunlock();

    return returnValue;
}

void IndexReaderWriter::exportData(const string &exportedDataFileName)
{
    // add write lock
    writelock();

    // merge the index
    // we don't have to update histogram information when we want to export.
    this->merge(false);
    writesCounterForMerge = 0;

    //get the export data
    this->index->_exportData(exportedDataFileName);

    // free write lock
    writeunlock();
}

void IndexReaderWriter::save()
{
    writelock();

    // we don't have to update histogram information when we want to export.
    this->merge(false);
    writesCounterForMerge = 0;

    this->index->_save();

    writeunlock();
}

void IndexReaderWriter::save(const std::string& directoryName)
{
    writelock();

    // we don't have to update histogram information when we want to export.
    this->merge(false);
    writesCounterForMerge = 0;

    this->index->_save(directoryName);

    writeunlock();
}

/*
 *  This function is not thread safe. Caller of this function should hold valid lock. This
 *  function should not be exposed outside core.
 */

INDEXWRITE_RETVAL IndexReaderWriter::merge(bool updateHistogram)
{
    if (this->cache != NULL)
        this->cache->clear();

    // increment the mergeCounterForUpdatingHistogram
    this->mergeCounterForUpdatingHistogram ++;

    struct timespec tstart;
    clock_gettime(CLOCK_REALTIME, &tstart);

    INDEXWRITE_RETVAL returnValue = this->index->_merge(updateHistogram);


    struct timespec tend;
    clock_gettime(CLOCK_REALTIME, &tend);
    unsigned time = (tend.tv_sec - tstart.tv_sec) * 1000 + (tend.tv_nsec - tstart.tv_nsec) / 1000000;

    // std::cout << "{\"merge\":\"success\",\"time\":\""<< time <<"\"}" << std::endl;

    indexHealthInfo.notifyMerge(time,  this->index->_getNumberOfDocumentsInIndex());
    return returnValue;
}

//http://publib.boulder.ibm.com/infocenter/iseries/v5r4/index.jsp?topic=%2Fapis%2Fusers_77.htm
void IndexReaderWriter::startMergeThreadLoop()
{
    int               rc;
    struct timespec   ts;
    struct timeval    tp;

    /*
     *  There should be only one merger thread per indexer object
     */
    rwMutexForWriter->lockWrite();
    if (mergeThreadStarted) {
    	rwMutexForWriter->unlockWrite();
    	Logger::warn("Only one merge thread per index is supported!");
    	return;
    }
    if (!this->index->isCommited()) {
        	rwMutexForWriter->unlockWrite();
        	Logger::warn("Merge thread can be called only after first commit!");
        	return;
    }
    rwMutexForWriter->unlockWrite();
    mergeThreadStarted = true;
    /*
     *  Initialize condition variable for the first time before loop starts.
     */
    pthread_cond_init(&countThresholdConditionVariable, NULL);

    while (1)
    {
        rc =  gettimeofday(&tp, NULL);
        // Convert from timeval to timespec
        ts.tv_sec  = tp.tv_sec;
        ts.tv_nsec = tp.tv_usec * 1000;
        ts.tv_sec += this->mergeEveryNSeconds;

        rwMutexForWriter->cond_timedwait(&countThresholdConditionVariable, &ts);

        if (mergeThreadStarted == false)
            break;
        else
        {
        	// check to see if it's the time to update histogram information
        	// if so, reset the merge counter for future.
            bool updateHistogramFlag = shouldUpdateHistogram();
            if(updateHistogramFlag == true){
            	this->resetMergeCounterForHistogram();
            }
            this->merge(updateHistogramFlag);
            writesCounterForMerge = 0;
            rwMutexForWriter->unlockWrite();
        }
    }
    pthread_cond_destroy(&countThresholdConditionVariable);
    rwMutexForWriter->unlockWrite();
    return;
}



}
}


