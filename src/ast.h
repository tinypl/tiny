#ifndef TINY_AST_H
#define TINY_AST_H

#include <cstdint>
#include <utility>
#include <variant>
#include <optional>
#include <filesystem>
#include <deque>

#include "unicode.h"

#include "nlohmann/json.hpp"
#include "metadata.h"
#include "file.h"

namespace tiny {
    // Forward declaration
    struct ASTNode;

    //! Alias for a vector of nodes
    using StatementList = std::vector<tiny::ASTNode>;

    //! Value is a variant that can hold any of a unicode string, an uint64, an int64, a long double or a boolean
    using Value = std::variant<
            tiny::String, // ID, string or char
            std::int64_t,            // Integer
            std::uint64_t,           // Unsigned integer
            long double,             // Decimal
            bool>;                   // Boolean

    /*!
     * \brief Transforms a Value into a std::string
     * \param val Value to transform
     * \return The underlying value of Value as a string.
     *
     * Transforms a Value into a std::string. If the value holds an UnicodeCodepoints the returned value is an UTF-8
     * encoded representation. If the held value is numeric the string transformation of the number is returned
     * (using std::to_string). Boolean values will return either "True" of "False" (first letter capitalized)
     * given the case
     */
    [[nodiscard]] std::string toString(tiny::Value val);

    //! Role of the Parameter
    enum class ParameterType {
        //! Default type
        None,
        //! Type of the node's value
        Type,
        //! Const modifier over the node's value
        Const,
        //! Pointer modifier over the node's value
        Pointer,
        //! Dereference modifier over the node's value
        Dereference,
        //! Value-at modifier over the node's value
        ValueAt,
        //! Used inside ranges to specify the identifier used by the range
        RangeIdentifier,
        //! Set when an error handler uses a callback function
        ErrorCallback,
        //! Used when an error handler is inlined. The name of the value of the error
        ErrorVarName,
        //! Used inside methods, functions and similar to indicate the name of the name of the procedure
        Name,
        //! Indicates that a collection access operator is used
        ComputedAccess,
    };

    //! A Parameter holds the complementary information of an ASTNode
    struct Parameter {
        Parameter() = default;

        /*!
         * \brief Create a Parameter of type t which holds value v
         * \param t Type of the parameter
         * \param v The value of the parameter
         */
        explicit Parameter(tiny::ParameterType t, tiny::Value v) : type(t), val(std::move(v)) {};

        /*!
         * \brief Create a Parameter of type t and no value
         * \param t Type of the parameter
         */
        explicit Parameter(tiny::ParameterType t) : type(t) {};

        //! Type of the Parameter. Indicates the role of the Parameter inside the ASTNode. Defaults to None
        tiny::ParameterType type = tiny::ParameterType::None;

        //! Optional value held by the parameter
        tiny::Value val;

        /*!
         * \brief Serializes the Parameter into a std::string encoded in UTF-8
         * \return A UTF-8 encoded std::string with a short description of the Parameter
         */
        [[nodiscard]] std::string toString() const;

        /*!
         * \brief Serializes the Parameter into a JSON
         * \return A nlohmann::json with the data of the Parameter
         */
        [[nodiscard]] nlohmann::json toJson() const;

        /*!
         * \brief Gets the value as a tiny::String. Throws NoSuchValue if val doesn't contain a string
         * \param meta The metadata of the base node searching the parameter. Required for error reporting
         * \return A tiny::String
         */
        [[nodiscard]] tiny::String getStringVal(const tiny::Metadata& meta) const;
    };

    //! The type of a given ASTNode
    enum class ASTNodeType {
        //! Default type
        None,

        //! Used inside a comma-separated expression list used in function calls
        ExpressionList,
        //! Basic expression
        ExpressionStatement,
        //! Indicates statements held inside a block
        BlockStatement,

        //! Literal integer value
        LiteralInt,
        //! Literal decimal value
        LiteralDecimal,
        //! Literal boolean value
        LiteralBool,
        //! Literal none value
        LiteralNone,
        //! Literal character
        LiteralChar,
        //! Literal string
        LiteralString,

        //! Additive operation node
        OpAddition,
        //! Subtraction operation node
        OpSubtraction,
        //! Multiplicative operation node
        OpMultiplication,
        //! Divisive operation node
        OpDivision,
        //! Exponentiation operation node
        OpExponentiate,

        //! Identifier such as function, variable and custom types
        Identifier,
        //! Immediate assignment of a value to a new variable
        Initialization,
        //! Assignment of a value to an existing variable
        Assignment,
        //! Assignment of the result of a variable plus another value to the variable
        AssignmentSum,
        //! Assignment of the result of a variable minus another value to the variable
        AssignmentSub,
        //! Assignment of the result of a variable times another value to the variable
        AssignmentMulti,
        //! Assignment of the result of a variable divided by another value to the variable
        AssignmentDiv,
        //! Declaration of a value without a value assignment
        VarDeclaration,

        //! For loop
        ForStatement,
        //! Iterator over a sequence of numbers
        RangeExpression,
        //! Initial value of the iteration
        RangeFromExpression,
        //! Optional upper bound of the iteration
        RangeToExpression,
        //! Optional step of the iteration
        RangeStepExpression,
        //! Iterator over a collection
        ForEachExpression,

        //! If branch
        IfStatement,
        //! Condition to branch
        BranchCondition,
        //! Result of the condition being truthful
        BranchConsequent,
        //! Result of the condition being false
        BranchAlternative,

        //! Equality comparator node
        CompareEq,
        //! Difference comparator node
        CompareNeq,
        //! Greater-than comparator node
        CompareGt,
        //! Greater-than or equals comparator node
        CompareGteq,
        //! Less-than comparator node
        CompareLt,
        //! Less-than or equals comparator node
        CompareLteq,

        //! And node
        LogicalAnd,
        //! Or node
        LogicalOr,

        //! Negation node
        UnaryNot,
        //! Negative-value node
        UnaryNegative,

        //! Error handler operator
        ErrorHandle,

        //! Base node of a function declaration
        FunctionDeclaration,
        //! Contains the arguments of a function declaration
        FunctionArgumentDeclList,
        //! Individual argument of a function declaration
        FunctionArgumentDecl,
        //! Contains the return values of a function declaration
        FunctionReturnDeclList,
        //! Individual return value of a function declaration
        FunctionReturnDecl,
        //! A function body
        FunctionBody,
        //! Return statement
        FunctionReturn,
        //! Base node of a method declaration
        MethodDeclaration,
        //! The type over which the method operates
        MethodType,

        //! A function call
        FunctionCall,
        //! The list of arguments passed into the function call
        FunctionCallArgumentList,

        //! Definition of a type
        Type,
        //! An expression that defines a type and a value (such as a variable or a function argument)
        TypedExpression,

        //! Access of a member object inside of a struct or an import (such as foo.bar or foo.bar())
        MemberAccess,
        //! Access of an item inside a collection (such as foo[0])
        IndexedAccess,

        //! Definition of a trait
        TraitDeclaration,
        //! A list of fields inside a trait definition
        TraitFieldList,
        //! Contains a list of the traits bound by a function, method or another trait (for example foo() [bar] {})
        TraitList,
        //! An individual trait
        Trait,
        //! Definition of a struct object
        StructDeclaration,
        //! A field inside of a struct
        StructField,
        //! Contains a list of fields inside a struct
        StructFieldList,
        //! A composing object inside of a struct definition
        Composition,
    };

    /*!
     * \brief An Abstract Syntax Tree node
     *
     * An ASTNode is a node inside an Abstract Syntax Tree. The node holds references to its children nodes, as well as
     * optional values associated to the node, such as parameters and a value. The node has a type indicating the
     * operation that the node represents
     */
    struct ASTNode {
    public:
        ASTNode() = default;

        /*!
         * \brief Constructs an empty node with the given type
         * \param meta Metadata
         * \param t Type of the node
         */
        explicit ASTNode(tiny::Metadata meta, tiny::ASTNodeType t) :type(t), meta(std::move(meta)) {};

        /*!
         * \brief Constructs a node withe the given type and one child
         * \param t Type of the node
         * \param c1 Child
         */
        explicit ASTNode(tiny::Metadata meta, tiny::ASTNodeType t, const tiny::ASTNode &c1);

        /*!
         * \brief Constructs a node withe the given type and two children
         * \param t Type of the node
         * \param c1 Child 1
         * \param c2 Child 2
         */
        explicit ASTNode(tiny::Metadata meta, tiny::ASTNodeType t, const tiny::ASTNode &c1, const tiny::ASTNode &c2);

        /*!
         * \brief Constructs a node withe the given type and three children
         * \param t Type of the node
         * \param c1 Child 1
         * \param c2 Child 2
         * \param c3 Child 3
         */
        explicit ASTNode(tiny::Metadata meta, tiny::ASTNodeType t, const tiny::ASTNode &c1,
                const tiny::ASTNode &c2, const tiny::ASTNode &c3);

        /*!
         * \brief Constructs a node withe the given type and value
         * \param t Type of the node
         * \param v Value of the node
         */
        explicit ASTNode(tiny::Metadata meta, tiny::ASTNodeType t, tiny::Value v): type(t),
                                                                                   meta(std::move(meta)),
                                                                                   val(std::move(v)) {};

        //! The type of the node. Defaults to None
        tiny::ASTNodeType type = ASTNodeType::None;
        //! A vector of Parameters for this node
        std::vector<tiny::Parameter> params;
        //! A vector of the children of this node
        std::vector<std::shared_ptr<tiny::ASTNode>> children; // TODO Perhaps use just the object to avoid memory fragmentation?
        //! Metadata relating to this node
        tiny::Metadata meta;
        //! The optional value held by this node
        tiny::Value val;

        /*!
         * \brief Serializes the node as a string descriptor, containing basic information about the node
         * \return A std::string with minimal basic about the node
         */
        [[nodiscard]] std::string toString() const;

        /*!
         * \brief Serializes the node as a JSON object and recursively serializes its children
         * \return A nlohmann::json with the data of the ASTNode and its descendants
         */
        [[nodiscard]] nlohmann::json toJson() const;

        /*!
         * \brief Fetches a Parameter by type
         * \param t Type of the Parameter to search for
         * \return The Parameter, if found
         *
         * Fetches a Parameter by type. If more than one Parameter of a given type is present, the behaviour is undefined.
         * Throws NoSuchParameter if the parameter doesn't exist.
         */
        [[nodiscard]] tiny::Parameter getParam(tiny::ParameterType t) const;

        /*!
         * \brief Returns whether the node contains a parameter of matching type
         * \param t Type of the Parameter to search for
         * \return True if it contains a parameter of type t, false otherwise
         */
        [[nodiscard]] bool hasParam(ParameterType t) const;

        /*!
         * \brief Adds a parameter to the node
         * \param p Parameter to add
         */
        void addParam(const tiny::Parameter &p);

        /*!
         * \brief Fetches a child node by type and throws if no such children exists
         * \param meta Metadata of the current context
         * \param t Type of the ASTNode to search for
         * \return An ASTNode shared pointer
         *
         * Fetches a child node by type. If more than one node of a given type is present, the behaviour is undefined.
         * Throws if no such child exists.
         */
        [[nodiscard]] std::shared_ptr<tiny::ASTNode> getChild(tiny::ASTNodeType t) const;

        /*!
         * \brief Fetches the first-most node, and throws if it doesn't exist
         * \param meta Metadata of the current context
         * \return An ASTNode shared pointer
         *
         * Fetches the first-most child. Throws if no such child exists.
         */
        [[nodiscard]] std::shared_ptr<tiny::ASTNode> getFirstChild() const;

        /*!
         * \brief Fetches the second-most node, and throws if it doesn't exist
         * \param meta Metadata of the current context
         * \return An ASTNode shared pointer
         *
         * Fetches the second-most child. Throws if no such child exists.
         */
        [[nodiscard]] std::shared_ptr<tiny::ASTNode> getSecondChild() const;

        /*!
         * \brief Adds a children node
         * \param c ASTNode to add
         */
        void addChildren(const tiny::ASTNode &c);

        /*!
         * \brief Adds all the node in the list as children
         * \param cs ASTNodes to add
         */
        void addChildren(const tiny::StatementList &cs);

        /*!
         * \brief Fetches the tiny::String from the value
         * \return The string held by the node
         *
         * Fetches the tiny::String from the value. Throws if no such child exists. Fails if the value doesn't contain a string
         */
        [[nodiscard]] tiny::String getStringVal() const;

        /*!
         * \brief Returns whether the node's type is an operation
         * \return True if the node's type is an operation, false otherwise
         */
        [[nodiscard]] bool isOperation() const;
    };

    //! An Import holds information on an individual import call such as the name of the module and its optional alias.
    struct Import {
        explicit Import() = default;

        /*!
         * \brief Creates a standard (non-aliased) import with the name of the module
         * \param modl Name of the module
         */
        explicit Import(tiny::String modl) :mod(std::move(modl)) {};

        /*!
         * \brief Creates an aliased import over the name of the module
         * \param modl Name of the module
         * \param als Alias of the imported module
         */
        explicit Import(tiny::String modl, tiny::String als) :mod(std::move(modl)), alias(std::move(als)) {};

        //! Name of the module getting imported
        tiny::String mod;
        //! Optional alias for the import
        tiny::String alias;

        /*!
         * \brief Serializes the Import as a JSON object
         * \return A nlohmann::json with the data of the Import
         */
        [[nodiscard]] nlohmann::json toJson() const;
    };

    /*!
     * \brief An ASTFile represents the Abstract Syntax Tree of a Tiny file
     *
     * An ASTFile represents the Abstract Syntax Tree of a Tiny file. It holds the AST generated by the parser, the
     * name of the module the file is a part of and the imports it needs. It also holds limited metadata about the file
     * such as its path.
    */
    struct ASTFile {
        ASTFile() = default;

        /*!
         * \brief Create a full ASTFile
         * \param fn Filename
         * \param modl Module name
         * \param imprts Vector lisitng the imported modules
         * \param stmts Vector of the AST roots
         */
        explicit ASTFile(tiny::File fn,
                         tiny::String modl,
                         std::vector<tiny::Import> imprts,
                         tiny::StatementList stmts) :
                file(fn),
                mod(std::move(modl)),
                imports(std::move(imprts)),
                statements(std::move(stmts)) {};

        //! File that generated this AST
        tiny::File file;
        //! Name of the module which the file is a part of
        tiny::String mod;
        //! Imports called by the code in the file
        std::vector<tiny::Import> imports;
        //! The AST
        tiny::StatementList statements;

        /*!
         * \brief Serializes the file into a JSON object
         * \return A nlohmann::json with the data of the ASTFile
         */
        [[nodiscard]] nlohmann::json toJson() const;

        /*!
         * \brief Creates a JSON dump of the AST
         * \param path Where to create the file
         * \return A nlohmann::json with the data of the Parameter
         */
        void dumpJson(const std::filesystem::path &path) const;
    };
}

#endif //TINY_AST_H
