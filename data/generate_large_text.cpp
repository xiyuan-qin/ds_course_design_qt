#include <iostream>
#include <fstream>
#include <vector>
#include <random>
#include <string>
using namespace std;

// 生成随机单词
string generateRandomWord(int len, mt19937& gen) {
    string word;
    static const char charset[] = "abcdefghijklmnopqrstuvwxyz";
    uniform_int_distribution<> dis(0, 25);
    
    for (int i = 0; i < len; ++i) {
        word += charset[dis(gen)];
    }
    return word;
}

int main() {
    // 设置输出文件
    ofstream outFile("test.txt");
    if (!outFile) {
        cout << "无法创建文件!" << endl;
        return 1;
    }

    // 初始化随机数生成器
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> word_len(4, 12);
    uniform_int_distribution<> pattern_dis(0, 100);

    // 预定义的高频词
    vector<string> highFreqWords = {
        "computer", "data", "algorithm", "system", "network",
        "program", "software", "database", "memory", "processor"
    };

    // 预定义的中频词
    vector<string> mediumFreqWords;
    for (int i = 0; i < 100; ++i) {
        mediumFreqWords.push_back(generateRandomWord(word_len(gen), gen));
    }

    // 生成约50万个单词
    const int TARGET_WORDS = 500000;
    
    for (int i = 0; i < TARGET_WORDS; ++i) {
        int pattern = pattern_dis(gen);
        
        if (pattern < 30) { // 30% 概率选择高频词
            outFile << highFreqWords[gen() % highFreqWords.size()] << " ";
        }
        else if (pattern < 60) { // 30% 概率选择中频词
            outFile << mediumFreqWords[gen() % mediumFreqWords.size()] << " ";
        }
        else { // 40% 概率生成随机词
            outFile << generateRandomWord(word_len(gen), gen) << " ";
        }

        // 每100个单词换行
        if (i % 100 == 99) outFile << "\n";
    }

    outFile.close();
    cout << "已生成大型测试文件 test.txt" << endl;
    return 0;
}
