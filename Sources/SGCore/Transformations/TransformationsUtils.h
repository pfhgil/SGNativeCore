//
// Created by stuka on 02.02.2024.
//

#ifndef SUNGEARENGINE_TRANSFORMATIONSUTILS_H
#define SUNGEARENGINE_TRANSFORMATIONSUTILS_H

#include "Transform.h"

namespace SGCore
{
    struct TransformationsUtils
    {
        /**
         * @param transform - Transform tuo update.
         * @param parent - Parent transform. May be nullptr.
         */
        static void updateTranslationMatrix(Transform& transform, Transform* parent) noexcept;

        /**
         * @param transform - Transform tuo update.
         * @param parent - Parent transform. May be nullptr.
         */
        static void updateRotationMatrix(Transform& transform, Transform* paren) noexcept;

        /**
         * @param transform - Transform tuo update.
         * @param parent - Parent transform. May be nullptr.
         */
        static void updateScaleMatrix(Transform& transform, Transform* paren) noexcept;

        /**
         * @param transform - Transform tuo update.
         * @param parent - Parent transform. May be nullptr.
         */
        static void updateModelMatrix(Transform& transform, Transform* paren) noexcept;

        /**
         * @param transform - Transform tuo update.
         * @param parent - Parent transform. May be nullptr.
         */
        static void updateTransform(Transform& transform, Transform* paren) noexcept;
    };
}

#endif //SUNGEARENGINE_TRANSFORMATIONSUTILS_H
