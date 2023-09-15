cc_binary(
    name='marshal',
    srcs=['marshal.cc'],
    deps=[
        '//json_helper:json_helper',
    ],
    # defs=['_JSON_HELPER_DEBUG=true'],
)

cc_binary(
    name='unmarshal',
    srcs=['unmarshal.cc'],
    deps=[
        '//json_helper:json_helper',
        '//logger:logger',
    ],
    # defs=['_JSON_HELPER_DEBUG=true'],
    # 调试模式, 支持 assert 宏
)
