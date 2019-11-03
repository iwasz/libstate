# To be documented
* Initial state == "INIT"_STATE (by convention)
* Action callback interface options : can return Done or void, can take an Event argument or void.



# TODO
* All sorts of misuses should be pointed out in compile time with meaningful messages (use static_asserts 
  whenever possible).
* Nullary conditions does not work.
* Meaningful message when condition argumnet type is incompatible with event type, and so on.
