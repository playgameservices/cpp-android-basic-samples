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

#ifndef JAVAUI_VIEW_H_
#define JAVAUI_VIEW_H_

//-------------------------------------------------
//Base class of JUIView
//-------------------------------------------------
class JUIBase
{
protected:
    std::map<std::string, ATTRIBUTE_PARAMETER_STORE> map_attribute_parameters;
    jobject obj_;
    jobject GetJobject()
    {
        return obj_;
    }
public:
    JUIBase() :
            obj_( NULL )
    {

    }
    virtual ~JUIBase()
    {

    }

    virtual void DispatchEvent( const int32_t message,
            const int32_t param1,
            const int32_t param2 )
    {
    }
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

            ATTRIBUTE_PARAMETER_STORE& p = map_attribute_parameters[ it->first ];
            switch( it->second )
            {
            case ATTRIBUTE_PARAMETER_INT:
                ndk_helper::JNIHelper::GetInstance()->CallVoidMethod( obj_, s.c_str(), "(I)V", (int32_t) t );
                p.type = ATTRIBUTE_PARAMETER_INT;
                p.i = (int32_t) t;
                break;
            case ATTRIBUTE_PARAMETER_FLOAT:
                ndk_helper::JNIHelper::GetInstance()->CallVoidMethod( obj_, s.c_str(), "(F)V", (float) t );
                p.type = ATTRIBUTE_PARAMETER_FLOAT;
                p.f = (float) t;
                break;
            case ATTRIBUTE_PARAMETER_BOOLEAN:
                ndk_helper::JNIHelper::GetInstance()->CallVoidMethod( obj_, s.c_str(), "(Z)V", (bool) t );
                p.type = ATTRIBUTE_PARAMETER_BOOLEAN;
                p.b = (bool) t;
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

            ATTRIBUTE_PARAMETER_STORE& p = map_attribute_parameters[ it->first ];
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

                p.type = ATTRIBUTE_PARAMETER_STRING;
                if( p.str != NULL )
                {
                    if( p.str->compare( str ) != 0 )
                    {
                        delete p.str;
                        p.str = new std::string( str );
                    }
                }
                else
                {
                    p.str = new std::string( str );
                }
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

            ATTRIBUTE_PARAMETER_STORE& p = map_attribute_parameters[ it->first ];
            switch( it->second )
            {
            case ATTRIBUTE_PARAMETER_IF:
                ndk_helper::JNIHelper::GetInstance()->CallVoidMethod( obj_, s.c_str(), "(IF)V", (int32_t) t,
                        (float) t2 );
                p.type = ATTRIBUTE_PARAMETER_IF;
                p.param_if.i1 = (int32_t) t;
                p.param_if.f2 = (float) t2;
                break;
            case ATTRIBUTE_PARAMETER_FF:
                ndk_helper::JNIHelper::GetInstance()->CallVoidMethod( obj_, s.c_str(), "(FF)V", (float) t, (float) t2 );
                p.type = ATTRIBUTE_PARAMETER_FF;
                p.param_ff.f1 = (float) t;
                p.param_ff.f2 = (float) t2;
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

    //Template for 4 parameters version of SetAttribute
    template<typename T, typename T2, typename T3, typename T4>
    bool SetAttribute( std::map<std::string, int32_t>& map,
            const char* strAttribute,
            T p1,
            T2 p2,
            T3 p3,
            T4 p4 )
    {
        std::map<std::string, int32_t>::iterator it = map.find( strAttribute );
        if( it != map.end() )
        {
            std::string s = std::string( "set" );
            s += it->first;

            ATTRIBUTE_PARAMETER_STORE& p = map_attribute_parameters[ it->first ];
            switch( it->second )
            {
            case ATTRIBUTE_PARAMETER_IIII:
                ndk_helper::JNIHelper::GetInstance()->CallVoidMethod( obj_, s.c_str(), "(IIII)V", (int32_t) p1,
                        (int32_t) p2, (int32_t) p3, (int32_t) p4 );
                p.type = ATTRIBUTE_PARAMETER_IIII;
                p.param_iiii.i1 = (int32_t) p1;
                p.param_iiii.i2 = (int32_t) p2;
                p.param_iiii.i3 = (int32_t) p3;
                p.param_iiii.i4 = (int32_t) p4;
                break;
            case ATTRIBUTE_PARAMETER_FFFI:
                ndk_helper::JNIHelper::GetInstance()->CallVoidMethod( obj_, s.c_str(), "(FFFI)V", (float) p1,
                        (float) p2, (float) p3, (int32_t) p4 );
                p.type = ATTRIBUTE_PARAMETER_FFFI;
                p.param_fffi.f1 = (float) p1;
                p.param_fffi.f2 = (float) p2;
                p.param_fffi.f3 = (float) p3;
                p.param_fffi.i = (int32_t) p4;
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
};

class JUIView: public JUIBase
{
    friend class JUIWindow;
    friend class JUILinearLayout;
    friend class JUIRelativeLayout;
    friend class JUIRadioGroup;
    friend class JUIToast;
    friend class JUIDialog;
    friend class JUIAlertDialog;
private:
    static ATTRIBUTE_TYPE attributes_[];
    int32_t array_current_rules_[ LAYOUT_PARAMETER_COUNT ];

    int32_t layoutWidth_;
    int32_t layoutHeight_;
    float layoutWeight_;
protected:
    static std::map<std::string, int32_t> map_attributes_;

    void RestoreParameters( std::map<std::string, int32_t>& map );
    virtual void Restore() = 0;

public:
    JUIView();
    virtual ~JUIView();

    void AddRule( const int32_t layoutParameterIndex,
            const int32_t parameter );
    void AddRule( const int32_t layoutParameterIndex,
            const JUIView* parameter );
    /*
     * Set LayoutParams for RelativeLayout
     */
    void SetLayoutParams( const int32_t width,
            const int32_t height );
    /*
     * Set LayoutParams for LinearLayout
     */
    void SetLayoutParams( const int32_t width,
            const int32_t height,
            const float f );

    template<typename T>
    bool SetAttribute( const char* strAttribute,
            const T t )
    {
        return SetAttribute( map_attributes_, strAttribute, t );
    }

    bool SetAttribute( const char* strAttribute,
            const char* str )
    {
        return JUIBase::SetAttribute( map_attributes_, strAttribute, str );
    }

    template<typename T, typename T2>
    bool SetAttribute( const char* strAttribute,
            T t,
            T2 t2 )
    {
        return JUIBase::SetAttribute( map_attributes_, strAttribute, t, t2 );
    }

    template<typename T, typename T2, typename T3, typename T4>
    bool SetAttribute( const char* strAttribute,
            T p1,
            T2 p2,
            T3 p3,
            T4 p4 )
    {
        return JUIBase::SetAttribute( map_attributes_, strAttribute, p1, p2, p3, p4 );
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
            case ATTRIBUTE_PARAMETER_BOOLEAN:
                ret = (T) ndk_helper::JNIHelper::GetInstance()->CallBooleanMethod( obj_, s.c_str(), "()Z" );
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

#endif /* JAVAUI_VIEW_H_ */
