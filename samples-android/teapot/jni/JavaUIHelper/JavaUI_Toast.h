/*
 * Copyright 2013 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef JAVAUI_TOAST_H_
#define JAVAUI_TOAST_H_

//-------------------------------------------------
//Base class of JUIToast
//-------------------------------------------------
class JUIToast
{
    friend class JUIWindow;
private:
    static ATTRIBUTE_TYPE attributes_[];

    void InitToast();

protected:
    jobject obj_;
    jobject GetJobject()
    {
        return obj_;
    }
    static std::map<std::string, int32_t> map_attributes_;

public:
    JUIToast();
    JUIToast( const char* str );
    JUIToast( const char* str,
            const int32_t duration );
    virtual ~JUIToast();

    void SetView( JUIView* view );
    void Show();
    void Cancel();

    //Template for 1 parameter version of SetAttribute
    template<typename T>
    bool SetAttribute( std::map<std::string, int32_t>& map,
            const char* strAttribute,
            const T t )
    {
        std::map<std::string, int32_t>::iterator it = map.find( strAttribute );
        if( it != map.end() )
        {
            std::string s = std::string( "set" );
            s += it->first;

            switch( it->second )
            {
            case ATTRIBUTE_PARAMETER_INT:
                ndk_helper::JNIHelper::GetInstance()->CallVoidMethod( obj_, s.c_str(), "(I)V", (int32_t) t );
                break;
            default:
                LOGI( "Attribute parameter does not match : %s", strAttribute );
                break;
            }
        }
        else
        {
            LOGI( "Attribute '%s' not found", strAttribute );
            return false;
        }
        return true;
    }

    bool SetAttribute( std::map<std::string, int32_t>& map,
            const char* strAttribute,
            const char* str )
    {
        std::map<std::string, int32_t>::iterator it = map.find( strAttribute );
        if( it != map.end() )
        {
            std::string s = std::string( "set" );
            s += it->first;

            switch( it->second )
            {
            case ATTRIBUTE_PARAMETER_STRING:
            {
                JNIEnv *env = ndk_helper::JNIHelper::GetInstance()->Lock();
                jstring string = env->NewStringUTF( str );
                string = (jstring) env->NewGlobalRef( string );
                ndk_helper::JNIHelper::GetInstance()->CallVoidMethod( obj_, s.c_str(), "(Ljava/lang/CharSequence;)V",
                        string );
                env->DeleteGlobalRef( string );
                ndk_helper::JNIHelper::GetInstance()->Unlock();

            }
                break;
            default:
                LOGI( "Attribute parameter does not match : %s", strAttribute );
                break;
            }
        }
        else
        {
            LOGI( "Attribute '%s' not found", strAttribute );
            return false;
        }
        return true;
    }

    //Template for 2 parameters version of SetAttribute
    template<typename T, typename T2>
    bool SetAttribute( std::map<std::string, int32_t>& map,
            const char* strAttribute,
            T t,
            T2 t2 )
    {
        std::map<std::string, int32_t>::iterator it = map.find( strAttribute );
        if( it != map.end() )
        {
            std::string s = std::string( "set" );
            s += it->first;

            switch( it->second )
            {
            case ATTRIBUTE_PARAMETER_FF:
                ndk_helper::JNIHelper::GetInstance()->CallVoidMethod( obj_, s.c_str(), "(FF)V", (float) t, (float) t2 );
                break;
            default:
                LOGI( "Attribute parameter does not match : %s", strAttribute );
                break;
            }
        }
        else
        {
            LOGI( "Attribute '%s' not found", strAttribute );
            return false;
        }
        return true;
    }

    //Template for 3 parameters version of SetAttribute
    template<typename T, typename T2, typename T3>
    bool SetAttribute( std::map<std::string, int32_t>& map,
            const char* strAttribute,
            T t,
            T2 t2,
            T3 t3 )
    {
        std::map<std::string, int32_t>::iterator it = map.find( strAttribute );
        if( it != map.end() )
        {
            std::string s = std::string( "set" );
            s += it->first;

            switch( it->second )
            {
            case ATTRIBUTE_PARAMETER_III:
                ndk_helper::JNIHelper::GetInstance()->CallVoidMethod( obj_, s.c_str(), "(III)V", (int32_t) t,
                        (int32_t) t2, (int32_t) t3 );
                break;
            default:
                LOGI( "Attribute parameter does not match : %s", strAttribute );
                break;
            }
        }
        else
        {
            LOGI( "Attribute '%s' not found", strAttribute );
            return false;
        }
        return true;
    }

    template<typename T>
    bool SetAttribute( const char* strAttribute,
            const T t )
    {
        return SetAttribute( map_attributes_, strAttribute, t );
    }

    bool SetAttribute( const char* strAttribute,
            const char* str )
    {
        return SetAttribute( map_attributes_, strAttribute, str );
    }

    template<typename T, typename T2>
    bool SetAttribute( const char* strAttribute,
            T t,
            T2 t2 )
    {
        return SetAttribute( map_attributes_, strAttribute, t, t2 );
    }

    template<typename T, typename T2, typename T3, typename T4>
    bool SetAttribute( const char* strAttribute,
            T p1,
            T2 p2,
            T3 p3,
            T4 p4 )
    {
        return SetAttribute( map_attributes_, strAttribute, p1, p2, p3, p4 );
    }

    template<typename T>
    bool GetAttribute( std::map<std::string, int32_t>& map,
            const char* strAttribute,
            T& value )
    {
        T ret;
        std::map<std::string, int32_t>::iterator it = map.find( strAttribute );
        if( it != map.end() )
        {
            std::string s = std::string( "get" );
            s += it->first;

            switch( it->second )
            {
            case ATTRIBUTE_PARAMETER_INT:
                ret = (T) ndk_helper::JNIHelper::GetInstance()->CallIntMethod( obj_, s.c_str(), "()I" );
                break;
            case ATTRIBUTE_PARAMETER_FLOAT:
                ret = (T) ndk_helper::JNIHelper::GetInstance()->CallFloatMethod( obj_, s.c_str(), "()F" );
                break;
            default:
                ret = 0;
                break;
            }
        }
        else
        {
            LOGI( "Attribute '%s' not found", strAttribute );
            return false;
        }
        value = ret;
        return true;
    }

    template<typename T>
    bool GetAttribute( const char* strAttribute,
            T& value )
    {
        return GetAttribute( map_attributes_, strAttribute, value );
    }

};

#endif /* JAVAUI_TOAST_H_ */
