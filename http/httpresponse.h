#ifndef HTTP_RESPONSE_H
#define HTTP_RESPONSE_H

#include <sys/stat.h>  // stat

#include <unordered_map>

#include "../log/log.h"

class HttpResponse {
   public:
    HttpResponse();
    ~HttpResponse();

    void init(const std::string& srcDir, std::string& path, bool isKeepAlive = false,
              int code = -1);
    void makeResponse(std::string& buffer);
    void unmapFile();
    char* file();
    size_t fileLen() const;
    void errorContent(std::string& buffer, std::string message);
    int code() const { return code_; }

   private:
    void addStateLine_(std::string& buffer);
    void addHeader_(std::string& buffer);
    void addContent_(std::string& buffer);
    void errorHtml_();
    std::string getFileType_();

    int code_;
    bool isKeepAlive_;
    std::string path_;
    std::string srcDir_;

    char* mmFile_;
    struct stat mmFileStat_;

    static const std::unordered_map<std::string, std::string> SUFFIX_TYPE;
    static const std::unordered_map<int, std::string> CODE_STATUS;
    static const std::unordered_map<int, std::string> CODE_PATH;
};

#endif  // HTTP_RESPONSE_H
