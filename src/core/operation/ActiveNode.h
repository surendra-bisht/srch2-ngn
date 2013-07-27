
// $Id: ActiveNode.h 3456 2013-06-14 02:11:13Z jiaying $

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

#ifndef __ACTIVENODE_H__
#define __ACTIVENODE_H__

#include "index/Trie.h"
#include <instantsearch/Term.h>
#include "util/BusyBit.h"
#include "util/Assert.h"

namespace srch2
{
namespace instantsearch
{

/*
PAN:
 */
typedef struct _PivotalActiveNode
{
    unsigned transformationdistance;
    unsigned short differ; // |p_{x+1}-p_i|
    unsigned short editdistanceofPrefix;
}PivotalActiveNode;


struct ResultNode {
    const TrieNode *node;
    int editDistance;
    int prefixLength;
    ResultNode(TrieNode* in_node):node(in_node){}
    ResultNode(const TrieNode* in_node, int in_editDistance, int in_prefixLength):node(in_node),
            editDistance(in_editDistance),prefixLength(in_prefixLength){}
};


class PrefixActiveNodeSet
{
public:
    typedef std::vector<const TrieNode* > TrieNodeSet;
    typedef ts_shared_ptr<TrieRootNodeAndFreeList > TrieRootNodeSharedPtr;

    BusyBit *busyBit;

private:
    std::vector<CharType> prefix;
    unsigned editDistanceThreshold;

    bool flagResultsCached;

    TrieRootNodeSharedPtr trieRootNodeSharedPtr;

    //PAN: A map from trie node to its pivotal active nodes
    std::map<const TrieNode*, PivotalActiveNode > PANMap;

    // group the trie nodes based on their edit distance to the prefix.
    // used only when it's called by an iterator
    std::vector<TrieNodeSet> trieNodeSetVector;
    bool trieNodeSetVectorComputed; // indicated if the trieNodeSetVector has been computed

public:

    void init(std::vector<CharType> &prefix, const unsigned editDistanceThreshold, const TrieRootNodeSharedPtr &trieRootNodeSharedPtr) {
        this->prefix = prefix;
        this->editDistanceThreshold = editDistanceThreshold;

        this->trieNodeSetVector.clear();
        this->trieNodeSetVectorComputed = false;

        this->flagResultsCached = false;
        this->busyBit = new BusyBit();

        this->trieRootNodeSharedPtr = trieRootNodeSharedPtr;
    };


    PrefixActiveNodeSet(std::vector<CharType> &prefix, const unsigned editDistanceThreshold, TrieRootNodeSharedPtr &trieRootNodeSharedPtr) {
        init(prefix, editDistanceThreshold, trieRootNodeSharedPtr);
    };

    /// A set of active nodes for an empty string and an edit-distance threshold
    PrefixActiveNodeSet(const TrieRootNodeSharedPtr &tsPtr, const unsigned editDistanceThreshold)
    {
        std::vector<CharType> emptyString;
        init(emptyString, editDistanceThreshold, tsPtr);

        //PAN:
        // Add the trie nodes up to the given depth
        const TrieNode *root = this->trieRootNodeSharedPtr->root;
        PivotalActiveNode pan;
        pan.transformationdistance = 0;
        pan.differ = 0; // |p_{x+1}-p_i|
        pan.editdistanceofPrefix = 0;
        _addPAN(root, pan);
        //if (editDistanceThreshold > 0)
        //    addTrieNodesUpToDepth(root, editDistanceThreshold, 0);


    };

    /// A set of active nodes for an empty string and an edit-distance threshold
    PrefixActiveNodeSet(const Trie *trie, const unsigned editDistanceThreshold)
    {
        trie->getTrieRootNode_ReadView(this->trieRootNodeSharedPtr);
        std::vector<CharType> emptyString;
        init(emptyString, editDistanceThreshold, this->trieRootNodeSharedPtr);

        //PAN:
        // Add the trie nodes up to the given depth
        const TrieNode *root = this->trieRootNodeSharedPtr->root;
        PivotalActiveNode pan;
        pan.transformationdistance = 0;
        pan.differ = 0; // |p_{x+1}-p_i|
        pan.editdistanceofPrefix = 0;
        _addPAN(root, pan);
        //if (editDistanceThreshold > 0)
        //    addTrieNodesUpToDepth(root, editDistanceThreshold, 0);
    };

    virtual ~PrefixActiveNodeSet() {
        delete this->busyBit;
    };

    PrefixActiveNodeSet *computeActiveNodeSetIncrementally(const CharType additionalChar);

    unsigned getEditDistanceThreshold() const {
        return editDistanceThreshold;
    }

    unsigned getNumberOfBytes() const {

        unsigned trieNodeSetVectorSize = 0;

        for (std::vector<TrieNodeSet>::const_iterator vectorIter = trieNodeSetVector.begin(); vectorIter != trieNodeSetVector.end(); vectorIter++ )
        {
            trieNodeSetVectorSize += (*vectorIter).capacity() * sizeof(*vectorIter);
        }
        trieNodeSetVectorSize += trieNodeSetVector.size() * sizeof(TrieNodeSet)
                              + (this->trieNodeSetVector.capacity() - trieNodeSetVector.size()) * sizeof(void*);

        return this->prefix.size()
                              + sizeof(this->editDistanceThreshold)
                              + trieNodeSetVectorSize;
    }

    unsigned getNumberOfActiveNodes() {
        return (unsigned) PANMap.size();
    }

    std::vector<CharType> *getPrefix() {
        return &prefix;
    }

    std::string getPrefixUtf8String() {
        return getUtf8String(prefix);
    }

    unsigned getPrefixLength() const {
        return prefix.size();
    }

    // Deprecated due to removal of TrieNode->getParent() pointers.
    void getComputedSimilarPrefixes(const Trie *trie, std::vector<std::string> &similarPrefixes);

    //typedef std::vector<TrieNode* > TrieNodeSet;
    std::vector<TrieNodeSet> *getTrieNodeSetVector() {

        // compute it only if necessary
        if (this->trieNodeSetVectorComputed)
            return &trieNodeSetVector;

        _computeTrieNodeSetVector();
        return &trieNodeSetVector;
    }

    bool isResultsCached() const    {
        return this->flagResultsCached;
    }

    void setResultsCached(bool flag) {
        this->flagResultsCached = flag;
    }

    unsigned getEditdistanceofPrefix(const TrieNode *&trieNode)
    {
    	std::map<const TrieNode*, PivotalActiveNode >::iterator iter = PANMap.find(trieNode);
    	ASSERT(iter != PANMap.end());
    	return iter->second.editdistanceofPrefix;
    }

    void printActiveNodes(const Trie* trie) const;// Deprecated due to removal of TrieNode->getParent() pointers.

private:

    //PAN:

    /// compute the pivotal active nodes based on one of the active nodes of the previous prefix
    /// add the new pivotal active nodes to newActiveNodeSet
    void _addPANSetForOneNode(const TrieNode *trieNode, PivotalActiveNode pan,
            const CharType additionalChar, PrefixActiveNodeSet *newActiveNodeSet);

    //PAN:
    /// Add a new pivotal active node with an edit distance.
    /// If the pivotal active node already exists in the set and had a distance no greater than the new one,
    /// then ignore this request.
    void _addPAN(const TrieNode *trieNode, PivotalActiveNode pan);


    //PAN:

    // add the descendants of the current node up to "depth" to the PANMao with
    // the corresponding edit distance.  The edit distance of the current node is "editDistance".
    void addPANUpToDepth(const TrieNode *trieNode, PivotalActiveNode pan, const unsigned curDepth, const unsigned depthLimit, const CharType additionalChar, PrefixActiveNodeSet *newActiveNodeSet);

    void _computeTrieNodeSetVector() {
        if (this->trieNodeSetVectorComputed)
            return;

        // VECTOR: initialize the vector
        this->trieNodeSetVector.resize(editDistanceThreshold + 1);
        for (unsigned i = 0; i <= editDistanceThreshold; i++)
            this->trieNodeSetVector[i].clear();

        // go over the map to populate the vectors.
        /*for (std::map<const TrieNode*, unsigned >::iterator mapIterator = trieNodeDistanceMap.begin();
                mapIterator != trieNodeDistanceMap.end(); mapIterator ++) {
            this->trieNodeSetVector[mapIterator->second].push_back(mapIterator->first);
        }*/

        for (std::map<const TrieNode*, PivotalActiveNode >::iterator mapIterator = PANMap.begin();
			   mapIterator != PANMap.end(); mapIterator ++) {
		   this->trieNodeSetVector[mapIterator->second.transformationdistance].push_back(mapIterator->first);
	   }

        // set the flag
        this->trieNodeSetVectorComputed = true;
    }
};

class ActiveNodeSetIterator
{
public:
    // generate an iterator for the active nodes whose edit distance is within the given @edUpperBound
    ActiveNodeSetIterator(PrefixActiveNodeSet *prefixActiveNodeSet, const unsigned edUpperBound) {
        _initActiveNodeIterator(prefixActiveNodeSet, edUpperBound);
    }

    void next() {
        if (isDone())
            return;

        offsetCursor ++;

        if (offsetCursor < trieNodeSetVector->at(editDistanceCursor).size()) 
            return;

        // reached the tail of the current vector
        editDistanceCursor ++;
        offsetCursor = 0;

        // move editDistanceCursor to the next non-empty vector
        while (editDistanceCursor <= this->edUpperBound &&
                trieNodeSetVector->at(editDistanceCursor).size() == 0)
            editDistanceCursor ++;
    }

    bool isDone() {
        if (editDistanceCursor <= this->edUpperBound &&
                offsetCursor < trieNodeSetVector->at(editDistanceCursor).size())
            return false;

        return true;
    }

    void getItem(const TrieNode *&trieNode, unsigned &distance) {
        if (isDone()) {
            trieNode = NULL;
            distance = 0;
        }
        else {
            trieNode = trieNodeSetVector->at(editDistanceCursor).at(offsetCursor);
            distance = editDistanceCursor;
        }

        //ASSERT(distance != 0);
    }

    // Get current active node, if we have finished the iteration, return NULL
    void getActiveNode(const TrieNode *&trieNode) {
	   if (isDone()) {
		   trieNode = NULL;
	   }
	   else {
		   // return current active node, which is the offsetCursor one in editDistanceCursor array
		   trieNode = trieNodeSetVector->at(editDistanceCursor).at(offsetCursor);
	   }
	}

	void refresh(){
		this->editDistanceCursor = 0;
		this->offsetCursor = 0;
		while (editDistanceCursor <= edUpperBound &&
			trieNodeSetVector->at(editDistanceCursor).size() == 0)
		editDistanceCursor ++;
	}

private:
    typedef std::vector<const TrieNode* > TrieNodeSet;

    std::vector<TrieNodeSet> *trieNodeSetVector;
    unsigned edUpperBound;
    unsigned editDistanceCursor;
    unsigned offsetCursor;

    // initialize an iterator to store all the active nodes whose edit distance to
    // the query prefix is within the bound @edUpperBound
    void _initActiveNodeIterator(PrefixActiveNodeSet *prefixActiveNodeSet, const unsigned edUpperBound) {
        // we materialize the vector of trie nodes (indexed by the edit distance) only during the
        // phase of an iterator
        this->trieNodeSetVector = prefixActiveNodeSet->getTrieNodeSetVector();

        ASSERT(edUpperBound < trieNodeSetVector->size());
        this->edUpperBound = edUpperBound;

        // initialize the cursors
        this->editDistanceCursor = 0;
        // Find the first valid active node
        while (editDistanceCursor <= edUpperBound &&
                trieNodeSetVector->at(editDistanceCursor).size() == 0)
            editDistanceCursor ++;

        this->offsetCursor = 0;
    }

};

// a structure to record information of each node

struct LeafNodeSetIteratorItem
{
    const TrieNode *prefixNode;
    const TrieNode *leafNode;
    unsigned distance;

    // TODO: OPT. Return the length of the prefix instead of the prefixNode pointer?
    LeafNodeSetIteratorItem(const TrieNode *prefixNode, const TrieNode *leafNode, unsigned distance)
    {
        this->prefixNode = prefixNode;
        this->leafNode = leafNode;
        this->distance = distance;
    }

};


class LeafNodeSetIterator
{
private:
    std::vector<LeafNodeSetIteratorItem > resultVector;
    unsigned cursor;

public:
    // for a set of active nodes, given a threshold edUpperBound, find
    // all the leaf nodes whose minimal edit distance (among all their prefixes)
    // is within the edUpperBound.
    // Provide an iterator that can return the leaf nodes sorted by their
    // minimal edit distance

    // Implementation: 1. Get the set of active nodes with an edit distance <= edUpperBound, sorted 
    //                 based on their edit distance,
    //                 2. get their leaf nodes, and keep track of the visited nodes so that
    //                   each node is visited only once.
    LeafNodeSetIterator(PrefixActiveNodeSet *prefixActiveNodeSet, const unsigned edUpperBound) {
        _initLeafNodeSetIterator(prefixActiveNodeSet, edUpperBound);
    }

    void next() {
        if (isDone())
            return;
        cursor ++;
    }

    bool isDone() {
        //if (cursor >= leafNodesVector.size())
        if (cursor >= resultVector.size())
            return true;
        return false;
    }

    void getItem(const TrieNode *&prefixNode, const TrieNode *&leafNode, unsigned &distance) {
        if (isDone()) {
            prefixNode = NULL;
            leafNode = NULL;
            distance = 0;
        }
        else {
            prefixNode = resultVector.at(cursor).prefixNode;
            leafNode = resultVector.at(cursor).leafNode;
            distance = resultVector.at(cursor).distance;
        }
    }

    // Deprecated due to removal of TrieNode->getParent() pointers.
    void printLeafNodes(const Trie* trie) const
    {
        typedef LeafNodeSetIteratorItem leafStar;
        typedef const TrieNode* trieStar;
        std::vector<leafStar>::const_iterator vecIter;
        std::cout << "LeafNodes " << std::endl;
        for ( vecIter = this->resultVector.begin(); vecIter!= this->resultVector.end(); vecIter++ )
        {
            leafStar temp = *vecIter;
            trieStar prefixNode = temp.leafNode;
            string prefix;
            trie->getPrefixString_NotThreadSafe(prefixNode, prefix);
            std::cout << "Prefix:" << prefix << "|Distance" << temp.distance << std::endl;
        }
    }

private:
    void _initLeafNodeSetIterator(PrefixActiveNodeSet *prefixActiveNodeSet, const unsigned edUpperBound) {

    	map<const TrieNode*, unsigned> activeNodes;
		const TrieNode *trieNode;
		unsigned distance;

		// assume the iterator returns the active nodes in an ascending order of their edit distance
		ActiveNodeSetIterator ani(prefixActiveNodeSet, edUpperBound);
		for (; !ani.isDone(); ani.next()){
			ani.getItem(trieNode, distance);
			activeNodes[trieNode] = distance; // initially all active nodes are not visited.
		}
		for(map<const TrieNode*, unsigned>::iterator iter = activeNodes.begin(),curIter; iter != activeNodes.end();)
		{
			curIter = iter;
			iter++;
			_appendLeafNodes(curIter, trieNode, iter);
		}

		// init the cursor
		cursor = 0;
    }

    // add the leaf nodes of the given trieNode to a vector.  Add those decendant nodes to visitedTrieNodes.
    // Ignore those decendants that are already in visitedTrieNodes
    void _appendLeafNodes(map<const TrieNode*, unsigned>::iterator prevActiveNode, const TrieNode *curNode, map<const TrieNode*, unsigned>::iterator &nextActiveNode) {
    	//meet the expect node
    	if(curNode == nextActiveNode->first){
    		if(nextActiveNode->second <= prevActiveNode->second){
    			prevActiveNode = nextActiveNode;
    		}
    		nextActiveNode++;
    	}

        if (curNode->isTerminalNode()) {
            // TODO: prefix might not be unique. Should we return the longest matching prefix?
            resultVector.push_back(LeafNodeSetIteratorItem(prevActiveNode->first, curNode, prevActiveNode->second));
        }

        // go through the children
        for (unsigned childIterator = 0; childIterator < curNode->getChildrenCount(); childIterator ++)
            _appendLeafNodes(prevActiveNode, curNode->getChild(childIterator), nextActiveNode);
    }
};

}}

#endif //__ACTIVENODE_H__
