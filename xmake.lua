set_config("plat", "linux")
set_project("cpp-util")
set_xmakever("2.7.8")

set_languages("c++17")
set_warnings("all")
add_rules("mode.debug", "mode.release")
set_optimize("faster")   -- faster: -O2  fastest: -O3  none: -O0
--add_cxflags("-Wno-narrowing", "-Wno-sign-compare", "-Wno-strict-aliasing")

add_cxflags("-fPIC")
add_requires("libbacktrace")
add_includedirs(".", "thirdparty/cpptoml/include")
add_installfiles("logger/*.h", {prefixdir = "include/cpputil/logger"})
add_installfiles("(util/**/*.h)", "(util/*.h)", {prefixdir = "include/cpputil"})
add_installfiles("logger/README.md", {prefixdir = "include/cpputil/logger"})

target("cpputil")
    set_kind("static")
    add_files("logger/*.cc|*_test.cc")  -- 添加 logger 下所有的 *.cc 文件, 但不包括 logger 下的 *_test.cc 文件
    add_files("thirdparty/backtrace/lib/libbacktrace.a")
    add_packages("libbacktrace", { public = true })
