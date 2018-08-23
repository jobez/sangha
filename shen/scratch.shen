(datatype subtype

  (subtype A B); X : A;
  _____________________
  X : B;)

(datatype process

  X : process;
  Y : process;
  ====================
  [ X | Y ] : process;)

(datatype name

  P : process;
  _________________
  [quote P] : name;)

(datatype zero
  _______________
  [] : zero;

  ______________________
  (subtype zero process);)

(datatype input
  Act : action;
  Proc : process;
  ===================
  [input Act Proc] : input;

  _______________________
  (subtype input process);)

(datatype drop

  Name : name;
  =====================
  [drop Name] : drop;

  _______________________
  (subtype drop process);)


(datatype par

  Proc : process;
  Procs : (list process);
  =============================
  [par Proc | Procs] : par;

  _______________________
  (subtype par process);)

(datatype lift

  Name : name;
  Process : process;
  =============================
  [lift Name Process] : lift;)

(datatype action
  N0 : name;
  N1 : name;
  =====================
  [action N0 N1] : action;
)

(define guard
  {name --> name --> action}
  NSubj NObj -> [action NSubj NObj])

(define ->input
  {name --> name --> process --> input}
  NSubj NObj Cont -> [input [action NSubj NObj]
                            Cont])
