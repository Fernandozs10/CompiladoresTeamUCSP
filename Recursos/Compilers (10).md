# Scanners

MSc. Gina Muñoz Salas

### Where we are

**Front End Optimizer Back End** **Scanner** Parser Elaborator Opt. 1 Opt. 2 Opt.*n* Selector Scheduler Allocator

Shared Infrastructure

The scanner is the only pass that touches*every*character of the input program. Its input is larger than any other pass’s, and the techniques that make it fast are simple enough to state in one lecture.

Scanners

### What a scanner does

**Characters in, words out** x = count * 2 Each call returns a pair*⟨lexeme*,*category⟩*, sometimes called a**token**. The lexeme is the *⟨*x, identifier*⟩* *⟨*=, assign*⟩* spelling; the category is the part of speech: a *⟨*count, identifier*⟩* terminal symbol of the grammar the parser will *⟨**, times*⟩* use. *⟨*2, number*⟩* The rules that group characters into words are the language’s*microsyntax*.

Scanners

### Three different times

**01 02 03** **Design time Build time Compile time** The compiler writer specifies the Tools turn that specification into The end user runs the compiler. microsyntax: how words are code, and a compiler turns the The scanner converts the applica- spelled. She also picks an imple-code into an executable scanner. tion code into a stream of classified mentation strategy. words.

Scanners

**01**

## Recognizing words

### A recognizer for one word

|c←NextChar();||s₀ n|s₁ e|s₂ w|s₃|
|---|---|---|---|---|---|
|if (c = ’n’) then||||||
|c←NextChar();||||||
|if (c = ’e’) then||||||
|c←NextChar();|One test per character. The double circle is anaccepting|||||
|if (c = ’w’) then report success; else try something else; else try something else; else try something else;|state. Transitions to the error states||drawn.|are implicit and never||

*e*

Scanners

### Recognizers combine

|||||s₂|w|s₃|
|---|---|---|---|---|---|---|
||||e||||
|||s₁|||||
||n||||||
||s₀||o|s₄|t|s₅|
|new,notandwhilein one recognizer: merge the start states, relabel, share common prefixes. Three accepting states, one transition per input character, running time proportional to the length of the input.|w|s₆|h|s₇|i|s₈|

l *s₉* e *s₁₀*

Scanners

### The formalism: finite automaton

**A five-tuple**(*S,*Σ*,δ,s₀,SA*) **Acceptance** *S*the finite set of states, including*se* The FA accepts*x₁x₂...xn* if and only if Σthe finite alphabet
*δ*(*...δ*(*δ*(*s₀,x₁*)*,x₂*)*...,xn*)*∈SA*
*δ*(*s,c*)the transition function *s₀* the start state Two ways to fail: some*xj* drives the FA into*se*, or the input runs out in a nonaccepting state. If the FA *SA ⊆S*the accepting states *passed through*an accepting state on the way, the input contains a valid word as a prefix: the observation scanners use to find word boundaries.

Scanners

### Infinite languages need cycles

*δ***as a table** *s₁* <u>δ 0 1...9 other</u> 0 *s₀ s₁ s₂ se* *s₀ s₁ se se se* *s₂ s₂ s₂ se*

1... 9 *s*2 0... 9**The skeleton never changes**
state*←s₀*; char*←*NextChar(); An unsigned integer is a zero, or a nonzero digit while (state*̸*=*se* and char*̸*=eof) do followed by zero or more digits. The self-loop state*←δ*(state, char); replaces an infinite path. char*←*NextChar(); if (state*∈SA*) then accept; else reject;

Scanners

### In class

**EXERCISE** **Build two recognizers**

1. An FA for identifiers: one alphabetic character followed by*up to five*alphanumeric characters.
2. An FA for a Pascal comment: an open brace, then zero or more characters drawn fromΣ*−{*}*}*, then a close brace. How many states does each need? Which one needs a cycle, and why?
Scanners

**02**

## Regular expressions

### Three operations, and nothing else

**01 02 03**

|Alternation||Concatenation||Kleene closure|||
|---|---|---|---|---|---|---|
|r|sdenotesL(r)∪L(s).||rsdenotes{xy|x∈L(r),y∈||r = ⋃︁|r : zero or more words||
|bow | row,|or equivalently|L(s)}.||fromL(r).|||
|(b|r)ow.||Writing means: in this order.|side by side|This is cyclic edge.|the notation|for the FA’s|

*∗ ∞ i* *i*=0 symbols

Precedence: parentheses, then closure, then concatenation, then alternation. The positive closure*r*+=*rr∗*and the finite closure*ri*are conveniences, both rewrite into the three basic operations, so we ignore them in the constructions.

Scanners

### Microsyntax of a real language

Identifier([A...Z]|[a...z]) ([A...Z]|[a...z]|[0...9])*∗*
Unsigned integer0|[1...9][0...9]*∗*

|Unsigned integer0|[1...9][0...9]|||
|---|---|---|
|Real number(0|[1...9][0...9]||)(ϵ|.[0...9]|
|Character string"(∧")|"||
|Line comment//(∧\n)|\n||
|Block comment/*(∧*|*|∧/)|*/|
|Keywordif|while return||

*∗ ∗*) *∗* *∗* + *∗*

*∧c*is the complement of*c*with respect toΣ, the operatorlexandflexspellˆ. Note how much of the complexity in the block-comment RE comes from a*two-character*delimiter.

Scanners

### More states cost space, not time

**Loose specification Tight specification** +
r[0...9] r([0...2]([0...9]|*ϵ*)|[4...9]
|(3(0|1|*ϵ*))) Two states. Acceptsr29. Also acceptsr99999, al- though no processor has 100,000 registers. More states, more transitions, more memory. Accepts r0,r00,r31; rejectsr32.

**The cost of running an FA is proportional to the length of the input**, not to the complexity of the RE or the number of states. Both automata above take exactly one transition per character. Build time may grow; run time does not.

Scanners

### In class

**EXERCISE** **Rewrite and design**

1. A six-character identifier is often written with a finite closure:
([A...Z]|[a...z])([A...Z]|[a...z]|[0...9])
5. Rewrite it using only alternation, concatenation and Kleene closure.

2. In PL/I a string starts and ends with", and a literal quotation mark is written by doubling it. Give an RE and an FA that recognize PL/I strings.
Scanners

**03**

## From RE to DFA

### The cycle of constructions

Thompson’s subset <u>construction construction</u> RE NFA DFA

Hopcroft’s algorithm

minimal DFA Kleene’s construction The cycle proves that REs and FAs have equivalent expressive power: anything an FA can recognize, an RE can specify, and vice versa. We will walk the first three arrows; Kleene’s construction is in §2.6.1.

Scanners

### Nondeterminism

**Where it comes from Two models of NFA behavior** An*ϵ*-transition consumes no input. It lets us **Omniscient.**At each choice, the NFA glue automata together — but it can give a guesses the transition that leads to accep- state two ways out on the same character. tance, if one exists. a **Cloning.**At each choice, the NFA clones itself and pursues every path at once. The set of si-

|n₀|ϵ n₁|a n₂|b n₃|
|---|---|---|---|
|Onainn₀: loop, or step throughϵ? Foraabthe first is|right; forab, the second.|||

multaneously active states is its*configuration*. An NFA with*N*states has at most*|*2*N|*configurations, a finite number. That is exactly why a DFA can simu- late it, at the price of space but not of time.

Scanners

### Thompson’s construction

<u>a</u> *ϵ ϵ* <u>a</u> *a|b* *a* *ϵ* <u>b</u> *ϵ*

a *ϵ* b *ab* *ϵ* a *ϵ* *∗* *a* *ϵ*

*ϵ* Template-driven: build a trivial NFA for each symbol, then apply one transformation per operator, in precedence order. Every NFA produced has*one*start state and*one*accepting state, nothing enters the start state, nothing leaves the accepting state — Scannersproperties that make the next construction easy to implement.

### The subset construction

*q₀ ←*FollowEpsilon({*n₀*}) **What it builds** Q*←q₀* WorkList*←*{*q₀*} *Q*holds every*valid configuration*the NFA can while (WorkList*̸*=*∅*) do reach;*T*holds the transitions between them. remove q from WorkList One DFA state per configuration, one DFA for each character c*∈*Σdo transition per entry of*T*. temp*←*FollowEpsilon(Delta(q,c)) if temp*̸*=*∅*then A configuration*qi* is accepting iff it contains an if temp*/∈*Q then accepting state of the NFA. add temp to Q and WorkList T[q,c]*←*temp *Q*can grow to*|*2*N|*states. It never shrinks, and that monotonicity is what guarantees the loop halts.

Scanners

### A fixed-point computation

**01 02 03** **Monotone function Bounded domain Unique result** Each pass over the worklist only Every*qi*is a subset of*N*. Since Set union is commutative and as- *N* *adds*to*Q*: *Qi⊆Qi*+1. Nothing *N*is finite, 2 is finite, so the loop sociative, so the order of removal is ever removed. cannot iterate forever. from the worklist cannot change the final*Q*: only the names of its elements.

This pattern, iterate a monotone function over a bounded domain until nothing changes, will return in data-flow analysis, in FIRST/FOLLOWsets, and in register allocation. Recognize it now and you will recognize it four more times this semester.

Scanners

### DFA minimization

**Hopcroft’s algorithm The two rules** *c* Build a partition*P*=*{p₁,...,pm}*of the DFA **1.**If*di,dj* are in the same set and*di −→dx*, *c* states, where each*pi* is a set of*behaviorally dj −→dy*, then*dx* and*dy* must also land in the *equivalent*states. same set — for every*c∈*Σ. Start with the coarsest split*P₀* =*{DA,D−* **2.**A set contains either accepting states or *DA}*, then refine until no set can be split. nonaccepting states, never both. algorithm Fewer states means a smaller table, which may fit in

|Hopcroft’s|only ever|splits sets;|it never|
|---|---|---|---|
|merges them.||||

the first-level cache. On modern machines that can matter more than the transition count.

Scanners

### The whole pipeline ona(b|c)*

**DFA after minimization**

*s₀* <u>a</u> *s₁* b, c

Thompson’s construction produces an NFA with ten states. The subset construction turns it into a four-state DFA. Minimization collapses it to two.**This is the automaton a human would have drawn by hand**, which is the point: the mechanical path arrives at the same place, without requiring insight.

Scanners

### In class

**EXERCISE** **Run the pipeline** Take the REwho | what | where.

1. Apply Thompson’s construction to build an NFA.
2. Apply the subset construction to build a DFA.
3. Minimize it. How many states does each stage have? Where does the sharing of the common prefixwhfinally appear?
Scanners

**04**

## From DFA to scanner

### A scanner is not a DFA

|Different model of execution||Different answer|||
|---|---|---|---|---|
|A DFA readsallits input and answers yes or||A DFA|returns a bit.|A scanner|
|no. A scanner readsenoughinput to find the next word, and leaves the stream positioned||⟨lexeme, category⟩.|||
|to find the one after it.||So each accepting state must map to exactly one category.|Build a separate NFA per rule,||
|So the scanner runs until it hitss|, and then|join them withϵ-transitions from a new start|||
|rolls back,|to the|state, and let the subset construction do the|||
|most recent accepting state.||work.|||

returns

*e* one character at a time,

It must therefore record the states it passed through, When two rules overlapthenmatches both the key- or invert*δ*. word rule and the identifier rule, the generator resolves it by precedence.lexandflexgive priority to the rule listed first. Scanners

### Two ways to handle keywords

**01 02** **One rule per keyword Preload the symbol table** The scanner returns the category through the same Fold keywords into the identifier rule, and preload the ta- mechanism it uses for numbers and identifiers. The DFA ble of identifier names with the keywords and their cat- gets more states and generation costs a little more, but it egories. The scanner then finds each keyword as an is still*O*(1)per character. already-categorized identifier.

Option 02 comes almost free: most scanners build that table anyway, both as a start on the compiler’s symbol table and as a way to map names to small integers for cheap comparison.

Scanners

### Whitespace decides more than it looks

**01 02 03** **Most languages Fortran 66 Python** Whitespace has no meaning; the Blanks are not significant at all. To The opposite extreme: the num- scanner discards it. Its real effect telldo 10 i = 1fromdo 10 i = ber of leading blanks determines is its*absence*from the REs, that 1,100, the scanner must read past block structure. Add a rule that is what separatesdofromi, and the=to the comma. Few lan-matches an end-of-line plus zero makesa * bscan likea*b. guages repeated the experiment. or more blanks, then compare its length with the previous one.

**ChocoPy inherits Python’s rule.**Your scanner will emitINDENTandDEDENTas words that appear nowhere in the source text, keep an indentation stack, and remember that a single dedent may need to close several blocks at once.

Scanners

**05**

## Implementing scanners

### Three implementation strategies

**01 02 03** **Table-driven Direct-coded Hand-coded** A fixed skeleton plus generated ta-The same DFA, represented*im-*Written by the compiler writer. Al- bles: *δ*for transitions,CharClass *plicitly*: one custom code frag-lows specializing individual paths, to map a character to a column. ment per state, transitions become dropping the state stack when Language knowledge lives entirely jumps. No address arithmetic, no there is one accepting state, and in the data. memory loads per character. returning a register*number*rather than a string.

All three are*O*(1)per character; they differ in the constants. Hand-coded scanners remain surprisingly common in production compilers,GCC4.0 used them in several front ends, despiteflexhaving been built for that project.

Scanners

### Where the time goes

**The table-driven inner loop What direct coding removes** while (state*̸*=*se*) do Each state becomes a labelled fragment. char*←*NextChar(); char,lexemeand the implicit state all fit in col*←*CharClass[char]; registers; the branch replaces the table lookup state*←δ*[state, col]; entirely. Two address computations and two loads per charac- *s*1: char*←*NextChar(); ter: lexeme*←*lexeme + char; @CharClass₀ + i*×*w push(*s₁*); @*δ₀* + (state*×*cols + col)*×*w if (’0’*≤*char*≤*’9’) then goto*s₂*; else goto*sout*;

Scanners

### Maximal munch and quadratic roll back

**The pathological case The fix** *∗* Takeab | (ab) cand the inputabababab. Keep a bit arrayFailed[state, position]. Each time roll back pops a dead-end transi-The scanner reads all eight characters before tion, mark it. Before taking a transition, test it can conclude the longest valid prefix isab. the bit and break out if it is already known to The next call reads six, then four, then two fail. *O*(*n*)in the worst case. Most programming languages have microsyntax sim-The cause is the desire to return the*longest*word that ple enough that this never arises, but the cost of insur- is a prefix of the input. ing against it is one bit test per character.

Scanners

### Two practical details

**Buffering the input Compressing the table** One function call per character is expensive *δ*has*|states|·|*Σ*|*entries. If it outgrows the relative to simulating the DFA. Read into a first-level cache, performance suffers. buffer instead and index through it;RollBack Identical*columns*are common, the ten digit becomes a pointer decrement. columns of an integer recognizer are the Use**two adjacent buffers**of*n*characters and same. Merge them and map characters to increment modulo 2*n*. AFencepointer marks columns throughCharClass. how far back it is safe to go. If the DFA is minimal, its*rows*cannot be identical, that With*n*=2048 or 4096, roll back is bounded by the is exactly what minimization guarantees. And the row longest whitespace-free string in the program. Few for*se* is never read, so it need not be stored. identifiers are 2048 characters long. Scanners

### Summary

The compiler writer specifies microsyntax with**regular expressions**. Thompson’s construction turns them into an**NFA**, the subset construction into a**DFA**, Hopcroft’s algorithm into a**minimal DFA**, and a skeleton or a code generator into a**scanner**. Every step is mechanical. That is the whole point: the scanner is the one part of the compiler where theory delivers a complete, automatic construction, and where the resulting program runs in*O*(1)time per character regardless of how complicated the specification was. **Front End Optimizer Back End** **Scanner** Parser Elaborator Opt. 1 Opt. 2 Opt.*n* Selector Scheduler Allocator

Shared Infrastructure

Scanners

### References

|Engineering a Compiler, 3rd ed.|The Dragon Book|
|---|---|
|Cooper & Torczon, 2023.|Aho, Lam, Sethi & Ullman.|
|Chapter 2: Scanners.|Chapter 3: Lexical Analysis.|

§2.2 recognizers · §2.3 regular expressions · §2.4 RE Complementary treatment of the same constructions, to DFA · §2.5 implementation · §2.6 advanced topics. with more detail onlex-style specifications.

Scanners

# Scanners

MSc. Gina Muñoz Salas