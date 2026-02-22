---

```md
# Professional Text Verifier (Spell Checker)

A high-performance **command-line spell checking system** implemented in **C++**, designed to efficiently validate English text and provide intelligent spelling suggestions using classic data structures and algorithms.

The project uses a **Trie-based dictionary** for fast word lookups, **dynamic programming** for spelling correction, and includes **performance analytics** to measure accuracy and response time.

---

## 🚀 Features

- Fast spell checking using a custom **Trie (prefix tree)**
- Intelligent spelling suggestions using **Levenshtein Edit Distance**
- Handles large-scale dictionaries (370,000+ English words)
- User-defined dictionary with add/remove and persistence support
- Real-time accuracy and performance tracking
- Supports single-word and paragraph-level text verification
- Interactive command-line interface (CLI)

---

## 🧠 Technical Overview

### Trie-Based Dictionary
- Stores all dictionary words in a prefix tree
- Enables **O(L)** lookup time, where *L* is the word length
- Supports efficient traversal and full dictionary enumeration
- Implemented using `std::unique_ptr` for memory safety

### Spell Checking Logic
- A word is considered correct if it exists in:
  - The main Trie dictionary, or
  - The user-defined dictionary (`unordered_set`)
- Input text is normalized (lowercasing, punctuation removal) before validation

### Spelling Suggestions
Suggestions are generated using a hybrid approach:
- Edit operations: insertion, deletion, substitution, and transposition
- **Levenshtein Edit Distance** computed using dynamic programming
- Dictionary-wide filtering for words within a small edit distance
- Suggestions are ranked and limited to the best matches

### User Dictionary
- Allows users to add and remove custom words
- Implemented using a hash-based container for **O(1)** average lookup
- Can be saved to disk for persistence across sessions

### Performance Analytics
The system tracks:
- Total number of spelling queries
- Number of correct queries
- Accuracy percentage
- Average query response time using `std::chrono`

---

## 📂 Project Structure

```

.
├── main.cpp          # Core implementation
├── dictionary.txt    # English dictionary (370K+ words)
├── user_dict.txt     # Optional user dictionary (generated at runtime)
└── README.md         # Project documentation

````

---

## ⚙️ How to Build and Run

### Prerequisites
- C++17 or later
- A standard C++ compiler (g++, clang, or MSVC)

### Compile
```bash
g++ -std=c++17 main.cpp -o text_verifier
````

### Run

```bash
./text_verifier
```

Make sure `dictionary.txt` is present in the same directory as the executable.

---

## 🖥️ Usage

After running the program, an interactive menu is displayed:

1. Check a single word
2. Check a paragraph of text
3. Add a word to the user dictionary
4. Remove a word from the user dictionary
5. Get spelling suggestions
6. View performance statistics
7. Reset statistics
8. Save user dictionary
9. Exit

---

## 📊 Example Output

```
INCORRECT spelling
Suggestions: example, samples, simple
Accuracy: 96.4%
Average query time: 0.03 ms
```

---

## 🛠️ Design Decisions

* **Trie over hash map for main dictionary**

  * Enables prefix traversal and scalable lookups
* **Hash-based user dictionary**

  * Efficient updates and exact lookups
* **Dynamic programming for edit distance**

  * Ensures accurate spelling suggestions
* **Hybrid architecture**

  * Balances performance, flexibility, and memory usage

---

## 🤝 Contributing

Contributions are welcome and appreciated.

If you would like to contribute:
- Fork the repository
- Create a new branch for your feature or fix
- Follow clean coding and C++ best practices
- Submit a pull request with a clear description of changes

Areas open for contribution include:
- Performance optimizations for suggestion generation
- Improved data structures for fuzzy matching
- Code refactoring and documentation improvements
- Additional test cases and input validation

---

## 💬 Feedback

Feedback and suggestions are always welcome.

If you find a bug, have a feature request, or want to share improvements:
- Open an issue describing the problem or idea
- Provide steps to reproduce bugs, if applicable
- Suggest enhancements with clear technical reasoning

Constructive feedback helps improve the project and is greatly appreciated.
---

## 📜 License

This project is intended for educational and learning purposes.
You are free to modify and extend it.

---

## 👤 Author

Developed by **[Priyansh]**