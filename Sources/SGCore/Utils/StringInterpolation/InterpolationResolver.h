//
// Created by stuka on 28.11.2024.
//

#ifndef SUNGEARENGINE_INTERPOLATIONRESOLVER_H
#define SUNGEARENGINE_INTERPOLATIONRESOLVER_H

#include "SGCore/Logger/Logger.h"
#include "SGCore/Utils/Utils.h"
#include "SGCore/Utils/TypeTraits.h"

namespace SGCore
{
    template<typename T>
    struct InterpolationMarkup
    {
        static SG_NOINLINE auto& getGlobalSubstitutedValues() noexcept
        {
            return s_globalMarkup;
        }

    private:
        static inline std::unordered_map<std::string, T> s_globalMarkup;
    };

    template<typename T>
    struct InterpolationResolver
    {
        static_assert(always_false<T>::value, "Implement InterpolationResolver for this type!");

        [[nodiscard]] static T resolve(const T& raw) noexcept;
    };

    template<>
    struct InterpolationResolver<std::filesystem::path>
    {
        [[nodiscard]] static std::filesystem::path resolve(const std::filesystem::path& raw) noexcept
        {
            const auto& globalMarkup = InterpolationMarkup<std::filesystem::path>::getGlobalSubstitutedValues();

            std::filesystem::path output;
            const std::string rawAsString = Utils::toUTF8(raw.u16string());

            std::string currentKey;
            bool enableKeyWrite = false;
            bool dollarFound = false;

            for(const char& c : rawAsString)
            {
                if(c == '$')
                {
                    dollarFound = true;
                    continue;
                }

                if(c == '{' && dollarFound)
                {
                    enableKeyWrite = true;
                    continue;
                }

                if(c == '}' && enableKeyWrite)
                {
                    enableKeyWrite = false;

                    auto foundLocalValue = globalMarkup.find(currentKey);
                    if(foundLocalValue != globalMarkup.end())
                    {
                        output /= foundLocalValue->second;
                    }
                    else
                    {
                        LOG_E(SGCORE_TAG,
                              "Can not find key '{}' to resolve path '{}'! Please, add this key to InterpolationMarkup<std::filesystem::path>!",
                              currentKey, rawAsString);
                    }

                    currentKey = "";
                    continue;
                }

                if(enableKeyWrite)
                {
                    currentKey += c;
                    continue;
                }

                output += c;
            }

            return output;
        }
    };

    template<>
    struct InterpolationResolver<std::string>
    {
        [[nodiscard]] static std::string resolve(const std::string& raw) noexcept
        {
            const auto& globalMarkup = InterpolationMarkup<std::string>::getGlobalSubstitutedValues();

            std::string output;

            std::string currentKey;
            bool enableKeyWrite = false;
            bool dollarFound = false;

            for(const char& c : raw)
            {
                if(c == '$')
                {
                    dollarFound = true;
                    continue;
                }

                if(c == '{' && dollarFound)
                {
                    enableKeyWrite = true;
                    continue;
                }

                if(c == '}' && enableKeyWrite)
                {
                    enableKeyWrite = false;

                    auto foundLocalValue = globalMarkup.find(currentKey);
                    if(foundLocalValue != globalMarkup.end())
                    {
                        output += foundLocalValue->second;
                    }
                    else
                    {
                        LOG_E(SGCORE_TAG,
                              "Can not find key '{}' to resolve string '{}'! Please, add this key to InterpolationMarkup<std::string>!",
                              currentKey, raw);
                    }

                    currentKey = "";
                    continue;
                }

                if(enableKeyWrite)
                {
                    currentKey += c;
                    continue;
                }

                output += c;
            }

            return output;
        }
    };
}

#endif //SUNGEARENGINE_INTERPOLATIONRESOLVER_H
