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
#ifndef _URLPARSER_H_
#define _URLPARSER_H_

namespace srch2
{
namespace httpwrapper
{

class URLParser
{
public:
    static const char queryDelimiter;
    static const char filterDelimiter;
    static const char fieldsAndDelimiter;
    static const char fieldsOrDelimiter;

    static const char* const searchTypeParamName;
    static const char* const keywordsParamName;
    static const char* const termTypesParamName;
    static const char* const termBoostsParamName;
    static const char* const fuzzyQueryParamName;
    static const char* const similarityBoostsParamName;
    static const char* const resultsToRetrieveStartParamName;
    static const char* const resultsToRetrieveLimitParamName;
    static const char* const attributeToSortParamName;
    static const char* const orderParamName;
    static const char* const lengthBoostParamName;
    static const char* const jsonpCallBackName;

    static const char* const leftBottomLatitudeParamName;
    static const char* const leftBottomLongitudeParamName;
    static const char* const rightTopLatitudeParamName;
    static const char* const rightTopLongitudeParamName;

    static const char* const centerLatitudeParamName;
    static const char* const centerLongitudeParamName;
    static const char* const radiusParamName;
    static const char* const nameParamName;
    static const char* const logNameParamName ;
    static const char* const setParamName;
    static const char* const shutdownForceParamName;
    static const char* const shutdownSaveParamName;
};

}
}


#endif //_URLPARSER_H_
