/*
*实现预处理模块
*读取并分析boost文档的html内容
*解析出文档的标题，url，正文(去除html标签)
*/

#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include "../common/util.hpp"

using std::string;
using std::vector;

// 定义一些相关的变量和结构体
string g_input_path = "../data/input/";  // 表示从哪个文档读取boost文档的html
string g_output_path = "../data/tmp/raw_input"; // 预处理模块的输出结果

// 表示一个文档
struct DocInfo
{
    string title; // 标题
    string url; // url
    string content; // 正文
};

bool EnumFile(const string &input_path,vector<string> *file_list)
{
    // 枚举目录使用boost
    namespace fs = boost::filesystem; //给filename取别名
    fs::path root_path(input_path);
    if (!fs::exists(root_path))
    {
        std::cout << "当前目录不存在" << std::endl;
        return  false;
    }
    // 递归遍历使用到一个核心的类
    fs::recursive_directory_iterator end_iter; //迭代器使用循环实现时自动完成递归

    // 把迭代器的默认构造函数生成的迭代器作为一个“哨兵”
    for(fs::recursive_directory_iterator iter(root_path); iter != end_iter; ++iter)
    {
        // 当前的路径对应是否为一个普通文件，是目录直接跳过
        if(!fs::is_regular_file(*iter))
            continue;
        
        // 当前路径对应的文件是不是一个html文件，如果是其他文件也跳过
        if(iter->path().extension() != ".html")
            continue;
        // 把得到的路径加入到最终结果的vector中
        file_list->push_back(iter->path().string());
    }
    return true;
}

// 找到html中的title标签
bool ParseTitle(const string &html, string *title)
{
    size_t beg = html.find("<title>");
    if(beg == string::npos)
    {
        std::cout << "标题未找到" << std::endl;
        return false;
    }

    size_t end = html.find("</title>");
    if(end == string::npos)
    {
        std::cout << "标题未找到" << std::endl;
        return false;
    }
    beg += string("<title>").size();
    if (beg >= end)
    {
        std::cout << "标题位置不合法" << std::endl;
        return false;
    }
    *title = html.substr(beg, end - beg);
    return true;
}

// 根据本地路径获取在线文档的路径
// 本地路径形如：
// ../data/input/html/thread.html
// 在线路径形如
// https://www.boost.org/doc/libs/1_53_0/doc/html/thread.html

bool ParseUrl(const string& file_path, string* url)
{
    string url_head = "https://www.boost.org/doc/libs/1_53_0/doc/";
    string url_tail = file_path.substr(g_input_path.size());
    *url = url_head + url_tail;
    return true;
}

// 针对读取出来的html内容进行去标签
bool ParseContent(const string& html, string* content)
{
    bool is_content = true;
    for(auto c : html)
    {
        if(is_content)
        {
            // 当前是正文
            if(c == '<')
            {
                is_content = false;  // 当前遇到了标签
            }

            else
            {
                // 这里需要单独处理\n,预期的结果是一个行文本文件
                // 最终结果raw_input中每一行对应到一个原始的html文档
                // 此时需要把html文档原来的\n都去掉
                if(c == '\n')
                    c = ' ';
                // 当前是普通字符，把结果写入到content中
                content->push_back(c);  // 遇到了普通字符，把结果写入到content中
            }
        }
        else
        {
            // 当前为标签
            if(c == '>')
                is_content = true; // 标签结束
            // 标签里的其他内容都直接忽略
        }
    }
    return true;
}

bool ParseFile(const string &file_path, DocInfo *doc_info)
{
    // 1. 读取文件内容
    // Read比较底层比较底层通用的函数，各个模块都会用到
    string html;
    bool ret = common::Util::Read(file_path, &html);
    if (!ret)
    {
        std::cout << "解析文件失败！读取文件失败！" << file_path << std::endl;
        return false;
    }

    // 2. 根据文件内容解析出标题,(html中有一个title标签)
    ret = ParseTitle(html, &doc_info->title);
    if(!ret)
    {
        std::cout << "解析标题失败！" << std::endl;
        return false;
    }

    // 3. 根据文件的路径，构造出对应的在线文档的url
    ret = ParseUrl(file_path, &doc_info->url);
    if(!ret)
    {
        std::cout << "解析url失败" << std::endl;
        return false;
    }
    // 4. 根据文件内容，进行去标签，作为doc_info中的content字段的内容
    ret = ParseContent(html, &doc_info->content);
    if(!ret)
    {
        std::cout << "解析正文失败" << std::endl;
        return false;
    }

    return true;
}

// ofstream类是没有拷贝构造的（不能拷贝）
// 按照参数传的时候，只能传引用或者指针
// 不能传const引用，否则无法执行里面的写文件操作。
// 每个doc_info就是一行
void WriteOutput(const DocInfo& doc_info, std::ofstream& ofstream)
{
    ofstream << doc_info.title << "\3" << doc_info.url << "\3"
             << doc_info.content << std::endl;
}

// 预处理过程的核心流程
// 1. 把input目录中所有的html路径都枚举出来
// 2. 根据枚举出来的路径依次读取每个文件内容，并进行解析
// 3. 把解析结果写入到最终的输出文件中
int main()
{
    // 1. 进行枚举路径
    vector<string> file_list;
    bool ret = EnumFile(g_input_path, &file_list);
    if(!ret)
    {
        std::cout << "枚举路径失败！" << std::endl;
        return 1;
    }

    // 2.遍历枚举出来的路径，针对每个文件，单独进行处理
    std::ofstream output_file(g_output_path.c_str());
    if(!output_file.is_open())
    {
        std::cout << "打开文件失败" << std::endl;
        return 1;
    }

    for(const auto & file_path : file_list)
    {
        std::cout << file_path << std::endl;
        // 通过一个函数来负责解析
        // 先创建一个DocInfo对象
        DocInfo doc_info;
        // 通过一个函数来负责解析工作
        ret = ParseFile(file_path, &doc_info);
        if(!ret)
        {
            std::cout << "解析该文件失败：" << file_path << std::endl;
            continue;
        }
        // 3. 把解析出来的结果写入到最终的输出文件中
        WriteOutput(doc_info, output_file);
    }
    output_file.close();
    return 0;
}