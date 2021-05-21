#ifndef _UTIL_H
#define _UTIL_H

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <boost/algorithm/string.hpp>

using std::cout;
using std::endl;
using std::string;
using std::vector;

namespace common
{
class Util
{
public:
    // 负责从指定的路径中，读取出文件的整体内容，读到output这个string里
    static bool Read(const string &input_path, string *output)
    {
        std::ifstream file(input_path.c_str());
        
        if(!file.is_open())
            return false;

        // 按行读取，把读到的每行结果追加到output中
        // getline功能就是读取文件中的一行
        // 如果读取成功，就把内容放到了line中，并返回true
        // 如果读取失败(读到文件末尾)，返回false
        string line;
        
        while (std::getline(file, line))
            *output += (line + "\n");
        
        file.close();
        return true;
    }

    // 基于boost中的字符串切分，封装一下
    // delimter表示分割符，按照啥字符来切分
    static void Split(const string& input, const string& delimiter, vector<string>* output)
    {
        boost::split(*output, input, boost::is_any_of(delimiter), boost::token_compress_off);
    }
};

}  // namespace common

#endif /* _UTIL_H */