#ifndef _SEARCHER_H
#define _SEARCHER_H

#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <fstream>
#include <ostream>
#include <stdint.h>
#include <algorithm>
#include <json/json.h>
#include "../common/util.hpp"
#include "../../cppjieba/include/cppjieba/Jieba.hpp"

using std::cout;
using std::endl;
using std::string;
using std::vector;
using std::unordered_map;

namespace searcher
{
////////////////////////////////////////
// 索引模块的内容
///////////////////////////////////////
// 先定义一个基本的索引中需要的结构
// 这个结构是正排索引的基础
// 正排索引是给定doc_id映射到文档内容(DocInfo对象)
struct DocInfo
{
    int64_t doc_id;
    string title;
    string url;
    string content;
};

// 倒排索引是给定词,映射到包含该文档的id列表（不光有文档id，还有权重信息，以及该词的内容）
struct Weight
{
    // 该词在哪个文档中出现
    int64_t doc_id;
    // 对应的权重是多少
    int weight;
    // 词是什么
    string word;
};

// 倒排拉链
typedef vector<Weight> InvertedList;

// Index类用于表示整个索引结构，并且提供一些外部调用的API
class Index
{
private:
    // 索引结构
    // 正排索引,数组下标对应doc_id
    vector<DocInfo> forward_index;
    // 倒排索引，使用一个hash表来表示映射关系
    unordered_map<string, InvertedList> inverted_index;
public:
    Index();
    // 提供一些对外调用的函数
    // 1.查正排, 返回指针可以使用NULL表示无效结果的情况
    const DocInfo *GetDocInfo(int64_t doc_id);
    // 2.查倒排
    const InvertedList *GetInvertedList(const string &key);
    // 3.构建索引
    bool Build(const string &input_path);
    // 4.分词函数
    void CutWord(const string& input, vector<string> *output);
private:
    DocInfo* BuildForward(const string &line);
    void BuildInverted(const DocInfo& doc_info);
    cppjieba::Jieba jieba;
};

////////////////////////////////////////
// 代码搜素模块的内容
///////////////////////////////////////

class Searcher
{
private:
    // 搜索过程中依赖搜索过程，就需要一个Index的指针。
    Index* index;
public:
    Searcher()
        :index(new Index()) {}
    bool Init(const string& input_path);
    bool Search(const string& query, string* output);
private:
    string GenerateDesc(const string& content, const string& word);
};

}   // namespace searcher
#endif /* _SEARCHER_H */