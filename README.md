# To be documented
* Initial state == "INIT"_STATE (by convention)
* Action callback interface options : can return Done or void, can take an Event argument or void.

# Milestones
1. Static, typesafe API.
1. Changing states.
1. Running actions in correct order (partially)
1. Retained input.

# TODOs
* [ ] When wrappers are sorted out, pay attention to function / method / lambda argument types (&, &&, universal && etc).
* [ ] state function should accept parameters in different order (more logical order) : exit, then  transitions, then exit. This is because
  exit action is only needed IF and only if there is at least one transition, but trnsitions can exist WITHOUT an exit action.
* [ ] Compilation problem when last action has a transition.
* [ ]All sorts of combinations should be possible when creating a state. With/without either of : entry, exit, transition. Transition 
should be with or without either of : condition, action (s).
* [ ] Make state with no actions possible.
* [ ] EventQueue by lvl reference (non const). Justify it in the docs (this queue is sort of a state machine internal state and this state
must be persisted between calls to run). 
* [ ] Consider adding input actions, I might be wrong, but I think I used one of those in MC60 (forgetting abit the name "input action").
* [ ] All sorts of misuses should be pointed out in compile time with meaningful messages (use static_asserts 
  whenever possible).
* [ ] Nullary conditions does not work.
* [ ] Meaningful message when condition argumnet type is incompatible with event type, and so on.
* [ ] Allow raising an event from inside of actions (somehow).
* [ ] The code is so generic, that it sometimes accepts wrong type of arguments,
* [ ] like you can pass transition instead of a state. It should be convinient if
the compiler reported shuch a misuse on early stage and with meaningful messages.
* [ ] configurable (as a template argument?) output class. Defaults to std::cout. oh,
it also can be a lambda passed to the run method or something.
* events as std::tuples or std::pairs, and then actions and conditions would get parameter pack of arguments (hana::unpacked or std::apply-ied).

Copy (in Polish) of TODOs from the previous version of the lib. 
 
* TODO Dodać globalny timeout. Jeśli w danym timeoucie nie uda się zmienić stanu, to trzeba podjąć jakąć
  akcję. Ej to chyba jest!
 
 * TODO dokumentacja z tutorialem jest niezbędna, bo ja sam mam czasem problemy. Jak są ewaluowane warunki,
  co do nich wpada, w jakiej kolejności i tak dalej. Opisać wszystkie aspekty działania : jak dwie maszyny
  mogą pracować na jedym input/output i tak dalej.
 
 * TODO opisać, że zawsze trzeba pamiętyać czy jest odpowiednia ilość czasu na sprawdzenie warunków. Podać taki przykład:
  Kiedy jest jeden warunek na przejście, który oczekuje jakichś danych, to nie ma problemu. Na przykład :
 
          m->state (INIT, State::INITIAL)->entry (at ("ATE1\r\n"))
                ->transition (CHECK_OPERATING_MODE)->when (&ok)->then (&delay);
 
  Maszyna będzie tak długo się kręciuć, aż dostanie "OK". Jeżeli jednak mamy warunki zdefiniowane tak, że oczekiwane wejście
  może nie zdążyć się pojawić, to trzeba dodać opóźnienie, żeby poczekać na te dane wejściowe. Przykład (błędny):
 
          m->state (CHECK_OPERATING_MODE)->entry (at ("AT+CWMODE_DEF?\r\n"))
                ->transition (VERIFY_CONNECTED_ACCESS_POINT)->when (eq ("+CWMODE_DEF:1"))->then (&delay)
                ->transition (SET_OPERATING_MODE)->when (&alwaysTrue)->then (&delay);
 
  W powyższym przykładzie warunki dwóch przejść są sprawdzane jedno po drugim. Najpierw maszyna sprawdzi, czy w kolejce
  wejściowej jest jakiś element z napisem "+CWMODE_DEF:1", a jeśli nie ma, to natychmiast przejdzie do stanu SET_OPERATING_MODE.
  Trzeba dać maszynie trochę czasu na oczekiwanie na dane. Powinno być:
 
          m->state (CHECK_OPERATING_MODE)->entry (and_action (at ("AT+CWMODE_DEF?\r\n"), &delay))
                ->transition (VERIFY_CONNECTED_ACCESS_POINT)->when (eq ("+CWMODE_DEF:1"))->then (&delay)
                ->transition (SET_OPERATING_MODE)->when (&alwaysTrue)->then (&delay);
 
  Taki błąd jak wyżej może prowadzić do trudnych do wykrycia konsekwencji. Jeśli jak wyżej mamy dwa przejścia stanowe, to
  dość łatwo będzie się zorientować, że mimo że modem wysłał odpowiedź "+CWMODE_DEF:1" to jesteśmy w słym stanie (w SET_OPERATING_MODE
  zamiast VERIFY_CONNECTED_ACCESS_POINT). Ale w przypadku gdy mamy takie przejście (błędne - brakuje delay):
 
          m->state (CLOSE_AND_RECONNECT)->entry (at ("AT+CIPCLOSE=0\r\n"))
                ->transition (CONNECT_TO_SERVER)->when(&alwaysTrue)->then (&delay);
 
  To w efekcie maszyna przejdzie prawidłowo do stanu CONNECT_TO_SERVER, ale *za szybko* i może złapać jescze dane wejściowe
  które jej nie interesują i które spowodują dalsze nieprawidłowe przejścia.
 
 * TODO Ten problem pojawia się częściej niż mi się wydawało rozwiązaniem jest :
        * Po każdym żądaniu do modemu powinniśmy odczekać chwilkę, bo przecież odpowiedzi
  może być więcej niż 1 i przychodzą w pewnym okresie czasu (po wysłaniu żądania,
  ale jest jednak pewne opóźniene). Może akcja at niech się łączy z akcją delay - przydał
  by się "szablon"!
        * Wrunki przejść powinniśmy konstruować tak, żeby wymagały wszystkich możliwych
        odpowiedzi z modemu, czyli trzba oczekiwac na echo, na jakieś tam dane i na OK.
        * TODO Oczekiwać odpowiedzi w odpowiedniej kolejności - czytaj następne TODO.
         Błędy niestety są trudne do wykrycia. Przykład z życia : miałem takie przejścia:
 
          m->state (PDP_CONTEXT_CHECK)->entry (at ("AT+QIACT?\r\n"))
                ->transition(DNS_CONFIG)->when (like ("+QIACT:%1,%"))->then (&delay)
                ->transition(ACTIVATE_PDP_CONTEXT)->when (anded (beginsWith ("AT+QIACT?"), &ok))->then (&delay);
 
  Tu mamy odpowiedź z modemu :
  IN : AT+QIACT?
  IN : +QIACT: 1,1,1,"37.249.238.180"
  IN : OK
 
  Uwaga! Są 3 odpowiedzi, ale już po drugiej warunek przejścia do DNS_CONFIG jest spełniony i wykonuje się przejście,
  następnie przychodzi odpowiedź OK co oznacza, że w kolejce zostaje OK i ono trafi jako wejście w następnym stanie
  (innymi słowy stan ACTIVATE_PDP_CONTEXT będzie myślał, że już ma odpowiedź OK).
 
        m->state (ACTIVATE_PDP_CONTEXT)->entry (at ("AT+QIACT=1\r\n"))
                ->transition(DNS_CONFIG)->when (anded (beginsWith ("AT+QIACT="), &ok))->then (&delay);
 
  IN : AT+QIACT=1
  IN : ERROR
 
  Czyli otrzymaliśmy błąd! Żadne przejście nie powinno sie wykonać. Ale niestety w kolejce zostało OK, więc się wykonuje
  przejście do DNS_CONFIG. A następnie przychodzi odpowiedź ERROR.
 
        m->state (DNS_CONFIG)->entry (at ("AT+QIDNSCFG=1,\"8.8.8.8\",\"8.8.4.4\"\r\n"))
                ->transition (INIT)->when (&error)->then (&longDelay)
                ->transition (NETWORK_GPS_USART_ECHO_OFF)->when (anded (beginsWith ("AT+QIDNSCFG="), &ok))->then (&delay);
 
  No tu jest kompletna klapa, bo idzie do init, bo ma error.
 
 * TODO Powyższy problem jest poważny, bo kolejny raz z nim walczę. Problem by się (częściowo?) rozwiązał gdyby
 warunki były spełnione, tylko gdy odpowiedzi przyjdą w odpowiedniej kolejności. TODO było coś takiego jak seq...
  
 * TODO Powstał problem odbierania danych binarnych. Ja zaszyłem w maszynie funkcjonalność operacji na zero-ended
  strings i przez to nie mam teraz jak obsłużyć danych binarnych. Jeszcze gorzej, że zaszyłem na stałe uzywanie
  konkretnej kolekcji StringQueue, której elementy mają zawsze 128B W przypadku próby odbierania większej ilości
  danych to będzie problem (przydał by się CircularBuffer). Natomiast w przypadku próby odbierania całego firmware,
  to już w ogóle będzie masakra (bo wtedy MySink musiałby kopiowac dane do jakiegoś mega bufora, albo wręcz nagrywać
  je bezpośrednio na flash).
 
 * TODO Pousuwać konstruktyory i metody, których nie używam w ogóle.
 
 * TODO Zależność od libmicro dać jako opcję (ifdef) EDIT : sporo rzeczy nie będzie działać, ale trudno.
 
 * TODO W walgrindzie jak się uruchomi unitytest to jeden nie przechodzi.
 
 * TODO Trzeba opisac jak działa NotCondition, bo to nieintuicyjnie działa gdy jest więcej niż 1 odpowiedź (? przykład ?).
 
 * TODO Easier interface for constructing the machine and for lambdas
 
 * TODO Event arguments would resolve the problem of passing data between actions in elegant way and
  would allow more flexible condition checks.
 
 * TODO It would be fantastic if transitions, entry and exit actions were fired in
  natural order that is entry , transition, transition action, exit. Now all actions
  are run all at once.
 
 * TODO Deferred event implenentation is shitty.
 
 * TODO strasznie ciężko jest się połapać co się dzieje kiedy maszyna źle działa. Trzebaby poprawić debugi
  tak, żeby pokazywały wejścia, zamiany stanów, wywołania akcji I WARUNKI we własciwej kolejności!
 
 * TODO Żeby dało się robić transition bez warunków (co by działo jak alwaysTrue).
 
 * TODO żeby do eventów moża było bez problemu dodawać dowlną liczbę argumentów o dowolnych typach.

* TODO żeby nazwy stanów nie musiałby być definiowane osobno w enumie, i żeby łatwo się je dało
 wypisać na debug. Może user-defined literals?
        




# Features
* No dynamic allocations.


# Dependencies
* Boost hana
* GSL
* etl
* Catch2

# Benchmarks
Time as Linux *time* command shows in *real* row. Binary size is in parentheses next to the run-time.

* libstatemachine **f3c19e87249232ecd65482eac8218fe744f37c15**
* libstate **df8621dd42cb67af13dcf43ce051c95cbdfbef00**

| libstatemachine Debug | Release      | libstate Debug | Release       |
| --------------------- | ------------ | -------------- | ------------- |
| 0,228s (482K)         | 0,085s (77K) | 0,560s (8,9M)  | 0,111s (223K) |

* libstate **de083d4458ebee313b995fb66eeafad6094f4bd3** - methodology of running state machine was fixed so it is the same as in the other benchmark (in libstatemachine).

| libstate Debug | Release       |
| -------------- | ------------- |
| 0,516s (15M)   | 0,107s (230K) |
