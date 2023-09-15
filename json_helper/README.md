# JsonHelper

## 编译

```bash
# 编译 json helper
blade build json_helper

# 单测
blade test json_helper/unit_test

# example
blade build json_helper/example
./build64_release/json_helper/example/marshal
```

debug 模式下打印了一些调试信息：

```bash
blade build json_helper/... -p debug
```

## 用法

详见 example 和 unit_test 文件夹。

## 后续

* 增加对 `std::shared_ptr` 的支持
