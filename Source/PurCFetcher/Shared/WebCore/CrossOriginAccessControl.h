/*
 * Copyright (C) 2008-2020 Apple Inc. All Rights Reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 *
 */

#pragma once

#include "HTTPHeaderNames.h"
#include "ReferrerPolicy.h"
#include "StoredCredentialsPolicy.h"
#include <wtf/Expected.h>
#include <wtf/Forward.h>
#include <wtf/OptionSet.h>

namespace WebCore {

class CachedResourceRequest;
class Document;
class HTTPHeaderMap;
class ResourceError;
class ResourceRequest;
class ResourceResponse;
class SecurityOrigin;

struct ResourceLoaderOptions;

WTF_EXPORT_DECLARATION bool isSimpleCrossOriginAccessRequest(const String& method, const HTTPHeaderMap&);
bool isOnAccessControlSimpleRequestMethodAllowlist(const String&);

void updateRequestReferrer(ResourceRequest&, ReferrerPolicy, const String&);
    
WTF_EXPORT_DECLARATION void updateRequestForAccessControl(ResourceRequest&, SecurityOrigin&, StoredCredentialsPolicy);

WTF_EXPORT_DECLARATION ResourceRequest createAccessControlPreflightRequest(const ResourceRequest&, SecurityOrigin&, const String&);
enum class SameOriginFlag { No, Yes };
CachedResourceRequest createPotentialAccessControlRequest(ResourceRequest&&, ResourceLoaderOptions&&, Document&, const String& crossOriginAttribute, SameOriginFlag = SameOriginFlag::No);

enum class HTTPHeadersToKeepFromCleaning : uint8_t {
    ContentType = 1 << 0,
    Referer = 1 << 1,
    Origin = 1 << 2,
    UserAgent = 1 << 3,
    AcceptEncoding = 1 << 4
};

OptionSet<HTTPHeadersToKeepFromCleaning> httpHeadersToKeepFromCleaning(const HTTPHeaderMap&);
WTF_EXPORT_DECLARATION void cleanHTTPRequestHeadersForAccessControl(ResourceRequest&, OptionSet<HTTPHeadersToKeepFromCleaning>);

class WTF_EXPORT_DECLARATION CrossOriginAccessControlCheckDisabler {
public:
    static CrossOriginAccessControlCheckDisabler& singleton();
    virtual ~CrossOriginAccessControlCheckDisabler() = default;
    void setCrossOriginAccessControlCheckEnabled(bool);
    virtual bool crossOriginAccessControlCheckEnabled() const;
private:
    bool m_accessControlCheckEnabled { true };
};

WTF_EXPORT_DECLARATION Expected<void, String> passesAccessControlCheck(const ResourceResponse&, StoredCredentialsPolicy, const SecurityOrigin&, const CrossOriginAccessControlCheckDisabler*);
WTF_EXPORT_DECLARATION Expected<void, String> validatePreflightResponse(const ResourceRequest&, const ResourceResponse&, StoredCredentialsPolicy, const SecurityOrigin&, const CrossOriginAccessControlCheckDisabler*);

WTF_EXPORT_DECLARATION Optional<ResourceError> validateCrossOriginResourcePolicy(const SecurityOrigin&, const URL&, const ResourceResponse&);
Optional<ResourceError> validateRangeRequestedFlag(const ResourceRequest&, const ResourceResponse&);
String validateCrossOriginRedirectionURL(const URL&);

} // namespace WebCore

namespace WTF {

template<> struct EnumTraits<WebCore::HTTPHeadersToKeepFromCleaning> {
    using values = EnumValues<
        WebCore::HTTPHeadersToKeepFromCleaning,
        WebCore::HTTPHeadersToKeepFromCleaning::ContentType,
        WebCore::HTTPHeadersToKeepFromCleaning::Referer,
        WebCore::HTTPHeadersToKeepFromCleaning::Origin,
        WebCore::HTTPHeadersToKeepFromCleaning::UserAgent,
        WebCore::HTTPHeadersToKeepFromCleaning::AcceptEncoding
    >;
};

} // namespace WTF
