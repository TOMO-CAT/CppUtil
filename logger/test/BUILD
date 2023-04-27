cc_binary(
    name='file_appender_test',
    srcs=[
        'file_appender_test.cpp',
    ],
    deps=[
        '//util:util',
        '//util/time:time',
        '//logger:logger',
    ],
    visibility=['PUBLIC'],
)

gen_rule(
    name='logger_test_gen',
    srcs=[
        'conf/logger.conf',
    ],
    outs=[
        'conf/logger.conf',
    ],
    # 在 BLADE_ROOT 目录执行
    cmd='cp logger/test/conf/logger.conf $FIRST_OUT',
)

cc_binary(
    name='logger_test',
    srcs=[
        'logger_test.cpp',
    ],
    deps=[
        '//logger:logger',
        ':logger_test_gen',
    ],
    visibility=['PUBLIC'],
    extra_cppflags=[
        '-rdynamic',
    ],
)

cc_binary(
    name='backtrace_test',
    srcs=[
        'backtrace_test.cc',
    ],
    deps=[
        '//thirdparty/backtrace:backtrace'
    ],
    # extra_cppflags=[
    #     '-rdynamic', '-g',
    # ],
)
