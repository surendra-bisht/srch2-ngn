
#include "CoreInfo.h"
#include "Shard.h"
#include "ConfigManager.h"

using namespace std;
namespace srch2is = srch2::instantsearch;
using namespace srch2::instantsearch;

namespace srch2 {
namespace httpwrapper {


ShardId CoreInfo_t::getPrimaryShardId(unsigned partitionId) const{
	ShardId rtn ;
	rtn.coreId = this->coreId;
	rtn.partitionId = partitionId;
	rtn.replicaId = 0;
	return rtn;
}

uint32_t CoreInfo_t::getDocumentLimit() const {
    return documentLimit;
}

uint64_t CoreInfo_t::getMemoryLimit() const {
    return memoryLimit;
}

uint32_t CoreInfo_t::getMergeEveryNSeconds() const {
    return mergeEveryNSeconds;
}

uint32_t CoreInfo_t::getMergeEveryMWrites() const {
    return mergeEveryMWrites;
}

uint32_t CoreInfo_t::getUpdateHistogramEveryPMerges() const {
    return updateHistogramEveryPMerges;
}

uint32_t CoreInfo_t::getUpdateHistogramEveryQWrites() const {
    return updateHistogramEveryQWrites;
}

int CoreInfo_t::getSearchType(const string &coreName) const {
	return configManager->getSearchType(coreName);
}

const std::string& CoreInfo_t::getScoringExpressionString() const
{
    return scoringExpressionString;
}

int CoreInfo_t::getSearchResponseJSONFormat() const {
    return searchResponseJsonFormat;
}

bool CoreInfo_t::getIsFuzzyTermsQuery() const
{
    return exactFuzzy;
}

const string &CoreInfo_t::getSrch2Home() const {
	return configManager->getSrch2Home();
}
const string& CoreInfo_t::getLicenseKeyFileName() const {
	return configManager->getLicenseKeyFileName();
}
const string& CoreInfo_t::getHTTPServerListeningHostname() const{
	return configManager->getHTTPServerListeningHostname();
}
const string& CoreInfo_t::getHTTPServerListeningPort() const {
	return configManager->getHTTPServerListeningPort();
}
const string& CoreInfo_t::getHTTPServerAccessLogFile() const {
	return configManager->getHTTPServerAccessLogFile();
}
const Logger::LogLevel& CoreInfo_t::getHTTPServerLogLevel() const{
	return configManager->getHTTPServerLogLevel();
}

float CoreInfo_t::getDefaultSpatialQueryBoundingBox() const{
	return configManager->getDefaultSpatialQueryBoundingBox();
}

unsigned int CoreInfo_t::getKeywordPopularityThreshold() const{
	return configManager->getKeywordPopularityThreshold();
}
const unsigned CoreInfo_t::getGetAllResultsNumberOfResultsThreshold() const{
	return configManager->getGetAllResultsNumberOfResultsThreshold();
}
const unsigned CoreInfo_t::getGetAllResultsNumberOfResultsToFindInEstimationMode() const{
	return configManager->getGetAllResultsNumberOfResultsToFindInEstimationMode();
}

unsigned int CoreInfo_t::getNumberOfThreads() const {
	return configManager->getNumberOfThreads();
}

bool CoreInfo_t::getQueryTermPrefixType() const
{
    return queryTermPrefixType;
}


float CoreInfo_t::getFuzzyMatchPenalty() const
{
    return fuzzyMatchPenalty;
}

float CoreInfo_t::getQueryTermSimilarityThreshold() const
{
    return queryTermSimilarityThreshold;
}

float CoreInfo_t::getQueryTermLengthBoost() const
{
    return queryTermLengthBoost;
}

float CoreInfo_t::getPrefixMatchPenalty() const
{
    return prefixMatchPenalty;
}

ResponseType CoreInfo_t::getSearchResponseFormat() const
{
    return searchResponseContent;
}


int CoreInfo_t::getDefaultResultsToRetrieve() const
{
    return resultsToRetrieve;
}

int CoreInfo_t::getOrdering() const {
	return configManager->getOrdering();
}

int CoreInfo_t::getAttributeToSort() const
{
    return attributeToSort;
}

unsigned short CoreInfo_t::getPort(PortType_t portType) const
{
    if (static_cast<unsigned int> (portType) >= ports.size()) {
        return 0;
    }

    unsigned short portNumber = ports[portType];
    return portNumber;
}

void CoreInfo_t::setPort(PortType_t portType, unsigned short portNumber)
{
    if (static_cast<unsigned int> (portType) >= ports.size()) {
        ports.resize(static_cast<unsigned int> (EndOfPortType), 0);
    }

    switch (portType) {
    case SearchPort:
    case SuggestPort:
    case InfoPort:
    case DocsPort:
    case UpdatePort:
    case SavePort:
    case ExportPort:
    case ResetLoggerPort:
        ports[portType] = portNumber;
        break;

    default:
        Logger::error("Unrecognized HTTP listening port type: %d", static_cast<int> (portType));
        break;
    }
}

// JUST FOR Wrapper TEST
void CoreInfo_t::setDataFilePath(const string& path) {
    dataFilePath = path;
}

}
}