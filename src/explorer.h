#ifndef TINY_EXPLORER_H
#define TINY_EXPLORER_H

#include <string>
#include <utility>
#include <filesystem>
#include <vector>

namespace tiny {

    //! The Explorer class navigates, finds and filters files. Under the hood it wraps std::filesystem.
    class Explorer {
    public:
        //! Creates an Explorer instance with the current working directory as base path.
        explicit Explorer() = default;

        /*!
         * \brief Creates an Explorer instance with the base directory set to the given path.
         * \param path Root to search in
         *
         * Creates an Explorer instance with the base directory set to the given path. If the given directory does not
         * exist, a std::runtime_exception will get thrown on creation.
         */
        explicit Explorer(std::string_view path, std::int32_t depth = 1) : path(path), searchDepth(depth)  {};

        /*!
         * \brief Creates an Explorer instance with the base directory set to the given path.
         * \param path Root to search in
         *
         * Creates an Explorer instance with the base directory set to the given path. If the given directory does not
         * exist, a std::runtime_exception will get thrown on creation.
         */
        explicit Explorer(std::filesystem::path path, std::int32_t depth = 1) : path(std::move(path)), searchDepth(depth) {};

        /*!
         * \brief Iterates over the set directory and searches for a matching filename
         * \param term A full filename or an extension wildcard
         * \return A vector with the results of the search. Empty vector if no matches where found.
         *
         * Iterates over the set directory and searches for a matching filename. The term might be a full filename
         * like example.txt or an extension wildcard like *.txt. The search will enter folders recursively for the depth
         * set with the setSearchDepth() function (defaults to 1).
         */
        [[nodiscard]] std::vector<std::filesystem::directory_entry> search(const std::string &term) const;

        /*!
         * \brief Executes a search over multiple terms. See the documentation for the 'search' function.
         * \param terms A full filename or an extension wildcard
         * \param folders A vector of folders allowed to be searched on. If empty, all folders are allowed
         * \return A vector with the results of the search. Empty vector if no matches where found.
         */
        [[nodiscard]] std::vector<std::filesystem::directory_entry>
        search(const std::vector<std::string> &terms, const std::vector<std::string> &folders = {}) const;

        /*!
         * \brief Returns the current search depth.
         * \return The current search depth.
         */
        [[nodiscard]] std::int32_t getSearchDepth() const;

        /*!
         * \brief Sets the recursive search depth. If 0 is set only the root folder will be examined
         * \param depth New search depth
         */
        void setSearchDepth(std::int32_t depth);

    private:
        //! The root directory for the search.
        std::filesystem::path path = std::filesystem::current_path();

        //! The maximum allowed search depth. Defaults to 1.
        std::int32_t searchDepth = 1;
    };
}

#endif //TINY_EXPLORER_H
