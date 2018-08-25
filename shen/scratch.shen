(datatype set

   _____________
   [] : (set A);

   X : A; Y : ((set A) & (without X));
   ===================================
   [X | Y] : (mode (set A) -);

   X : (set A);
   _____________
   X : (list A);

   X : P; X : Q;
   ====================
   X : (mode (P & Q) -);

   ________________
   [] : (without X);

   if (not (= X Z))
   Y : (without Z);
   ______________________
   [X | Y] : (without Z);

   ______________________________________________
   (not (element? X Y)) : verified >> Y : (without X);)

(lisp. "(DEFUN to-bits (INTEGER)
  (LET ((BITS '()))
    (DOTIMES (POS (INTEGER-LENGTH INTEGER) BITS)
      (PUSH (LDB (BYTE 1 POS) INTEGER) BITS))))")

(lisp. "(DEFUN to-number (NLIST)
  (IF NLIST
      (LET ((X (CAR NLIST))
            (Xs (CDR NLIST)))
        (+ (* (EXPT (1-  (LENGTH NLIST))
                    2)
              X)
           (to-number Xs)))
      0))" )

(define foldl
  F Acc [] -> Acc
  F Acc [X | Rest] -> (foldl F (F Acc X) Rest))

(define parse-ml
  ML-Rules -> (compile (function <ml-rules>)
                       ML-Rules))

(defcc <ml-rules>
  <ml-rule> , <ml-rules> := [<ml-rule> | <ml-rules>];
  <ml-rule> := [<ml-rule>];)

(defcc <ml-rule>
  Constructor of <datatypes> := [Constructor | <datatypes>];
  <datatype> := <datatype>;)

(defcc <datatypes>
  <datatype> <datatypes> := [ <datatype> | <datatypes>];
  <e>;)

(defcc <datatype>
  Datatype := Datatype where (not (= Datatype ,));)

(define ml->shen
  Datatype [Constructor | Datatypes] ->
  (let
      Vars (map (function var) Datatypes)
      Conclusion (conclusion Constructor Datatype Vars)
      Premises (premises Vars Datatypes)
    (append Premises [======] Conclusion))

  Datatype Instance -> [__________________
                        Instance : Datatype;])

(define conclusion
  Constructor Datatype Vars ->
  [(cons-form [Constructor | Vars]) : Datatype;])

(define premises
  [] [] -> []
  [V | Vs] [D | Ds] -> [ V : D ; | (premises Vs Ds)])

(define cons-form
  [X | Y] -> [cons (cons-form X) (cons-form Y)]
  X -> X)

(define var
  _ -> (gensym (protect X)))

(defmacro ml-macro
  [ml-datatype Datatype = | ML-Rules] ->
  [datatype Datatype |
            (mapcan (/. ML-Rule
                        (ml->shen Datatype ML-Rule))
                    (parse-ml ML-Rules))])

(define parse-ml
  ML-Rules -> (compile (function <ml-rules>)
                       ML-Rules))


(ml-datatype process =
             zero,
             input of action process,
             lift of name process,
             par of (list process),
             drop of name)

(ml-datatype action = action of name name)

(ml-datatype name =
             quote of process,
             address of number)

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


(define guard
  {name --> name --> action}
  NSubj NObj -> [action NSubj NObj])

(define ->input
  {name --> name --> process --> process}
  NSubj NObj Cont -> [input [action NSubj NObj]
                            Cont])

(define prefix
  {action --> process --> process}
  [action [quote Proc1]
          [quote Proc2]] Cont ->
          [input [action
                  [quote
                   Proc1]
                  [quote Proc2]]
                 Cont])

(define ->lift
  {name --> process --> process}
  NSubj Cont -> [lift NSubj Cont])

(define ->drop
  {name --> process}
  N -> [drop N])

(define ->par
  {process --> process --> process}
  [par Proclist1] [par Proclist2] -> [par (append Proclist1 Proclist2)]
  [par LProclist] Proc -> [par (append LProclist [Proc])]
  Proc [par RProclist] -> [par (cons Proc RProclist)]
  Proc1 Proc2 -> [par [Proc1 Proc2]])

(define parstar
  {(list process) --> process}
  [] -> zero
  [Proclisthd | Proclisttl] ->
  (->par Proclisthd (parstar Proclisttl)))

(define action->nsubj
  {action --> name}
  [action NSubj NObj] -> NSubj)

(define process-quote-depth
  {process --> number}
  zero -> 0
  [input Action Cont] ->
  (let
      QDSubj (name-quote-depth
              (action->nsubj Action))
      QDCont (process-quote-depth Cont)

    (if (>= QDSubj QDCont)
        QDSubj
        QDCont))
  [lift NSubj Cont] ->
  (let
      QDSubj (name-quote-depth NSubj)
      QDCont (process-quote-depth Cont)

    (if (>= QDSubj QDCont)
        QDSubj
        QDCont)))

(define name-quote-depth
  {name --> number}
  [quote Proc] ->
  (+ 1
     (process-quote-depth Proc)))


(datatype rho-compare-process
  __________________________________________
  quote-depth-ordered-process : rho-compare-process;

  __________________________________________
  unordered-process : rho-compare-process;)

(datatype rho-compare-process-name

  __________________________________________
  quote-depth-ordered-process-name : rho-compare-name;
  __________________________________________
  unordered-process-name : rho-compare-name;

  )


(define compare-proc
  {rho-compare-process
   -->
   process
   -->
   process
   --> number}

  quote-depth-ordered-process
  Proc1
  Proc2 ->
  (let
      Qdp1 (process-quote-depth Proc1)
      Qdp2 (process-quote-depth Proc2)
    (if (< Qdp1 Qdp2)
        -1
        (if (= Qdp1 Qdp2)
            0
            1)))
  unordered-process
  Proc1
  Proc2 ->
  (let
      Qdp1 (process-quote-depth Proc1)
      Qdp2 (process-quote-depth Proc2)
    (if (= Proc1 Proc2)
        0
        (if (< Qdp1 Qdp2)
            -1
            1))))

\* elements in lhs not in rhs *\



(define processes->names
  {(list process) --> (list name)}
  [] -> [[quote zero]]
  [Prochd | Proctl] -> (union (free-names Prochd)
                              (processes->names Proctl)))

(define free-names
  {process --> (list name)}
  zero -> []
  [input [action NSubj NObj] Cont] -> (cons
                                       NSubj
                                       (difference
                                        (free-names Cont)
                                        [NObj]))
  [lift NSubj Cont] -> (cons NSubj
                             (free-names Cont))
  [drop Name] -> [Name]
  [par Processes] -> (processes->names Processes))

(define processes->par
  {(list process) --> process}
  [] -> zero
  [Prochd | Proctl] -> (->par Prochd (processes->par Proctl)))

(define calculate-next-name
  {process --> name}
  zero -> [quote zero]
  [input [action [quote PSubj] [quote PObj]] Cont] ->
  [quote (parstar [PSubj PObj Cont])]
  [lift [quote PSubj] Cont] -> [quote (->par PSubj Cont)]
  [drop [quote Proc]] -> [quote (->par Proc Proc) ]
  [par Processes] ->  [quote (processes->par Processes)]
  [par []] -> [quote zero])


\* why does map typecheck in this case? *\

(define syntactic-substitution
  {process --> name --> name --> process}
  zero _ _ -> zero
  [input [action NSubj NObj] Cont] NSource NTarget ->
  (let
      Obj (if (name-equivalent NObj NTarget)
               (calculate-next-name [input [action NSubj NObj] Cont])
               NObj)
      N0 (if (name-equivalent NSubj NTarget)
             NSource
             NSubj)
      Contt (syntactic-substitution
            (if (name-equivalent NObj NTarget)
                (syntactic-substitution Cont Obj NObj)
                Cont)
            NSource
            NTarget)
    [input [action N0 Obj] Contt])
  [lift NSubj Cont] NSource NTarget ->
  [lift (if (name-equivalent NSubj NTarget)
            NSource
            NSubj)
        (syntactic-substitution Cont NSource NTarget)]
  [drop Name] NSource NTarget ->
  [drop (if (name-equivalent Name NTarget)
            NSource
            Name)]
  [par Proclist] NSource NTarget ->
  [par (map (/. Proc
                (syntactic-substitution
                 Proc NSource NTarget))
            Proclist)])

(define alpha-equivalent
  {process --> process --> boolean}
  [input [action NSubj1 NObj1] Cont1]  [input [action NSubj2 NObj2] Cont2] ->
  (and (name-equivalent NSubj1 NSubj2)
       (= Cont1 (syntactic-substitution Cont2 NObj1 NObj2)))
  Proc1 Proc2 -> (= Proc1 Proc2))



(define divide
  {(A --> boolean) --> (list A) --> (list A) --> (list A) --> ( (list A) * (list A))}
  _ [] Yes No -> (@p Yes No)
  F [X | Y] Yes No -> (if (F X)
                          (divide F Y
                                  [X | Yes] No)
                          (divide F Y
                                  Yes [X | No])))

(define partition
  {(A --> boolean) --> (list A) --> (list (list A))}
  _ [] -> []
  R [X | Y] -> (let Divide (divide R
                                   [X | Y] [] [])
                    Yes (fst Divide)
                    No (snd Divide)
                 [Yes | (partition R No)]))

\* 0 is the identity for part *\
(define structurally-equivalent-helper3
  {(list (list process)) --> boolean}
  [[] NotEquiv] -> false
  [[SingleEquivProc] NotEquiv] -> (structurally-equivalent zero [par NotEquiv])
  [[Equivhd | Equivtail] _] -> false)


(define structurally-equivalent-helper2
  {(list process) --> (list process)  -->
   (boolean * process * (list process)) -->
   (list process) --> boolean}
  _ _ (@p Answer _ _) [] -> Answer
  Proclisttl1 ProcsNotEquiv (@p false R L) [Prochd | Proctl] ->
  (if (structurally-equivalent
       (parstar (cons R
                      (append L ProcsNotEquiv)))
       [par Proclisttl1])
      (structurally-equivalent-helper2
       Proclisttl1 ProcsNotEquiv (@p true R L) Proctl)
       (structurally-equivalent-helper2
       Proclisttl1 ProcsNotEquiv (@p false zero (tail ProcsNotEquiv)) Proctl))

  Proclisttl1 ProcsNotEquiv (@p true R L) [_ | Proctl] ->
  (structurally-equivalent-helper2 Proclisttl1 ProcsNotEquiv (@p true R L) Proctl))

(define structurally-equivalent-helper1
  { (list process) --> (list (list process)) --> boolean}
  _ [[] No] -> false
  Proclisttl1 [YesEquiv NotEquiv] -> (structurally-equivalent-helper2
                                      Proclisttl1 NotEquiv
                                      (@p false zero (tail YesEquiv)) YesEquiv))

(define structurally-equivalent
  {process --> process --> boolean}
  zero [par []] -> true
  [par []] zero -> true
  zero [par [Proclisthd | Proclisttl]] ->
  (and (structurally-equivalent zero Proclisthd)
       (structurally-equivalent zero [par Proclisttl]))
  [par [Proclisthd | Proclisttl]] zero ->
  (structurally-equivalent zero [par (cons Proclisthd Proclisttl)])
  [input [action NSubj1 NObj1] Cont1] [input [action NSubj2 NObj2] Cont2] ->
  (and (name-equivalent NSubj1 NSubj2)
       (structurally-equivalent Cont1
                                (syntactic-substitution Cont2 NObj1 NObj2)))
  \* par is commutative and associative' *\
  [par [Proclisthd1 | Proclisttl1 ]] [par Proclist2] ->
  (structurally-equivalent-helper1 Proclisttl1 (partition
                                                (/. Proc (structurally-equivalent
                                                          Proclisthd1 Proc))
                                                Proclist2))
  [par Proclist1] [par [Proclisthd2 | Proclisttl2 ]] ->
  (structurally-equivalent [par [Proclisthd2 | Proclisttl2]] [par Proclist1])
  Proc1 [par Proclist] -> (structurally-equivalent-helper3 (partition
                                                            (/. Proc
                                                                (structurally-equivalent Proc1 Proc))
                                                            Proclist))
  [par Proclist] Proc2 ->
  (structurally-equivalent Proc2 [par Proclist])
  Proc1 Proc2 -> (= Proc1 Proc2))


(define name-equivalent
  {name --> name --> boolean}
  [quote [drop N1]] N2 -> (name-equivalent N1 N2)
  N1 [quote [drop N2]] -> (name-equivalent N1 N2)
  [quote P1] [quote P2] -> (structurally-equivalent P1 P2)
  [address Debruijnidx1] [address Debruijnidx2] ->
  (= Debruijnidx1 Debruijnidx2))


(define substitute
  {name --> name --> process --> process}
  _ _ zero -> zero
  Y X [input [action A B] Q] ->
  (let A' (if (name-equivalent A X)
              Y
              A)
       B' (if (name-equivalent B X)
              [quote [par [[drop B] Q]]]
              B)
       Q'' (if (name-equivalent B X)
               (substitute B' B Q)
               Q)
       Q' (substitute Y X Q)
    [input [action A' B'] Q'])
  Y X [lift A Q] ->
  (let A' (if (name-equivalent A X)
              Y
              A)
       Q'  (substitute Y X Q)
    [lift A' Q'])
  Y X [par Proclist] ->
  (let Proclist' (map (/. Proc
                          (substitute Y X Proc))
                      Proclist)
    [par Proclist'])
  Y X [drop A] ->
  (let A' (if (name-equivalent A X)
              Y
              A)
    [drop A']))



(define deBruijnify
  {process --> number --> number --> number --> process}
  zero L W H -> zero
  [input [action [quote Px] Y] Q] L W H ->
  (let Dbnidx (to-number
               (append
                (to-bits L)
                (to-bits W)
                (to-bits H)))
       Dbny [address Dbnidx]
       Q'  (deBruijnify Q (+ L 1) W H)
       Q'' (substitute Dbny Y Q')
       X  [quote (deBruijnify Px L W (+ H 1))]
    [input [action X Dbny] Q''])
  [lift [quote Px] Q] L W H ->
  (let X [quote (deBruijnify Px L W (+ 1 H))]
       Q' (deBruijnify Q L W H)
    [lift X Q'])
  [par [Proclisthd | Proclisttl]] L W H ->
  [par (cons
        (deBruijnify Proclisthd L W H)
        (map (/. Proc
                  (deBruijnify Proc L W (+ H 1)))
              Proclisttl))]
  [drop [quote Px]] L W H ->
  (let X [quote (deBruijnify Px L W (+ H 1))]
    [drop X])
  [drop [address Addr]] L W H -> [drop [address Addr]])

(set n0 [quote zero])
(set n2 [quote [drop [quote [drop (value n0)]]]])

(set p1 [par [[drop (value n2)]
              zero]])

(set p2 [input [action (value n0)
                       (value n2)]
               (value p1)])

(set p3 [input [action (value n2)
                       (value n0)]
               (value p1)])

(deBruijnify (value p2) 0 0 0)
(deBruijnify (value p3) 0 0 0)
