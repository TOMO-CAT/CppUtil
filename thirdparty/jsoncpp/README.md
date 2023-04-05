# JsonCpp

## 安装

```bash
cd /tmp
wget https://github.com/open-source-parsers/jsoncpp/archive/refs/tags/1.6.0.zip
unzip 1.6.0.zip
cd jsoncpp-1.6.0/
mkdir build && cd build
cmake .. -DCMAKE_INSTALL_PREFIX=.
make
make install

cp -r ./include ~/project/CppUtil/thirdparty/jsoncpp
cp -r ./lib ~/project/CppUtil/thirdparty/jsoncpp
```
