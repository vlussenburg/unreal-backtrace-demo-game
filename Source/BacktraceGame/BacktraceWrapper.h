#include <unordered_map>
#include <string>
#include <vector>

#if PLATFORM_ANDROID
#include "Android/AndroidApplication.h"
#include <jni.h>
#endif

DECLARE_LOG_CATEGORY_EXTERN(Backtrace, Log, All);
DEFINE_LOG_CATEGORY(Backtrace);

namespace BacktraceIO
{
#if PLATFORM_ANDROID
    // Pointer representing Java HashMap class
    jclass mapClassGlobalRef = nullptr;
    // HashMap::init method
    jmethodID initMap = nullptr;
    // HashMap::put method
    jmethodID putMap = nullptr;

    // Pointer representing Java List class
    jclass listClassGlobalRef = nullptr;
    // List::init method
    jmethodID initList = nullptr;
    // List::add method
    jmethodID addList = nullptr;

    void FInitializeStlStringStringMapToJavaHashMap(JNIEnv* Env)
    {
        mapClassGlobalRef = Env->FindClass("java/util/HashMap");
        FAndroidApplication::CheckJavaException();

        initMap = Env->GetMethodID(mapClassGlobalRef, "<init>", "()V");
        FAndroidApplication::CheckJavaException();

        putMap = Env->GetMethodID(mapClassGlobalRef, "put",
            "(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;");
        FAndroidApplication::CheckJavaException();
    }

    void FInitializeStlVectorStringToJavaListString(JNIEnv* Env)
    {
        listClassGlobalRef = Env->FindClass("java/util/ArrayList");
        FAndroidApplication::CheckJavaException();

        initList = Env->GetMethodID(listClassGlobalRef, "<init>", "()V");
        FAndroidApplication::CheckJavaException();

        addList = Env->GetMethodID(listClassGlobalRef, "add", "(Ljava/lang/Object;)Z");
        FAndroidApplication::CheckJavaException();
    }

    // Attribution: https://stackoverflow.com/a/53624436/15063264
    jobject FStlStringStringMapToJavaHashMap(JNIEnv* env,
        const std::unordered_map<std::string, std::string>& map) 
    {
        if (mapClassGlobalRef == nullptr || initMap == nullptr || putMap == nullptr) {
            UE_LOG(Backtrace, Error, TEXT("Required object(s) are null"));
            return nullptr;
        }
        if (env == nullptr) {
            UE_LOG(Backtrace, Error, TEXT("JNI env is null"));
            return nullptr;
        }

        jobject hashMap = env->NewObject(mapClassGlobalRef, initMap);

        std::unordered_map<std::string, std::string>::const_iterator citr = map.begin();
        for (; citr != map.end(); ++citr) {
            jstring keyJava = env->NewStringUTF(citr->first.c_str());
            jstring valueJava = env->NewStringUTF(citr->second.c_str());

            env->CallObjectMethod(hashMap, putMap, keyJava, valueJava);

            env->DeleteLocalRef(keyJava);
            env->DeleteLocalRef(valueJava);
        }

        jobject hashMapGlobal = static_cast<jobject>(env->NewGlobalRef(hashMap));
        env->DeleteLocalRef(hashMap);

        jboolean flag = env->ExceptionCheck();
        if (flag) {
            env->ExceptionDescribe();
            env->ExceptionClear();
            UE_LOG(Backtrace, Error, TEXT("Detected JNI Exception"));
            return nullptr;
        }
        return hashMapGlobal;
    }

    jobject FStlVectorStringToJavaListString(JNIEnv* env,
        const std::vector<std::string>& vector)
    {
        if (listClassGlobalRef == nullptr || initList == nullptr || addList == nullptr) {
            UE_LOG(Backtrace, Error, TEXT("Required object(s) are null"));
            return nullptr;
        }
        if (env == nullptr) {
            UE_LOG(Backtrace, Error, TEXT("JNI env is null"));
            return nullptr;
        }

        jobject list = env->NewObject(listClassGlobalRef, initList);

        std::vector<std::string>::const_iterator citr = vector.begin();
        for (; citr != vector.end(); ++citr) {
            jstring stringJava = env->NewStringUTF(citr->c_str());

            env->CallBooleanMethod(list, addList, stringJava);

            env->DeleteLocalRef(stringJava);
        }

        jobject listGlobal = static_cast<jobject>(env->NewGlobalRef(list));
        env->DeleteLocalRef(list);

        jboolean flag = env->ExceptionCheck();
        if (flag) {
            env->ExceptionDescribe();
            env->ExceptionClear();
            UE_LOG(Backtrace, Error, TEXT("Detected JNI Exception"));
            return nullptr;
        }
        return listGlobal;
    }

    std::unordered_map<std::string, std::string> ConvertTMapToStdMap(const TMap<FString, FString>& Map)
    {
        std::unordered_map<std::string, std::string> Result;
        for (const auto& Pair : Map)
        {
            std::string Key = std::string(TCHAR_TO_UTF8(*(Pair.Key)));
            std::string Value = std::string(TCHAR_TO_UTF8(*(Pair.Value)));
            Result[Key] = Value;
        }
        return Result;
    }

    std::vector<std::string> ConvertTArrayToStdVector(const TArray<FString>& Array)
    {
        std::vector<std::string> Result;
        for (const auto& String : Array)
        {
            std::string StdString = std::string(TCHAR_TO_UTF8(*(String)));
            Result.push_back(StdString);
        }
        return Result;
    }

#endif
    // Attribution: https://forums.unrealengine.com/development-discussion/android-development/106221-custom-java-method-called-from-c-causes-classnotfoundexception
    bool FInitializeBacktraceClient(const TMap<FString, FString>& Attributes, const TArray<FString>& Attachments)
    {
        FString MethodName = "initializeBacktraceClient";
        FString MethodSignature = "(Ljava/util/Map;Ljava/util/List;)V";
#if PLATFORM_ANDROID
        JNIEnv* Env = FAndroidApplication::GetJavaEnv(false);
        FAndroidApplication::CheckJavaException();

        FInitializeStlStringStringMapToJavaHashMap(Env);
        FInitializeStlVectorStringToJavaListString(Env);

        jobject Activity = FAndroidApplication::GetGameActivityThis();
        FAndroidApplication::CheckJavaException();

        if (Activity != nullptr)
        {
            jclass ActivityClass = Env->GetObjectClass(Activity);
            FAndroidApplication::CheckJavaException();
            
            if (ActivityClass != nullptr)
            {
                jmethodID Method = Env->GetMethodID(ActivityClass, TCHAR_TO_ANSI(*MethodName), TCHAR_TO_ANSI(*MethodSignature));
                FAndroidApplication::CheckJavaException();

                if (Method != nullptr)
                {
                    auto StdAttributes = ConvertTMapToStdMap(Attributes);
                    auto StdAttachments = ConvertTArrayToStdVector(Attachments);

                    jobject JavaAttributes = FStlStringStringMapToJavaHashMap(Env, StdAttributes);
                    jobject JavaAttachments = FStlVectorStringToJavaListString(Env, StdAttachments);

                    if (JavaAttributes != nullptr && JavaAttachments != nullptr)
                    {
                        Env->CallVoidMethod(Activity, Method, JavaAttributes, JavaAttachments);
                        FAndroidApplication::CheckJavaException();

                        Env->DeleteGlobalRef(JavaAttributes);
                        FAndroidApplication::CheckJavaException();
                        Env->DeleteGlobalRef(JavaAttachments);
                        FAndroidApplication::CheckJavaException();
                    }
                    else
                    {
                        UE_LOG(Backtrace, Error, TEXT("Could not create arguments for Java initializeBacktraceClient method"));
                        return false;
                    }
                }
                else
                {
                    UE_LOG(Backtrace, Error, TEXT("Could not get method ID for Java initializeBacktraceClient method"));
                    return false;
                }
            }
            else
            {
                UE_LOG(Backtrace, Error, TEXT("Could not get Java GameActivity class to extract initializeBacktraceClient method"));
                return false;
            }
        }
        else
        {
            UE_LOG(Backtrace, Error, TEXT("Could not get GameActivity object"));
            return false;
        }

        return true;
#else
        return false;
#endif
    }
};
