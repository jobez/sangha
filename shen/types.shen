\* https://i.imgur.com/p4eYydh.png *\

(datatype globals
  ______________________
  name-quote-depth : (name --> number);

  _____________________________________
  free-names-helper : ((list name) --> process --> (list name));

  _________________________________________
  free-names : (process --> (list name));

  _________________________________________
  name-equivalent : (name --> name --> boolean);

  ____________________________________________________________
  structurally-equivalent : (process --> process --> boolean);

  ___________________________________________
  to-bits : (number --> (list number));

  ___________________________________________
  to-number : ((list number) --> number);

  __________
  (value n0) : name;

  __________
  (value n2) : name;

  ___________
  (value p1) : process;
  ____________
  (value p2) : process;
  _____________
  (value p3) : process;
  )

(ml-datatype process =
             zero,
             input of action process,
             output of name process,
             par of (list process),
             eval of name)

(ml-datatype action = action of name name)

(ml-datatype name =
             quote of process,
             address of number)
