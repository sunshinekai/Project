#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include "../searcher/searcher.h"
#include "../../cpp-httplib/httplib.h"

using std::cout;
using std::endl;
using std::vector;
using std::string;
using std::unordered_map;

int main()
{
    using namespace httplib;
    // 1.创建searcher对象
    searcher::Searcher searcher;
    bool ret = searcher.Init("../data/tmp/raw_input");
    if(!ret)
    {
        std::cout << "Searcher初始化失败" << std::endl;
        return 1;
    }

    // 2.创建server对象
    Server server;
    server.Get("/searcher", [&searcher](const Request& req, Response& resp) {
        if(!req.has_param("query"))
        {
            resp.set_content("请求参数错误", "text/plain; charset = UTF-8");
            return;
        }

        string query = req.get_param_value("query");
        std::cout << "收到查询词：" << query << std::endl;
        string results;
        searcher.Search(query, &results);
        resp.set_content(results, "application/json; charset = UTF-8");
    });

    // 告诉服务器，静态资源存放在wwwroot目录下，(html,css,js,图片...)
    // 服务器启动之后，可以通过http://101.200.56.151:10001/index.html来访问该页面
    server.set_base_dir("./wwwroot");
    // 3.启动服务器
    server.listen("0.0.0.0", 10001);

    return 0;
}