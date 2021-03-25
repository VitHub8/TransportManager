#pragma once

#include <iostream>
#include <map>
#include <string>
#include <variant>
#include <vector>
#include <iomanip>


namespace Json {

    class Node : std::variant<std::vector<Node>,
                              std::map<std::string, Node>,
                              int,
                              double,
                              bool,
                              std::string> {
    public:
        using variant::variant;

        const auto& AsArray() const {
            return std::get<std::vector<Node>>(*this);
        }
        const auto& AsMap() const {
            return std::get<std::map<std::string, Node>>(*this);
        }
        int AsInt() const {
            return std::get<int>(*this);
        }
        bool AsBool() const {
            return std::get<bool>(*this);
        }

        double AsDouble() const {
            if (std::holds_alternative<double>(*this)) {
                return std::get<double>(*this);
            }
            if (std::holds_alternative<int>(*this)) {
                return static_cast<double>(AsInt());
            }
        }

        const auto& AsString() const {
            return std::get<std::string>(*this);
        }

        void Print(std::ostream& os) const {
            if (std::holds_alternative<int>(*this)) {
                PrintI(os, AsInt());
            } else if (std::holds_alternative<double>(*this)) {
                PrintD(os, AsDouble());
            } else if (std::holds_alternative<std::string>(*this)) {
                PrintS(os, AsString());
            } else if (std::holds_alternative<std::map<std::string, Node>>(*this)) {
                PrintM(os, AsMap());
            } else if (std::holds_alternative<std::vector<Node>>(*this)) {
                PrintV(os, AsArray());
            }
        }

    private:

        static void PrintM(std::ostream& os, const std::map<std::string, Node>& map) {
            using namespace std;
            os << "{"s;
            bool first = true;
            for (const auto& [k, v] : map) {
                if (!first) {
                    os << ","s;
                }
                first = false;
                os << quoted(k) << ":"s;
                v.Print(os);
            }
            os << "}"s;
        }

        static void PrintV(std::ostream& os, const std::vector<Node>& v) {
            using namespace std;
            os << "["s;
            bool first = true;
            for (const auto& x : v) {
                if (!first) {
                    os << ","s;
                }
                first = false;
                x.Print(os);
            }
            os << "]"s;
        }

        static void PrintS(std::ostream& os, const std::string& s) {
            using namespace std;
            os << quoted(s);
        }

        static void PrintI(std::ostream& os, int num) {
            os << std::to_string(num);
        }

        static void PrintD(std::ostream& os, double num) {
            using namespace std;
            os << fixed << setprecision(16) << num;
        }

    };


    class Document {
    public:
        explicit Document(Node root);

        const Node& GetRoot() const;

    private:
        Node root;
    };

    Document Load(std::istream& input);
    void PrintDoc(std::ostream& os, const Document& doc);

}
