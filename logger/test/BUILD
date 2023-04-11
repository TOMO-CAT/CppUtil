cc_binary(
    name='file_appender_test',
    srcs=[
        'file_appender_test.cpp',
    ],
    deps=[
        '//util:util',
        '//util/time_util:time_util',
        '//logger:logger',
    ],
    visibility=['PUBLIC'],
)

cc_binary(
    name='logger_test',
    srcs=[
        'logger_test.cpp',
    ],
    deps=[
        '//logger:logger',
    ],
    visibility=['PUBLIC'],
)


cc_library(
    name='test',
    hdrs=[],
    srcs=[],
    deps=[
        ':file_appender_test',
        ':logger_test',
    ],
    visibility=['PUBLIC'],
)
