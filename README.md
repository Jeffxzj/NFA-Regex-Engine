# A NFA-Based Regex Engine

## Introduction

The regex engine we implemented supports the **POSIX-Extended Regular Expressions** (ERE) standard, which includes the following syntax. ERE is used by Unix-tools like `egrep` and `awk`. 

| Metacharacter | Description                                                  |
| :-----------: | :----------------------------------------------------------- |
|       .       | Matches any single character                                 |
|      [ ]      | A bracket expression. Matches a single character that is contained within the brackets. |
|     [^ ]      | Matches a single character that is not contained within the brackets. |
|       ^       | Matches the starting position within the string.             |
|       $       | Matches the ending position of the string or the position just before a string-ending newline |
|      ( )      | A marked subexpression is also called a block or capturing group. |
|       *       | Matches the preceding element zero or more times.            |
|       +       | Matches the preceding element one or more times.             |
|       ?       | Matches the preceding element one or zero times.             |
|      \|       | Matches the preceding element or the following element       |
|    {m, n}     | Matches the preceding element at least *m* and not more than *n* times |
|      {m}      | Matches the preceding element exactly *m* times.             |
|     {m,}      | Matches the preceding element at least *m* times.            |
|     {,n}      | Matches the preceding element not more than *n* times.       |

**Character classes**

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

### NFA

A **nondeterministic finite automaton** (NFA) are widely used in implementation of regular expressions.  

## Usage

1. Go to the project folder, configure and compile the project.

   ```shell
   mkdir build && cd build
   cmake ..
   make -j
   ```

2. Run the executable with the folder containing test cases as its first argument. This following command will run all the testcase files under `test` folder and redirect the output to the file `result.txt`. 

   ```shell
   ./regex ../test/ >& result.txt
   ```

3. Our program also supports different level of debug information, adding the following environment variables before running executable can produce different debug information for different modules. 



## Implementation

### Software Environment 

We wrote this project using C++ with C++20 standard, without any third-party library, only the std libraries are included. We build the project on a machine with following configuration.

- Ubuntu 20.04.3 LTS, Linux Kernel 5.4.0.-91-generic
- gcc version 9.4.0
- cmake version 3.16.3

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

The `build_graph()` function is the main function to iterate the tokens and generating the NFA graph after parsing it will store the graph in 

#### Automata

### 

### Usage:

1. Software Testing


#### Test Case Format

Test cases are under the `test/` folder, they should have `.txt` file extension. Its format is shown below. 

```
V	[]

V	[^]

I	[b-a]

V	[a-z]
	0	1	a
	-	-	?\n?
```

The first letter denotes whether it is a valid regex expression, 'V' for valid, 'I' for invalid.  A tab (must be a tab, containing any spaces will cause an error) goes after the letter, then goes the input regex expression. If there is nothing below the expression, the regex engine will only parse the expression, and do no matching. 

If the test case has string to match, the first number below the expression is the start position of matching, the second is the match length of that expression, the third is the string to be matched. The three arguments are also separated using one tab.   

#### Output Format 

Output containing both stdout and stderr, so if redirecting it to a file, `>&` should be used. 

#### Code Coverage

A regex engine is a complex system, we first use groups of test cases to ensure a full code coverage, that is the our test cases can cover every conditional branch of our code.

We find a website [regular-expression-test-cases](https://blog.robertelder.org/regular-expression-test-cases/) that help use find some useful test cases that can cover most code for a regex engine. We also build our own cases to cover as much code as possible.

The code coverage tool we are using  is `gcov` and its extension `lcov` to see the results. To compile the project with `gcov`, we add compiler flags `-fprofile-arcs -ftest-coverage` to CMake file.

```cmake
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -ggdb -g3 -fno-omit-frame-pointer -fprofile-arcs -ftest-coverage")
```

Under the `build/`  directory we first run the regex engine with all test cases redirecting its stdout and stderr to `result.txt`. Then we use `lov` to generate a `test_cov.info`  file and use `genhtml` to generate `out/`  folder containing a code coverage report. To see the report, just open the `out/index.html` using any web browser.

```shell
# run the engine
./regex ../test/ >& result.txt 
# generate lcov info
lcov -c --directory CMakeFiles/regex.dir/ --output-file test_cov.info 
# generate html coverage report
genhtml test_cov.info  --output-directory outw
```

## Evaluation

## Future Work

## Reference

1. https://en.m.wikibooks.org/wiki/Regular_Expressions/POSIX-Extended_Regular_Expressions
2. https://en.wikipedia.org/wiki/Regular_expression#POSIX_basic_and_extended
3. https://en.wikipedia.org/wiki/Nondeterministic_finite_automaton
3. https://regex101.com/
3. https://blog.robertelder.org/regular-expression-test-cases/
3. https://medium.com/@naveen.maltesh/generating-code-coverage-report-using-gnu-gcov-lcov-ee54a4de3f11

