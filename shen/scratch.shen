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
             drop of name,
             par of (list process) )

(ml-datatype action = action of name name)

(ml-datatype name = quote of process)

(datatype globals
  ______________________
  name-quote-depth : (name --> number);)

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


(define un
  {(set A) --> (set A) --> (set A)}
  [] S -> S
  [X | Y] S -> [X | (un Y S)]     where (not (element? X (un Y S)))
  [_ | Y] S -> (un Y S))

\* elements that are in A but not in B *\

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
