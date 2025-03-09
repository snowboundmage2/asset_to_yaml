#ifndef GLOBALINFO_H
#define GLOBALINFO_H

#include <string>

class GlobalInfo {
public:
    // Constructor
    GlobalInfo();

    // Destructor
    ~GlobalInfo();

    // Getters and Setters
    std::string getInfo() const;
    void setInfo(const std::string &info);

private:
    std::string info;
};

#endif // GLOBALINFO_H