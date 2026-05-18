#ifndef TEXT_TRANSFORM_H
#define TEXT_TRANSFORM_H

#include <algorithm>
#include <cctype>
#include <string>

class text_transform {
public:
    virtual ~text_transform() = default;

    [[nodiscard]] std::string name() const { return transform_name; }
    [[nodiscard]] virtual std::string apply(const std::string& text) const = 0;

protected:
    explicit text_transform(std::string name)
        : transform_name(std::move(name))
    {
    }

private:
    std::string transform_name;
};

class uppercase_transform : public text_transform {
public:
    uppercase_transform()
        : text_transform("To Uppercase")
    {
    }

    [[nodiscard]] std::string apply(const std::string& text) const override
    {
        std::string result = text;
        std::transform(result.begin(), result.end(), result.begin(), [](const unsigned char ch) {
            return static_cast<char>(std::toupper(ch));
        });
        return result;
    }
};

class lowercase_transform : public text_transform {
public:
    lowercase_transform()
        : text_transform("To Lowercase")
    {
    }

    [[nodiscard]] std::string apply(const std::string& text) const override
    {
        std::string result = text;
        std::transform(result.begin(), result.end(), result.begin(), [](const unsigned char ch) {
            return static_cast<char>(std::tolower(ch));
        });
        return result;
    }
};

class capitalize_transform : public text_transform {
public:
    capitalize_transform()
        : text_transform("Capitalize Words")
    {
    }

    [[nodiscard]] std::string apply(const std::string& text) const override
    {
        std::string result = text;
        bool capitalize_next = true;
        for (auto& ch : result) {
            if (std::isspace(static_cast<unsigned char>(ch))) {
                capitalize_next = true;
            } else if (capitalize_next) {
                ch = static_cast<char>(std::toupper(static_cast<unsigned char>(ch)));
                capitalize_next = false;
            } else {
                ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
            }
        }
        return result;
    }
};

class sentence_case_transform : public text_transform {
public:
    sentence_case_transform()
        : text_transform("Sentence Case")
    {
    }

    [[nodiscard]] std::string apply(const std::string& text) const override
    {
        std::string result = text;
        bool capitalize_next = true;
        for (auto& ch : result) {
            if (ch == '.') {
                capitalize_next = true;
            } else if (std::isalpha(static_cast<unsigned char>(ch))) {
                if (capitalize_next) {
                    ch = static_cast<char>(std::toupper(static_cast<unsigned char>(ch)));
                    capitalize_next = false;
                } else {
                    ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
                }
            }
        }
        return result;
    }
};

class swap_case_transform : public text_transform {
public:
    swap_case_transform()
        : text_transform("Swap Case")
    {
    }

    [[nodiscard]] std::string apply(const std::string& text) const override
    {
        std::string result = text;
        for (auto& ch : result) {
            if (const auto uch = static_cast<unsigned char>(ch); std::isupper(uch)) {
                ch = static_cast<char>(std::tolower(uch));
            } else if (std::islower(uch)) {
                ch = static_cast<char>(std::toupper(uch));
            }
        }
        return result;
    }
};

#endif // TEXT_TRANSFORM_H
