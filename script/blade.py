#!/usr/bin/python3
"""
功能: 负责给项目中的 C++ 代码文件夹生成对应的 BLADE 文件
用法: python3 blade.py [-h] [-r] folder_path
参数:
    positional arguments:
        folder_path      需要处理的文件夹路径

    options:
        -h, --help       显示帮助信息
        -r, --recursive  递归给每个子文件夹都生成 BLADE 文件
"""

import argparse
import logging
import colorlog
import glob
import re
import sys
import os

BASE_DIR = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
LOG_FILE_PATH = os.path.join(BASE_DIR, 'logs/gen_blade_build.log')

# 不需要生成 deps 的头文件
IGNORE_HEADER_FILE = [
    "gtest/gtest.h"
]

# 根据特殊规则生成 deps 的头文件
HEADER_FILE_TO_DEP = {
    "zlib.h": "#z",
    "curl/curl.h": "#curl",
    "lz4.h": "#lz4",
    "libavcodec/avcodec.h": "#avcodec",
    "libavutil/frame.h": "#avutil",
    "libavutil/imgutils.h": "#avutil",
    "libavutil/opt.h": "#avutil",
    "libavutil/pixfmt.h": "#avutil",
    "libswscale/swscale.h": "#swscale",
}

logger = logging.getLogger("GenBladeBuild")


def init_logger():
    """
    初始化日志
    """
    logger.setLevel(logging.DEBUG)

    # 日志文件夹不存在时创建
    log_dir = os.path.dirname(LOG_FILE_PATH)
    if not os.path.exists(log_dir):
        print(f"log directory [{log_dir}] don't exist, create it!")
        os.mkdir(log_dir)

    # 控制台带颜色输出
    log_colors = {
        'DEBUG': 'white',
        'INFO': 'green',
        'WARNING': 'yellow',
        'ERROR': 'red',
        'CRITICAL': 'purple'
    }
    console_handler = logging.StreamHandler()
    console_handler.setLevel(logging.WARNING)
    console_handler.setFormatter(
        colorlog.ColoredFormatter('%(log_color)s[%(levelname)s][%(filename)s:%(lineno)d] %(message)s', log_colors=log_colors))
    logger.addHandler(console_handler)

    # 日志落盘
    formatter = logging.Formatter(
        '[%(asctime)s][%(levelname)s][%(filename)s:%(lineno)d] %(message)s')
    file_handler = logging.FileHandler(LOG_FILE_PATH)
    file_handler.setLevel(logging.DEBUG)
    file_handler.setFormatter(formatter)
    logger.addHandler(file_handler)


def gen_blade_build_recursively(folder_path):
    """
    为 folder_path 文件夹及其子文件夹生成 Blade BUILD 文件
    """
    for root, dirs, files in os.walk(folder_path):
        gen_blade_build(root)
    return


def gen_blade_proto_deps(proto_file_list):
    """
    生成 proto 文件列表的 Blade deps 元素
    """
    # import 的 proto 文件的正则表达式
    IMPORT_REGEX_PATTERN = re.compile(r'^import\s+["](.*?)["]')

    folder_path = os.path.dirname(proto_file_list[0])
    logger.debug(f"generate Blade proto deps for folder [{folder_path}]")

    # 获取所有 import 的 proto 文件
    import_proto_files = []
    for file_path in proto_file_list:
        with open(file_path, 'r') as file:
            for line in file:
                match = IMPORT_REGEX_PATTERN.search(line)
                if match:
                    import_proto_files.append(match.group(1))
        logger.debug(f"import_proto_files in file [{file_path}]: {import_proto_files}")

    # 生成 deps
    deps = []
    for proto_file in import_proto_files:
        proto_file_folder = os.path.dirname(proto_file)

        # 1. 如果是本文件夹中的文件则跳过
        if proto_file_folder == folder_path:
            continue

        # 2. 不存在时报错
        if not os.path.exists(proto_file):
            logger.error(f"proto file [{proto_file}] don't exist!")
            sys.exit(1)

        # 3. 普通 proto 文件
        deps.append("//" + proto_file_folder + ":" + os.path.basename(proto_file_folder) + "_proto")
    return deps


def gen_blade_deps(file_list, binary=False, is_main_binary=False):
    """
    生成文件列表 file_list 的 Blade deps 元素
    @param binary: 表示是否需要生成 binary, 此时需要依赖当前文件夹作为依赖
    @param is_main_binary: 依赖的当前文件是否是 main 函数文件夹
    """
    # 头文件的正则表达式, 这里不考虑尖括号的系统头文件, 只考虑双引号
    # 同时为了剔除注释文件, 我们只考虑以 `#include` 开头的文件
    INCLUDE_REGEX_PATTERN = re.compile(r'^#include\s+["](.*?)["]')
    SYSTEM_INCLUDE_REGEX_PATTERN = re.compile(r'^#include\s+[<](.*?)[>]')

    if len(file_list) <= 0:
        return []

    folder_path = os.path.dirname(file_list[0])
    logger.debug(f"generate Blade deps for folder [{folder_path}]")

    # 获取所有包含的非系统头文件
    header_files = []
    system_header_files = []
    for file_path in file_list:
        with open(file_path, 'r') as file:
            for line in file:
                match = INCLUDE_REGEX_PATTERN.search(line)
                if match:
                    header_files.append(match.group(1))
                match = SYSTEM_INCLUDE_REGEX_PATTERN.search(line)
                if match:
                    system_header_files.append(match.group(1))
        logger.debug(f"header_files in file [{file_path}]: {header_files}")
        logger.debug(f"system header files in file [{file_path}]: {system_header_files}")

    # 根据头文件列表生成 Blade deps
    deps = []
    for header_file in header_files:
        header_folder = os.path.dirname(header_file)

        # 1. 如果是本文件夹中的头文件, 非 binary 直接跳过
        if header_folder == folder_path:
            if binary:
                if is_main_binary:
                    deps.append(":" + os.path.basename(header_folder) + "_lib")
                else:
                    deps.append(":" + os.path.basename(header_folder))
            continue

        # 2. 对于一些特殊头文件直接忽略或直接塞入特殊的 dep
        if header_file in IGNORE_HEADER_FILE:
            continue
        if header_file in HEADER_FILE_TO_DEP:
            deps.append(HEADER_FILE_TO_DEP[header_file])
            continue
        if header_file == "opencv2/opencv.hpp":
            deps += ["#opencv_core", "#opencv_highgui"]
            continue
        if header_file == "opencv2/imgcodecs.hpp":
            deps += ["#opencv_imgcodecs", "#opencv_imgproc"]
            continue
        if header_folder == "opencv2":
            deps += ["#opencv_core"]
            continue

        # 3. 头文件不包含在当前工作目录中, 可能是 thirdparty 或者系统文件夹
        # 注意这里 proto 或者 gen_rule 生成的文件可能确实不存在, 所以判断的是 header_folder 而非 header_file
        if not os.path.exists(header_folder):
            # 3.1 thirdparty 头文件
            candidate_header_file_list = glob.glob(os.path.join(
                "thirdparty/**", header_file), recursive=True)
            candidate_header_file_list = [
                item for item in candidate_header_file_list if not item.startswith("thirdparty/blade-build")]
            if len(candidate_header_file_list) > 1:
                logger.error(
                    f"ambiguous thirdparty header file [{header_file}], candidate file list: [{candidate_header_file_list}]")
                sys.exit(1)
            elif len(candidate_header_file_list) == 1:
                thirdparty_header_file = candidate_header_file_list[0]
                thirdparty_folder = thirdparty_header_file.split('/')[1]
                if thirdparty_folder == "grpc-v1.48.1":
                    deps.append("//thirdparty/" + thirdparty_folder + ":grpc")
                else:
                    deps.append("//thirdparty/" + thirdparty_folder + ":" + thirdparty_folder)
            else:
                # 3.2 其他头文件
                logger.warning(
                    f"header folder [{header_folder}] don't exist, header file [{header_file}]")
            continue

        # 4. 对于 pb 头文件而言, 需要加上特殊 `_proto` 后缀
        if header_file.endswith(".pb.h"):
            deps.append("//" + header_folder + ":" + os.path.basename(header_folder) + "_proto")
            continue

        # 5. folder_path 子文件夹中的头文件, 使用相对路径
        if header_folder.startswith(folder_path):
            deps.append(os.path.relpath(header_folder, folder_path) + ":" + os.path.basename(header_folder))
            continue

        # 6. 对于一些特殊头文件需要建立一个内存表进行映射
        # 7. 对于 thirdparty 中的头文件应该不用特殊处理?

        # 8. 普通头文件用全目录
        deps.append("//" + header_folder + ":" + os.path.basename(header_folder))

    for header_file in system_header_files:
        if header_file in HEADER_FILE_TO_DEP:
            deps.append(HEADER_FILE_TO_DEP[header_file])
            continue

    # 对 deps 列表进行去重排序
    deps = sorted(set(deps))
    logger.debug(f"deps: {deps}")
    return deps


def gen_blade_list_str(elem_list):
    """
    生成 blade 列表字符串
    例如输入是 ['a.h', 'b.h', 'x.cc', 'y.cc'] 列表
    输出应该是:
        'a.h',
        'b.h',
        'x.cc',
        'y.cc',
    """
    elem_list = sorted(set(elem_list))
    new_elem_list = []
    for elem in elem_list:
        new_elem_list.append("'" + elem + "',")
    return "\n        ".join(new_elem_list)


def contains_main_function(file_path):
    """
    判断 cc 文件中是否包含 main 函数开头的行
    """
    with open(file_path, 'r') as file:
        for line in file:
            line = line.strip()
            # 检查行是否以 `int main(` 开头
            if line.startswith("int main("):
                return True
    return False


def gen_blade_build(folder_path):
    """
    为 folder_path 文件夹生成 Blade BUILD 文件
    """
    print("-------------------------------------------------------------")
    print(f"generate Blade BUILD file for folder [{folder_path}]")

    # 检查是否是文件夹, 不是则跳过
    if not os.path.isdir(folder_path):
        return

    # 暂时不支持为 thirdparty 文件夹下的文件生成 BUILD 文件
    if folder_path.startswith("thirdparty"):
        logger.error(f"cannot generate Blade BUILD for folder [{folder_path}] in thirdparty")
        sys.exit(1)

    # 去除后面多余的斜杠, 否则 basename 会取到空的文件夹名
    folder_path = folder_path.rstrip('/')

    # 获取文件夹名称
    name = os.path.basename(folder_path)
    logger.debug(f"generate blade build file for [{folder_path}]")
    logger.debug(f"name: [{name}]")

    # 查找文件夹内的所有头文件 (*.h 和 *.hpp)
    hdrs = glob.glob(os.path.join(folder_path, "*.h")) + \
        glob.glob(os.path.join(folder_path, "*.hpp"))

    # 查找文件夹中的所有 .py 文件
    py_files = glob.glob(os.path.join(folder_path, "*.py"))

    # 查找文件夹内的所有 *.cc 文件
    # 分离出单测 *test.cc、包含 main 函数的 cc 文件和普通 cc 文件
    all_srcs = glob.glob(os.path.join(folder_path, "*.cc"))
    all_srcs = sorted(set(all_srcs))
    test_srcs = []
    main_srcs = []
    normal_srcs = []
    for cc_file_path in all_srcs:
        if contains_main_function(cc_file_path):
            main_srcs.append(cc_file_path)
        elif cc_file_path.endswith('_test.cc'):
            test_srcs.append(cc_file_path)
        else:
            normal_srcs.append(cc_file_path)

    # 查找文件夹中所有 *.proto 文件
    proto_files = glob.glob(os.path.join(folder_path, "*.proto"))

    logger.debug(f"hdrs = {hdrs}")
    logger.debug(f"test_srcs = {test_srcs}")
    logger.debug(f"main_srcs = {main_srcs}")
    logger.debug(f"normal_srcs = {normal_srcs}")
    logger.debug(f"proto_files = {proto_files}")

    # 如果文件都是空的则跳过
    if len(hdrs) == 0 and len(test_srcs) == 0 and len(main_srcs) == 0 and len(normal_srcs) == 0 and len(proto_files) == 0:
        logger.warning(f"[{name}] don't have any files to generate Blade BUILD rules")
        return

    # 跳过特殊文件夹
    if len(proto_files) > 0 and (len(all_srcs) + len(hdrs)) > 0:
        logger.warning(f"skip specific folder [{name}]")
        return
    if len(py_files) > 0:
        logger.warning(f"skip folder [{name}] that contains python files")
        return

    # 生成 Blade BUILD 内容
    # 如果是包含非 _test.cc 的 binary 文件, 那么需要修改 name
    is_main_binary = False
    for file in main_srcs:
        if not file.endswith('_test.cc'):
            is_main_binary = True
            name = name + "_lib"

    BLADE_BUILD_FILE_CONTENT_LIST = []
    if len(proto_files) > 0:
        BLADE_BUILD_FILE_CONTENT_LIST.append(gen_blade_proto_library(proto_files))
    else:
        if len(hdrs) + len(normal_srcs) > 0:
            BLADE_BUILD_FILE_CONTENT_LIST.append(gen_blade_cc_library(name, hdrs, normal_srcs))
        for test_file in test_srcs:
            BLADE_BUILD_FILE_CONTENT_LIST.append(gen_blade_cc_test(test_file, is_main_binary))
        for main_file in main_srcs:
            BLADE_BUILD_FILE_CONTENT_LIST.append(gen_blade_cc_binary(main_file, is_main_binary))

    blade_build_file_content = "\n".join(BLADE_BUILD_FILE_CONTENT_LIST)
    # 写入 BLADE 文件
    blade_build_file_path = os.path.join(folder_path, "BUILD")
    with open(blade_build_file_path, 'w') as f:
        f.write(blade_build_file_content)

    print("-------------------------------------------------------------")


def gen_blade_proto_library(proto_files):
    """
    根据给定的 proto 文件生成 proto 规则
    """
    PROTO_LIBRARY_TEMPLATE = r"""
proto_library(
    name='${PROTO_LIBRARY_NAME}',
    srcs=[
        ${PROTO_FILES}
    ],
    deps=[
        ${DEPS}
    ],
    visibility=['PUBLIC'],
)

cc_library(
    name='${NAME}',
    hdrs=[],
    deps=[
        ':${PROTO_LIBRARY_NAME}',
    ],
    visibility=['PUBLIC'],
)
"""
    proto_folder_name = os.path.basename(os.path.dirname(proto_files[0]))
    deps = gen_blade_proto_deps(proto_files)
    proto_files = [os.path.basename(file_path) for file_path in proto_files]
    res = PROTO_LIBRARY_TEMPLATE.replace("${PROTO_FILES}", gen_blade_list_str(proto_files))
    res = res.replace("${DEPS}", gen_blade_list_str(deps))
    res = res.replace("${NAME}", proto_folder_name)
    res = res.replace("${PROTO_LIBRARY_NAME}", proto_folder_name + "_proto")

    res = delete_line_with_only_space(res)

    return res


def gen_blade_cc_library(name, hdrs, srcs):
    if len(hdrs) == 0:
        return ""

    CC_LIBRARY_TEMPLATE = r"""
cc_library(
    name='${NAME}',
    hdrs=[
        ${HDRS}
    ],
    srcs=[
        ${SRCS}
    ],
    deps=[
        ${DEPS}
    ],
    visibility=['PUBLIC'],
)
"""
    deps = gen_blade_deps(hdrs + srcs)
    hdrs = [os.path.basename(file_path) for file_path in hdrs]
    srcs = [os.path.basename(file_path) for file_path in srcs]
    res = CC_LIBRARY_TEMPLATE.replace("${HDRS}", gen_blade_list_str(hdrs))
    res = res.replace("${SRCS}", gen_blade_list_str(srcs))
    res = res.replace("${DEPS}", gen_blade_list_str(deps))
    res = res.replace("${NAME}", name)

    res = delete_line_with_only_space(res)

    logger.debug(f"cc_library=\n{res}")
    return res


def delete_line_with_only_space(str):
    """
    删除字符串中只包含空格的行
    """
    # 剔除最前面的换行符
    str = str.lstrip('\n')
    # 剔除非换行符的空行
    lines = [line for line in str.split('\n') if len(line) == 0 or line.strip()]
    str = '\n'.join(lines)
    return str


def gen_blade_cc_test(test_srcs, is_main_binary=False):
    """
    根据给定的单个 test.cc 文件生成 cc_test 规则
    """
    CC_TEST_TEMPLATE = r"""
cc_test(
    name='${TEST_NAME}',
    srcs=[
        ${SRCS}
    ],
    deps=[
        ${DEPS}
    ],
    exclusive=True,
)
"""
    deps = gen_blade_deps([test_srcs], True, is_main_binary)
    test_srcs = os.path.basename(test_srcs)
    res = CC_TEST_TEMPLATE.replace("${SRCS}", gen_blade_list_str([test_srcs]))
    res = res.replace("${DEPS}", gen_blade_list_str(deps))
    res = res.replace("${TEST_NAME}", os.path.basename(test_srcs).rstrip(".cc"))

    # 删除包含空格的行
    res = delete_line_with_only_space(res)

    logger.debug(f"cc_test=\n[{res}]")
    return res


def gen_blade_cc_binary(binary_srcs, is_main_file=False):
    """
    根据给定的包含 main 文件的 cc 文件生成 cc_binary 规则
    """
    CC_BINARY_TEMPLATE = r"""
cc_binary(
    name="${NAME}",
    srcs=[
        ${SRCS}
    ],
    deps=[
        ${DEPS}
    ],
    ${DYNAMIC_LINK}
)
"""
    deps = gen_blade_deps([binary_srcs], True, is_main_file)
    folder_name = os.path.basename(os.path.dirname(binary_srcs))
    binary_srcs = os.path.basename(binary_srcs)
    res = CC_BINARY_TEMPLATE.replace("${SRCS}", gen_blade_list_str([binary_srcs]))
    res = res.replace("${DEPS}", gen_blade_list_str(deps))
    if is_main_file:
        res = res.replace("${NAME}", folder_name)
        res = res.replace("${DYNAMIC_LINK}", "dynamic_link=True,")
    else:
        res = res.replace("${NAME}", os.path.basename(binary_srcs).rstrip(".cc"))
        res = res.replace("${DYNAMIC_LINK}", "    ")
    res = delete_line_with_only_space(res)
    return res


def main():
    # 初始化日志
    init_logger()
    logger.info(f"log will be printed to [{LOG_FILE_PATH}]")

    # 解析参数
    parser = argparse.ArgumentParser(
        description="Generate blade BUILD file for the specified folder")
    parser.add_argument("folder_path", type=str,
                        help="Path to the folder to be processed")
    parser.add_argument("-r", "--recursive", action="store_true",
                        help="Process subfolders recursively")
    args = parser.parse_args()
    folder_path = args.folder_path
    recursive = args.recursive
    logger.info(f"folder_path: [{folder_path}] is_recursive: [{recursive}]")

    # 为这个文件夹生成 Blade BUILD 文件
    if not recursive:
        gen_blade_build(folder_path)
    else:
        print(f"gen Blade BUILD recursively for folder [{folder_path}]")
        gen_blade_build_recursively(folder_path)

    logger.info(f"generate blade build files for [{folder_path}] done!")


if __name__ == '__main__':
    main()
