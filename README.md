# A NFA-Based Regex Engine

## Introduction

The regex engine we implemented supports the **POSIX-Extended Regular Expressions** (ERE) standard, which includes the following syntax. ERE is used by Unix-tools like `egrep` and `awk`.

#### Table of Metacharacters

| Metacharacter | Description                                                  |
| :-----------: | :----------------------------------------------------------- |
|       .       | Matches any single character.                                |
|      [ ]      | A bracket expression. Matches a single character that is contained within the brackets. |
|     [^ ]      | Matches a single character that is not contained within the brackets. |
|       ^       | Matches the starting position within the string.             |
|       $       | Matches the ending position of the string or the position just before a string-ending newline. |
|      ( )      | A marked subexpression is also called a block or capturing group. |
|       *       | Matches the preceding element zero or more times.            |
|       +       | Matches the preceding element one or more times.             |
|       ?       | Matches the preceding element one or zero times.             |
|      \|       | Matches the preceding element or the following element.      |
|    {m, n}     | Matches the preceding element at least *m* and not more than *n* times. |
|      {m}      | Matches the preceding element exactly *m* times.             |
|     {m,}      | Matches the preceding element at least *m* times.            |
|     {,n}      | Matches the preceding element not more than *n* times.       |

#### **Character classes**

The character classes only can be used in bracket expression '[ ]' .

| Metacharacter | Similar To             | Description                          |
| :-----------: | ---------------------- | ------------------------------------ |
|  `[:upper:]`  | `[A-Z]`                | uppercase letters                    |
|  `[:lower:]`  | `[a-z]`                | lowercase letters                    |
|  `[:alpha:]`  | `[[:upper:][:lower:]]` | upper- and lowercase letters         |
|  `[:alnum:]`  | `[[:alpha:][:digit:]]` | digits, upper- and lowercase letters |
|  `[:digit:]`  | `[0-9]`                | digits                               |
| `[:xdigit:]`  | `[0-9A-Fa-f]`          | hexadecimal digits                   |
|  `[:punct:]`  | `[.,!?:…]`             | punctuation                          |
|  `[:blank:]`  | `[ \t]`                | space and TAB characters only        |
|  `[:space:]`  | `[ \t\n\r\f\v]`        | blank (whitespace) characters        |
|  `[:cntrl:]`  |                        | control characters                   |
|  `[:graph:]`  | `[^ \t\n\r\f\v]`       | printed characters                   |
|  `[:print:]`  | `[^\t\n\r\f\v]`        | printed characters and space         |

## Usage

1. Go to the project folder, configure and compile the project.

   ```shell
   mkdir build && cd build
   cmake ..
   make -j
   ```

2. Run the executable with the folder containing test cases as its first argument. This following command will run all the testcase files under `test` folder and redirect the output (including `stdout` and `stderr` )  to the file `result.txt`.

   ```shell
   ./regex ../test/
   ```

3. Our program also supports different level of debug information, adding the following environment variables before running executable can produce different debug information for different modules.

   ```shell
   # print all debug info
   REGEX_DEBUG=1 ./regex ../test/
   # print debug info of the tokenizer (tokenizing result of the expression)
   REGEX_TOKENIZER_DEBUG=1 ./regex ../test/
   # print debug info of the parser (NFA graph of the expression)
   REGEX_PARSER_DEBUG=1 ./regex ../test/
   # print debug info of the automata (full matching results)
   REGEX_AUTOMATA_DEBUG=1 ./regex ../test/
   ```

## Implementation

### Software Environment

We wrote this project using C++ with C++20 standard, without any third-party library, only the std libraries are included. We build the project on a machine with following configuration. The total source lines of code (SLOC) is about 3000.

### Tokenizer

First step of dealing with a regex expression is to tokenize it since it got different types of metacharacters. In this step, we also do some expression validation jobs, such as checking if the parentheses/brackets/braces are matched, if character range is valid ([b-a] will throw an error) and so on, which eases the parser's job.

One optimization we take is that using the `TokenType::ATOM` to represent a series of chars, as a string, when matching we do not match one char by one char which reduce the NFA states.

For example, a regex expression `(a[^bx-z])|xy{2,3}` will be tokenized into:

```
LEFT_PARENTHESES
ATOM: a
LEFT_BRACKETS_NOT
ATOM: b
CHARACTER_RANGE: [x-z]
RIGHT_BRACKETS
RIGHT_PARENTHESES
VERTICAL_BAR
ATOM: xy
LEFT_BRACES
NUMERIC: 2
COMMA
NUMERIC: 3
RIGHT_BRACES
```

### Regex Graph

We use the traditional NFA (nondeterministic finite automaton) method represent regex the expression. The `Node` and `Edge` class and the connect functions inside `RegGraph`  can combine any NFA graphs the expression needs.

### Parser

After tokenizing the regex expression, we now got a series of tokens, then we need to parse it and at the mean time build the NFA graph for the regex expression using the `RegGraph` structure. Finally, the output of parser should be one NFA graph representing the whole regex expression.

```c++
class Parser {
public:
  using GraphStack = std::vector<std::pair<TokenType, std::vector<RegGraph>>>;
  GraphStack graph_stack;

  RegexTokenizer &tokenizer;
  RegGraph regex_graph;
  bool debug;


  std::optional<std::string> build_graph();
  RegGraph pop_and_join();

  Parser(RegexTokenizer &tokenizer) : tokenizer(tokenizer), debug{false} {
    debug =
        std::getenv("REGEX_DEBUG") != nullptr ||
        std::getenv("REGEX_PARSER_DEBUG") != nullptr;
  }
};
```

The `build_graph()` function is the main function to iterate the tokens and generating the sub NFA graphs and finally it will store the whole NFA graph to `regex_graph` of the `Parser`.

The `graph_stack` is the a stack to store some middle state graphs. It is really important for dealing with expressions with subexpressions, like `(a([b]|(c|d)))` , stack is a perfect data structure to simulate recursion. When we meet a ')', we will concatenate all the graphs in its level, pop this level and  push to the graph vector in the last level. Finally we will got a stack only with one level of graph vector and we will concatenate all the graphs in it to build the final NFA graph representing the input expression.

### Automata

We use an NFA with stack to match input, the longest first match is returned. The stack is used for tracking the match count in expression `{n,m}`. Dead loop is avoided by tracking the previous states while matching.

## Software Testing

### Input Test Cases Format

Test cases are under the `test/` folder, they should have `.txt` file extension. Its format is shown below.

```
V	a{3-4}
	9	14	a aa aaa aaaa
	-	-	a

V	[^]
	1	1	^a

I	[b-a]

VE	a*
	9	3	a ba baa aaa ba b

```

The first letter denotes whether it is a valid regex expression, 'V' for valid, 'I' for invalid, 'VE' shows that the expression will match null character.  A **tab** (must be a tab, containing any spaces will cause an error) goes after the letter, then goes the input regex expression. If there is nothing below the expression, the regex engine will only parse the expression, and do no matching.

The lines below the expression are the correct matching results and strings to matched, the first number and second number is the **correct** matching result the `Automata` returns, which is the **first longest** matching result. The first number is the **start position** of that match, and the second is the **length** of that match.  For example, the above `a{3-4}` will match 'aaa' and 'aaaa', but our `Automata` will only return result of 'aaaa', that is (9, 4). (The REGEX_AUTOMATA_DEBUG=1 mode can see all matching results).

If the expression cannot match that string, we put two `-` chars before the string, to show that the `AutoMata` should not return any match results on that string. Note that these three arguments are also separated with one tab.

### Output Format

Output containing both stdout and stderr, so if redirecting it to a file, `>&` should be used.

The output without debug info is shown below, every matching string and its matching result are printed below that expression. A successful match is shown below

```
+---------------------------------------
| TESTING REGEX: a{3,6}
+---------------------------------------

========== [ MATCHING ] ==========


---------- [  RESULT  ] ----------
NO_MATCH

========== [ MATCHING ] ==========
a aa aaa aaaa aaaaaaaaaa

---------- [  RESULT  ] ----------
MATCH: start: 14, size: 6, string: aaaaaa
```

If there is an match error, there will be a warning print like:

```
MATCH: start: 0, size: 0, string: abc
expect start: 0, expect size: 1
Warn at file ~/workspace/CS290P-Project/src/main.cpp, line 63: match error
```

### Code Coverage

A regex engine is a complex system, we first use groups of test cases to ensure a full code coverage, that is the our test cases can cover every conditional branch of our code.

We find a website [regular-expression-test-cases](https://blog.robertelder.org/regular-expression-test-cases/) that help use find some useful test cases that can cover most code for a regex engine. We also build our own cases to cover as much code as possible.

The code coverage tool we are using  is `gcov` and its extension `lcov` to see the results. To compile the project with `gcov`, we add compiler flags `-fprofile-arcs -ftest-coverage` to CMake file.

```cmake
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -ggdb -g3 -fno-omit-frame-pointer -fprofile-arcs -ftest-coverage")
```

Under the `build/`  directory we first run the regex engine with all test cases redirecting its stdout and stderr to `result.txt`. Then we use `lov` to generate a `test_cov.info`  file and use `genhtml` to generate `out/`  folder containing a code coverage report. To see the report, just open the `out/index.html` using any web browser.

```shell
# install lcov if you do not have one on your local environment
sudo apt install lcov
# run the engine
./regex ../test/ >& result.txt
# generate lcov info
lcov -c --directory CMakeFiles/regex.dir/ --output-file test_cov.info
# generate html coverage report
genhtml test_cov.info  --output-directory out
```

#### Coverage Result

![lcov_include.png](https://s2.loli.net/2021/12/30/w8ntXEKkclOjUQ3.png)

![lcov_src.png](https://s2.loli.net/2021/12/30/UHnlSIiykQvEb9j.png)

The LCOV report shows that our test cases cover all the functions and almost every lines of code except some default branches that will not be executed by a correct testing.

### memory leak

We use Valgrind to check memory leak. There are no leaks possible.

### Interesting Test Cases

Besides test cases to improve code coverage, we also include some real-world regex expressions in `test/usecases.txt'` to show the functionality of our regex engine. It includes the regex expressions to match email address/md5 hash/http links. The most longest one is an expression to match the multiples of 3, which we found it really interesting. The final test results show that our regex engine can pass all these test cases.

## Future Work

## Reference

1. https://en.m.wikibooks.org/wiki/Regular_Expressions/POSIX-Extended_Regular_Expressions
2. https://en.wikipedia.org/wiki/Regular_expression#POSIX_basic_and_extended
3. https://en.wikipedia.org/wiki/Nondeterministic_finite_automaton
4. https://regex101.com/
5. https://blog.robertelder.org/regular-expression-test-cases/
6. https://medium.com/@naveen.maltesh/generating-code-coverage-report-using-gnu-gcov-lcov-ee54a4de3f11
