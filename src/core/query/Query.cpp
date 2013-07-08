
// $Id: Query.cpp 3513 2013-06-29 00:27:49Z jamshid.esmaelnezhad $

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

#include <vector>
//#include <instantsearch/Query.h>
//#include <instantsearch/Term.h>
#include <instantsearch/Ranker.h>
#include "util/Log.h"
#include "record/LocationRecordUtil.h"
#include <string>

#include <iostream>

using std::vector;
namespace srch2
{
namespace instantsearch
{
class Term;

struct Query::Impl
{
    QueryType type;
    vector<Term* > *terms;
    unsigned sortableAttributeId;
    float lengthBoost;
    float prefixMatchPenalty;
    srch2::instantsearch::SortOrder order;
    Shape *range;
    Ranker *ranker;

    std::string nonSearchableAttributeName ;
    std::string nonSearchableAttributeValue ;

    POST_PROCESSING_FILTER postProcessingFilterCode;
    ATTRIBUTE_CRITERION_OPERATION postProcessingFilterOperationCode;

    Impl()
    {
        sortableAttributeId = 0;
        order = srch2::instantsearch::Descending;
        lengthBoost = 0.5;
        prefixMatchPenalty = 0.90;

        range = NULL;
    }

    virtual ~Impl()
    {
        if (this->terms != NULL)
        {
            for (vector<Term *>::iterator iter = this->terms->begin(); iter != this->terms->end(); iter++)
            {
                if (*iter != NULL)
                    delete *iter;
            }
            delete this->terms;
        }
        delete this->ranker;
        
        if (range != NULL)
            delete range;
    }
};

const srch2::instantsearch::Ranker *Query::getRanker() const
{
    return this->impl->ranker;
}

void Query::add(Term *term)
{
    impl->terms->push_back(term);
}

void Query::setRange(const double &lat_LB, const double &lng_LB, const double &lat_RT, const double &lng_RT)
{
    if (this->impl->range != NULL)
        delete this->impl->range;

    Rectangle *rect = new Rectangle();
    rect->min.x = lat_LB;
    rect->min.y = lng_LB;
    rect->max.x = lat_RT;
    rect->max.y = lng_RT;

    this->impl->range = rect;
}

void Query::setRange(const double &lat_CT, const double &lng_CT, const double &radius)
{
    if (this->impl->range != NULL)
        delete this->impl->range;

    Point p;
    p.x = lat_CT;
    p.y = lng_CT;

    Circle *circle = new Circle(p, radius);

    this->impl->range = circle;
}

void Query::setLengthBoost(float lengthBoost)
{
    if (lengthBoost > 0.0 && lengthBoost < 1.0)
    {
        this->impl->lengthBoost = lengthBoost;
    }
    else
    {
        this->impl->lengthBoost = 0.5;
    }
}

float Query::getLengthBoost() const
{
    return this->impl->lengthBoost;
}

void Query::setPrefixMatchPenalty(float prefixMatchPenalty)
{
    if (prefixMatchPenalty > 0.0 && prefixMatchPenalty < 1.0) {
        this->impl->prefixMatchPenalty = prefixMatchPenalty;
    }
    else {
        this->impl->prefixMatchPenalty = 0.95;
    }

}

float Query::getPrefixMatchPenalty() const
{
    return this->impl->prefixMatchPenalty;
}
    

Query::Query(QueryType type):impl(new Impl)
{
    impl->type = type;
    impl->terms = new vector<Term* >();

    switch ( impl->type )
    {
        case srch2::instantsearch::TopKQuery:
            impl->ranker = new DefaultTopKRanker();
            break;
        case srch2::instantsearch::GetAllResultsQuery:
            impl->ranker = new GetAllResultsRanker();
            break;
        case srch2::instantsearch::MapQuery:
            impl->ranker = new SpatialRanker();
            break;
        default:
            impl->ranker = new DefaultTopKRanker();
    };
}

Query::Query(QueryType type, const Ranker *ranker)
{
    impl->type = type;
    impl->terms = new vector<Term* >();
    //impl->ranker = new Ranker(ranker);
}

Query::~Query()
{
    if (impl != NULL)
    {
        delete impl;
    }
}

const vector<Term* >* Query::getQueryTerms() const
{
    return impl->terms;
}

QueryType Query::getQueryType() const
{
    return impl->type;
}

void Query::getRange(vector<double> &values) const
{
    impl->range->getValues(values);
}

void Query::setSortableAttribute(unsigned sortableAttributeId, srch2::instantsearch::SortOrder order)
{
    this->impl->sortableAttributeId = sortableAttributeId;
    this->impl->order = order;
}

unsigned Query::getSortableAttributeId() const
{
    return this->impl->sortableAttributeId;
}

// TODO temperory functions, to test range search filter
void Query::setNonSearchableAttributeName(std::string name){
	this->impl->nonSearchableAttributeName = name;
}
std::string Query::getNonSearchableAttributeName() const{
	return this->impl->nonSearchableAttributeName;
}

void Query::setNonSearchableAttributeValue(std::string value){
	this->impl->nonSearchableAttributeValue = value;
}
std::string Query::getNonSearchableAttributeValue() const{
	return this->impl->nonSearchableAttributeValue;
}


void Query::setPostProcessingFilter(POST_PROCESSING_FILTER code){
	this->impl->postProcessingFilterCode = code;
}
POST_PROCESSING_FILTER Query::getPostProcessingFilter() const{
	return this->impl->postProcessingFilterCode;
}
void Query::setPostProcessingFilterOperation(ATTRIBUTE_CRITERION_OPERATION code){
	this->impl->postProcessingFilterOperationCode = code;
}
ATTRIBUTE_CRITERION_OPERATION Query::getPostProcessingFilterOperation() const{
	return this->impl->postProcessingFilterOperationCode;
}

srch2::instantsearch::SortOrder Query::getSortableAttributeIdSortOrder() const
{
    return this->impl->order;
}

}}