#ifndef __COM_SRCH2_String_Engine__
#define __COM_SECH2_String_Engine__

/*****************************************************************************
 *                                                                           *
 *              AUTHOR : RJ ATWAL                                            *
 *                                                                           *
 *                                                                           * 
  ****************************************************************************/

#include <jni.h>

#ifdef __cplusplus
extern "C" {
#endif
/*
 * Class:     com_srch2_StringEngine
 * Method:    createSearchableEngine
 * Signature: (Ljava/lang/Class;Ljava/lang/reflect/Method;
               Ljava/lang/reflect/Constructor)J
 */
JNIEXPORT jlong JNICALL Java_com_srch2_StringEngine_createStringEngine
  (JNIEnv *, jobject, jclass, jobject, jobject);

/*
 * Class:     com_srch2_StringEngine
 * Method:    setString
 * Signature: (JLcom/srch2/SearchableString;)V
 */
JNIEXPORT void JNICALL Java_com_srch2_StringEngine_setString
  (JNIEnv *, jobject, jlong, jobject);

/*
 * Class:     com_srch2_StringEngine
 * Method:    getString
 * Signature: (J)Lcom/srch2/SearchableString;
 */
JNIEXPORT jobject JNICALL Java_com_srch2_StringEngine_getString
  (JNIEnv *, jobject, jlong);
/*
 * Class:     com_srch2_StringEngine
 * Method:    deleteSearchableEngine
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_com_srch2_StringEngine_deleteStringEngine
  (JNIEnv *, jobject, jlong);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __COM_SRCH2_String_Engine__ */