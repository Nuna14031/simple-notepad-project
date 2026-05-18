#ifndef SPELL_CHECKER_H
#define SPELL_CHECKER_H

#include <fstream>
#include <set>
#include <string>
#include <vector>

class spell_checker {
public:
    explicit spell_checker(const std::string& path)
    {
        std::ifstream file(path);
        std::string word;
        while (std::getline(file, word)) {
            if (!word.empty()) {
                dictionary.insert(word);
            }
        }
    }

    bool is_correct(const std::string& word) const
    {
        std::string cleaned;
        for (char ch : word) {
            if (std::isalpha(static_cast<unsigned char>(ch))) {
                cleaned += static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
            }
        }
        if (cleaned.empty()) {
            return true;
        }
        return dictionary.count(cleaned) > 0;
    }

    std::vector<std::string> suggestions(const std::string& word) const
    {
        std::string target;
        for (char ch : word) {
            if (std::isalpha(static_cast<unsigned char>(ch))) {
                target += static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
            }
        }

        std::vector<std::string> result;
        for (const std::string& dict_word : dictionary) {
            if (result.size() >= 5) {
                break;
            }
            if (dict_word.empty() || target.empty()) {
                continue;
            }
            if (dict_word[0] == target[0]
                && std::abs(static_cast<int>(dict_word.size()) - static_cast<int>(target.size())) <= 2) {
                result.push_back(dict_word);
                }
        }
        return result;
    }

    bool loaded() const
    {
        return !dictionary.empty();
    }

private:
    std::set<std::string> dictionary;
};

#endif // SPELL_CHECKER_H