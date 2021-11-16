/*
 * @file fetcher.c
 * @author XueShuming
 * @date 2021/11/16
 * @brief The impl for fetcher api.
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

#include "fetcher.h"

int pcfetcher_init(size_t max_conns, size_t cache_quota)
{
    (void)max_conns;
    (void)cache_quota;
    return 0;
}

int pcfetcher_term(void)
{
    return 0;
}

#if 0
pcfetcher_connid pcfetcher_get_connid(void);
#endif

void pcfetcher_set_cookie(const char* url, const char* cookie, double expires,
        bool secure)
{
    (void)url;
    (void)cookie;
    (void)expires;
    (void)secure;
}

const char* pcfetcher_get_cookie(const char* url)
{
    (void)url;
    return NULL;
}

void pcfetcher_remove_cookie(const char* url)
{
    (void)url;
}

purc_variant_t pcfetcher_request_async(
        const char* url,
        enum pcfetcher_request_method method,
        purc_variant_t params,
        uint32_t timeout,
        response_handler handler,
        void* ctxt)
{
    (void)url;
    (void)method;
    (void)params;
    (void)timeout;
    (void)handler;
    (void)ctxt;
    return NULL;
}

purc_rwstream_t pcfetcher_request_sync(
        const char* url,
        enum pcfetcher_request_method method,
        purc_variant_t params,
        uint32_t timeout,
        struct pcfetcher_resp_header *resp_header)
{
    (void)url;
    (void)method;
    (void)params;
    (void)timeout;
    (void)resp_header;
    return NULL;
}


int pcfetch_check_response(void)
{
    return 0;
}


