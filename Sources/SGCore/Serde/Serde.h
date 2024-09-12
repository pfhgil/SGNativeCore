//
// Created by Ilya on 14.08.2024.
//

#ifndef SUNGEARENGINE_SERDE_H
#define SUNGEARENGINE_SERDE_H

#include "SGCore/Utils/TypeTraits.h"
#include "SGCore/Utils/Utils.h"
#include "SGCore/Main/CoreGlobals.h"

namespace SGCore::Serde
{
    enum class FormatType
    {
        JSON,
        BSON,
        YAML
    };

    // ==============================================================================
    // ==============================================================================
    // ==============================================================================

    // USE THAT STRUCTS TO DEFINE BASE CLASSES OF SERIALIZABLE TYPE AND DERIVED CLASSES
    /**
     * Contains base types
     * @tparam Cls
     */
    template<typename... Cls>
    struct BaseTypes
    {
        static constexpr std::uint64_t base_classes_count = sizeof...(Cls);

        template<std::int64_t Idx>
        using get_base_type = extract<Idx, Cls...>::type;
    };

    /**
     * Contains derived types
     * @tparam Cls
     */
    template<typename... Cls>
    struct DerivedTypes
    {
        static constexpr std::uint64_t derived_classes_count = sizeof...(Cls);

        template<std::int64_t Idx>
        using get_derived_type = extract<Idx, Cls...>::type;
    };

    template<typename T, FormatType TFormatType>
    struct SerdeSpec;

    // ==============================================================================
    // ==============================================================================
    // ==============================================================================

    /**
     * Container that contains pointers to object format specific types.\n
     * You must implement all next functions and members in your specialization of this struct.\n
     * DO NOT STORE ANYWHERE. DOES NOT OWN ANYTHING.
     * @tparam T
     * @tparam TFormatType
     */
    template<FormatType TFormatType>
    struct DeserializableValueContainer
    {
        friend struct Serializer;

        template<typename T0, FormatType TFormatType0>
        friend struct DeserializableValueView;

        template<typename T>
        std::optional<T> getMember(const std::string& memberName) noexcept
        {

        }

        /**
         * Getting this container value as array.
         * @param f
         */
        template<typename T>
        [[nodiscard]] std::vector<T> getAsArray() noexcept
        {

        }

        /**
        * Getting this container value as int64.
        * @param f
        */
        [[nodiscard]] std::int64_t getAsInt64() const noexcept
        {

        }

        /**
        * Getting this container value as float.
        * @param f
        */
        [[nodiscard]] float getAsFloat() const noexcept
        {

        }

        /**
        * Getting this container value as string.
        * @param f
        */
        template<typename CharT>
        [[nodiscard]] std::basic_string<CharT> getAsString() const noexcept
        {

        }

        /**
        * Getting this container value as bool.
        * @param f
        */
        [[nodiscard]] bool getAsBool() const noexcept
        {

        }

        /**
         * Getting first iterator of members container.
         * @return First iterator of members container (begin).
         */
        [[nodiscard]] int memberBegin() const noexcept
        {

        }

        /**
         * Getting last iterator of members container.
         * @return Last iterator of members container (end).
         */
        [[nodiscard]] int memberEnd() const noexcept
        {

        }

        /**
         * Getting member by iterator.
         * @tparam T - Type of member.
         * @param iterator - Member iterator.
         * @return Value of member.
         */
        template<typename T>
        T getMember(const int& iterator) noexcept
        {

        }

        /**
         * Getting name of member by iterator.
         * @param memberIterator
         * @return
         */
        [[nodiscard]] std::string getMemberName(const int& memberIterator) const noexcept
        {

        }

        [[nodiscard]] bool isNull() const noexcept
        {

        }

    private:
        DeserializableValueContainer<TFormatType>* m_parent { };
        std::string* m_outputLog { };
    };

    template<>
    struct DeserializableValueContainer<FormatType::JSON>
    {
        friend struct Serializer;

        template<typename T0, FormatType TFormatType0>
        friend struct DeserializableValueView;

        template<typename T>
        std::optional<T> getMember(const std::string& memberName) noexcept
        {
            if(!(m_thisValue || m_document))
            {
                if(m_outputLog)
                {
                    *m_outputLog = "Error: Can not get member with name '" + memberName + "': m_thisValue or m_document is null.\n";
                }

                return std::nullopt;
            }

            if(!m_thisValue->HasMember(memberName.c_str()))
            {
                if(m_outputLog)
                {
                    *m_outputLog = "Error: Can not get member with name '" + memberName + "': this member does not exist.\n";
                }

                return std::nullopt;
            }

            // getting member
            auto& member = (*m_thisValue)[memberName.c_str()];

            if(!member.HasMember("typeName"))
            {
                if(m_outputLog)
                {
                    *m_outputLog = "Error: Can not get member with name '" + memberName + "': this member does not have 'typeName' section.\n";
                }

                return std::nullopt;
            }

            // getting typeName section of member
            const std::string& typeName = member["typeName"].GetString();

            if(!member.HasMember("value"))
            {
                if(m_outputLog)
                {
                    *m_outputLog = "Error: Can not get member with name '" + memberName + "': this member does not have 'value' section.\n";
                }

                return std::nullopt;
            }

            // getting value section of member
            auto& memberValue = member["value"];

            T outputValue { };

            // creating value view of member
            DeserializableValueView<T, FormatType::JSON> valueView { };
            valueView.getValueContainer().m_document = m_document;
            valueView.getValueContainer().m_thisValue = &memberValue;
            valueView.getValueContainer().m_parent = this;
            valueView.getValueContainer().m_outputLog = m_outputLog;
            valueView.getValueContainer().m_typeName = typeName;
            valueView.m_data = &outputValue;

            // deserializing member with dynamic checks
            Serializer::deserializeWithDynamicChecks(valueView);

            return outputValue;
        }

        template<typename T>
        [[nodiscard]] std::vector<T> getAsArray() noexcept
        {
            if(!(m_thisValue || m_document))
            {
                if(m_outputLog)
                {
                    *m_outputLog = "Error: Can not get value as array: m_thisValue or m_document is null.\n";
                }

                return { };
            }

            if(!m_thisValue->IsArray())
            {
                if(m_outputLog)
                {
                    *m_outputLog = "Error: Can not get value as array: value is not an array.\n";
                }

                return { };
            }

            std::vector<T> outputValue;

            for(std::size_t i = 0; i < m_thisValue->Size(); ++i)
            {
                T tmpVal { };

                auto& rootValue = (*m_thisValue)[i];

                if(!rootValue.HasMember("typeName"))
                {
                    if(m_outputLog)
                    {
                        *m_outputLog = "Error: Can not get member with index '" + std::to_string(i) +
                                       "' from array: this member does not have 'typeName' section.\n";
                    }

                    continue;
                }

                // getting 'typeName' section
                const std::string typeNameSection = rootValue["typeName"].GetString();

                if(!rootValue.HasMember("value"))
                {
                    if(m_outputLog)
                    {
                        *m_outputLog = "Error: Can not get member with index '" + std::to_string(i) +
                                       "' from array: this member does not have 'value' section.\n";
                    }

                    continue;
                }

                auto& valueSection = rootValue["value"];

                // creating value view of member
                DeserializableValueView<T, FormatType::JSON> valueView { };
                valueView.getValueContainer().m_document = m_document;
                valueView.getValueContainer().m_thisValue = &valueSection;
                valueView.getValueContainer().m_parent = this;
                valueView.getValueContainer().m_outputLog = m_outputLog;
                valueView.getValueContainer().m_typeName = typeNameSection;
                valueView.m_data = &tmpVal;

                Serializer::deserializeWithDynamicChecks(valueView);

                outputValue.push_back(tmpVal);
            }

            return outputValue;
        }

        [[nodiscard]] std::int64_t getAsInt64() const noexcept
        {
            if(!m_thisValue)
            {
                if(m_outputLog)
                {
                    *m_outputLog = "Error: Can not get value as int64: m_thisValue is null.\n";
                }

                return { };
            }

            return m_thisValue->GetInt64();
        }

        [[nodiscard]] float getAsFloat() const noexcept
        {
            if(!m_thisValue)
            {
                if(m_outputLog)
                {
                    *m_outputLog = "Error: Can not get value as float: m_thisValue is null.\n";
                }

                return { };
            }

            return m_thisValue->GetFloat();
        }

        template<typename CharT>
        [[nodiscard]] std::basic_string<CharT> getAsString() const noexcept
        {
            if(!m_thisValue)
            {
                if(m_outputLog)
                {
                    *m_outputLog = "Error: Can not get value as float: m_thisValue is null.\n";
                }

                return { };
            }

            return SGCore::Utils::template fromUTF8<CharT>(m_thisValue->GetString());
        }

        [[nodiscard]] bool getAsBool() const noexcept
        {
            if(!m_thisValue) return false;

            return m_thisValue->GetBool();
        }

        [[nodiscard]] rapidjson::Value::MemberIterator memberBegin() const noexcept
        {
            if(!m_thisValue) return { };

            return m_thisValue->MemberBegin();
        }

        [[nodiscard]] rapidjson::Value::MemberIterator memberEnd() const noexcept
        {
            if(!m_thisValue) return { };

            return m_thisValue->MemberEnd();
        }

        template<typename T>
        std::optional<T> getMember(const rapidjson::Value::MemberIterator& memberIterator) noexcept
        {
            if(!(m_thisValue || m_document))
            {
                if(m_outputLog)
                {
                    *m_outputLog = "Error: Can not get member with name '" + std::string(memberIterator->name.GetString()) +
                            "': m_thisValue or m_document is null.\n";
                }

                return std::nullopt;
            }

            // getting member
            auto& member = memberIterator->value;

            if(!member.HasMember("typeName"))
            {
                if(m_outputLog)
                {
                    *m_outputLog = "Error: Can not get member with name '" + std::string(memberIterator->name.GetString()) +
                            "': this member does not have 'typeName' section.\n";
                }

                return std::nullopt;
            }

            // getting typeName section of member
            const std::string& typeName = member["typeName"].GetString();

            if(!member.HasMember("value"))
            {
                if(m_outputLog)
                {
                    *m_outputLog = "Error: Can not get member with name '" + std::string(memberIterator->name.GetString()) +
                            "': this member does not have 'value' section.\n";
                }

                return std::nullopt;
            }

            // getting value section of member
            auto& memberValue = member["value"];

            T outputValue { };

            // creating value view of member
            DeserializableValueView<T, FormatType::JSON> valueView { };
            valueView.getValueContainer().m_document = m_document;
            valueView.getValueContainer().m_thisValue = &memberValue;
            valueView.getValueContainer().m_parent = this;
            valueView.getValueContainer().m_outputLog = m_outputLog;
            valueView.getValueContainer().m_typeName = typeName;
            valueView.m_data = &outputValue;

            // deserializing member with dynamic checks
            Serializer::deserializeWithDynamicChecks(valueView);

            return outputValue;
        }

        [[nodiscard]] std::string getMemberName(const rapidjson::Value::MemberIterator& memberIterator) const noexcept
        {
            return memberIterator->name.GetString();
        }

        [[nodiscard]] bool isNull() const noexcept
        {
            return m_thisValue && m_thisValue->IsNull();
        }

    private:
        DeserializableValueContainer<FormatType::JSON>* m_parent { };
        rapidjson::Document* m_document { };
        rapidjson::Value* m_thisValue { };
        std::string* m_outputLog { };
        std::string m_typeName;
    };

    /**
     * Container that contains pointers to object format specific types.\n
     * You must implement all next functions and members in your specialization of this struct.\n
     * DO NOT STORE ANYWHERE. DOES NOT OWN ANYTHING.
     * @tparam T
     * @tparam TFormatType
     */
    template<FormatType TFormatType>
    struct SerializableValueContainer
    {
        friend struct Serializer;

        template<typename T0, FormatType TFormatType0>
        friend struct SerializableValueView;

        /**
         * Add member to this container.\n
         * If member with that name is already exists than value of this member will be changed.
         * @tparam T
         * @param name
         * @param value
         */
        template<typename T>
        SerializableValueView<T, TFormatType> addMember(const std::string& name, const T& value) noexcept
        {

        }

        /**
         * Add value in this container if it is array.
         * @tparam T
         * @param name
         * @param value
         */
        template<typename T>
        void pushBack(const T& value) noexcept
        {

        }

        /**
         * Setting this container value as null.
         * @param f
         */
        void setAsNull() noexcept
        {

        }

        /**
         * Setting this container value as float.
         * @param f
         */
        void setAsFloat(const float& f) noexcept
        {

        }

        /**
         * Setting this container value as int64.
         * @param f
         */
        void setAsInt64(const std::int64_t& i) noexcept
        {

        }

        /**
         * Setting this container value as string.
         * @param str
         */
        template<typename CharT>
        void setAsString(const std::basic_string<CharT>& str) noexcept
        {

        }

        /**
         * Setting this container value as array.\n
         * After calling this function you can call function pushBack.
         * @param f
         */
        void setAsArray() noexcept
        {

        }

        /**
         * Setting this container value as bool.
         * @param str
         */
        void setAsBool(bool b) noexcept
        {

        }

        /**
         * Setter for type name of value.
         * @param typeName
         */
        void setTypeName(const std::string& typeName) noexcept
        {

        }

        /**
         * Getter for type name of value.
         * @param typeName
         */
        [[nodiscard]] std::string getTypeName(const std::string& typeName) const noexcept
        {

        }

        /**
         * Returning parent container.
         * @return
         */
        auto& getParent() noexcept
        {
            return *m_parent;
        }

    private:
        SerializableValueContainer<TFormatType>* m_parent { };
    };

    /**
     * Value container for JSON format.
     */
    template<>
    struct SerializableValueContainer<FormatType::JSON>
    {
        friend struct Serializer;

        template<typename T0, FormatType TFormatType0>
        friend struct SerializableValueView;

        // todo: ADD createMember FUNCTION

        template<typename T>
        SerializableValueView<T, FormatType::JSON> addMember(const std::string& name, const T& value) noexcept
        {
            if (!(m_thisValue || m_document)) return { };

            // removing member with this name if it is already exists
            if (m_thisValue->IsObject() && m_thisValue->HasMember(name.c_str()))
            {
                m_thisValue->RemoveMember(name.c_str());
            }

            // creating key that contains passed name
            rapidjson::Value valueNameKey(rapidjson::kStringType);
            // creating value root section
            rapidjson::Value valueRootSection(rapidjson::kObjectType);

            // setting name
            valueNameKey.SetString(name.c_str(), name.length(), m_document->GetAllocator());

            // creating type name of T value
            rapidjson::Value typeNameSectionValue(rapidjson::kStringType);
            typeNameSectionValue.SetString(SerdeSpec<T, FormatType::JSON>::type_name.c_str(),
                                           SerdeSpec<T, FormatType::JSON>::type_name.length());

            // creating section that will contain all members of T0
            rapidjson::Value valueSectionValue(rapidjson::kObjectType);

            // ==== value serialization code

            // creating view of value with format pointers
            SerializableValueView<T, FormatType::JSON> valueView {};
            valueView.m_data = &value;
            valueView.getValueContainer().m_document = m_document;
            valueView.getValueContainer().m_thisValue = &valueSectionValue;
            valueView.getValueContainer().m_typeNameValue = &typeNameSectionValue;
            valueView.getValueContainer().m_parent = this;

            // serializing value with attempt at dynamic casts to derived types
            Serializer::serializeWithDynamicChecks<T, FormatType::JSON>(valueView);

            // =======================

            // adding typeName section
            valueRootSection.AddMember("typeName", typeNameSectionValue, m_document->GetAllocator());

            // adding value section
            valueRootSection.AddMember("value", valueSectionValue, m_document->GetAllocator());

            if(m_thisValue->IsArray())
            {
                m_thisValue->PushBack(valueRootSection, m_document->GetAllocator());

                return valueView;
            }

            // adding value section to document
            m_thisValue->AddMember(valueNameKey, valueRootSection, m_document->GetAllocator());

            return valueView;
        }

        template<typename T>
        void pushBack(const T& value) noexcept
        {
            addMember("", value);
        }

        void setAsNull() noexcept
        {
            if(!m_thisValue) return;

            m_thisValue->SetNull();
        }

        void setAsFloat(const float& f) noexcept
        {
            if(!m_thisValue) return;

            m_thisValue->SetFloat(f);
        }

        void setAsInt64(const std::int64_t& i) noexcept
        {
            if(!m_thisValue) return;

            m_thisValue->SetInt64(i);
        }

        template<typename CharT>
        void setAsString(const std::basic_string<CharT>& str) noexcept
        {
            if(!(m_thisValue || m_document)) return;

            const std::string utf8String = SGCore::Utils::toUTF8(str);

            m_thisValue->SetString(utf8String.c_str(), utf8String.length(), m_document->GetAllocator());
        }

        void setAsArray() noexcept
        {
            if(!m_thisValue) return;

            m_thisValue->SetArray();
        }

        void setAsBool(bool b) noexcept
        {
            if(!m_thisValue) return;

            m_thisValue->SetBool(b);
        }

        void setTypeName(const std::string& typeName) noexcept
        {
            if(!(m_typeNameValue || m_document)) return;

            m_typeNameValue->SetString(typeName.c_str(), typeName.length(), m_document->GetAllocator());
        }

        [[nodiscard]] std::string getTypeName(const std::string& typeName) const noexcept
        {
            if(!m_typeNameValue) return "";

            return m_typeNameValue->GetString();
        }

        auto& getParent() noexcept
        {
            return *m_parent;
        }

    private:
        SerializableValueContainer<FormatType::JSON>* m_parent { };
        rapidjson::Document* m_document { };
        rapidjson::Value* m_thisValue { };
        rapidjson::Value* m_typeNameValue { };
    };

    // ==============================================================================
    // ==============================================================================
    // ==============================================================================

    /**
     * View of value with format type`s specific members. Contains serializable value info container.\n
     * You must implement all next functions and members in your specialization of this struct.\n
     * DO NOT STORE ANYWHERE. DOES NOT OWN ANYTHING.
     * @tparam T
     * @tparam TFormatType
     */
    template<typename T, FormatType TFormatType>
    struct SerializableValueView
    {
        friend struct Serializer;

        // making all SerializableValueView as friends
        template<typename T0, FormatType TFormatType0>
        friend struct SerializableValueView;

        static inline constexpr FormatType format_type = TFormatType;

        const T* m_data { };

        auto& getValueContainer() noexcept
        {
            return m_valueContainer;
        }

    private:
        std::string m_version;

        SerializableValueContainer<TFormatType> m_valueContainer { };
    };

    /**
     * View of value with format type`s specific members. Contains deserializable value info container.\n
     * You must implement all next functions and members in your specialization of this struct.\n
     * DO NOT STORE ANYWHERE. DOES NOT OWN ANYTHING.
     * @tparam T
     * @tparam TFormatType
     */
    template<typename T, FormatType TFormatType>
    struct DeserializableValueView
    {
        friend struct Serializer;

        // making all SerializableValueView as friends
        template<typename T0, FormatType TFormatType0>
        friend struct DeserializableValueView;

        static inline constexpr FormatType format_type = TFormatType;

        T* m_data { };

        auto& getValueContainer() noexcept
        {
            return m_valueContainer;
        }

    private:
        std::string m_version;

        DeserializableValueContainer<TFormatType> m_valueContainer { };
    };

    // ==============================================================================
    // ==============================================================================
    // ==============================================================================

    /**
     * Primary SerdeSpec structure. Use this structure body as example to implement custom SerdeSpec.\n
     * @tparam T
     */
    template<typename T, FormatType TFormatType>
    struct SerdeSpec : BaseTypes<>, DerivedTypes<>
    {
        static_assert(always_false<T>::value,
                      "This type is not serializable. Please, implement specialization of SerdeSpec for this type.");

        /**
         * The type name of T
         */
        static inline const std::string type_name = "unknown";
        /**
         * Is T is pointer type
         */
        static inline constexpr bool is_pointer_type = false;
        /**
         * OPTIONAL (IF TYPE IS POINTER): type that pointer points to
         */
        using element_type = void;

        /**
         * Serializes only T type members.
         * @param valueView
         */
        static void serialize(SerializableValueView<T, TFormatType>& valueView)
        {

        }

        /**
         * Deserializes only T type members.
         * @param valueView
         */
        static void deserialize(DeserializableValueView<T, TFormatType>& valueView)
        {

        }

        /**
         * OPTIONAL (IF TYPE IS POINTER): getting raw pointer to object
         * @param valueView
         */
        template<typename ValueViewT>
        static T* getObjectRawPointer(ValueViewT& valueView) noexcept
        {

        }

        /**
         * OPTIONAL (IF TYPE IS POINTER): setting raw pointer to object
         * @param valueView
         */
        template<typename ValueViewT, typename T0>
        static void setObjectRawPointer(ValueViewT& valueView, T0* pointer) noexcept
        {

        }
    };


    // ================================================================
    // ================================================================
    // ================================================================

    struct Utils
    {
        /**
         * Is SerdeSpec of T provides base types.
         * @tparam T
         * @return
         */
        template<typename T, FormatType TFormatType>
        static consteval bool isBaseTypesProvided()
        {
            if constexpr(requires { SerdeSpec<T, TFormatType>::base_classes_count; })
            {
                if constexpr(SerdeSpec<T, TFormatType>::base_classes_count > 0)
                {
                    if constexpr(requires { typename SerdeSpec<T, TFormatType>::template get_base_type<0>; })
                    {
                        return true;
                    }
                }
            }

            return false;
        }

        /**
        * Is SerdeSpec of T provides derived types.
        * @tparam T
        * @return
        */
        template<typename T, FormatType TFormatType>
        static consteval bool isDerivedTypesProvided()
        {
            if constexpr(requires { SerdeSpec<T, TFormatType>::derived_classes_count; })
            {
                if constexpr(SerdeSpec<T, TFormatType>::derived_classes_count > 0)
                {
                    if constexpr(requires { typename SerdeSpec<T, TFormatType>::template get_derived_type<0>; })
                    {
                        return true;
                    }
                }
            }

            return false;
        }
    };

    // ================================================================
    // ================================================================
    // ================================================================

    struct Serializer
    {
        template<typename T, FormatType TFormatType>
        friend struct SerializableValueView;

        template<FormatType TFormatType>
        friend struct SerializableValueContainer;

        template<FormatType TFormatType>
        friend struct DeserializableValueContainer;

        /**
         * Converts object to some format (JSON, BSON, YAML, etc)
         * @tparam T
         * @param value
         * @param formatType
         * @return
         */
        template<typename T>
        static std::string toFormat(T& value, FormatType formatType = m_defaultFormatType) noexcept
        {
            switch (formatType)
            {
                case FormatType::JSON:
                {
                    rapidjson::Document document;
                    document.SetObject();

                    return toJSON(document, value);
                }
                case FormatType::BSON:
                    break;
                case FormatType::YAML:
                    break;
            }

            return "";
        }

        /**
         * Deserializes object from some format (JSON, BSON, YAML, etc)
         * @tparam T
         * @param value
         * @param formatType
         * @return
         */
        template<typename T>
        static void fromFormat(const std::string& formattedText,
                               T& outValue,
                               std::string& outputLog,
                               FormatType formatType = m_defaultFormatType) noexcept
        {
            switch (formatType)
            {
                case FormatType::JSON:
                {
                    rapidjson::Document document;
                    document.Parse(formattedText.c_str());

                    fromJSON(document, outputLog, outValue);

                    break;
                }
                case FormatType::BSON:
                    break;
                case FormatType::YAML:
                    break;
            }
        }

    private:
        static inline FormatType m_defaultFormatType = SGCore::Serde::FormatType::JSON;

        /**
         * Converts object to JSON format
         * @tparam T
         * @param toDocument
         * @param value
         * @return
         */
        template<typename T>
        static std::string toJSON(rapidjson::Document& toDocument,
                                  const T& value) noexcept
        {
            // adding version of document
            toDocument.AddMember("version", "1", toDocument.GetAllocator());

            // adding type name of T
            rapidjson::Value typeNameSectionValue(rapidjson::kStringType);
            typeNameSectionValue.SetString(SerdeSpec<T, FormatType::JSON>::type_name.c_str(),
                                           SerdeSpec<T, FormatType::JSON>::type_name.length());

            // creating section that will contain all members of T
            rapidjson::Value valueSectionValue(rapidjson::kObjectType);

            // ==== serialization code

            // creating view of value with format pointers
            SerializableValueView<T, FormatType::JSON> valueView  { };
            valueView.m_data = &value;
            valueView.m_version = "1";
            valueView.getValueContainer().m_document = &toDocument;
            valueView.getValueContainer().m_thisValue = &valueSectionValue;
            valueView.getValueContainer().m_typeNameValue = &typeNameSectionValue;

            // serializing value with attempt at dynamic casts to derived types
            serializeWithDynamicChecks<T, FormatType::JSON>(valueView);

            // =======================

            // adding type name of T after serializing (because type name can be changed)
            toDocument.AddMember("typeName", typeNameSectionValue, toDocument.GetAllocator());

            // adding value section to document
            toDocument.AddMember("value", valueSectionValue, toDocument.GetAllocator());

            // converting to string
            rapidjson::StringBuffer stringBuffer;
            stringBuffer.Clear();
            rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(stringBuffer);
            toDocument.Accept(writer);

            return stringBuffer.GetString();
        }

        /**
         * Converts JSON document to object.
         * @tparam T
         * @param toDocument
         * @param value
         * @return
         */
        template<typename T>
        static void fromJSON(rapidjson::Document& document,
                             std::string& outputLog,
                             T& outValue) noexcept
        {
            // trying to get 'version' member from document
            if(!document.HasMember("version"))
            {
                outputLog = "Error: Broken JSON document: root member 'version' does not exist.\n";
                return;
            }

            // getting 'version' member from document
            const std::string version = document["version"].GetString();

            // trying to get 'typeName' member from document
            if(!document.HasMember("typeName"))
            {
                outputLog = "Error: Broken JSON document: root member 'typeName' does not exist.\n";
                return;
            }

            // getting 'typeName' member from document
            const std::string typeName = document["typeName"].GetString();

            // trying to get 'value' member from document
            if(!document.HasMember("value"))
            {
                outputLog = "Error: Broken JSON document: root member 'value' does not exist.\n";
                return;
            }

            // getting 'value' member from document
            auto& jsonValue = document["value"];

            DeserializableValueView<T, FormatType::JSON> valueView;
            valueView.m_data = &outValue;
            valueView.m_version = version;
            valueView.getValueContainer().m_document = &document;
            valueView.getValueContainer().m_thisValue = &jsonValue;
            valueView.getValueContainer().m_typeName = typeName;
            valueView.getValueContainer().m_outputLog = &outputLog;

            deserializeWithDynamicChecks(valueView);
        }

        /**
         * Serializes value with attempt at dynamic casts to derived types.\n
         * If all attempts of casting value to derived types are failed than serializing only T type members and members of all base types.
         * @tparam T
         * @param valueView
         */
        template<typename T, FormatType TFormatType>
        static void serializeWithDynamicChecks(SerializableValueView<T, TFormatType>& valueView)
        {
            if constexpr(SerdeSpec<T, TFormatType>::is_pointer_type) // serializing value using dynamic checks
            {
                // getting element_type that pointer contains
                using ptr_element_type = SerdeSpec<T, TFormatType>::element_type;

                // creating view that contains element_type object
                SerializableValueView<ptr_element_type, TFormatType> tmpView { };
                tmpView.getValueContainer() = valueView.getValueContainer();
                tmpView.m_version = valueView.m_version;
                tmpView.m_data = SerdeSpec<T, TFormatType>::getObjectRawPointer(valueView);

                // serializing base types
                serializeBaseTypes(tmpView);

                // serializing derived types
                serializeDerivedTypes(tmpView);

                // serializing element_type type members
                SerdeSpec<T, TFormatType>::serialize(valueView);

                return;
            }

            // serialize without dynamic checks (static serialization)

            // serializing base types
            serializeBaseTypes(valueView);

            // serializing only T type members
            SerdeSpec<T, TFormatType>::serialize(valueView);
        }

        /**
         * Deserializes value with attempt at dynamic casts to derived types.\n
         * If all attempts of casting value to derived types are failed than serializing only T type members and members of all base types.\n
         * If one of derived types is matches to typeName than it will be deserialized.
         * @tparam T
         * @param valueView
         */
        template<typename T, FormatType TFormatType>
        static void deserializeWithDynamicChecks(DeserializableValueView<T, TFormatType>& valueView)
        {
            if constexpr(SerdeSpec<T, TFormatType>::is_pointer_type) // deserializing value using dynamic checks
            {
                // getting element_type that pointer contains
                using ptr_element_type = SerdeSpec<T, TFormatType>::element_type;

                // creating view that contains element_type object
                DeserializableValueView<ptr_element_type, TFormatType> tmpView { };
                tmpView.getValueContainer() = valueView.getValueContainer();
                tmpView.m_version = valueView.m_version;

                // trying to deserialize T as one of its derived types
                deserializeAsOneOfDerivedTypes(tmpView);

                // if one of derived types T was deserialized
                if(tmpView.m_data)
                {
                    // setting new raw pointer to value view
                    SerdeSpec<T, TFormatType>::setObjectRawPointer(valueView, tmpView.m_data);
                }
                else
                {
                    // deserializing element_type type members. SerdeSpec deserialize of pointer must allocate pointer
                    SerdeSpec<T, TFormatType>::deserialize(valueView);

                    // assigning allocated pointer
                    tmpView.m_data = SerdeSpec<T, TFormatType>::getObjectRawPointer(valueView);

                    // deserializing base types of T
                    deserializeBaseTypes(tmpView);
                }

                return;
            }

            // deserialize without dynamic checks (static deserialization)

            // deserializing base types
            deserializeBaseTypes(valueView);

            // deserializing only T type members
            SerdeSpec<T, TFormatType>::deserialize(valueView);
        }

        // =====================================================================================
        // =====================================================================================
        // =====================================================================================

        /**
         * Serializes base type (BaseT) of parent type (OriginalT)
         * @tparam OriginalT
         * @tparam BaseT
         * @param valueView
         */
        template<typename OriginalT, typename BaseT, FormatType TFormatType>
        static void serializeBaseType(SerializableValueView<OriginalT, TFormatType>& valueView) noexcept
        {
            // converting OriginalT value view to BaseT value view to pass into SerdeSpec
            SerializableValueView<BaseT, TFormatType> tmpView { };
            tmpView.getValueContainer() = valueView.getValueContainer();
            tmpView.m_version = valueView.m_version;
            tmpView.m_data = &(static_cast<const BaseT&>(*valueView.m_data));

            // serialize BaseT`s base types
            serializeBaseTypes(tmpView);

            // passing tmpView with BaseT to SerdeSpec
            SerdeSpec<BaseT, TFormatType>::serialize(tmpView);
        }

        /**
         * Implementation of serializing base types of T. Uses unpacking.
         * @tparam T
         * @tparam Indices
         * @param valueView
         */
        template<typename T, FormatType TFormatType, std::size_t... Indices>
        static void serializeBaseTypesImpl(SerializableValueView<T, TFormatType>& valueView,
                                           std::index_sequence<Indices...>) noexcept
        {
            // unpacking variadic template
            (serializeBaseType<T,
                    typename SerdeSpec<T, TFormatType>::template get_base_type<Indices>,
                    TFormatType>(valueView), ...);
        }

        /**
         * Serializes all base types of T.
         * @tparam T
         * @param valueView
         */
        template<typename T, FormatType TFormatType>
        static void serializeBaseTypes(SerializableValueView<T, TFormatType>& valueView) noexcept
        {
            // serializing base types only if information of them was provided
            if constexpr(Utils::isBaseTypesProvided<T, TFormatType>())
            {
                serializeBaseTypesImpl<T, TFormatType>
                        (valueView, std::make_index_sequence<SerdeSpec<T, TFormatType>::base_classes_count> {});
            }
        }

        // =====================================================================================
        // =====================================================================================
        // =====================================================================================

        /**
         * Serializes derived type (BaseT) of parent type (OriginalT)
         * @tparam OriginalT
         * @tparam BaseT
         * @param valueView
         */
        template<typename OriginalT, typename DerivedT, FormatType TFormatType>
        static void serializeDerivedType(SerializableValueView<OriginalT, TFormatType>& valueView) noexcept
        {
            const auto* derivedTypeObj = dynamic_cast<const DerivedT*>(valueView.m_data);
            if(derivedTypeObj)
            {
                // converting OriginalT value view to DerivedT value view to pass into SerdeSpec
                SerializableValueView<DerivedT, TFormatType> tmpView { };
                tmpView.getValueContainer() = valueView.getValueContainer();
                tmpView.m_version = valueView.m_version;
                tmpView.m_data = derivedTypeObj;

                // setting new type name
                tmpView.getValueContainer().setTypeName(SerdeSpec<DerivedT, TFormatType>::type_name);

                // trying to serialize as one of DerivedT`s derived types
                serializeDerivedTypes<DerivedT, TFormatType>(tmpView);

                // passing tmpView with DerivedT to SerdeSpec
                SerdeSpec<DerivedT, TFormatType>::serialize(tmpView);
            }
        }

        /**
         * Implementation of serializing derived types of T. Uses unpacking.
         * @tparam T
         * @tparam Indices
         * @param valueView
         */
        template<typename T, FormatType TFormatType, std::size_t... Indices>
        static void serializeDerivedTypesImpl(SerializableValueView<T, TFormatType>& valueView,
                                              std::index_sequence<Indices...>) noexcept
        {
            // unpacking variadic template
            (serializeDerivedType<T,
                    typename SerdeSpec<T, TFormatType>::template get_derived_type<Indices>,
                    TFormatType>(valueView), ...);
        }

        /**
         * Serializes all derived types of T.
         * @tparam T
         * @param valueView
         */
        template<typename T, FormatType TFormatType>
        static void serializeDerivedTypes(SerializableValueView<T, TFormatType>& valueView) noexcept
        {
            // serializing derived types only if information of them was provided
            if constexpr(Utils::isDerivedTypesProvided<T, TFormatType>())
            {
                std::printf("as derived\n");
                serializeDerivedTypesImpl<T, TFormatType>
                        (valueView, std::make_index_sequence<SerdeSpec<T, TFormatType>::derived_classes_count> {});
            }
        }

        // =====================================================================================
        // =====================================================================================
        // =====================================================================================

        /**
         * Serializes base type (BaseT) of parent type (OriginalT)
         * @tparam OriginalT
         * @tparam BaseT
         * @param valueView
         */
        template<typename OriginalT, typename BaseT, FormatType TFormatType>
        static void deserializeBaseType(DeserializableValueView<OriginalT, TFormatType>& valueView) noexcept
        {
            // converting OriginalT value view to BaseT value view to pass into SerdeSpec
            DeserializableValueView<BaseT, TFormatType> tmpView { };
            tmpView.getValueContainer() = valueView.getValueContainer();
            tmpView.m_version = valueView.m_version;
            tmpView.m_data = &(static_cast<BaseT&>(*valueView.m_data));

            // serialize BaseT`s base types
            deserializeBaseTypes(tmpView);

            // passing tmpView with BaseT to SerdeSpec
            SerdeSpec<BaseT, TFormatType>::deserialize(tmpView);
        }

        /**
         * Implementation of serializing base types of T. Uses unpacking.
         * @tparam T
         * @tparam Indices
         * @param valueView
         */
        template<typename T, FormatType TFormatType, std::size_t... Indices>
        static void deserializeBaseTypesImpl(DeserializableValueView<T, TFormatType>& valueView,
                                             std::index_sequence<Indices...>) noexcept
        {
            // unpacking variadic template
            (deserializeBaseType<T,
                    typename SerdeSpec<T, TFormatType>::template get_base_type<Indices>,
                    TFormatType>(valueView), ...);
        }

        /**
         * Deserializes all base types of T.
         * @tparam T
         * @param valueView
         */
        template<typename T, FormatType TFormatType>
        static void deserializeBaseTypes(DeserializableValueView<T, TFormatType>& valueView) noexcept
        {
            // deserializing base types only if information of them was provided
            if constexpr(Utils::isBaseTypesProvided<T, TFormatType>())
            {
                deserializeBaseTypesImpl<T, TFormatType>
                        (valueView, std::make_index_sequence<SerdeSpec<T, TFormatType>::base_classes_count> {});
            }
        }

        // =========================================================================

        /**
         * Tries to deserialize value in OriginalT value view as DerivedT.
         * @tparam OriginalT
         * @tparam DerivedT
         * @tparam TFormatType
         * @param valueView
         */
        template<typename OriginalT, typename DerivedT, FormatType TFormatType>
        static void deserializeAsDerivedType(DeserializableValueView<OriginalT, TFormatType>& valueView) noexcept
        {
            // creating temporary view that contains pointer to DerivedT
            DeserializableValueView<DerivedT, TFormatType> tmpView { };
            tmpView.getValueContainer() = valueView.getValueContainer();
            tmpView.m_version = valueView.m_version;

            // typeNames are equal. DerivedT is suitable
            if(valueView.getValueContainer().m_typeName == SerdeSpec<DerivedT, TFormatType>::type_name)
            {
                // allocating object of DerivedT
                auto* derivedObject = new DerivedT();
                tmpView.m_data = derivedObject;

                // deserializing base types of DerivedT
                deserializeBaseTypes(tmpView);

                // deserializing members of DerivedT
                SerdeSpec<DerivedT, TFormatType>::deserialize(tmpView);

                // assigning allocated pointer to original valueView
                valueView.m_data = derivedObject;

                return;
            }

            // if DerivedT is not suitable than continue to search
            deserializeAsOneOfDerivedTypes(tmpView);

            // assigning allocated pointer to original valueView
            valueView.m_data = tmpView.m_data;
        }

        template<typename T, FormatType TFormatType, std::size_t... Indices>
        static void deserializeAsOneOfDerivedTypesImpl(DeserializableValueView<T, TFormatType>& valueView,
                                                       std::index_sequence<Indices...>) noexcept
        {
            // unpacking variadic template
            (deserializeAsDerivedType<T,
                    typename SerdeSpec<T, TFormatType>::template get_derived_type<Indices>,
                    TFormatType>(valueView), ...);
        }

        /**
         * Tries to deserialize document value in valueView as one of derived types of T.
         * @tparam T
         * @tparam TFormatType
         * @param valueView
         */
        template<typename T, FormatType TFormatType>
        static void deserializeAsOneOfDerivedTypes(DeserializableValueView<T, TFormatType>& valueView) noexcept
        {
            // deserializing base types only if information of them was provided
            if constexpr(Utils::isDerivedTypesProvided<T, TFormatType>())
            {
                deserializeAsOneOfDerivedTypesImpl<T, TFormatType>
                        (valueView, std::make_index_sequence<SerdeSpec<T, TFormatType>::derived_classes_count> {});
            }
        }
    };
}

#endif //SUNGEARENGINE_SERDE_H