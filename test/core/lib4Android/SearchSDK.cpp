#include "Srch2Android.h"
#include "util/Logger.h"

using namespace srch2::sdk;
using srch2::util::Logger;

#ifdef __cplusplus
extern "C" {
#endif
JNIEXPORT void Java_com_srch2_mobile_ndksearch_Srch2Lib_setLoggerFile(
		JNIEnv* env, jobject javaThis, jstring logfile) {
	const char *nativeStringLogFile = env->GetStringUTFChars(logfile, NULL);
	FILE* fpLog = fopen(nativeStringLogFile, "a");
	Logger::setOutputFile(fpLog);
	env->ReleaseStringUTFChars(logfile, nativeStringLogFile);
}
#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
extern "C" {
#endif
JNIEXPORT jlong Java_com_srch2_mobile_ndksearch_Srch2Lib_createIndex(
		JNIEnv* env, jobject javaThis, jstring testFile, jstring indexDir,
		jint lineLimit, jboolean isGeo) {
	Logger::console("createIndex");
	const char *nativeStringDataFile = env->GetStringUTFChars(testFile, NULL);
	const char *nativeStringIndexPath = env->GetStringUTFChars(indexDir, NULL);

	string strIndexPath(nativeStringIndexPath);
	string strTestFile(nativeStringDataFile);

	srch2::instantsearch::TermType termType = PREFIX;

	Logger::console("Read data from %s", strTestFile.c_str());
	Logger::console("Save index to %s", strIndexPath.c_str());

	Indexer* indexer = createIndex(strTestFile, strIndexPath, lineLimit, isGeo);

	env->ReleaseStringUTFChars(testFile, nativeStringDataFile);
	env->ReleaseStringUTFChars(indexDir, nativeStringIndexPath);
	long ptr = (long) indexer;
	Logger::console("createIndex done");
	return ptr;
}
#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
extern "C" {
#endif
JNIEXPORT jlong Java_com_srch2_mobile_ndksearch_Srch2Lib_loadIndex(JNIEnv* env,
		jobject javaThis, jstring indexDir) {
	Logger::console("loadIndex");
	const char *nativeStringIndexPath = env->GetStringUTFChars(indexDir, NULL);
	string strIndexDir(nativeStringIndexPath);

	Logger::console("Load index from %s", strIndexDir.c_str());
	Indexer* indexer = loadIndex(strIndexDir);
	env->ReleaseStringUTFChars(indexDir, nativeStringIndexPath);
	long ptr = (long) indexer;
	Logger::console("loadIndex done");
	return ptr;
}
#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
extern "C" {
#endif
JNIEXPORT void Java_com_srch2_mobile_ndksearch_Srch2Lib_saveIndex(JNIEnv* env,
		jobject javaThis, jlong ptr) {
	Logger::console("saveIndex");
	Indexer* index = (Indexer*) ptr;
	saveIndex(index);
	Logger::console("saveIndex done");
}
#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
extern "C" {
#endif
JNIEXPORT jstring Java_com_srch2_mobile_ndksearch_Srch2Lib_queryRaw(JNIEnv* env,
		jobject javaThis, jlong indexPtr, jstring queryStr, jboolean isGeo) {

	const char *nativeStringQuery = env->GetStringUTFChars(queryStr, NULL);
	string queryString(nativeStringQuery);
	Logger::console("query:%s", nativeStringQuery);
	Indexer* indexer = (Indexer*) indexPtr;
	IndexSearcher *indexSearcher = IndexSearcher::create(indexer);
	const Analyzer *analyzer = indexer->getAnalyzer();

	QueryResults* queryResults = query(analyzer, indexSearcher, queryString, 2,
			srch2::instantsearch::PREFIX);

	string result = printQueryResult(queryResults, indexer);
	jstring jstr = env->NewStringUTF(result.c_str());
	env->ReleaseStringUTFChars(queryStr, nativeStringQuery);
	return jstr;
}
#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
extern "C" {
#endif
JNIEXPORT jobject Java_com_srch2_mobile_ndksearch_Srch2Lib_query(JNIEnv* env,
		jobject javaThis, jlong indexPtr, jstring queryStr, jboolean isGeo) {

	const char *nativeStringQuery = env->GetStringUTFChars(queryStr, NULL);
	string queryString(nativeStringQuery);
	Logger::console("query:%s", nativeStringQuery);
	Indexer* indexer = (Indexer*) indexPtr;
	IndexSearcher *indexSearcher = IndexSearcher::create(indexer);
	const Analyzer *analyzer = indexer->getAnalyzer();

	QueryResults* queryResults = query(analyzer, indexSearcher, queryString, 2,
			srch2::instantsearch::PREFIX);

    // Find java ArrayList
    jclass clsArrayList = env->FindClass("java/util/ArrayList");
    jmethodID constructor = env->GetMethodID(clsArrayList, "<init>", "(I)V");
    Logger::console("arrayListConstructor: %d", constructor);
    jmethodID arrayListAdd = env->GetMethodID(clsArrayList, "add", "(Ljava/lang/Object;)Z");
    Logger::console("arrayListAdd: %d", arrayListAdd);

	int count = queryResults->getNumberOfResults();
    jobject objArrayList = env->NewObject(clsArrayList, constructor, count);

    // Find com.srch2.mobile.ndksearch.Hit
    jclass clsHit = env->FindClass("com/srch2/mobile/ndksearch/Hit");
    Logger::console("classHit: %d", clsHit);
    //public Hit(float score, String record, String[] keywords, int[] eds)
    jmethodID constructorHit = env->GetMethodID(clsHit, "<init>", "(FLjava/lang/String;[Ljava/lang/String;[I)V");
    Logger::console("classHit constructor: %d", constructorHit);

    // Foreach queryResult
    vector<string> matchedKeywords;
    vector<unsigned> editDistances;
    for(int i = 0; i < count ; i++){
        float score = queryResults->getResultScore(i);
		string record = queryResults->getInMemoryRecordString(i);
		queryResults->getMatchingKeywords(i, matchedKeywords);
		queryResults->getEditDistances(i, editDistances);

        // create record string
        jstring jstrRecord = env->NewStringUTF(record.c_str());
        // create keywords array
        int size = matchedKeywords.size();
        jobjectArray jstrArray = (jobjectArray)env->NewObjectArray(size,
                env->FindClass("java/lang/String"), env->NewStringUTF(""));
        for(int j = 0; j < size;j++){
            env->SetObjectArrayElement(jstrArray, j, env->NewStringUTF( matchedKeywords[j].c_str() ) );
        }

        // create edit distance array
        jintArray jintArray = env->NewIntArray(size);
        jint* eds = new jint[size];
        for(int j = 0; j < size; j++){
            eds[j] = editDistances[j];
        }
        env->SetIntArrayRegion(jintArray, 0, size, eds);
        delete [] eds;

        jobject hit = env->NewObject(clsHit, constructorHit, score, jstrRecord, jstrArray, jintArray);

        Logger::console("set %d",i);
        env->CallVoidMethod(objArrayList, arrayListAdd, hit);
    }
        
    env->DeleteLocalRef(clsArrayList);
    env->DeleteLocalRef(clsHit);
	return objArrayList;
}
#ifdef __cplusplus
}
#endif

//TODO: delete the index
