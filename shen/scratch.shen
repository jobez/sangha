(datatype globals
  ______________________
  append : (list process -->
           (list process) -->
            (list process));
  )

\* (datatype subtype *\

\*   (subtype A B); X : A; *\
\*   _____________________ *\
\*   X : B;) *\


(datatype process

  [X | Y] : (list process);
  _________________________
  [ X | Y ] : process;


  )

(synonyms processes (list processes) )

(datatype name

  P : process;
  _________________
  [quote P] : name;)

(datatype zero
  _______________
  [] : zero;

  Zero : zero;
  ==============
  zero : process;
  )

(datatype input
  Act : action;
  Proc : process;
  ===================
  [input Act Proc] : input;

  Input : input;
  =========================
  Input : process;

  )

(datatype drop

  Name : name;
  =====================
  [drop Name] : drop;

  Drop : drop;
  ============
  Drop : process;
  )


(datatype par

  Proc : (list process);
  =============================
  [par Proc] : par;


  Proc : process;
  =============================
  [par Proc] : par;


  Par : par;
  ==============
  Par : process;
  )

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

(define prefix
  {action --> process --> input}
  [action [quote Proc1] [quote Proc2]] Cont -> [input [action [quote Proc1]
                                                              [quote Proc2]]
                                                      Cont])

(define ->lift
  {name --> process --> lift}
  NSubj Cont -> [lift NSubj Cont])

(define ->drop
  {name --> drop}
  N -> [drop N])

(define ->par
  {process --> process --> par}
  [par Proclist1] [par Proclist2] -> [par (append Proclist1 Proclist2)]
  [par LProclist] Proc -> [par (append LProclist [Proc])]
  Proc [par RProclist] -> [par (cons Proc RProclist)]
  Proc1 Proc2 -> [par [Proc1 Proc2]])
