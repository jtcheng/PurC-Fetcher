/*
 * Copyright (C) 2010-2018 Apple Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

//#include "APIObject.h"
#include "PolicyDecision.h"
#include "FrameLoaderTypes.h"
#include <wtf/CompletionHandler.h>
#include <wtf/Vector.h>

namespace API {
class WebsitePolicies;
}

namespace PurcFetcher {

class SafeBrowsingWarning;

enum class ProcessSwapRequestedByClient : bool { No, Yes };
enum class ShouldExpectSafeBrowsingResult : bool { No, Yes };
enum class ShouldExpectAppBoundDomainResult : bool { No, Yes };

class WebFramePolicyListenerProxy : public API::ObjectImpl<API::Object::Type::FramePolicyListener> {
public:

    using Reply = CompletionHandler<void(PurcFetcher::PolicyAction, API::WebsitePolicies*, ProcessSwapRequestedByClient, RefPtr<SafeBrowsingWarning>&&, Optional<NavigatingToAppBoundDomain>)>;
    static Ref<WebFramePolicyListenerProxy> create(Reply&& reply, ShouldExpectSafeBrowsingResult expectSafeBrowsingResult, ShouldExpectAppBoundDomainResult expectAppBoundDomainResult)
    {
        return adoptRef(*new WebFramePolicyListenerProxy(WTFMove(reply), expectSafeBrowsingResult, expectAppBoundDomainResult));
    }
    ~WebFramePolicyListenerProxy();

    void use(API::WebsitePolicies* = nullptr, ProcessSwapRequestedByClient = ProcessSwapRequestedByClient::No);
    void download();
    void ignore();
    
    void didReceiveSafeBrowsingResults(RefPtr<SafeBrowsingWarning>&&);
    void didReceiveAppBoundDomainResult(Optional<NavigatingToAppBoundDomain>);

private:
    WebFramePolicyListenerProxy(Reply&&, ShouldExpectSafeBrowsingResult, ShouldExpectAppBoundDomainResult);

    Optional<std::pair<RefPtr<API::WebsitePolicies>, ProcessSwapRequestedByClient>> m_policyResult;
    Optional<RefPtr<SafeBrowsingWarning>> m_safeBrowsingWarning;
    Optional<Optional<NavigatingToAppBoundDomain>> m_isNavigatingToAppBoundDomain;
    Reply m_reply;
};

} // namespace PurcFetcher
