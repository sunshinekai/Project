#include "searcher.h"

namespace searcher
{                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                            
    /////////////////////////////////////////////////////////////////////////////////////
    // 以下代码为Index模块的代码
    /////////////////////////////////////////////////////////////////////////////////////
    const char* const DICT_PATH = "../jieba_dict/jieba.dict.utf8";
    const char* const HMM_PATH = "../jieba_dict/hmm_model.utf8";
    const char* const USER_DICT_PATH = "../jieba_dict/user.dict.utf8";
    const char* const IDF_PATH = "../jieba_dict/idf.utf8";
    const char* const STOP_WORD_PATH = "../jieba_dict/stop_words.utf8";
    Index::Index()
        :jieba(DICT_PATH, HMM_PATH, USER_DICT_PATH, IDF_PATH, STOP_WORD_PATH){}
    
    const DocInfo *Index::GetDocInfo(int64_t doc_id)
    {
        if(doc_id < 0 || doc_id >= forward_index.size())
        {
            return nullptr;
        }
        return &forward_index[doc_id];
    }

    const InvertedList *Index::GetInvertedList(const string &key)
    {
        auto it = inverted_index.find(key);
        if(it == inverted_index.end())
            return nullptr;

        return &it->second;
    }

    bool Index::Build(const string &input_path)
    {
        // 1. 按行读取输入文件内容(raw_input文件)
        // raw_input的结构:是一个行文本文件，每一行对应一个文档
        // 每一行又分成三个部分，使用/3来切分，分别是标题，url，正文
        std::cerr << "开始构建索引" << std::endl;
        std::ifstream file(input_path.c_str());
        if(!file.is_open())
        {
            std::cout << "raw_input 文件打开失败" << std::endl;
            return false;
        }
        
        string line;
        while (std::getline(file, line))
        {
            // 2. 针对当前行，解析成DocInfo对象，并构造为正排索引
            DocInfo *doc_info = BuildForward(line);
            if (doc_info == nullptr)
            {
                // 当前文档构造正排出现问题
                std::cout << "构建正排失败！" << std::endl;
                continue;
            }
            // 3. 根据当前DocInfo对象，进行解析，构造成倒排索引                                                                                                                                                                                                                                                                     
            BuildInverted(*doc_info);
            if(doc_info->doc_id % 100 == 0)
                std::cerr << doc_info->doc_id << std::endl;
        }
        std::cerr << "结束构建索引" << std::endl;
        file.close();
        return true;
    }

    //  核心操作：按照|3对line进行切分，第一个部分是标题, 第二个部分是url，第三个部分是正文
    DocInfo *Index::BuildForward(const string &line)
    {
        // 1. 先把line按照\3切分成3个部分
        vector<string> tokens;
        common::Util::Split(line, "\3", &tokens);
        if(tokens.size() != 3)
            // 如果当前结果不是3份，认为当前这一行是存在问题的，认为文档构造失败
            return nullptr;

        // 2.把切分的结果填充到DocInfo中
        DocInfo doc_info;
        doc_info.doc_id = forward_index.size();
        doc_info.title = tokens[0];
        doc_info.url = tokens[1];
        doc_info.content = tokens[2];
        forward_index.push_back(std::move(doc_info));
        
        // 3. 返回结果
        // return &doc_info 野指针问题， C++经典错误
        return &forward_index.back();
    }
    
    // 每次遍历到一个文档,就要分析这个文档，并且把相关信息，更新到倒排结构中
    void Index::BuildInverted(const DocInfo& doc_info)
    {
        // 创建专门用于统计词频的结构
        struct WordCnt
        {
            int title_cnt;
            int content_cnt;
            WordCnt() : title_cnt(0), content_cnt(0) {}
        };
        unordered_map<string, WordCnt> word_cnt_map;
        
        // 1. 针对标题进行分词
        vector<string> title_token;
        CutWord(doc_info.title, &title_token);
        
        // 2. 遍历分词结果，统计每个词出现的次数
        for(string word : title_token)
        {
            // key不存在就添加，存在就修改
            // 不区分大小写[合理做法]
            // 在统计之前把单词转成小写，使用boost
            boost::to_lower(word);
            ++word_cnt_map[word].title_cnt;
        }

        // 3. 针对正文进行分词
        vector<string> content_token;
        CutWord(doc_info.content, &content_token);
        // 4. 遍历分词结果，统计每个词出现的次数
        for (string word : content_token)
        {
            boost::to_lower(word);
            ++word_cnt_map[word].content_cnt;
        }
        
        // 5. 根据统计结果，整合出weight对象，并把结果更新到倒排索引中即可
        for(const auto& word_pair : word_cnt_map)
        {
            // 构造Weight对象
            Weight weight;
            weight.doc_id = doc_info.doc_id;
            // 权重 = 标题出现次数 * 10 + 正文出现次数
            weight.weight = 10 * word_pair.second.title_cnt + word_pair.second.content_cnt;
            weight.word = word_pair.first;

            // 把weight对象插入到倒排索引中，先找到对应的倒排索引拉链，然后追加到拉链末尾即可
            InvertedList& inverted_list = inverted_index[word_pair.first];
            inverted_list.push_back(std::move(weight));
        }
    }

    void Index::CutWord(const string& input, vector<string> *output)
    {
        jieba.CutForSearch(input, *output);
    }

    /////////////////////////////////////////////////////////////////////////////////////
    // 以下代码为searcher模块的代码
    ////////////////////////////////////////////////////////////////////////////////////
    bool Searcher::Init(const string& input_path)
    {
        return index->Build(input_path);
    }
    
    // 把查询词进行搜索，得到搜索结果
    bool Searcher::Search(const string& query, string* output)
    {
        // 1.【分词】 针对查询词进行分词
        vector<string> tokens;
        index->CutWord(query, &tokens);
        // 2.【触发】 根据分词结果查倒排，把相关的文档全部获取
        vector<Weight> all_token_result;
        for (string word : tokens)
        {
            // 查倒排索引的时候，需要把查询词统一转成小写
            boost::to_lower(word);
            auto* inverted_list = index->GetInvertedList(word);
            if(inverted_list == nullptr)
            {
                // 该词在倒排索引中不存在，在所有文档中都没有出现过，此时得到的倒排拉链就是nullptr
                continue;
            }
            // tokens 包含多个结果，把多个结果合并到一起才能进行统一的排序
            all_token_result.insert(all_token_result.end(), inverted_list->begin(), inverted_list->end());
        }
        // 3.【排序】 把查到的文档的倒排拉链合并到一起并按照权重进行降序排序
        // 升序排序，就写成w1.weight < w2.weight
        // 降序排序，就写成w1.weight > w2.weight
        std::sort(all_token_result.begin(), all_token_result.end(), [](const Weight&w1, const Weight& w2){
            return w1.weight > w2.weight;
        });
        // 4.【包装结束】 把得到的倒排拉链的文档id，然后去查正排，再把doc_info构造成最终预期的格式(JSON)
        Json::Value results; // rerults包含了若干个搜索结果，每个结果就是JSON
        for(const auto& weight : all_token_result)
        {
            // 根据weight中的doc_id查正排
            const DocInfo* doc_info = index->GetDocInfo(weight.doc_id);
            // 把这个doc_info对象再进一步包装成一个JSON对象
            Json::Value result;
            result["title"] = doc_info->title;
            result["url"] = doc_info->url;
            result["desc"] = GenerateDesc(doc_info->content, weight.word);
            results.append(result);
        }
        // 把得到的results这个JSON对象序列化成字符串，写入到output中

        Json::StreamWriterBuilder jsrocd;
        auto write(jsrocd.newStreamWriter());
        std::ostringstream os;
        write->write(results, &os);
        *output = os.str();
        // Json::FastWriter writer;
        // *output = writer.write(results);
    
        return true;
    }

    string Searcher::GenerateDesc(const string& content, const string& word)
    {
        // 根据正文，找到word出现的位置
        // 以该位置为中心，往前找60个字节，作为描述的起始位置。
        // 再从起始位置开始往后找160个字节，作为整个描述
        // 边界条件：
        // a. 前面不够60个字节，从60开始
        // b. 后面内容不够，到末尾结束
        // c. 后面内容显示不下，可以使用...省略号表示

        // 1. 先找到word在正文中出现的位置
        size_t first_pos = content.find(word);
        size_t desc_beg = 0;
        if(first_pos == string::npos)
        {
            // 该词在正文中不存在，只在标题中出现
            // 如果找不到，从头部作为起始位置
            if(content.size() < 160)
                return content;

            string desc = content.substr(0, 160);
            desc[desc.size() - 1] = '.';
            desc[desc.size() - 2] = '.';
            desc[desc.size() - 3] = '.';
            return desc;
        }
        // 2. 找到了first_pos位置，以这个位置为基准，再往前找一些字节
        desc_beg = first_pos < 60 ? 0 : first_pos - 60;

        if(desc_beg + 160 >= content.size())
        {
            // desc_beg后面不够160，直接到末尾结束
            return content.substr(desc_beg);
        }
        else
        {
            string desc = content.substr(desc_beg, 160);
            desc[desc.size() - 1] = '.';
            desc[desc.size() - 2] = '.';
            desc[desc.size() - 3] = '.';
            return desc;
        }
    }
}   // namespace searcher