#ifndef SPELL_CHECKER_H
#define SPELL_CHECKER_H

#include <algorithm>
#include <fstream>
#include <set>
#include <string>
#include <vector>

class spell_checker {
public:
    explicit spell_checker(const std::string& dictionary_path)
    {
        std::ifstream file(dictionary_path);
        std::string word;
        while (std::getline(file, word)) {
            if (!word.empty()) {
                dictionary.insert(word);
            }
        }
    }

    [[nodiscard]] bool is_correct(const std::string& word) const
    {
        std::string cleaned = to_lower_alpha(word);
        if (cleaned.empty()) {
            return true;
        }
        return dictionary.count(cleaned) > 0;
    }

    [[nodiscard]] std::vector<std::string> suggestions(const std::string& word, int max_count = 5) const
    {
        const std::string target = to_lower_alpha(word);
        if (target.empty()) {
            return {};
        }

        std::vector<std::pair<int, std::string>> candidates;

        for (const auto& dict_word : dictionary) {
            if (std::abs(static_cast<int>(dict_word.size()) - static_cast<int>(target.size())) > 3) {
                continue;
            }
            const int dist = edit_distance(target, dict_word);
            if (dist <= 2) {
                candidates.emplace_back(dist, dict_word);
            }
        }

        std::sort(candidates.begin(), candidates.end(),
            [](const auto& a, const auto& b) { return a.first < b.first; });

        std::vector<std::string> result;
        for (int i = 0; i < std::min(max_count, static_cast<int>(candidates.size())); ++i) {
            result.push_back(candidates[i].second);
        }
        return result;
    }

    [[nodiscard]] bool loaded() const { return !dictionary.empty(); }

private:
    std::set<std::string> dictionary;

    [[nodiscard]] static std::string to_lower_alpha(const std::string& word)
    {
        std::string result;
        result.reserve(word.size());
        for (const unsigned char ch : word) {
            if (std::isalpha(ch)) {
                result += static_cast<char>(std::tolower(ch));
            }
        }
        return result;
    }

    [[nodiscard]] static int edit_distance(const std::string& a, const std::string& b)
    {
        const int m = static_cast<int>(a.size());
        const int n = static_cast<int>(b.size());
        std::vector<std::vector<int>> dp(m + 1, std::vector<int>(n + 1, 0));

        for (int i = 0; i <= m; ++i) {
            dp[i][0] = i;
        }
        for (int j = 0; j <= n; ++j) {
            dp[0][j] = j;
        }

        for (int i = 1; i <= m; ++i) {
            for (int j = 1; j <= n; ++j) {
                if (a[i - 1] == b[j - 1]) {
                    dp[i][j] = dp[i - 1][j - 1];
                } else {
                    dp[i][j] = 1 + std::min({ dp[i - 1][j], dp[i][j - 1], dp[i - 1][j - 1] });
                }
            }
        }
        return dp[m][n];
    }
};

#endif // SPELL_CHECKER_H