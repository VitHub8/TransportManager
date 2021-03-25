#include "json.h"
#include <exception>

using namespace std;

namespace Json {

    Document::Document(Node root) : root(move(root)) {
    }

    const Node& Document::GetRoot() const {
        return root;
    }

    Node LoadNode(istream& input);

    Node LoadBool(istream& input) {
        string result;
        while (isalpha(input.peek())) {
            result.push_back(input.get());
        }
        if (result == "true"s) {
            return Node(true);
        } else if (result == "false"s) {
            return Node(false);
        }
    }

    Node LoadArray(istream& input) {
        vector<Node> result;

        for (char c; input >> c && c != ']'; ) {
            if (c != ',') {
                input.putback(c);
            }
            result.push_back(LoadNode(input));
        }
        return Node(move(result));
    }

    Node LoadDouble(istream& input) {
        double result = 0;
        double order_num = 10.0;
        while (isdigit(input.peek())) {
            result += (input.get() - '0')/order_num;
            order_num *=10;
        }
        return Node(double(result));
    }

    Node LoadInt(istream& input) {
        int result = 0;
        while (isdigit(input.peek())) {
            result *= 10;
            result += input.get() - '0';
        }
        return Node(result);
    }

    Node LoadNumber(istream& input, int SIGN) {
        if (!isdigit(input.peek())) {
            throw invalid_argument("wrong number");
        }
        Node result = Node((int)LoadInt(input).AsInt()*SIGN);
        if (input.peek() == '.') {
            input.get();
            Node temp = LoadDouble(input);
            result = Node((double)(result.AsDouble() + temp.AsDouble() * (double)SIGN));
        }
        return result;
    }


    Node LoadString(istream& input) {
        string line;
        getline(input, line, '"');
        return Node(move(line));
    }

    Node LoadDict(istream& input) {
        map<string, Node> result;

        for (char c; input >> c && c != '}'; ) {
            if (c == ',') {
                input >> c;
            }
            string key = LoadString(input).AsString();
            input >> c;
            result.emplace(move(key), LoadNode(input));
        }
        return Node(move(result));
    }

    Node LoadNode(istream& input) {
        char c;
        input >> c;
        if (c == '[') {
            return LoadArray(input);
        } else if (c == '{') {
            return LoadDict(input);
        } else if (c == '"') {
            return LoadString(input);
        } else if (c == '-') {
            return LoadNumber(input, -1);
        } else if (isdigit(c)) {
            input.putback(c);
            return LoadNumber(input, 1);
        } else {
            input.putback(c);
            return LoadBool(input);
        }
    }

    Document Load(istream& input) {
        return Document{LoadNode(input)};
    }

    void PrintDoc(std::ostream& os, const Document& doc) {
        doc.GetRoot().Print(os);
    }

}
