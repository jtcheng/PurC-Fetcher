
#include "capi/fetcher.h"

#include <wtf/RunLoop.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

const char* def_url = "https://hybridos.fmsoft.cn";
const char* url = NULL;

void async_response_handler(
        purc_variant_t request_id, void* ctxt,
        const struct pcfetcher_resp_header *resp_header,
        purc_rwstream_t resp)
{
    UNUSED_PARAM(request_id);
    UNUSED_PARAM(ctxt);
    UNUSED_PARAM(resp);
    fprintf(stderr, "....................................\n");
    fprintf(stderr, "%s\n", url);
    fprintf(stderr, ".................head begin\n");
    fprintf(stderr, "ret_code=%d\n", resp_header->ret_code);
    fprintf(stderr, "mime_type=%s\n", resp_header->mime_type);
    fprintf(stderr, "sz_resp=%ld\n", resp_header->sz_resp);
    fprintf(stderr, ".................head end\n");
    fprintf(stderr, ".................body begin\n");
    if (resp) {
        size_t sz_content = 0;
        size_t sz_buffer = 0;
        char* buf = (char*)purc_rwstream_get_mem_buffer_ex(resp, &sz_content,
                &sz_buffer, false);
        fprintf(stderr, "buffer size=%ld\n", sz_buffer);
        fprintf(stderr, "body size=%ld|buflen=%ld\n", sz_content,
                buf ? strlen(buf) : 0);
        fprintf(stderr, "%s\n", buf ? buf : NULL);
    }
    fprintf(stderr, ".................body end\n");
    fprintf(stderr, "....................................\n");
    RunLoop::main().stop();
}


int main(int argc, char** argv)
{
    (void)argc;
    (void)argv;

    purc_instance_extra_info info;
    info.enable_remote_fetcher = true;
    purc_init ("cn.fmsoft.hybridos.sample", "pcfetcher", &info);

    RunLoop::initializeMain();
    AtomString::init();
    WTF::RefCountedBase::enableThreadingChecksGlobally();

    url = argv[1] ? argv[1] : def_url;

    pcfetcher_init(10, 1024);
    pcfetcher_request_async(
                url,
                PCFETCHER_REQUEST_METHOD_GET,
                NULL,
                0,
                async_response_handler,
                NULL);

    RunLoop::run();
    fprintf(stderr, "....................................after runloop\n");

    pcfetcher_term();
    purc_cleanup();

    return 0;
}

