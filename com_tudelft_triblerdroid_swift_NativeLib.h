/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
/* Header for class com_tudelft_triblerdroid_swift_NativeLib */

#ifndef _Included_com_tudelft_triblerdroid_swift_NativeLib
#define _Included_com_tudelft_triblerdroid_swift_NativeLib
#ifdef __cplusplus
extern "C" {
#endif
/*
 * Class:     com_tudelft_triblerdroid_swift_NativeLib
 * Method:    Init
 * Signature: (Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_tudelft_triblerdroid_swift_NativeLib_Init
  (JNIEnv *, jobject, jstring, jstring);

/*
 * Class:     com_tudelft_triblerdroid_swift_NativeLib
 * Method:    Mainloop
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_tudelft_triblerdroid_swift_NativeLib_Mainloop
  (JNIEnv *, jobject);

/*
 * Class:     com_tudelft_triblerdroid_swift_NativeLib
 * Method:    Shutdown
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_tudelft_triblerdroid_swift_NativeLib_Shutdown
  (JNIEnv *, jobject);

/*
 * Class:     com_tudelft_triblerdroid_swift_NativeLib
 * Method:    asyncGetResult
 * Signature: (I)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_tudelft_triblerdroid_swift_NativeLib_asyncGetResult
  (JNIEnv *, jobject, jint);

/*
 * Class:     com_tudelft_triblerdroid_swift_NativeLib
 * Method:    asyncOpen
 * Signature: (Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_com_tudelft_triblerdroid_swift_NativeLib_asyncOpen
  (JNIEnv *, jobject, jstring, jstring, jstring);

/*
 * Class:     com_tudelft_triblerdroid_swift_NativeLib
 * Method:    hashCheckOffline
 * Signature: (Ljava/lang/String;)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_tudelft_triblerdroid_swift_NativeLib_hashCheckOffline
  (JNIEnv *, jobject, jstring);

/*
 * Class:     com_tudelft_triblerdroid_swift_NativeLib
 * Method:    SetTracker
 * Signature: (Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_com_tudelft_triblerdroid_swift_NativeLib_SetTracker
  (JNIEnv *, jobject, jstring);

/*
 * Class:     com_tudelft_triblerdroid_swift_NativeLib
 * Method:    asyncGetHTTPProgress
 * Signature: (Ljava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_com_tudelft_triblerdroid_swift_NativeLib_asyncGetHTTPProgress
  (JNIEnv *, jobject, jstring);

/*
 * Class:     com_tudelft_triblerdroid_swift_NativeLib
 * Method:    asyncGetStats
 * Signature: (Ljava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_com_tudelft_triblerdroid_swift_NativeLib_asyncGetStats
  (JNIEnv *, jobject, jstring);

/*
 * Class:     com_tudelft_triblerdroid_swift_NativeLib
 * Method:    LiveCreate
 * Signature: (Ljava/lang/String;)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_tudelft_triblerdroid_swift_NativeLib_LiveCreate
  (JNIEnv *, jobject, jstring);

/*
 * Class:     com_tudelft_triblerdroid_swift_NativeLib
 * Method:    LiveAdd
 * Signature: (Ljava/lang/String;[BII)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_tudelft_triblerdroid_swift_NativeLib_LiveAdd
  (JNIEnv *, jobject, jstring, jbyteArray, jint, jint);

#ifdef __cplusplus
}
#endif
#endif
