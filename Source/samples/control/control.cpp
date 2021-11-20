
#include "capi/fetcher.h"

#include <wtf/RunLoop.h>

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv)
{
    (void)argc;
    (void)argv;

    purc_instance_extra_info info = {0, 0};
    purc_init ("cn.fmsoft.hybridos.sample", "pcfetcher", &info);

    RunLoop::initializeMain();
    AtomString::init();
    WTF::RefCountedBase::enableThreadingChecksGlobally();

    pcfetcher_init(10, 1024);
    struct pcfetcher_resp_header resp_header;
    purc_rwstream_t resp = pcfetcher_request_sync(
        "https://www.baidu.com",
        PCFETCHER_REQUEST_METHOD_GET,
        NULL,
        10,
        &resp_header);

    fprintf(stderr, "....................................\n");
    fprintf(stderr, ".................head begin\n");
    fprintf(stderr, "ret_code=%d\n", resp_header.ret_code);
    fprintf(stderr, "mime_type=%s\n", resp_header.mime_type);
    fprintf(stderr, "sz_resp=%ld\n", resp_header.sz_resp);
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

    RunLoop::run();

    pcfetcher_term();
    purc_cleanup();

    return 0;
}
