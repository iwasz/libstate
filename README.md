# To be documented
* Initial state is the state whih was first added (convention).
* Action callback interface options : can return Done or void, can take an Event argument or void.

# Milestones
1. Static, typesafe API.
1. Changing states.
1. Running actions in correct order (partially)
1. Retained input.

# TODOs
* [ ] Return neneric std::duration instead aof a std::chrono::nanoseconds
* [ ] There should be an error when adding exit to entry like that : ```entry ([] {}, exit ([] {}))```. This is obviously a mistake.
* [ ] When wrappers are sorted out, pay attention to function / method / lambda argument types (&, &&, universal && etc).
* [ ] state function should accept parameters in different order (more logical order) : exit, then  transitions, then exit. This is because
  exit action is only needed IF and only if there is at least one transition, but trnsitions can exist WITHOUT an exit action.
* [ ] Compilation problem when last action has a transition.
* [ ] All sorts of combinations should be possible when creating a state. With/without either of : entry, exit, transition. Transition 
should be with or without either of : condition, action (s).
* [ ] Make state with no actions possible.
* [ ] EventQueue by lvl reference (non const). Justify it in the docs (this queue is sort of a state machine internal state and this state
must be persisted between calls to run). 
* [ ] Consider adding input actions, I might be wrong, but I think I used one of those in MC60 (forgetting abit the name "input action").
* [ ] All sorts of misuses should be pointed out in compile time with meaningful messages (use static_asserts whenever possible).
* [ ] Nullary conditions does not work.
* [ ] Meaningful message when condition argumnet type is incompatible with event type, and so on.
* [ ] Allow raising an event from inside of actions (somehow).
* [ ] The code is so generic, that it sometimes accepts wrong type of arguments, like you can pass transition instead of a state. It should be that the compiler reported shuch a misuse on early stage and with meaningful messages.
* [ ] Configurable (as a template argument?) output class. Defaults to std::cout. oh, it also can be a lambda passed to the run method or something. Another idea is to have "instrument" / "augument" type passed to the run method. It could do all sorts of additional things like logging, measuring performance, making stats etc.
* [ ] Events as std::tuples or std::pairs, and then actions and conditions would get parameter pack of arguments (hana::unpacked or std::apply-ied).
* [x] Use std::tuple instead of boost::hana::tuple
* [x] Remove dependency on hana.
* [x] Test benchmark size on Cortex-M4 (RTTI is on)
  * [ ] Remove references to type_info, cannot use the library with RTTI on a µC (well you can, but nobody does that).

Copy (in Polish) of TODOs from the previous version of the lib. 
 
* [ ] Dodać globalny timeout. Jeśli w danym timeoucie nie uda się zmienić stanu, to trzeba podjąć jakąć 
* [ ] dokumentacja z tutorialem jest niezbędna, bo ja sam mam czasem problemy. Jak są ewaluowane warunki, co do nich wpada, w jakiej kolejności i tak dalej. Opisać wszystkie aspekty działania : jak dwie maszyny mogą pracować na jedym input/output i tak dalej.
* [ ] opisać, że zawsze trzeba pamiętyać czy jest odpowiednia ilość czasu na sprawdzenie warunków. Podać taki przykład:
  
  Kiedy jest jeden warunek na przejście, który oczekuje jakichś danych, to nie ma problemu. Na przykład :

  ```c++ 
  m->state (INIT, State::INITIAL)->entry (at ("ATE1\r\n"))
        ->transition (CHECK_OPERATING_MODE)->when (&ok)->then (&delay);
  ```      
 
  Maszyna będzie tak długo się kręciuć, aż dostanie "OK". Jeżeli jednak mamy warunki zdefiniowane tak, że oczekiwane wejście
  może nie zdążyć się pojawić, to trzeba dodać opóźnienie, żeby poczekać na te dane wejściowe. Przykład (błędny):
   
  ```c++ 
  m->state (CHECK_OPERATING_MODE)->entry (at ("AT+CWMODE_DEF?\r\n"))
      ->transition (VERIFY_CONNECTED_ACCESS_POINT)->when (eq ("+CWMODE_DEF:1"))->then (&delay)
      ->transition (SET_OPERATING_MODE)->when (&alwaysTrue)->then (&delay);
  ```
 
  W powyższym przykładzie warunki dwóch przejść są sprawdzane jedno po drugim. Najpierw maszyna sprawdzi, czy w kolejce
  wejściowej jest jakiś element z napisem "+CWMODE_DEF:1", a jeśli nie ma, to natychmiast przejdzie do stanu SET_OPERATING_MODE.
  Trzeba dać maszynie trochę czasu na oczekiwanie na dane. Powinno być:
 
  ```c++
  m->state (CHECK_OPERATING_MODE)->entry (and_action (at ("AT+CWMODE_DEF?\r\n"), &delay))
                ->transition (VERIFY_CONNECTED_ACCESS_POINT)->when (eq ("+CWMODE_DEF:1"))->then (&delay)
                ->transition (SET_OPERATING_MODE)->when (&alwaysTrue)->then (&delay);
  ```
 
  Taki błąd jak wyżej może prowadzić do trudnych do wykrycia konsekwencji. Jeśli jak wyżej mamy dwa przejścia stanowe, to
  dość łatwo będzie się zorientować, że mimo że modem wysłał odpowiedź "+CWMODE_DEF:1" to jesteśmy w słym stanie (w SET_OPERATING_MODE zamiast VERIFY_CONNECTED_ACCESS_POINT). Ale w przypadku gdy mamy takie przejście (błędne - brakuje delay):

  ```c++
  m->state (CLOSE_AND_RECONNECT)->entry (at ("AT+CIPCLOSE=0\r\n"))
      ->transition (CONNECT_TO_SERVER)->when(&alwaysTrue)->then (&delay);
  ```
 
  To w efekcie maszyna przejdzie prawidłowo do stanu CONNECT_TO_SERVER, ale *za szybko* i może złapać jescze dane wejściowe
  które jej nie interesują i które spowodują dalsze nieprawidłowe przejścia.
 
  Ten problem pojawia się częściej niż mi się wydawało rozwiązaniem jest :
    * [ ] Po każdym żądaniu do modemu powinniśmy odczekać chwilkę, bo przecież odpowiedzi może być więcej niż 1 i przychodzą w pewnym okresie czasu (po wysłaniu żądania, ale jest jednak pewne opóźniene). Może akcja at niech się łączy z akcją delay - przydał by się "szablon"!        
    * [ ] Wrunki przejść powinniśmy konstruować tak, żeby wymagały wszystkich możliwych odpowiedzi z modemu, czyli trzba oczekiwac na echo, na jakieś tam dane i na OK.
    * [ ] Oczekiwać odpowiedzi w odpowiedniej kolejności - czytaj następne TODO. Błędy niestety są trudne do wykrycia. Przykład z życia : miałem takie przejścia:
 
      ```c++
      m->state (PDP_CONTEXT_CHECK)->entry (at ("AT+QIACT?\r\n"))
                ->transition(DNS_CONFIG)->when (like ("+QIACT:%1,%"))->then (&delay)
                ->transition(ACTIVATE_PDP_CONTEXT)->when (anded (beginsWith ("AT+QIACT?"), &ok))->then (&delay);
      ```                
 
      Tu mamy odpowiedź z modemu :

      ```
      IN : AT+QIACT?
      IN : +QIACT: 1,1,1,"37.249.238.180"
      IN : OK
      ```
 
      Uwaga! Są 3 odpowiedzi, ale już po drugiej warunek przejścia do   DNS_CONFIG jest spełniony i wykonuje się przejście, następnie   przychodzi odpowiedź OK co oznacza, że w kolejce zostaje OK i ono trafi jako wejście w następnym stanie (innymi słowy stan   ACTIVATE_PDP_CONTEXT będzie myślał, że już ma odpowiedź OK).
 
      ```c++
      m->state (ACTIVATE_PDP_CONTEXT)->entry (at ("AT+QIACT=1\r\n"))
                ->transition(DNS_CONFIG)->when (anded (beginsWith ("AT+QIACT="), &ok))->then (&delay);
      ```
    
      ``` 
      IN : AT+QIACT=1
      IN : ERROR
      ```

      Czyli otrzymaliśmy błąd! Żadne przejście nie powinno sie wykonać. Ale niestety w kolejce zostało OK, więc się wykonuje
  przejście do DNS_CONFIG. A następnie przychodzi odpowiedź ERROR.

      ```c++ 
      m->state (DNS_CONFIG)->entry (at ("AT+QIDNSCFG=1,\"8.8.8.8\",\"8.8.4.4\"\r\n"))
                ->transition (INIT)->when (&error)->then (&longDelay)
                ->transition (NETWORK_GPS_USART_ECHO_OFF)->when (anded (beginsWith ("AT+QIDNSCFG="), &ok))->then (&delay);
      ```
 
      No tu jest kompletna klapa, bo idzie do init, bo ma error.
 
      Powyższy problem jest poważny, bo kolejny raz z nim walczę. Problem by się (częściowo?) rozwiązał gdyby warunki były spełnione, tylko gdy odpowiedzi przyjdą w odpowiedniej kolejności. TODO było coś takiego jak seq...
  
* [ ] Powstał problem odbierania danych binarnych. Ja zaszyłem w maszynie funkcjonalność operacji na zero-ended
  strings i przez to nie mam teraz jak obsłużyć danych binarnych. Jeszcze gorzej, że zaszyłem na stałe uzywanie
  konkretnej kolekcji StringQueue, której elementy mają zawsze 128B W przypadku próby odbierania większej ilości
  danych to będzie problem (przydał by się CircularBuffer). Natomiast w przypadku próby odbierania całego firmware,
  to już w ogóle będzie masakra (bo wtedy MySink musiałby kopiowac dane do jakiegoś mega bufora, albo wręcz nagrywać
  je bezpośrednio na flash).
* [ ] Pousuwać konstruktyory i metody, których nie używam w ogóle.
* [ ] Zależność od libmicro dać jako opcję (ifdef) EDIT : sporo rzeczy nie będzie działać, ale trudno.
* [ ] W walgrindzie jak się uruchomi unitytest to jeden nie przechodzi. 
* [ ] Trzeba opisac jak działa NotCondition, bo to nieintuicyjnie działa gdy jest więcej niż 1 odpowiedź (? przykład ?).
* [ ] Easier interface for constructing the machine and for lambdas 
* [ ] Event arguments would resolve the problem of passing data between actions in elegant way and would allow more flexible condition checks. ???  
* [ ] It would be fantastic if transitions, entry and exit actions were fired in natural order, that is : entry, transition, transition action, exit. Now all actions are run after the state is changed (logs are very confusing).
* [ ] Deferred event implenentation is shitty. 
* [ ] Strasznie ciężko jest się połapać co się dzieje kiedy maszyna źle działa. Trzebaby poprawić debugi tak, żeby pokazywały wejścia, zamiany stanów, wywołania akcji I WARUNKI we własciwej kolejności! 
* [ ] Żeby dało się robić transition bez warunków (co by działo jak alwaysTrue).
* [ ] żeby do eventów moża było bez problemu dodawać dowlną liczbę argumentów o dowolnych typach.
* [ ] żeby nazwy stanów nie musiałby być definiowane osobno w enumie, i żeby łatwo się je dało wypisać na debug. Może user-defined literals?
        
# Features
* No dynamic allocations.


# Dependencies
* Boost hana
* GSL
* ~~etl~~
* Catch2

# Benchmarks
Time as Linux *time* command shows in *real* row. Binary size is in parentheses next to the run-time.

* libstatemachine **f3c19e87249232ecd65482eac8218fe744f37c15**
* libstate **df8621dd42cb67af13dcf43ce051c95cbdfbef00** (*Approach 1*)

| libstatemachine Debug | Release / stripped |
| --------------------- | ------------------ |
| 0,228s (482K/111K)    | 0,085s (77K/67K)   |

| libstate Debug | Release       |
| -------------- | ------------- |
| 0,560s (8,9M)  | 0,111s (223K) |

* libstate **de083d4458ebee313b995fb66eeafad6094f4bd3** - methodology of running state machine was fixed so it is the same as in the other benchmark (in libstatemachine).

| libstate Debug | Release       |
| -------------- | ------------- |
| 0,516s (15M)   | 0,107s (230K) |

* libstate **a912e495e0d5e4ef1e16e130574c90ef5bbb0483**
Now I'm averaging 10 runs of ```time ./benchmark```.

| libstate Debug | Release       |
| -------------- | ------------- |
| 0,408s (6.1M)  | 0,100s (230K) |

* libstate **53ac495682a705c2812fd109f7d6aca1551eb541** (I think this was *approach 2*)

| libstate Debug | Release           |
| -------------- | ----------------- |
| 0,356s (23M)   | **0,084s** (462K) |

* **3681b3454cb59a7a2b82abd1a807307320be2298** *Approach 3* implemented for the first time.

| libstate Debug | Release       |
| -------------- | ------------- |
| 0,3104s (878K) | 0,094s (261K) |

# Motivation, goals
1. **Versatile API**. Expressive, compact definition in one place. No class-per-state approach.
1. **Avoid dynamic allocation**. This is to be used in µCs by potential users. I don't want to force anybody to use a heap in a µC if they don't want to. In the old version the while configuration was constructed on the heap. This configuration does not change during program runtime. The only reason for the heap usage was to construct runtime polymorphic object structure. 
1. **Lightweight**. To have a small (in all possible meanings of this word i.e. executable size, code size, code needed to initialize the thing etc.) and fast implementation that encourages a user to throw a state machine wenever he fancy (in few lines of his code).
   1. Small RAM footprint
   2. Small flash footprint.
   3. Fast execution.
   4. Fast compilation.
   5. Not too much function nesting for ease of debugging on a µC.
   6. Readable compile erorr messages.
2. **Fix all the problems of the older version** (which piled up). Most notably : allow for arbitrary event types, and event collection types without having to state them explicitly in every state, transition, action, condition etc. Arbitrary functions / function-objects etc. for actions.

## Previous implementation (old)
Like I mentioned above, every piece of configuration was created on the heap. Initially the *event* type was fixed to character string, and was not easily replaceable. This soon has proven to be a flaw as I needed to support binary data (I was dealing with modems at that time). At the end a typical configuration looked like that:

```c++
m->state (AT_QBTPWR)->entry (at ("AT+QBTPWR=1\r\n"))
        ->transition (AT_QBTVISB)->when (anded<BinaryEvent> (beginsWith<BinaryEvent> ("AT+QBTPWR"), &ok))->then (&delay);

m->state (AT_QBTVISB)->entry (at ("AT+QBTVISB=0\r\n"))
        ->transition (AT_QBTGATSREG)->when (anded<BinaryEvent> (beginsWith<BinaryEvent> ("AT+QBTVISB"), &ok))->then (&delay);

m->state (AT_QBTGATSREG)->entry (at ("AT+QBTGATSREG=1,\"ABC2\"\r\n"))
        ->transition (AT_QBTGATSL)->when (anded<BinaryEvent> (beginsWith<BinaryEvent> ("AT+QBTGATSREG"), &ok))->then (&delay)
        ->transition (PIN_STATUS_CHECK)->when (beginsWith<BinaryEvent> ("+CME ERROR:"));
```

## First attepmpt (new)
Both state collection and the algorithm was template generated i.e. all the states and their internals were hana::tuples. Compile time iteration over compile time collections was implemented. Implication is that I couldn't store intermediate results between ```Machine::run``` calls because I would never know what is it's type (i.e. current state type). The way it worked in case of a delay action is it had to find the current state at the beginning of every run method call. So the drawback was : executable was big, and ran slower than the old version because the current state had to be found every time the ```Machine::run``` was run even though it was found in the previous run.

### Output binary size analysis (of first attempt)
I reimplemented this from the start the other day and carefully obseved how output binary size increased after each slight change. 

Most size increase came from too **many nested iterations** over std::tuples. At some point I had this nesting:

* For every state - check if this state is the "current state" (16 states).
  * For every transition - check its condition (2 transitions for every state)
    * For every state - find current state and run its exit actrions (16)
    * For every state - find the state we are transitioning to and run its entry actions (16)

This was naiive implementation and it had 16 * 2 * 16 * 16 = 1024 if branches that had to be generated. 

After reducing the nesting to only 2 levels (16*2) size dropped drastically (to the levels more than satifactory, but event type was set to int so comparison was not conclusive).

Next code bloat came from **chainging the event type from int to std::string**. After this the output size increased 10 times from 22Ki to 233Ki. I cannot get lower than 230Ki whatever approach I used (-O3 wise).

Then I noticed third issue (well mistake actually) that I passed ```const char *``` as an event even though conditions and actions worked on ```std::strings```. This generated lots of additional conversions. Simply adding a "s" suffix reduced the output even further. 

Lastly the actions itself has huge impact on binary size. And this is very important to remember because this is the user who writes them. But I don't understand it throughly. For example these implementations of ```ref``` used in ```tupleTest``` yelds different binary sizes :

```cpp
class res {
public:
        res (std::string m) : message (std::move (m)) {}
        void operator() (std::string const &) { results2.push_back (message); } // 98.4Ki (-O3 not stripped)

private:
        std::string message;
        // std::array<char, 1024> ppp; // This does not change much
};
```

Lambdas do better.

```cpp
auto res = [&results] (std::string const &message) { return [&results, message] { results.push_back (message); }; }; // ~75Ki
```

Eliminating std::string gives the best results:

```cpp
auto res = [&results] (const char *const message) { return [&results, message] (auto const &ev) { results.emplace_back (message); }; }; // 30.8Ki
```

I also found it difficult to get track of all the measurements which I made after every slight change for -O3, -O0 and -O0-stripped versions. I easilly got confused and compared wrong measurements. Another thing I didn't know is that even -O3 binary can be stripped. In my case ```.strtab``` and ```.symtab``` sections shrank and I got more tnah 50% savings.


## Second attempt
Implementation 2 used type Erasure which made runtime polymorphic interface for state and transition and thus made storing a pointer (to a base class) to them possible. This made it significantly faster but executable size is even bigger than before (presumably because of new templates ErasedState and ErasedTransition).

## Third attept
Mix of 3 approaches from above. Most notably states are created on prealocated memowry block using placement new (as the most meory hungry objects which hold everything). 

## To try
* Next thing to try is to optimize for executable size by changing indexToState impl and getTransition. Both methods use compile time iterations which I think is the main cause of binary bloat. 
* Another thing I think of is to minimize number of templates i.e. now I have State, ErasedState and ErasedStateBase so instead of 1 template 3 are beeing instantiated (executable produced by 2nd implementation is more than twice the size of the 1st).
  * [x] Try -fvisibility=hidden. Does not help, as it is to be used with libraries. Compiling a library GCC does not know which symbols has to be public or private, because they aren't used yet (by the final executable). Some of the symbols are pure implementation details of the library and thus should be marked hidden, and others are meant to be used by the end uuser and those should be visible. At the other hand when complilin a final executable GCC can fingure out which symbols are used, and which not at all, and thus it can potentially get rid of unused ones, so it make no difference is you use -fvisibility=hidden.
  * [x] ```-fdata-sections -ffunction-sections -fno-unwind-tables``` and ```-Wl,--gc-sections``` . No impact on executable size whatsoever.
  * [ ] Try switching to std::tuple.
* Compile Error messages are my main concern now. They render this implementation next to unusable. This is ofcourse because the types are so long.
  * Try clang. 
  * Try some gcc switches.
  * Split the types somehow? But I don't see how that could be possible.

