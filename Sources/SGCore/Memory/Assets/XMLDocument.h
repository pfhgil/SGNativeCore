//
// Created by ilya on 23.02.24.
//

#ifndef SUNGEARENGINE_XMLDOCUMENT_H
#define SUNGEARENGINE_XMLDOCUMENT_H

#include "IAsset.h"

namespace SGCore
{
    struct XMLDocument : public IAsset
    {
        sg_implement_asset_type_id(XMLDocument, 10)

        pugi::xml_document m_document;
        
    protected:
        void doLoad(const std::filesystem::path& path) override;
    };
}

#endif //SUNGEARENGINE_XMLDOCUMENT_H
