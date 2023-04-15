cc_library(
    name='http_client',
    srcs=[
        'http_client.cc',
    ],
    hdrs=[
        'http_client.h',
    ],
    deps=[
        '//logger:logger',
        '#curl',
        # '//thirdparty/curl:curl',
    ],
    visibility=['PUBLIC'],
    # 定义宏
    defs=['_HTTP_CLIENT_DEBUG=true'],
)
