/*
 * @file fetcher-session.cpp
 * @author XueShuming
 * @date 2021/11/17
 * @brief The impl for fetcher session.
 *
 * Copyright (C) 2021 FMSoft <https://www.fmsoft.cn>
 *
 * This file is a part of PurC (short for Purring Cat), an HVML interpreter.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "config.h"
#include "fetcher-session.h"
#include "NetworkResourceLoadParameters.h"
#include "NetworkConnectionToWebProcessMessages.h"
#include "WebResourceLoaderMessages.h"
#include "ResourceError.h"

#include <wtf/RunLoop.h>

using namespace PurCFetcher;

PcFetcherSession::PcFetcherSession(uint64_t sessionId,
        IPC::Connection::Identifier identifier)
    : m_sessionId(sessionId)
    , m_connection(IPC::Connection::createClientConnection(identifier, *this))
{
    m_connection->open();
}

PcFetcherSession::~PcFetcherSession()
{
}

void PcFetcherSession::close()
{
    m_connection->invalidate();
}

purc_variant_t PcFetcherSession::requestAsync(
        const char* url,
        enum pcfetcher_request_method method,
        purc_variant_t params,
        uint32_t timeout,
        response_handler handler,
        void* ctxt)
{
    std::unique_ptr<WTF::URL> wurl = makeUnique<URL>(URL(), url);;
    ResourceRequest request;
    request.setURL(*wurl);
    request.setHTTPMethod("GET");
    request.setTimeoutInterval(timeout);

    uint64_t networkId = ProcessIdentifier::generate().toUInt64();
    NetworkResourceLoadParameters loadParameters;
    loadParameters.identifier = networkId;
    loadParameters.request = request;
    loadParameters.webPageProxyID = WebPageProxyIdentifier::generate();
    loadParameters.webPageID = PageIdentifier::generate();
    loadParameters.webFrameID = FrameIdentifier::generate();
    loadParameters.parentPID = getpid();

    m_connection->send(Messages::NetworkConnectionToWebProcess::ScheduleResourceLoad(
                loadParameters), 0);

    fprintf(stderr, "...............................before wait\n");
    wait(30);
    fprintf(stderr, "...............................after wait\n");

    UNUSED_PARAM(url);
    UNUSED_PARAM(method);
    UNUSED_PARAM(params);
    UNUSED_PARAM(timeout);
    UNUSED_PARAM(handler);
    UNUSED_PARAM(ctxt);
    return PURC_VARIANT_INVALID;
}

purc_rwstream_t PcFetcherSession::requestSync(
        const char* url,
        enum pcfetcher_request_method method,
        purc_variant_t params,
        uint32_t timeout,
        struct pcfetcher_resp_header *resp_header)
{
    UNUSED_PARAM(url);
    UNUSED_PARAM(method);
    UNUSED_PARAM(params);
    UNUSED_PARAM(timeout);
    UNUSED_PARAM(resp_header);
    return NULL;
}

void PcFetcherSession::wait(uint32_t timeout)
{
    m_waitForSyncReplySemaphore.waitFor(Seconds(timeout));
}

void PcFetcherSession::wakeUp(void)
{
    m_waitForSyncReplySemaphore.signal();
}

void PcFetcherSession::didClose(IPC::Connection&)
{
}

void PcFetcherSession::didReceiveInvalidMessage(IPC::Connection&,
        IPC::MessageName)
{
}

void PcFetcherSession::didReceiveMessage(IPC::Connection&,
        IPC::Decoder& decoder)
{
    fprintf(stderr, "%s:%d:%s   ############################## %s |destinationID=%ld\n", __FILE__, __LINE__, __func__, description(decoder.messageName()), decoder.destinationID());
    if (decoder.messageName() == Messages::WebResourceLoader::DidReceiveResponse::name()) {
        IPC::handleMessage<Messages::WebResourceLoader::DidReceiveResponse>(decoder, this, &PcFetcherSession::didReceiveResponse);
        return;
    }
    if (decoder.messageName() == Messages::WebResourceLoader::DidReceiveSharedBuffer::name()) {
        IPC::handleMessage<Messages::WebResourceLoader::DidReceiveSharedBuffer>(decoder, this, &PcFetcherSession::didReceiveSharedBuffer);
        return;
    }
    if (decoder.messageName() == Messages::WebResourceLoader::DidFinishResourceLoad::name()) {
        IPC::handleMessage<Messages::WebResourceLoader::DidFinishResourceLoad>(decoder, this, &PcFetcherSession::didFinishResourceLoad);
        return;
    }
    if (decoder.messageName() == Messages::WebResourceLoader::DidFailResourceLoad::name()) {
        IPC::handleMessage<Messages::WebResourceLoader::DidFailResourceLoad>(decoder, this, &PcFetcherSession::didFailResourceLoad);
        return;
    }
}

void PcFetcherSession::didReceiveSyncMessage(IPC::Connection& connection,
        IPC::Decoder& decoder, std::unique_ptr<IPC::Encoder>& replyEncoder)
{
    UNUSED_PARAM(connection);
    UNUSED_PARAM(decoder);
    UNUSED_PARAM(replyEncoder);
}

void PcFetcherSession::didReceiveResponse(const PurCFetcher::ResourceResponse& response, bool needsContinueDidReceiveResponseMessage)
{
    UNUSED_PARAM(response);
    UNUSED_PARAM(needsContinueDidReceiveResponseMessage);
    fprintf(stderr, "%s:%d:%s   url=%s\n", __FILE__, __LINE__, __func__, response.url().string().characters8());
    fprintf(stderr, "%s:%d:%s   code=%d\n", __FILE__, __LINE__, __func__, response.httpStatusCode());
    fprintf(stderr, "%s:%d:%s   mime=%s\n", __FILE__, __LINE__, __func__, response.mimeType().characters8());
    fprintf(stderr, "%s:%d:%s   content=%Ld\n", __FILE__, __LINE__, __func__, response.expectedContentLength());
}

void PcFetcherSession::didReceiveSharedBuffer(IPC::SharedBufferDataReference&& data, int64_t encodedDataLength)
{
    UNUSED_PARAM(data);
    UNUSED_PARAM(encodedDataLength);
    fprintf(stderr, "%s:%d:%s   encodedDataLength=%ld\n", __FILE__, __LINE__, __func__, encodedDataLength);
//    fprintf(stderr, "%s:%d:%s   data.size=%ld\n", __FILE__, __LINE__, __func__, data.size());
//    fprintf(stderr, "%s:%d:%s   data=%s\n", __FILE__, __LINE__, __func__, data.data());
}

void PcFetcherSession::didFinishResourceLoad(const NetworkLoadMetrics& networkLoadMetrics)
{
    UNUSED_PARAM(networkLoadMetrics);
    fprintf(stderr, "%s:%d:%s  complete=%d|thread_id=0x%lX\n", __FILE__, __LINE__, __func__, networkLoadMetrics.isComplete(),pthread_self());
    wakeUp();
}

void PcFetcherSession::didFailResourceLoad(const ResourceError& error)
{
    fprintf(stderr, "%s:%d:%s  error type=%d\n", __FILE__, __LINE__, __func__, (int)error.type());
    wakeUp();
}

