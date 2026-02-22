#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <memory>
#include <vector>
#include <algorithm>
#include <chrono>
#include <unordered_set>
#include <sstream>
#include <cctype>

using namespace std;

class Trie {
private:
    struct Node {
        bool isEnd = false;
        unordered_map<char, unique_ptr<Node>> children;
        
        bool hasChild(char c) const { 
            return children.find(c) != children.end(); 
        }
        Node* getChild(char c) { 
            return children[c].get(); 
        }
        void addChild(char c) { 
            children[c] = make_unique<Node>(); 
        }
    };

    unique_ptr<Node> root;
    
    void collectAllWords(Node* node, const string& prefix, vector<string>& words) const {
        if (node->isEnd) {
            words.push_back(prefix);
        }
        
        for (const auto& child_pair : node->children) {
            char ch = child_pair.first;
            Node* child = child_pair.second.get();
            collectAllWords(child, prefix + ch, words);
        }
    }

    size_t countWords(Node* node) const {
        size_t count = 0;
        if (node->isEnd) {
            count++;
        }
        
        for (const auto& child_pair : node->children) {
            count += countWords(child_pair.second.get());
        }
        return count;
    }

public:
    Trie() : root(make_unique<Node>()) {}

    void insert(const string& word) {
        Node* current = root.get();
        for (char c : word) {
            if (!current->hasChild(c)) {
                current->addChild(c);
            }
            current = current->getChild(c);
        }
        current->isEnd = true;
    }

    bool search(const string& word) const {
        Node* current = root.get();
        for (char c : word) {
            if (!current->hasChild(c)) {
                return false;
            }
            current = current->getChild(c);
        }
        return current->isEnd;
    }

    bool startsWith(const string& prefix) const {
        Node* current = root.get();
        for (char c : prefix) {
            if (!current->hasChild(c)) {
                return false;
            }
            current = current->getChild(c);
        }
        return true;
    }

    vector<string> getAllWords() const {
        vector<string> words;
        collectAllWords(root.get(), "", words);
        sort(words.begin(), words.end());
        return words;
    }

    vector<string> getWordsWithPrefix(const string& prefix) const {
        vector<string> words;
        
        Node* current = root.get();
        for (char c : prefix) {
            if (!current->hasChild(c)) {
                return words;
            }
            current = current->getChild(c);
        }
        
        collectAllWords(current, prefix, words);
        sort(words.begin(), words.end());
        return words;
    }

    size_t getWordCount() const {
        return countWords(root.get());
    }
};

class TextVerifier {
private:
    Trie dictionary;
    unordered_set<string> userDictionary;
    string dictionaryPath;
    
    mutable size_t totalQueries = 0;
    mutable size_t correctQueries = 0;
    mutable chrono::duration<double> totalQueryTime{0};

    int calculateEditDistance(const string& word1, const string& word2) const {
        size_t len1 = word1.size();
        size_t len2 = word2.size();
        vector<vector<int>> dp(len1 + 1, vector<int>(len2 + 1, 0));

        for (size_t i = 0; i <= len1; i++) dp[i][0] = i;
        for (size_t j = 0; j <= len2; j++) dp[0][j] = j;

        for (size_t i = 1; i <= len1; i++) {
            for (size_t j = 1; j <= len2; j++) {
                int cost = (word1[i - 1] == word2[j - 1]) ? 0 : 1;
                dp[i][j] = min({dp[i - 1][j] + 1, dp[i][j - 1] + 1, dp[i - 1][j - 1] + cost});
            }
        }
        return dp[len1][len2];
    }

    vector<string> generateEdits(const string& word) const {
        vector<string> edits;
        const string alphabet = "abcdefghijklmnopqrstuvwxyz";
        
        for (size_t i = 0; i < word.length(); i++) {
            edits.push_back(word.substr(0, i) + word.substr(i + 1));
        }
        
        for (size_t i = 0; i < word.length() - 1; i++) {
            string edit = word;
            swap(edit[i], edit[i + 1]);
            edits.push_back(edit);
        }
        
        for (size_t i = 0; i < word.length(); i++) {
            for (char c : alphabet) {
                string edit = word;
                edit[i] = c;
                edits.push_back(edit);
            }
        }
        
        for (size_t i = 0; i <= word.length(); i++) {
            for (char c : alphabet) {
                edits.push_back(word.substr(0, i) + c + word.substr(i));
            }
        }
        
        return edits;
    }

    vector<string> generateEditDistanceSuggestions(const string& word) const {
        vector<pair<string, int>> candidates;
        
        auto edits = generateEdits(word);
        for (const auto& edit : edits) {
            if (edit != word && checkSpelling(edit)) {
                int distance = calculateEditDistance(word, edit);
                candidates.push_back(make_pair(edit, distance));
            }
        }
        
        auto allWords = dictionary.getAllWords();
        for (const auto& dictWord : allWords) {
            int distance = calculateEditDistance(word, dictWord);
            if (distance <= 2) {
                candidates.push_back(make_pair(dictWord, distance));
            }
        }
        
        sort(candidates.begin(), candidates.end(),
            [](const pair<string, int>& a, const pair<string, int>& b) {
                return a.second < b.second;
            });
        
        vector<pair<string, int>> unique_candidates;
        for (size_t i = 0; i < candidates.size(); ++i) {
            if (i == 0 || candidates[i].first != candidates[i-1].first) {
                unique_candidates.push_back(candidates[i]);
            }
        }
        
        vector<string> suggestions;
        for (const auto& candidate : unique_candidates) {
            suggestions.push_back(candidate.first);
            if (suggestions.size() >= 10) break;
        }
        
        return suggestions;
    }

    string toLower(const string& str) const {
        string result = str;
        transform(result.begin(), result.end(), result.begin(), ::tolower);
        return result;
    }

public:
    TextVerifier() : dictionaryPath("dictionary.txt") {}
    
    bool loadDictionary(const string& filePath = "") {
        string path = filePath.empty() ? dictionaryPath : filePath;
        ifstream file(path);
        
        if (!file.is_open()) {
            cerr << "Error: Could not open dictionary file: " << path << endl;
            return false;
        }

        dictionary = Trie();
        string word;
        size_t count = 0;

        while (getline(file, word)) {
            word = toLower(word);
            word.erase(remove_if(word.begin(), word.end(), ::isspace), word.end());
            
            if (!word.empty()) {
                dictionary.insert(word);
                count++;
            }
        }

        file.close();
        cout << "Loaded " << count << " words from: " << path << endl;
        return true;
    }

    bool checkSpelling(const string& word) const {
        auto start = chrono::high_resolution_clock::now();
        
        string lowerWord = toLower(word);
        
        bool result = dictionary.search(lowerWord) || 
                      userDictionary.find(lowerWord) != userDictionary.end();
        
        auto end = chrono::high_resolution_clock::now();
        
        totalQueries++;
        if (result) correctQueries++;
        totalQueryTime += (end - start);
        
        return result;
    }

    vector<string> getSuggestions(const string& word, int maxSuggestions = 5) const {
        vector<string> suggestions = generateEditDistanceSuggestions(toLower(word));
        
        if (suggestions.size() > static_cast<size_t>(maxSuggestions)) {
            suggestions.resize(maxSuggestions);
        }
        
        return suggestions;
    }

    void addToUserDictionary(const string& word) {
        string lowerWord = toLower(word);
        userDictionary.insert(lowerWord);
        cout << "Added '" << word << "' to user dictionary" << endl;
    }

    bool removeFromUserDictionary(const string& word) {
        string lowerWord = toLower(word);
        bool removed = userDictionary.erase(lowerWord) > 0;
        if (removed) {
            cout << "Removed '" << word << "' from user dictionary" << endl;
        }
        return removed;
    }

    bool saveUserDictionary(const string& filePath = "user_dict.txt") const {
        ofstream file(filePath);
        
        if (!file.is_open()) {
            cerr << "Error: Could not save user dictionary" << endl;
            return false;
        }
        
        for (const auto& word : userDictionary) {
            file << word << "\n";
        }
        
        file.close();
        cout << "Saved user dictionary to: " << filePath << endl;
        return true;
    }

    size_t getDictionarySize() const {
        return dictionary.getWordCount();
    }

    size_t getUserDictionarySize() const {
        return userDictionary.size();
    }

    double getAccuracy() const {
        return totalQueries == 0 ? 100.0 : (static_cast<double>(correctQueries) / totalQueries) * 100.0;
    }

    double getAverageQueryTime() const {
        return totalQueries == 0 ? 0.0 : totalQueryTime.count() / totalQueries;
    }

    void showStatistics() const {
        cout << "\n=== Text Verifier Statistics ===" << endl;
        cout << "Total queries: " << totalQueries << endl;
        cout << "Correct queries: " << correctQueries << endl;
        cout << "Accuracy: " << getAccuracy() << "%" << endl;
        cout << "Average query time: " << getAverageQueryTime() * 1000 << " ms" << endl;
        cout << "Dictionary size: " << getDictionarySize() << " words" << endl;
        cout << "User dictionary: " << getUserDictionarySize() << " words" << endl;
        cout << "================================\n" << endl;
    }

    void resetStatistics() {
        totalQueries = 0;
        correctQueries = 0;
        totalQueryTime = chrono::duration<double>::zero();
        cout << "Statistics reset" << endl;
    }

    vector<string> checkText(const string& text) const {
        vector<string> misspelled;
        istringstream stream(text);
        string word;
        
        while (stream >> word) {
            word.erase(remove_if(word.begin(), word.end(), ::ispunct), word.end());
            if (!word.empty() && !checkSpelling(word)) {
                misspelled.push_back(word);
            }
        }
        
        return misspelled;
    }

    void setDictionaryPath(const string& path) { 
        dictionaryPath = path; 
    }
    
    string getDictionaryPath() const { 
        return dictionaryPath; 
    }
};

void displayMenu() {
    cout << "\n=== Text Verifier Menu ===" << endl;
    cout << "1. Check single word" << endl;
    cout << "2. Check text paragraph" << endl;
    cout << "3. Add word to user dictionary" << endl;
    cout << "4. Remove word from user dictionary" << endl;
    cout << "5. Show suggestions for word" << endl;
    cout << "6. Show statistics" << endl;
    cout << "7. Reset statistics" << endl;
    cout << "8. Save user dictionary" << endl;
    cout << "9. Exit" << endl;
    cout << "Choose option: ";
}

int main() {
    cout << "=== Professional Text Verifier ===" << endl;
    
    TextVerifier checker;
    
    if (!checker.loadDictionary()) {
        cout << "Failed to load dictionary. Exiting." << endl;
        return 1;
    }
    
    int choice;
    string input;
    
    while (true) {
        displayMenu();
        cin >> choice;
        cin.ignore();
        
        switch (choice) {
            case 1: {
                cout << "Enter word to check: ";
                cin >> input;
                if (checker.checkSpelling(input)) {
                    cout << "CORRECT spelling" << endl;
                } else {
                    cout << "INCORRECT spelling" << endl;
                    auto suggestions = checker.getSuggestions(input);
                    if (!suggestions.empty()) {
                        cout << "Suggestions: ";
                        for (size_t i = 0; i < suggestions.size(); ++i) {
                            cout << suggestions[i];
                            if (i < suggestions.size() - 1) cout << ", ";
                        }
                        cout << endl;
                    }
                }
                break;
            }
            
            case 2: {
                cout << "Enter text to check: ";
                getline(cin, input);
                auto misspelled = checker.checkText(input);
                if (misspelled.empty()) {
                    cout << "All words are correctly spelled!" << endl;
                } else {
                    cout << "Misspelled words: ";
                    for (size_t i = 0; i < misspelled.size(); ++i) {
                        cout << misspelled[i];
                        if (i < misspelled.size() - 1) cout << ", ";
                    }
                    cout << endl;
                }
                break;
            }
            
            case 3: {
                cout << "Enter word to add to user dictionary: ";
                cin >> input;
                checker.addToUserDictionary(input);
                break;
            }
            
            case 4: {
                cout << "Enter word to remove from user dictionary: ";
                cin >> input;
                if (!checker.removeFromUserDictionary(input)) {
                    cout << "Word not found in user dictionary" << endl;
                }
                break;
            }
            
            case 5: {
                cout << "Enter word for suggestions: ";
                cin >> input;
                auto suggestions = checker.getSuggestions(input);
                if (suggestions.empty()) {
                    cout << "No suggestions found" << endl;
                } else {
                    cout << "Suggestions: ";
                    for (size_t i = 0; i < suggestions.size(); ++i) {
                        cout << suggestions[i];
                        if (i < suggestions.size() - 1) cout << ", ";
                    }
                    cout << endl;
                }
                break;
            }
            
            case 6:
                checker.showStatistics();
                break;
                
            case 7:
                checker.resetStatistics();
                break;
                
            case 8:
                checker.saveUserDictionary();
                break;
                
            case 9:
                cout << "Goodbye!" << endl;
                return 0;
                
            default:
                cout << "Invalid option!" << endl;
        }
    }
    
    return 0;
}